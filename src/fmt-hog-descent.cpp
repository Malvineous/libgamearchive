/**
 * @file  fmt-hog-descent.cpp
 * @brief Implementation of Descent .HOG file reader/writer.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/HOG_Format
 *
 * Copyright (C) 2010-2015 Adam Nielsen <malvineous@shikadi.net>
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

#include "fmt-hog-descent.hpp"

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

ArchiveType_HOG_Descent::ArchiveType_HOG_Descent()
{
}

ArchiveType_HOG_Descent::~ArchiveType_HOG_Descent()
{
}

std::string ArchiveType_HOG_Descent::code() const
{
	return "hog-descent";
}

std::string ArchiveType_HOG_Descent::friendlyName() const
{
	return "Descent HOG file";
}

std::vector<std::string> ArchiveType_HOG_Descent::fileExtensions() const
{
	std::vector<std::string> vcExtensions;
	vcExtensions.push_back("hog");
	return vcExtensions;
}

std::vector<std::string> ArchiveType_HOG_Descent::games() const
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Descent");
	return vcGames;
}

ArchiveType::Certainty ArchiveType_HOG_Descent::isInstance(
	stream::input& content) const
{
	stream::pos lenArchive = content.size();

	// TESTED BY: fmt_hog_descent_isinstance_c02
	if (lenArchive < HOG_HEADER_LEN) return DefinitelyNo; // too short

	char sig[HOG_HEADER_LEN];
	content.seekg(0, stream::start);
	content.read(sig, HOG_HEADER_LEN);

	// TESTED BY: fmt_hog_descent_isinstance_c00
	if (strncmp(sig, "DHF", HOG_HEADER_LEN) == 0) return DefinitelyYes;

	// TESTED BY: fmt_hog_descent_isinstance_c01
	return DefinitelyNo;
}

std::shared_ptr<Archive> ArchiveType_HOG_Descent::create(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	content->seekp(0, stream::start);
	content->write("DHF", 3);
	return std::make_shared<Archive_HOG_Descent>(std::move(content));
}

std::shared_ptr<Archive> ArchiveType_HOG_Descent::open(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	return std::make_shared<Archive_HOG_Descent>(std::move(content));
}

SuppFilenames ArchiveType_HOG_Descent::getRequiredSupps(stream::input& content,
	const std::string& filenameArchive) const
{
	// No supplemental types/empty list
	return SuppFilenames();
}


Archive_HOG_Descent::Archive_HOG_Descent(std::unique_ptr<stream::inout> content)
	:	FATArchive(std::move(content), HOG_FIRST_FILE_OFFSET, HOG_MAX_FILENAME_LEN)
{
	stream::pos lenArchive = this->content->size();

	this->content->seekg(HOG_FIRST_FILE_OFFSET, stream::start); // skip sig

	// We still have to perform sanity checks in case the user forced an archive
	// to open even though it failed the signature check.
	if (this->content->tellg() != HOG_FIRST_FILE_OFFSET) {
		throw stream::error("File too short");
	}

	stream::pos offNext = HOG_FIRST_FILE_OFFSET;
	for (int i = 0; (offNext + HOG_FAT_ENTRY_LEN <= lenArchive); i++) {
		if (i >= HOG_SAFETY_MAX_FILECOUNT) {
			throw stream::error("too many files or corrupted archive");
		}

		auto f = this->createNewFATEntry();

		*this->content
			>> nullPadded(f->strName, HOG_FILENAME_FIELD_LEN)
			>> u32le(f->storedSize)
		;

		f->iIndex = i;
		f->iOffset = offNext;
		f->lenHeader = HOG_FAT_ENTRY_LEN;
		f->type = FILETYPE_GENERIC;
		f->fAttr = EA_NONE;
		f->bValid = true;
		f->realSize = f->storedSize;

		// Update the offset for the next file
		offNext += HOG_FAT_ENTRY_LEN + f->storedSize;
		if (offNext > lenArchive) {
			std::cerr << "Warning: File has been truncated or is not in HOG format, "
				"file list may be incomplete or complete garbage..." << std::endl;
			break;
		}
		this->content->seekg(f->storedSize, stream::cur);
		this->vcFAT.push_back(std::move(f));
	}
}

Archive_HOG_Descent::~Archive_HOG_Descent()
{
}

void Archive_HOG_Descent::updateFileName(const FATEntry *pid, const std::string& strNewName)
{
	// TESTED BY: fmt_hog_descent_rename
	assert(strNewName.length() <= HOG_MAX_FILENAME_LEN);
	this->content->seekp(pid->iOffset, stream::start);
	*this->content << nullPadded(strNewName, HOG_FILENAME_FIELD_LEN);
	return;
}

void Archive_HOG_Descent::updateFileOffset(const FATEntry *pid, stream::delta offDelta)
{
	// This format doesn't have any offsets that need updating.  As this function
	// is only called when removing a file, the "offsets" will be sorted out
	// when the FAT entry is removed later.
	return;
}

void Archive_HOG_Descent::updateFileSize(const FATEntry *pid, stream::delta sizeDelta)
{
	// TESTED BY: fmt_hog_descent_insert*
	// TESTED BY: fmt_hog_descent_resize*
	this->content->seekp(pid->iOffset + HOG_FAT_FILESIZE_OFFSET, stream::start);
	*this->content << u32le(pid->storedSize);
	return;
}

void Archive_HOG_Descent::preInsertFile(const FATEntry *idBeforeThis, FATEntry *pNewEntry)
{
	// TESTED BY: fmt_hog_descent_insert*
	assert(pNewEntry->strName.length() <= HOG_MAX_FILENAME_LEN);

	if (this->vcFAT.size() + 1 > HOG_MAX_FILECOUNT) {
		throw stream::error("too many files, maximum is "
			TOSTRING(HOG_MAX_FILECOUNT) " files");
	}

	// Set the format-specific variables
	pNewEntry->lenHeader = HOG_FAT_ENTRY_LEN;

	this->content->seekp(pNewEntry->iOffset, stream::start);
	this->content->insert(HOG_FAT_ENTRY_LEN);
	*this->content << nullPadded(pNewEntry->strName, HOG_FILENAME_FIELD_LEN);
	*this->content << u32le(pNewEntry->storedSize);

	// Update the offsets now the embedded FAT entry has been inserted
	this->shiftFiles(NULL, pNewEntry->iOffset, pNewEntry->lenHeader, 0);

	return;
}

} // namespace gamearchive
} // namespace camoto
