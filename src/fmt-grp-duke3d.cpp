/**
 * @file   fmt-grp-duke3d.cpp
 * @brief  Implementation of Duke3D .GRP file reader/writer.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/GRP_Format
 *
 * Copyright (C) 2010 Adam Nielsen <malvineous@shikadi.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <boost/algorithm/string.hpp>
#include <camoto/iostream_helpers.hpp>
#include <camoto/util.hpp>

#include "fmt-grp-duke3d.hpp"

#define GRP_FILECOUNT_OFFSET    12
#define GRP_HEADER_LEN          16  // "KenSilverman" header + u32le file count
#define GRP_FAT_OFFSET          GRP_HEADER_LEN
#define GRP_FILENAME_FIELD_LEN  12
#define GRP_MAX_FILENAME_LEN    GRP_FILENAME_FIELD_LEN
#define GRP_FAT_ENTRY_LEN       16  // filename + u32le size
#define GRP_FIRST_FILE_OFFSET   GRP_FAT_OFFSET  // empty archive only

#define GRP_SAFETY_MAX_FILECOUNT  8192 // Maximum value we will load

#define GRP_FATENTRY_OFFSET(e) (GRP_HEADER_LEN + e->iIndex * GRP_FAT_ENTRY_LEN)

#define GRP_FILENAME_OFFSET(e) GRP_FATENTRY_OFFSET(e)
#define GRP_FILESIZE_OFFSET(e) (GRP_FATENTRY_OFFSET(e) + GRP_FILENAME_FIELD_LEN)

namespace camoto {
namespace gamearchive {

GRPType::GRPType()
	throw ()
{
}

GRPType::~GRPType()
	throw ()
{
}

std::string GRPType::getArchiveCode() const
	throw ()
{
	return "grp-duke3d";
}

std::string GRPType::getFriendlyName() const
	throw ()
{
	return "Duke Nukem 3D Group File";
}

// Get a list of the known file extensions for this format.
std::vector<std::string> GRPType::getFileExtensions() const
	throw ()
{
	std::vector<std::string> vcExtensions;
	vcExtensions.push_back("grp");
	return vcExtensions;
}

std::vector<std::string> GRPType::getGameList() const
	throw ()
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Duke Nukem 3D");
	vcGames.push_back("Redneck Rampage");
	vcGames.push_back("Shadow Warrior");
	return vcGames;
}

E_CERTAINTY GRPType::isInstance(iostream_sptr psArchive) const
	throw (std::ios::failure)
{
	psArchive->seekg(0, std::ios::end);
	io::stream_offset lenArchive = psArchive->tellg();

	// TESTED BY: fmt_grp_duke3d_isinstance_c02
	if (lenArchive < GRP_FAT_ENTRY_LEN) return EC_DEFINITELY_NO; // too short

	char sig[12];
	psArchive->seekg(0, std::ios::beg);
	psArchive->read(sig, 12);

	// TESTED BY: fmt_grp_duke3d_isinstance_c00
	if (strncmp(sig, "KenSilverman", 12) == 0) return EC_DEFINITELY_YES;

	// TESTED BY: fmt_grp_duke3d_isinstance_c01
	return EC_DEFINITELY_NO;
}

ArchivePtr GRPType::newArchive(iostream_sptr psArchive, MP_SUPPDATA& suppData) const
	throw (std::ios::failure)
{
	psArchive->seekp(0, std::ios::beg);
	psArchive->write("KenSilverman\0\0\0\0", 16);
	return ArchivePtr(new GRPArchive(psArchive));
}

// Preconditions: isInstance() has returned > EC_DEFINITELY_NO
ArchivePtr GRPType::open(iostream_sptr psArchive, MP_SUPPDATA& suppData) const
	throw (std::ios::failure)
{
	return ArchivePtr(new GRPArchive(psArchive));
}

MP_SUPPLIST GRPType::getRequiredSupps(const std::string& filenameArchive) const
	throw ()
{
	// No supplemental types/empty list
	return MP_SUPPLIST();
}


GRPArchive::GRPArchive(iostream_sptr psArchive)
	throw (std::ios::failure) :
		FATArchive(psArchive, GRP_FIRST_FILE_OFFSET)
{
	this->psArchive->seekg(12, std::ios::beg); // skip "KenSilverman" sig

	// We still have to perform sanity checks in case the user forced an archive
	// to open even though it failed the signature check.
	if (this->psArchive->tellg() != 12) throw std::ios::failure("file too short");

	uint32_t numFiles;
	this->psArchive >> u32le(numFiles);

	if (numFiles >= GRP_SAFETY_MAX_FILECOUNT) {
		throw std::ios::failure("too many files or corrupted archive");
	}

	io::stream_offset offNext = GRP_HEADER_LEN + (numFiles * GRP_FAT_ENTRY_LEN);
	for (int i = 0; i < numFiles; i++) {
		FATEntry *fatEntry = new FATEntry();
		EntryPtr ep(fatEntry);

		fatEntry->iIndex = i;
		fatEntry->iOffset = offNext;
		fatEntry->lenHeader = 0;
		fatEntry->type = FILETYPE_GENERIC;
		fatEntry->fAttr = 0;
		fatEntry->bValid = true;

		// Read the data in from the FAT entry in the file
		this->psArchive
			>> nullPadded(fatEntry->strName, GRP_FILENAME_FIELD_LEN)
			>> u32le(fatEntry->iSize);

		this->vcFAT.push_back(ep);
		offNext += fatEntry->iSize;
	}
}

GRPArchive::~GRPArchive()
	throw ()
{
}

// Does not invalidate existing EntryPtrs
void GRPArchive::rename(EntryPtr& id, const std::string& strNewName)
	throw (std::ios_base::failure)
{
	// TESTED BY: fmt_grp_duke3d_rename
	assert(this->isValid(id));
	FATEntry *pEntry = dynamic_cast<FATEntry *>(id.get());

	if (strNewName.length() > GRP_MAX_FILENAME_LEN) {
		throw std::ios_base::failure("new filename too long, max is "
			TOSTRING(GRP_MAX_FILENAME_LEN) " chars");
	}

	this->psArchive->seekp(GRP_FILENAME_OFFSET(pEntry));
	this->psArchive << nullPadded(strNewName, GRP_FILENAME_FIELD_LEN);

	pEntry->strName = strNewName;

	return;
}

void GRPArchive::updateFileOffset(const FATEntry *pid, std::streamsize offDelta)
	throw (std::ios::failure)
{
	// This format doesn't have any offsets that need updating.  As this function
	// is only called when removing a file, the "offsets" will be sorted out
	// when the FAT entry is removed later.
	return;
}

void GRPArchive::updateFileSize(const FATEntry *pid, std::streamsize sizeDelta)
	throw (std::ios::failure)
{
	// TESTED BY: fmt_grp_duke3d_insert*
	// TESTED BY: fmt_grp_duke3d_resize*
	this->psArchive->seekp(GRP_FILESIZE_OFFSET(pid));
	this->psArchive << u32le(pid->iSize);
	return;
}

FATArchive::FATEntry *GRPArchive::preInsertFile(const FATEntry *idBeforeThis, FATEntry *pNewEntry)
	throw (std::ios::failure)
{
	// TESTED BY: fmt_grp_duke3d_insert*
	if (pNewEntry->strName.length() > GRP_MAX_FILENAME_LEN) {
		throw std::ios::failure("maximum filename length is "
			TOSTRING(GRP_MAX_FILENAME_LEN) " chars");
	}

	// Set the format-specific variables
	pNewEntry->lenHeader = 0;

	// Because the new entry isn't in the vector yet we need to shift it manually
	pNewEntry->iOffset += GRP_FAT_ENTRY_LEN;

	this->psArchive->seekp(GRP_FATENTRY_OFFSET(pNewEntry));
	this->psArchive->insert(GRP_FAT_ENTRY_LEN);
	boost::to_upper(pNewEntry->strName);

	this->psArchive
		<< nullPadded(pNewEntry->strName, GRP_FILENAME_FIELD_LEN)
		<< u32le(pNewEntry->iSize);

	// Update the offsets now there's a new FAT entry taking up space.
	this->shiftFiles(GRP_FAT_OFFSET + this->vcFAT.size() * GRP_FAT_ENTRY_LEN, GRP_FAT_ENTRY_LEN, 0);

	this->updateFileCount(this->vcFAT.size() + 1);
	return pNewEntry;
}

void GRPArchive::preRemoveFile(const FATEntry *pid)
	throw (std::ios::failure)
{
	// TESTED BY: fmt_grp_duke3d_remove*

	// Update the offsets now there's one less FAT entry taking up space.  This
	// must be called before the FAT is altered, because it will write a new
	// offset into the FAT entry we're about to erase (and if we erase it first
	// it'll overwrite something else.)
	this->shiftFiles(GRP_FAT_OFFSET + this->vcFAT.size() * GRP_FAT_ENTRY_LEN, -GRP_FAT_ENTRY_LEN, 0);

	this->psArchive->seekp(GRP_FATENTRY_OFFSET(pid));
	this->psArchive->remove(GRP_FAT_ENTRY_LEN);

	this->updateFileCount(this->vcFAT.size() - 1);
	return;
}

void GRPArchive::updateFileCount(uint32_t iNewCount)
	throw (std::ios_base::failure)
{
	// TESTED BY: fmt_grp_duke3d_insert*
	// TESTED BY: fmt_grp_duke3d_remove*
	this->psArchive->seekp(GRP_FILECOUNT_OFFSET);
	this->psArchive << u32le(iNewCount);
	return;
}

} // namespace gamearchive
} // namespace camoto
