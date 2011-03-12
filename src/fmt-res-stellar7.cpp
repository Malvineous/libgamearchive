/**
 * @file   fmt-res-stellar7.cpp
 * @brief  Implementation of Stellar 7 .RES file reader/writer.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/RES_Format_(Stellar_7)
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

#include "fmt-res-stellar7.hpp"

#define RES_FAT_OFFSET            0
#define RES_FIRST_FILE_OFFSET     RES_FAT_OFFSET
#define RES_FAT_FILENAME_OFFSET   0
#define RES_MAX_FILENAME_LEN      4
#define RES_FAT_FILESIZE_OFFSET   4
#define RES_FAT_ENTRY_LEN         8  // filename + u32le size
#define RES_SAFETY_MAX_FILECOUNT  8192  // Don't read more than this many files

namespace camoto {
namespace gamearchive {

RESType::RESType()
	throw ()
{
}

RESType::~RESType()
	throw ()
{
}

std::string RESType::getArchiveCode() const
	throw ()
{
	return "res-stellar7";
}

std::string RESType::getFriendlyName() const
	throw ()
{
	return "Stellar 7 Resource File";
}

// Get a list of the known file extensions for this format.
std::vector<std::string> RESType::getFileExtensions() const
	throw ()
{
	std::vector<std::string> vcExtensions;
	vcExtensions.push_back("res");
	return vcExtensions;
}

std::vector<std::string> RESType::getGameList() const
	throw ()
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Stellar 7");
	return vcGames;
}

E_CERTAINTY RESType::isInstance(iostream_sptr psArchive) const
	throw (std::ios::failure)
{
	psArchive->seekg(0, std::ios::end);
	io::stream_offset lenArchive = psArchive->tellg();

	psArchive->seekg(0, std::ios::beg);

	io::stream_offset offNext = 0; // offset of first file (+1 for KenSilverman sig)
	int i;
	for (i = 0; (
		(i < RES_SAFETY_MAX_FILECOUNT) &&
		(offNext + RES_FAT_ENTRY_LEN <= lenArchive)
	); i++) {

		// Make sure there aren't any invalid characters in the filename
		char fn[RES_MAX_FILENAME_LEN];
		psArchive->read(fn, RES_MAX_FILENAME_LEN);
		for (int j = 0; j < RES_MAX_FILENAME_LEN; j++) {
			if (!fn[j]) break; // stop on terminating null

			// Fail on control characters in the filename
			// TESTED BY: fmt_res_stellar7_isinstance_c01
			if (fn[j] < 32) return EC_DEFINITELY_NO;
		}
		uint32_t isfolder_length;
		psArchive >> u32le(isfolder_length);
		uint32_t iSize = isfolder_length & 0x7FFFFFFF;
		offNext += RES_FAT_ENTRY_LEN + iSize;

		// Make sure the files don't run past the end of the archive
		// TESTED BY: fmt_res_stellar7_isinstance_c02
		if (offNext > lenArchive) return EC_DEFINITELY_NO;

		psArchive->seekg(iSize, std::ios::cur);
	}

	if (i == RES_SAFETY_MAX_FILECOUNT) return EC_POSSIBLY_YES;

	// TESTED BY: fmt_res_stellar7_isinstance_c00
	return EC_DEFINITELY_YES;
}

ArchivePtr RESType::open(iostream_sptr psArchive, MP_SUPPDATA& suppData) const
	throw (std::ios::failure)
{
	ArchivePtr root(new RESArchiveFolder(psArchive));
	return root;
}

MP_SUPPLIST RESType::getRequiredSupps(const std::string& filenameArchive) const
	throw ()
{
	// No supplemental types/empty list
	return MP_SUPPLIST();
}


RESArchiveFolder::RESArchiveFolder(iostream_sptr psArchive)
	throw (std::ios::failure) :
		FATArchive(psArchive, RES_FIRST_FILE_OFFSET, RES_MAX_FILENAME_LEN)
{
	this->psArchive->seekg(0, std::ios::end);
	io::stream_offset lenArchive = this->psArchive->tellg();

	this->psArchive->seekg(0, std::ios::beg);

	io::stream_offset offNext = 0;
	for (int i = 0; (
		(i < RES_SAFETY_MAX_FILECOUNT) &&
		(offNext + RES_FAT_ENTRY_LEN <= lenArchive)
	); i++) {

		FATEntry *fatEntry = new FATEntry();
		EntryPtr ep(fatEntry);

		// Read the data in from the FAT entry in the file
		uint32_t isfolder_length;
		this->psArchive
			>> nullPadded(fatEntry->strName, RES_MAX_FILENAME_LEN)
			>> u32le(isfolder_length)
		;

		fatEntry->iIndex = i;
		fatEntry->iOffset = offNext;
		fatEntry->lenHeader = RES_FAT_ENTRY_LEN;
		fatEntry->type = FILETYPE_GENERIC;
		fatEntry->fAttr = 0;
		if (isfolder_length & 0x80000000) fatEntry->fAttr |= EA_FOLDER;
		fatEntry->iSize = isfolder_length & 0x7FFFFFFF;
		fatEntry->bValid = true;

		this->vcFAT.push_back(ep);

		// Update the offset for the next file
		offNext += RES_FAT_ENTRY_LEN + fatEntry->iSize;
		if (offNext > lenArchive) {
			std::cerr << "Warning: File has been truncated or is not in RES format, "
				"file list may be incomplete or complete garbage..." << std::endl;
			break;
		}
		this->psArchive->seekg(fatEntry->iSize, std::ios::cur);
	}
}

RESArchiveFolder::~RESArchiveFolder()
	throw ()
{
}

ArchivePtr RESArchiveFolder::openFolder(const EntryPtr& id)
	throw (std::ios::failure)
{
	// Make sure we're opening a folder
	assert(id->fAttr & EA_FOLDER);

	iostream_sptr folderContents = this->open(id);
	return ArchivePtr(new RESArchiveFolder(folderContents));
}

void RESArchiveFolder::updateFileName(const FATEntry *pid, const std::string& strNewName)
	throw (std::ios::failure)
{
	// TESTED BY: fmt_res_stellar7_rename
	assert(strNewName.length() <= RES_MAX_FILENAME_LEN);
	this->psArchive->seekp(pid->iOffset + RES_FAT_FILENAME_OFFSET);
	this->psArchive << nullPadded(strNewName, RES_MAX_FILENAME_LEN);
	return;
}

void RESArchiveFolder::updateFileOffset(const FATEntry *pid, std::streamsize offDelta)
	throw (std::ios::failure)
{
	// This format doesn't have any offsets that need updating.  As this function
	// is only called when removing a file, the "offsets" will be sorted out
	// when the FAT entry is removed later.
	return;
}

void RESArchiveFolder::updateFileSize(const FATEntry *pid, std::streamsize sizeDelta)
	throw (std::ios::failure)
{
	// TESTED BY: fmt_res_stellar7_insert*
	// TESTED BY: fmt_res_stellar7_resize*
	this->psArchive->seekp(pid->iOffset + RES_FAT_FILESIZE_OFFSET);
	this->psArchive << u32le(pid->iSize);
	return;
}

FATArchive::FATEntry *RESArchiveFolder::preInsertFile(const FATEntry *idBeforeThis, FATEntry *pNewEntry)
	throw (std::ios::failure)
{
	// TESTED BY: fmt_res_stellar7_insert*
	assert(pNewEntry->strName.length() <= RES_MAX_FILENAME_LEN);

	// Set the format-specific variables
	pNewEntry->lenHeader = RES_FAT_ENTRY_LEN;

	this->psArchive->seekp(pNewEntry->iOffset);
	this->psArchive->insert(RES_FAT_ENTRY_LEN);
	boost::to_upper(pNewEntry->strName);
	this->psArchive << nullPadded(pNewEntry->strName, RES_MAX_FILENAME_LEN);
	this->psArchive << u32le(pNewEntry->iSize);

	// Since we've inserted some data for the embedded header, we need to update
	// the other file offsets accordingly.
	this->shiftFiles(NULL, pNewEntry->iOffset, pNewEntry->lenHeader, 0);

	return pNewEntry;
}

} // namespace gamearchive
} // namespace camoto
