/*
 * fmt-hog-descent.cpp - Implementation of Descent .HOG file reader/writer.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/HOG_Format
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

#include "fmt-hog-descent.hpp"
#include <camoto/iostream_helpers.hpp>
#include <camoto/debug.hpp>

#define HOG_HEADER_LEN            3
#define HOG_MAX_FILENAME_LEN      12
#define HOG_FILENAME_FIELD_LEN    13 // one more as it must always have a null
#define HOG_FAT_FILESIZE_OFFSET   13
#define HOG_FAT_ENTRY_LEN         17
#define HOG_FIRST_FILE_OFFSET     HOG_HEADER_LEN

#define HOG_MAX_FILECOUNT         250  // Maximum value supported by Descent
#define HOG_SAFETY_MAX_FILECOUNT  1024 // Maximum value we will load

namespace camoto {
namespace gamearchive {

refcount_declclass(HOGType);

HOGType::HOGType()
	throw ()
{
	refcount_qenterclass(HOGType);
}

HOGType::~HOGType()
	throw ()
{
	refcount_qexitclass(HOGType);
}

std::string HOGType::getArchiveCode() const
	throw ()
{
	return "hog-descent";
}

std::string HOGType::getFriendlyName() const
	throw ()
{
	return "Descent HOG file";
}

// Get a list of the known file extensions for this format.
std::vector<std::string> HOGType::getFileExtensions() const
	throw ()
{
	std::vector<std::string> vcExtensions;
	vcExtensions.push_back("hog");
	return vcExtensions;
}

std::vector<std::string> HOGType::getGameList() const
	throw ()
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Descent");
	return vcGames;
}

E_CERTAINTY HOGType::isInstance(iostream_sptr psArchive) const
	throw (std::ios::failure)
{
	psArchive->seekg(0, std::ios::end);
	io::stream_offset lenArchive = psArchive->tellg();

	// TESTED BY: fmt_hog_descent_isinstance_c02
	if (lenArchive < HOG_HEADER_LEN) return EC_DEFINITELY_NO; // too short

	char sig[HOG_HEADER_LEN];
	psArchive->seekg(0, std::ios::beg);
	psArchive->read(sig, HOG_HEADER_LEN);

	// TESTED BY: fmt_hog_descent_isinstance_c00
	if (strncmp(sig, "DHF", HOG_HEADER_LEN) == 0) return EC_DEFINITELY_YES;

	// TESTED BY: fmt_hog_descent_isinstance_c01
	return EC_DEFINITELY_NO;
}

ArchivePtr HOGType::newArchive(iostream_sptr psArchive, MP_SUPPDATA& suppData) const
	throw (std::ios::failure)
{
	psArchive->seekp(0, std::ios::beg);
	psArchive->write("DHF", 3);
	return ArchivePtr(new HOGArchive(psArchive));
}

// Preconditions: isInstance() has returned > EC_DEFINITELY_NO
ArchivePtr HOGType::open(iostream_sptr psArchive, MP_SUPPDATA& suppData) const
	throw (std::ios::failure)
{
	return ArchivePtr(new HOGArchive(psArchive));
}

MP_SUPPLIST HOGType::getRequiredSupps(const std::string& filenameArchive) const
	throw ()
{
	// No supplemental types/empty list
	return MP_SUPPLIST();
}


refcount_declclass(HOGArchive);

HOGArchive::HOGArchive(iostream_sptr psArchive)
	throw (std::ios::failure) :
		FATArchive(psArchive, HOG_FIRST_FILE_OFFSET)
{
	psArchive->seekg(0, std::ios::end);
	io::stream_offset lenArchive = psArchive->tellg();

	psArchive->seekg(HOG_FIRST_FILE_OFFSET, std::ios::beg); // skip sig

	// We still have to perform sanity checks in case the user forced an archive
	// to open even though it failed the signature check.
	if (psArchive->tellg() != HOG_FIRST_FILE_OFFSET) {
		throw std::ios::failure("File too short");
	}

	io::stream_offset offNext = HOG_FIRST_FILE_OFFSET;
	for (int i = 0; (offNext + HOG_FAT_ENTRY_LEN <= lenArchive); i++) {
		uint8_t buf[HOG_FAT_ENTRY_LEN];
		psArchive->read((char *)buf, HOG_FAT_ENTRY_LEN);
		FATEntry *pEntry = new FATEntry();
		pEntry->iIndex = i;
		pEntry->strName = string_from_buf(buf, HOG_MAX_FILENAME_LEN);
		pEntry->iOffset = offNext;
		pEntry->iSize = u32le_from_buf(&buf[HOG_FAT_FILESIZE_OFFSET]);
		pEntry->lenHeader = HOG_FAT_ENTRY_LEN;
		pEntry->type = FILETYPE_GENERIC;
		pEntry->fAttr = 0;
		pEntry->bValid = true;
		this->vcFAT.push_back(EntryPtr(pEntry));

		// Update the offset for the next file
		offNext += HOG_FAT_ENTRY_LEN + pEntry->iSize;
		if (offNext > lenArchive) {
			std::cerr << "Warning: File has been truncated or is not in HOG format, "
				"file list may be incomplete or complete garbage..." << std::endl;
			break;
		}
		psArchive->seekg(pEntry->iSize, std::ios::cur);
		if (i >= HOG_SAFETY_MAX_FILECOUNT) {
			throw std::ios::failure("too many files or corrupted archive");
		}
	}

	refcount_qenterclass(HOGArchive);
}

HOGArchive::~HOGArchive()
	throw ()
{
	refcount_qexitclass(HOGArchive);
}

// Does not invalidate existing EntryPtrs
void HOGArchive::rename(EntryPtr& id, const std::string& strNewName)
	throw (std::ios_base::failure)
{
	// TESTED BY: fmt_hog_descent_rename
	assert(this->isValid(id));
	FATEntry *pEntry = dynamic_cast<FATEntry *>(id.get());

	if (strNewName.length() > HOG_MAX_FILENAME_LEN) {
		throw std::ios_base::failure("new filename too long, max is "
			TOSTRING(HOG_MAX_FILENAME_LEN) " chars");
	}

	this->psArchive->seekp(pEntry->iOffset);
	this->psArchive << nullPadded(strNewName, HOG_FILENAME_FIELD_LEN);

	pEntry->strName = strNewName;

	return;
}

void HOGArchive::updateFileOffset(const FATEntry *pid, std::streamsize offDelta)
	throw (std::ios::failure)
{
	// This format doesn't have any offsets that need updating.  As this function
	// is only called when removing a file, the "offsets" will be sorted out
	// when the FAT entry is removed later.
	return;
}

void HOGArchive::updateFileSize(const FATEntry *pid, std::streamsize sizeDelta)
	throw (std::ios::failure)
{
	// TESTED BY: fmt_hog_descent_insert*
	// TESTED BY: fmt_hog_descent_resize*
	this->psArchive->seekp(pid->iOffset + HOG_FAT_FILESIZE_OFFSET);
	this->psArchive << u32le(pid->iSize);
	return;
}

FATArchive::FATEntry *HOGArchive::preInsertFile(const FATEntry *idBeforeThis, FATEntry *pNewEntry)
	throw (std::ios::failure)
{
	// TESTED BY: fmt_hog_descent_insert*
	if (pNewEntry->strName.length() > HOG_MAX_FILENAME_LEN) {
		throw std::ios::failure("maximum filename length is "
			TOSTRING(HOG_MAX_FILENAME_LEN) " chars");
	}
	if (this->vcFAT.size() + 1 > HOG_MAX_FILECOUNT) {
		throw std::ios::failure("too many files, maximum is "
			TOSTRING(HOG_MAX_FILECOUNT) " files");
	}

	// Set the format-specific variables
	pNewEntry->lenHeader = HOG_FAT_ENTRY_LEN;

	this->psArchive->seekp(pNewEntry->iOffset);
	this->psArchive->insert(HOG_FAT_ENTRY_LEN);
	this->psArchive << nullPadded(pNewEntry->strName, HOG_FILENAME_FIELD_LEN);
	this->psArchive << u32le(pNewEntry->iSize);

	// Update the offsets now the embedded FAT entry has been inserted
	this->shiftFiles(pNewEntry->iOffset, pNewEntry->lenHeader, 0);

	return pNewEntry;
}

void HOGArchive::preRemoveFile(const FATEntry *pid)
	throw (std::ios::failure)
{
	// TESTED BY: fmt_hog_descent_remove*

	// Don't need to do anything, FATArchive takes care of the embedded FAT
	return;
}

} // namespace gamearchive
} // namespace camoto
