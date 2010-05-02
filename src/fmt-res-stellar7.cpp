/*
 * fmt-res-stellar7.cpp - Implementation of Stellar 7 .RES file reader/writer.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/RES_Format
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

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/progress.hpp>
#include <boost/shared_array.hpp>
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <exception>
#include <string.h>

#include "fmt-res-stellar7.hpp"
#include "iostream_helpers.hpp"
#include "debug.hpp"

#define RES_FAT_OFFSET            0
#define RES_FIRST_FILE_OFFSET     RES_FAT_OFFSET
#define RES_FAT_FILENAME_OFFSET   0
#define RES_MAX_FILENAME_LEN      4
#define RES_FAT_FILESIZE_OFFSET   4
#define RES_FAT_ENTRY_LEN         8  // filename + u32le size
#define RES_SAFETY_MAX_FILECOUNT  8192  // Don't read more than this many files

namespace camoto {
namespace gamearchive {

refcount_declclass(RESType);

RESType::RESType()
	throw ()
{
	refcount_qenterclass(RESType);
}

RESType::~RESType()
	throw ()
{
	refcount_qexitclass(RESType);
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
		uint8_t buf[RES_FAT_ENTRY_LEN];
		psArchive->read((char *)buf, RES_FAT_ENTRY_LEN);

		// Make sure there are no control characters in the filenames
		// TESTED BY: fmt_res_stellar7_isinstance_c01
		for (int j = 0; (buf[j] != 0) && (j < RES_MAX_FILENAME_LEN); j++) {
			if (buf[j] < 32) return EC_DEFINITELY_NO;
		}
		uint32_t isfolder_length = u32le_from_buf(&buf[RES_FAT_FILESIZE_OFFSET]);
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

// Preconditions: isInstance() has returned > EC_DEFINITELY_NO
ArchivePtr RESType::open(iostream_sptr psArchive, MP_SUPPDATA& suppData) const
	throw (std::ios::failure)
{
	ArchivePtr root(new RESArchiveFolder(psArchive));
	return ArchivePtr(new SubdirArchive(root));
}

MP_SUPPLIST RESType::getRequiredSupps(const std::string& filenameArchive) const
	throw ()
{
	// No supplemental types/empty list
	return MP_SUPPLIST();
}



refcount_declclass(RESArchiveFolder);

RESArchiveFolder::RESArchiveFolder(iostream_sptr psArchive)
	throw (std::ios::failure) :
		FATArchive(psArchive, RES_FIRST_FILE_OFFSET)
{
	psArchive->seekg(0, std::ios::end);
	io::stream_offset lenArchive = psArchive->tellg();

	psArchive->seekg(0, std::ios::beg);

	io::stream_offset offNext = 0;
	for (int i = 0; (
		(i < RES_SAFETY_MAX_FILECOUNT) &&
		(offNext + RES_FAT_ENTRY_LEN <= lenArchive)
	); i++) {
		uint8_t buf[RES_FAT_ENTRY_LEN];
		psArchive->read((char *)buf, RES_FAT_ENTRY_LEN);
		RESEntry *pEntry = new RESEntry();
		pEntry->iIndex = i;
		pEntry->strName = string_from_buf(buf, RES_MAX_FILENAME_LEN);
		pEntry->iOffset = offNext;
		uint32_t isfolder_length = u32le_from_buf(&buf[RES_FAT_FILESIZE_OFFSET]);
		pEntry->isFolder = isfolder_length & 0x80000000;
		pEntry->iSize = isfolder_length & 0x7FFFFFFF;
		pEntry->lenHeader = RES_FAT_ENTRY_LEN;
		pEntry->eType = EFT_USEFILENAME;
		pEntry->fAttr = 0;
		pEntry->bValid = true;
		this->vcFAT.push_back(EntryPtr(pEntry));

		// Update the offset for the next file
		offNext += RES_FAT_ENTRY_LEN + pEntry->iSize;
		if (offNext > lenArchive) {
			std::cerr << "Warning: File has been truncated or is not in RES format, "
				"file list may be incomplete or complete garbage..." << std::endl;
			break;
		}
		psArchive->seekg(pEntry->iSize, std::ios::cur);
	}
	refcount_qenterclass(RESArchiveFolder);
}

RESArchiveFolder::~RESArchiveFolder()
	throw ()
{
	refcount_qexitclass(RESArchiveFolder);
}

// Does not invalidate existing EntryPtrs
void RESArchiveFolder::rename(EntryPtr& id, const std::string& strNewName)
	throw (std::ios_base::failure)
{
	// TESTED BY: fmt_res_stellar7_rename
	assert(this->isValid(id));
	FATEntry *pEntry = dynamic_cast<FATEntry *>(id.get());

	if (strNewName.length() > RES_MAX_FILENAME_LEN) {
		throw std::ios_base::failure("new filename too long, max is "
			TOSTRING(RES_MAX_FILENAME_LEN) " chars");
	}

	this->psArchive->seekp(pEntry->iOffset + RES_FAT_FILENAME_OFFSET);
	this->psArchive << nullPadded(strNewName, RES_MAX_FILENAME_LEN);
	pEntry->strName = strNewName;

	return;
}

ArchivePtr RESArchiveFolder::openFolder(const EntryPtr& id)
	throw (std::ios::failure)
{
	const SubdirEntry *sd = dynamic_cast<const SubdirEntry *>(id.get());
	assert(sd);
	assert(sd->isFolder == true);

	iostream_sptr folderContents = this->open(id);
	return ArchivePtr(new RESArchiveFolder(folderContents));
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

void RESArchiveFolder::preInsertFile(const FATEntry *idBeforeThis, FATEntry *pNewEntry)
	throw (std::ios::failure)
{
	// TESTED BY: fmt_res_stellar7_insert*
	if (pNewEntry->strName.length() > RES_MAX_FILENAME_LEN) {
		throw std::ios::failure("maximum filename length is "
			TOSTRING(RES_MAX_FILENAME_LEN) " chars"
		);
	}

	// Set the format-specific variables
	pNewEntry->lenHeader = RES_FAT_ENTRY_LEN;

	this->psArchive->seekp(pNewEntry->iOffset);
	this->psArchive->insert(RES_FAT_ENTRY_LEN);
	boost::to_upper(pNewEntry->strName);
	this->psArchive << nullPadded(pNewEntry->strName, RES_MAX_FILENAME_LEN);
	this->psArchive << u32le(pNewEntry->iSize);

	// Since we've inserted some data for the embedded header, we need to update
	// the other file offsets accordingly.
	this->shiftFiles(pNewEntry->iOffset, pNewEntry->lenHeader, 0);

	return;
}

void RESArchiveFolder::preRemoveFile(const FATEntry *pid)
	throw (std::ios::failure)
{
	// TESTED BY: fmt_res_stellar7_remove*

	// We don't have to do anything here, because the embedded FAT will be
	// removed by FATArchive and there's no other FAT to update.

	return;
}

} // namespace gamearchive
} // namespace camoto
