/**
 * @file   fmt-dat-hugo.cpp
 * @brief  Implementation of Hugo scenery .DAT file reader/writer.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/DAT_Format_%28Hugo%29
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
#include <camoto/debug.hpp>
#include "fmt-dat-hugo.hpp"

#define DAT_FAT_ENTRY_LEN        8  // u32le offset + u32le size
#define DAT_FIRST_FILE_OFFSET    0

#define DAT_FATENTRY_OFFSET(e)   (e->iIndex * DAT_FAT_ENTRY_LEN)

#define DAT_FILESIZE_OFFSET(e)   (DAT_FATENTRY_OFFSET(e) + 4)
#define DAT_FILEOFFSET_OFFSET(e)  DAT_FATENTRY_OFFSET(e)

namespace camoto {
namespace gamearchive {

refcount_declclass(DAT_HugoType);

DAT_HugoType::DAT_HugoType()
	throw ()
{
	refcount_qenterclass(DAT_HugoType);
}

DAT_HugoType::~DAT_HugoType()
	throw ()
{
	refcount_qexitclass(DAT_HugoType);
}

std::string DAT_HugoType::getArchiveCode() const
	throw ()
{
	return "dat-hugo";
}

std::string DAT_HugoType::getFriendlyName() const
	throw ()
{
	return "Hugo DAT File";
}

// Get a list of the known file extensions for this format.
std::vector<std::string> DAT_HugoType::getFileExtensions() const
	throw ()
{
	std::vector<std::string> vcExtensions;
	vcExtensions.push_back("dat");
	return vcExtensions;
}

std::vector<std::string> DAT_HugoType::getGameList() const
	throw ()
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Hugo II, Whodunit?");
	vcGames.push_back("Hugo III, Jungle of Doom!");
	return vcGames;
}

E_CERTAINTY DAT_HugoType::isInstance(iostream_sptr psArchive) const
	throw (std::ios::failure)
{
	psArchive->seekg(0, std::ios::end);
	io::stream_offset lenArchive = psArchive->tellg();

	// Because there's no header, an empty file could be in this format.
	// TESTED BY: fmt_dat_hugo_isinstance_c04
	if (lenArchive == 0) return EC_POSSIBLY_YES;

	// TESTED BY: fmt_dat_hugo_isinstance_c02
	if (lenArchive < DAT_FAT_ENTRY_LEN) return EC_DEFINITELY_NO; // too short

	psArchive->seekg(0, std::ios::beg);

	uint32_t fatEnd, firstLen;
	psArchive >> u32le(fatEnd) >> u32le(firstLen);

	// TESTED BY: fmt_dat_hugo_isinstance_c03
	if (fatEnd + firstLen > lenArchive)
		return EC_DEFINITELY_NO; // first file finishes after EOF

	uint32_t numFiles = fatEnd / DAT_FAT_ENTRY_LEN;

	uint32_t offEntry, lenEntry;
	for (int i = 1; i < numFiles; i++) {
		psArchive
			>> u32le(offEntry)
			>> u32le(lenEntry)
		;

		// If a file entry points past the end of the archive then it's an invalid
		// format.
		// TESTED BY: fmt_dat_hugo_isinstance_c01
		if (offEntry + lenEntry > lenArchive) return EC_DEFINITELY_NO;
	}

	// If we've made it this far, this is almost certainly a DAT file.

	// TESTED BY: fmt_dat_hugo_isinstance_c00
	return EC_DEFINITELY_YES;
}

ArchivePtr DAT_HugoType::newArchive(iostream_sptr psArchive, MP_SUPPDATA& suppData) const
	throw (std::ios::failure)
{
	// Return an empty file
	return ArchivePtr(new DAT_HugoArchive(psArchive, iostream_sptr(), NULL));
}

ArchivePtr DAT_HugoType::open(iostream_sptr psArchive, MP_SUPPDATA& suppData) const
	throw (std::ios::failure)
{
	if (suppData.find(EST_FAT) != suppData.end()) {
		return ArchivePtr(new DAT_HugoArchive(psArchive, suppData[EST_FAT].stream, suppData[EST_FAT].fnTruncate));
	}
	return ArchivePtr(new DAT_HugoArchive(psArchive, iostream_sptr(), NULL));
}

MP_SUPPLIST DAT_HugoType::getRequiredSupps(const std::string& filenameArchive) const
	throw ()
{
	// If this is 'scenery1.dat' then the rest of its data is in 'scenery2.dat'
	// so we will include it as a supp.
	MP_SUPPLIST supps;
	std::string::size_type slash = filenameArchive.find_last_of('/');
	std::string filenameBase;
	if (slash != std::string::npos) {
		filenameBase = filenameArchive.substr(slash+1);
	} else {
		filenameBase = filenameArchive;
	}
	if (boost::iequals(filenameBase, "scenery2.dat")) {
		std::string firstFilename = filenameArchive;
		firstFilename[firstFilename.length() - 5] = '1';
		supps[EST_FAT] = firstFilename;
	}
	return supps;
}


refcount_declclass(DAT_HugoArchive);

DAT_HugoArchive::DAT_HugoArchive(iostream_sptr psArchive, iostream_sptr psFAT, FN_TRUNCATE fnTruncFAT)
	throw (std::ios::failure) :
		FATArchive(psArchive, DAT_FIRST_FILE_OFFSET),
		fnTruncFAT(fnTruncFAT)
{
	iostream_sptr fatStream;
	if (psFAT) {
		this->psFAT.reset(new segmented_stream(psFAT));
		fatStream = this->psFAT;
	} else fatStream = this->psArchive;

	fatStream->seekg(0, std::ios::end);
	io::stream_offset lenFAT = fatStream->tellg();

	this->psArchive->seekg(0, std::ios::end);
	io::stream_offset lenArchive = this->psArchive->tellg();

	// Empty files could be empty archives, so only attempt to read if the file
	// is non-empty.
	if ((lenFAT != 0) || (lenArchive != 0)) {

		if (lenFAT < DAT_FAT_ENTRY_LEN) {
			throw std::ios::failure("Archive too short - no FAT terminator!");
		}

		uint32_t fatEnd;
		fatStream->seekg(0, std::ios::beg);
		fatStream >> u32le(fatEnd);
		if (fatEnd >= lenFAT) {
			throw std::ios::failure("Archive corrupt - FAT truncated!");
		}
		uint32_t numFiles = fatEnd / DAT_FAT_ENTRY_LEN;
		this->vcFAT.reserve(numFiles);

		io::stream_offset lastOffset = 0;
		int curFile = 1;
		int firstIndexInSecondArch = 0;
		fatStream->seekg(0, std::ios::beg);
		for (int i = 0; i < numFiles; i++) {
			DAT_HugoEntry *fatEntry = new DAT_HugoEntry();
			EntryPtr ep(fatEntry);

			// Read the data in from the FAT entry in the file
			fatStream
				>> u32le(fatEntry->iOffset)
				>> u32le(fatEntry->iSize)
			;

			// If suddenly the offsets revert back to zero, it means we've reached
			// the second file (scenery2.dat)
			if ((fatEntry->iOffset != 0) || (fatEntry->iSize != 0)) {
				if (fatEntry->iOffset < lastOffset) {
					curFile++;
					firstIndexInSecondArch = i;
				}
				lastOffset = fatEntry->iOffset;
			} else {
				// TODO: mark as spare/unused FAT entry
			}

			fatEntry->iIndex = i - firstIndexInSecondArch;
			fatEntry->lenHeader = 0;
			fatEntry->eType = EFT_USEFILENAME;
			fatEntry->fAttr = 0;
			fatEntry->bValid = true;
			fatEntry->strName = std::string();

			fatEntry->file = curFile;

			if (
				(psFAT && (curFile == 2)) ||
				(!psFAT && (curFile == 1))
			) {
				// If we have a FAT we're only interested in files in the second FAT,
				// otherwise we want the first FAT.
				this->vcFAT.push_back(ep);
			}
		}
	} // else file is blank, treat as empty archive

	refcount_qenterclass(DAT_HugoArchive);
}

DAT_HugoArchive::~DAT_HugoArchive()
	throw ()
{
	refcount_qexitclass(DAT_HugoArchive);
}

void DAT_HugoArchive::rename(EntryPtr& id, const std::string& strNewName)
	throw (std::ios_base::failure)
{
	throw std::ios::failure("This archive format has no filenames to rename!");
	return;
}

void DAT_HugoArchive::updateFileOffset(const FATEntry *pid,
	std::streamsize offDelta
)
	throw (std::ios::failure)
{
	// TESTED BY: fmt_dat_hugo_insert*
	// TESTED BY: fmt_dat_hugo_resize*

	this->psArchive->seekp(DAT_FILEOFFSET_OFFSET(pid));
	this->psArchive << u32le(pid->iOffset);
	return;
}

void DAT_HugoArchive::updateFileSize(const FATEntry *pid,
	std::streamsize sizeDelta
)
	throw (std::ios::failure)
{
	// TESTED BY: fmt_dat_hugo_insert*
	// TESTED BY: fmt_dat_hugo_resize*
	this->psArchive->seekp(DAT_FILESIZE_OFFSET(pid));
	this->psArchive << u32le(pid->iSize);
	return;
}

FATArchive::FATEntry *DAT_HugoArchive::preInsertFile(
	const FATEntry *idBeforeThis, FATEntry *pNewEntry
)
	throw (std::ios::failure)
{
	// Set the format-specific variables
	pNewEntry->lenHeader = 0;

	// Because the new entry isn't in the vector yet we need to shift it manually
	pNewEntry->iOffset += DAT_FAT_ENTRY_LEN;

	this->psArchive->seekp(DAT_FATENTRY_OFFSET(pNewEntry));
	this->psArchive->insert(DAT_FAT_ENTRY_LEN);

	// Write out the entry
	this->psArchive
		<< u32le(pNewEntry->iOffset)
		<< u32le(pNewEntry->iSize)
	;

	// Update the offsets now there's a new FAT entry taking up space.
	this->shiftFiles(this->vcFAT.size() * DAT_FAT_ENTRY_LEN,
		DAT_FAT_ENTRY_LEN, 0);

	return pNewEntry;
}

void DAT_HugoArchive::preRemoveFile(const FATEntry *pid)
	throw (std::ios::failure)
{
	// TESTED BY: fmt_dat_hugo_remove*

	// Update the offsets now there's one less FAT entry taking up space.  This
	// must be called before the FAT is altered, because it will write a new
	// offset into the FAT entry we're about to erase (and if we erase it first
	// it'll overwrite something else.)
	this->shiftFiles(this->vcFAT.size() * DAT_FAT_ENTRY_LEN,
		-DAT_FAT_ENTRY_LEN, 0);

	this->psArchive->seekp(DAT_FATENTRY_OFFSET(pid));
	this->psArchive->remove(DAT_FAT_ENTRY_LEN);

	return;
}

} // namespace gamearchive
} // namespace camoto