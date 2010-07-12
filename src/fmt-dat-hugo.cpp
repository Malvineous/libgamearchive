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

//#include <boost/filesystem/operations.hpp>
//#include <boost/filesystem/path.hpp>
//#include <boost/progress.hpp>
//#include <boost/shared_array.hpp>
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <exception>
#include <string.h>

#include <camoto/iostream_helpers.hpp>
#include <camoto/debug.hpp>
#include "fmt-dat-hugo.hpp"

//#define DAT_FILECOUNT_OFFSET     0
//#define DAT_MAX_FILENAME_LEN     12
//#define DAT_FILENAME_FIELD_LEN   14
#define DAT_FAT_ENTRY_LEN        8  // u32le offset + u32le size
#define DAT_FAT_OFFSET           0
#define DAT_FIRST_FILE_OFFSET    8

#define DAT_FATENTRY_OFFSET(e)   (DAT_FAT_OFFSET + e->iIndex * DAT_FAT_ENTRY_LEN)

#define DAT_FILENAME_OFFSET(e)    DAT_FATENTRY_OFFSET(e)
#define DAT_FILESIZE_OFFSET(e)   (DAT_FATENTRY_OFFSET(e) + 4)
#define DAT_FILEOFFSET_OFFSET(e) DAT_FATENTRY_OFFSET(e)

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
	// TESTED BY: fmt_dat_hugo_isinstance_c02
	if (lenArchive < DAT_FAT_ENTRY_LEN) return EC_DEFINITELY_NO; // too short

	psArchive->seekg(0, std::ios::beg);

	uint32_t fatEnd, firstLen;
	psArchive >> u32le(fatEnd) >> u32le(firstLen);
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
		// TESTED BY: fmt_dat_hugo_isinstance_c?? - TODO
		if (offEntry + lenEntry > lenArchive) return EC_DEFINITELY_NO;
	}

	// If we've made it this far, this is almost certainly a DAT file.

	// TESTED BY: fmt_dat_hugo_isinstance_c00
	return EC_DEFINITELY_YES;
}

ArchivePtr DAT_HugoType::newArchive(iostream_sptr psArchive, MP_SUPPDATA& suppData) const
	throw (std::ios::failure)
{
	psArchive->seekg(0, std::ios::beg);
	psArchive << u32le(0) << u32le(0); // FAT terminator
	return ArchivePtr(new DAT_HugoArchive(psArchive));
}

// Preconditions: isInstance() has returned > EC_DEFINITELY_NO
ArchivePtr DAT_HugoType::open(iostream_sptr psArchive, MP_SUPPDATA& suppData) const
	throw (std::ios::failure)
{
	return ArchivePtr(new DAT_HugoArchive(psArchive));
}

MP_SUPPLIST DAT_HugoType::getRequiredSupps(const std::string& filenameArchive) const
	throw ()
{
	// No supplemental types/empty list
	return MP_SUPPLIST();
}


refcount_declclass(DAT_HugoArchive);

DAT_HugoArchive::DAT_HugoArchive(iostream_sptr psArchive)
	throw (std::ios::failure) :
		FATArchive(psArchive, DAT_FIRST_FILE_OFFSET)
{
	this->psArchive->seekg(0, std::ios::end);
	io::stream_offset lenArchive = this->psArchive->tellg();
	if (lenArchive < DAT_FAT_ENTRY_LEN) {
		throw std::ios::failure("Archive too short - no FAT terminator!");
	}

	this->psArchive->seekg(0, std::ios::beg);

	uint32_t fatEnd;
	this->psArchive >> u32le(fatEnd);
	if (fatEnd >= lenArchive) {
		throw std::ios::failure("Archive corrupt - first file starts after EOF!");
	}
	uint32_t numFiles = fatEnd / DAT_FAT_ENTRY_LEN;
	this->vcFAT.reserve(numFiles);

	this->psArchive->seekg(0, std::ios::beg);
	for (int i = 0; i < numFiles; i++) {
		FATEntry *fatEntry = new FATEntry();
		EntryPtr ep(fatEntry);

		fatEntry->iIndex = i;
		fatEntry->lenHeader = 0;
		fatEntry->eType = EFT_USEFILENAME;
		fatEntry->fAttr = 0;
		fatEntry->bValid = true;
		fatEntry->strName = std::string();

		// Read the data in from the FAT entry in the file
		this->psArchive
			>> u32le(fatEntry->iOffset)
			>> u32le(fatEntry->iSize)
		;

		if ((fatEntry->iOffset == 0) || (fatEntry->iSize == 0)) {
			// TODO: mark as spare/unused FAT entry
		}

		// Offset doesn't include the two byte file count
		//fatEntry->iOffset += DAT_FAT_OFFSET;

		this->vcFAT.push_back(ep);
	}

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

	// Offsets don't start from the beginning of the archive
	uint32_t deltaOffset = pid->iOffset - DAT_FAT_OFFSET;

	this->psArchive->seekp(DAT_FILEOFFSET_OFFSET(pid));
	this->psArchive << u32le(deltaOffset);
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
	boost::to_upper(pNewEntry->strName);

	// Offsets don't start from the beginning of the archive
	uint32_t deltaOffset = pNewEntry->iOffset - DAT_FAT_OFFSET;

	// Write out the entry
	this->psArchive
		<< u32le(deltaOffset)
		<< u32le(pNewEntry->iSize)
	;

	// Update the offsets now there's a new FAT entry taking up space.
	this->shiftFiles(DAT_FAT_OFFSET + this->vcFAT.size() * DAT_FAT_ENTRY_LEN,
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
	this->shiftFiles(DAT_FAT_OFFSET + this->vcFAT.size() * DAT_FAT_ENTRY_LEN,
		-DAT_FAT_ENTRY_LEN, 0);

	this->psArchive->seekp(DAT_FATENTRY_OFFSET(pid));
	this->psArchive->remove(DAT_FAT_ENTRY_LEN);

	return;
}

} // namespace gamearchive
} // namespace camoto
