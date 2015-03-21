/**
 * @file  fmt-res-stellar7.cpp
 * @brief Implementation of Stellar 7 .RES file reader/writer.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/RES_Format_(Stellar_7)
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

ArchiveType_RES_Stellar7::ArchiveType_RES_Stellar7()
{
}

ArchiveType_RES_Stellar7::~ArchiveType_RES_Stellar7()
{
}

std::string ArchiveType_RES_Stellar7::code() const
{
	return "res-stellar7";
}

std::string ArchiveType_RES_Stellar7::friendlyName() const
{
	return "Stellar 7 Resource File";
}

std::vector<std::string> ArchiveType_RES_Stellar7::fileExtensions() const
{
	std::vector<std::string> vcExtensions;
	vcExtensions.push_back("res");
	return vcExtensions;
}

std::vector<std::string> ArchiveType_RES_Stellar7::games() const
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Stellar 7");
	return vcGames;
}

ArchiveType::Certainty ArchiveType_RES_Stellar7::isInstance(
	stream::input& content) const
{
	stream::pos lenArchive = content.size();

	content.seekg(0, stream::start);

	stream::pos offNext = 0;
	int i;
	for (i = 0; (
		(i < RES_SAFETY_MAX_FILECOUNT) &&
		(offNext + RES_FAT_ENTRY_LEN <= lenArchive)
	); i++) {

		// Make sure there aren't any invalid characters in the filename
		char fn[RES_MAX_FILENAME_LEN];
		content.read(fn, RES_MAX_FILENAME_LEN);
		for (int j = 0; j < RES_MAX_FILENAME_LEN; j++) {
			if (!fn[j]) break; // stop on terminating null

			// Fail on control characters in the filename
			// TESTED BY: fmt_res_stellar7_isinstance_c01
			if (fn[j] < 32) return DefinitelyNo;
		}
		uint32_t isfolder_length;
		content >> u32le(isfolder_length);
		uint32_t iSize = isfolder_length & 0x7FFFFFFF;
		offNext += RES_FAT_ENTRY_LEN + iSize;

		// Make sure the files don't run past the end of the archive
		// TESTED BY: fmt_res_stellar7_isinstance_c02
		if (offNext > lenArchive) return DefinitelyNo;

		content.seekg(iSize, stream::cur);
	}

	if (i == RES_SAFETY_MAX_FILECOUNT) return PossiblyYes;

	// TESTED BY: fmt_res_stellar7_isinstance_c00
	return DefinitelyYes;
}

std::shared_ptr<Archive> ArchiveType_RES_Stellar7::create(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	return this->open(std::move(content), suppData);
}

std::shared_ptr<Archive> ArchiveType_RES_Stellar7::open(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	return std::make_shared<Archive_RES_Stellar7_Folder>(std::move(content));
}

SuppFilenames ArchiveType_RES_Stellar7::getRequiredSupps(stream::input& content,
	const std::string& filenameArchive) const
{
	// No supplemental types/empty list
	return SuppFilenames();
}


Archive_RES_Stellar7_Folder::Archive_RES_Stellar7_Folder(
	std::unique_ptr<stream::inout> content)
	:	Archive_FAT(std::move(content), RES_FIRST_FILE_OFFSET, RES_MAX_FILENAME_LEN)
{
	stream::pos lenArchive = this->content->size();

	this->content->seekg(0, stream::start);

	stream::pos offNext = 0;
	for (int i = 0; (
		(i < RES_SAFETY_MAX_FILECOUNT) &&
		(offNext + RES_FAT_ENTRY_LEN <= lenArchive)
	); i++) {
		auto f = this->createNewFATEntry();

		// Read the data in from the FAT entry in the file
		uint32_t isfolder_length;
		*this->content
			>> nullPadded(f->strName, RES_MAX_FILENAME_LEN)
			>> u32le(isfolder_length)
		;

		f->iIndex = i;
		f->iOffset = offNext;
		f->lenHeader = RES_FAT_ENTRY_LEN;
		f->type = FILETYPE_GENERIC;
		f->fAttr = File::Attribute::Default;
		if (isfolder_length & 0x80000000) f->fAttr |= File::Attribute::Folder;
		f->storedSize = isfolder_length & 0x7FFFFFFF;
		f->bValid = true;
		f->realSize = f->storedSize;

		// Update the offset for the next file
		offNext += RES_FAT_ENTRY_LEN + f->storedSize;
		this->content->seekg(f->storedSize, stream::cur);

		this->vcFAT.push_back(std::move(f));

		if (offNext > lenArchive) {
			std::cerr << "Warning: File has been truncated or is not in RES format, "
				"file list may be incomplete or complete garbage..." << std::endl;
			break;
		}
	}
}

Archive_RES_Stellar7_Folder::~Archive_RES_Stellar7_Folder()
{
}

std::shared_ptr<Archive> Archive_RES_Stellar7_Folder::openFolder(
	const FileHandle& id)
{
	// Make sure we're opening a folder
	assert(id->fAttr & File::Attribute::Folder);

	return std::make_shared<Archive_RES_Stellar7_Folder>(
		this->open(id, true)
	);
}

void Archive_RES_Stellar7_Folder::updateFileName(const FATEntry *pid,
	const std::string& strNewName)
{
	// TESTED BY: fmt_res_stellar7_rename
	assert(strNewName.length() <= RES_MAX_FILENAME_LEN);
	this->content->seekp(pid->iOffset + RES_FAT_FILENAME_OFFSET, stream::start);
	*this->content << nullPadded(strNewName, RES_MAX_FILENAME_LEN);
	return;
}

void Archive_RES_Stellar7_Folder::updateFileOffset(const FATEntry *pid,
	stream::delta offDelta)
{
	// This format doesn't have any offsets that need updating.  As this function
	// is only called when removing a file, the "offsets" will be sorted out
	// when the FAT entry is removed later.
	return;
}

void Archive_RES_Stellar7_Folder::updateFileSize(const FATEntry *pid,
	stream::delta sizeDelta)
{
	// TESTED BY: fmt_res_stellar7_insert*
	// TESTED BY: fmt_res_stellar7_resize*
	this->content->seekp(pid->iOffset + RES_FAT_FILESIZE_OFFSET, stream::start);
	*this->content << u32le(pid->storedSize);
	return;
}

void Archive_RES_Stellar7_Folder::preInsertFile(const FATEntry *idBeforeThis,
	FATEntry *pNewEntry)
{
	// TESTED BY: fmt_res_stellar7_insert*
	assert(pNewEntry->strName.length() <= RES_MAX_FILENAME_LEN);

	// Set the format-specific variables
	pNewEntry->lenHeader = RES_FAT_ENTRY_LEN;

	this->content->seekp(pNewEntry->iOffset, stream::start);
	this->content->insert(RES_FAT_ENTRY_LEN);
	boost::to_upper(pNewEntry->strName);
	*this->content << nullPadded(pNewEntry->strName, RES_MAX_FILENAME_LEN);
	*this->content << u32le(pNewEntry->storedSize);

	// Since we've inserted some data for the embedded header, we need to update
	// the other file offsets accordingly.
	this->shiftFiles(NULL, pNewEntry->iOffset, pNewEntry->lenHeader, 0);

	return;
}

} // namespace gamearchive
} // namespace camoto
