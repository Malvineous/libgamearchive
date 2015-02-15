/**
 * @file  fmt-resource-tim-fat.cpp
 * @brief File reader/writer for The Incredible Machine resource FAT files.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/TIM_Resource_Format
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

#include "fmt-resource-tim-fat.hpp"

#define TIM_FIRST_FILE_OFFSET     6
#define TIM_MAX_FILENAME_LEN      12
#define TIM_FILENAME_FIELD_LEN    13
#define TIM_FILECOUNT_OFFSET      4

// Embedded FAT (no offset, has sig)
#define TIM_EFAT_FILENAME_OFFSET  0
#define TIM_EFAT_FILESIZE_OFFSET  13
#define TIM_EFAT_ENTRY_LEN        15  // filename + u16le count

#define TIM_CONTENT_ITEM_LEN      8   // FILE_ENTRY length

namespace camoto {
namespace gamearchive {

ArchiveType_Resource_TIM_FAT::ArchiveType_Resource_TIM_FAT()
{
}

ArchiveType_Resource_TIM_FAT::~ArchiveType_Resource_TIM_FAT()
{
}

std::string ArchiveType_Resource_TIM_FAT::code() const
{
	return "resource-tim-fat";
}

std::string ArchiveType_Resource_TIM_FAT::friendlyName() const
{
	return "FAT for The Incredible Machine Resource File";
}

std::vector<std::string> ArchiveType_Resource_TIM_FAT::fileExtensions() const
{
	std::vector<std::string> vcExtensions;
	vcExtensions.push_back("map");
	return vcExtensions;
}

std::vector<std::string> ArchiveType_Resource_TIM_FAT::games() const
{
	std::vector<std::string> vcGames;
	vcGames.push_back("The Incredible Machine");
	return vcGames;
}

ArchiveType::Certainty ArchiveType_Resource_TIM_FAT::isInstance(
	stream::input& content) const
{
	try {
		stream::len lenArchive = content.size();
		// TESTED BY: fmt_resource_tim_fat_isinstance_c01
		if (lenArchive < TIM_FIRST_FILE_OFFSET) return DefinitelyNo; // too short

		uint16_t numFiles;
		content.seekg(TIM_FILECOUNT_OFFSET, stream::start);
		content >> u16le(numFiles);

		stream::pos step = TIM_FIRST_FILE_OFFSET;
		uint16_t count;
		stream::len dataSize;
		for (int i = 0; (i < numFiles) && (step < lenArchive); i++) {
			step += TIM_FILENAME_FIELD_LEN; // skip filename
			content.seekg(step, stream::start);
			content >> u16le(count);
			dataSize = count * TIM_CONTENT_ITEM_LEN;
			step += 2 + dataSize;
		}
		// There should be no data following the last file.
		// TESTED BY: fmt_resource_tim_fat_isinstance_c02
		if (step != lenArchive) return DefinitelyNo;

	} catch (const stream::incomplete_read&) {
		// TESTED BY: fmt_resource_tim_fat_isinstance_c03
		return DefinitelyNo;
	}

	// If we've made it this far, this is almost certainly in the correct format.
	// TESTED BY: fmt_resource_tim_fat_isinstance_c00
	return DefinitelyYes;
}

std::unique_ptr<Archive> ArchiveType_Resource_TIM_FAT::create(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	content->seekp(0, stream::start);
	content->write("\x00\x00" "\x00\x00" "\x00\x00", 6);
	return std::make_unique<Archive_Resource_TIM_FAT>(std::move(content));
}

std::unique_ptr<Archive> ArchiveType_Resource_TIM_FAT::open(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	return std::make_unique<Archive_Resource_TIM_FAT>(std::move(content));
}

SuppFilenames ArchiveType_Resource_TIM_FAT::getRequiredSupps(stream::input& content,
	const std::string& filenameArchive) const
{
	// No supplemental types/empty list
	return SuppFilenames();
}


Archive_Resource_TIM_FAT::Archive_Resource_TIM_FAT(std::shared_ptr<stream::inout> content)
	:	FATArchive(content, TIM_FIRST_FILE_OFFSET, TIM_MAX_FILENAME_LEN)
{
	this->content->seekg(TIM_FILECOUNT_OFFSET, stream::start);
	uint16_t numFiles;
	*this->content >> u16le(numFiles);
	stream::pos pos = TIM_FILECOUNT_OFFSET + 2;
	for (unsigned int i = 0; i < numFiles; i++) {
		auto f = this->createNewFATEntry();
		uint16_t count;
		*this->content
			>> nullPadded(f->strName, 13)
			>> u16le(count)
		;
		f->storedSize = count * TIM_CONTENT_ITEM_LEN;
		f->realSize = f->storedSize;
		f->iOffset = pos;
		f->iIndex = i;
		f->lenHeader = TIM_EFAT_ENTRY_LEN;
		f->type = FILETYPE_GENERIC;
		f->fAttr = 0;
		f->bValid = true;

		this->content->seekg(f->storedSize, stream::cur);
		pos += TIM_EFAT_ENTRY_LEN + f->storedSize;

		this->vcFAT.push_back(std::move(f));
	}
}

Archive_Resource_TIM_FAT::~Archive_Resource_TIM_FAT()
{
}

void Archive_Resource_TIM_FAT::updateFileName(const FATEntry *pid, const std::string& strNewName)
{
	// TESTED BY: fmt_resource_tim_fat_rename
	assert(strNewName.length() <= TIM_MAX_FILENAME_LEN);

	this->content->seekp(pid->iOffset + TIM_EFAT_FILENAME_OFFSET, stream::start);
	*this->content << nullPadded(strNewName, TIM_MAX_FILENAME_LEN);

	return;
}

void Archive_Resource_TIM_FAT::updateFileOffset(const FATEntry *pid, stream::delta offDelta)
{
	// Nothing to do
	return;
}

void Archive_Resource_TIM_FAT::updateFileSize(const FATEntry *pid, stream::delta sizeDelta)
{
	// TESTED BY: fmt_resource_tim_fat_insert*
	// TESTED BY: fmt_resource_tim_fat_resize*

	if (pid->storedSize % TIM_CONTENT_ITEM_LEN) {
		throw stream::error("Files in this archive must be a multiple of "
			TOSTRING(TIM_CONTENT_ITEM_LEN) " bytes.");
	}

	// Update embedded FAT
	this->content->seekp(pid->iOffset + TIM_EFAT_FILESIZE_OFFSET, stream::start);
	uint16_t actualSize = pid->storedSize / TIM_CONTENT_ITEM_LEN;
	*this->content << u16le(actualSize);

	return;
}

void Archive_Resource_TIM_FAT::preInsertFile(const FATEntry *idBeforeThis, FATEntry *pNewEntry)
{
	// TESTED BY: fmt_resource_tim_fat_insert*
	int len = pNewEntry->strName.length();
	assert(len <= TIM_MAX_FILENAME_LEN);

	if (pNewEntry->storedSize % TIM_CONTENT_ITEM_LEN) {
		throw stream::error("Files in this archive must be a multiple of "
			TOSTRING(TIM_CONTENT_ITEM_LEN) " bytes.");
	}

	// Set the format-specific variables
	pNewEntry->lenHeader = TIM_EFAT_ENTRY_LEN;

	boost::to_upper(pNewEntry->strName);

	// Write out the new embedded FAT entry
	this->content->seekp(pNewEntry->iOffset, stream::start);
	this->content->insert(TIM_EFAT_ENTRY_LEN);

	uint16_t actualSize = pNewEntry->storedSize / TIM_CONTENT_ITEM_LEN;

	// Write the header
	*this->content
		<< nullPadded(pNewEntry->strName, TIM_FILENAME_FIELD_LEN)
		<< u16le(actualSize)
	;

	// Since we've inserted some data for the embedded header, we need to update
	// the other file offsets accordingly.  This call updates the offset of the
	// files, then calls updateFileOffset() on them, using the *new* offset, so
	// we need to do this after the insert() call above to make sure the extra
	// data has been inserted.  Then when updateFileOffset() writes data out it
	// will go into the correct spot.
	this->shiftFiles(NULL, pNewEntry->iOffset, pNewEntry->lenHeader, 0);

	this->updateFileCount(this->vcFAT.size() + 1);
	return;
}

void Archive_Resource_TIM_FAT::preRemoveFile(const FATEntry *pid)
{
	// TESTED BY: fmt_resource_tim_fat_remove*
	this->updateFileCount(this->vcFAT.size() - 1);
	return;
}

void Archive_Resource_TIM_FAT::updateFileCount(uint32_t iNewCount)
{
	// TESTED BY: fmt_resource_tim_fat_insert*
	// TESTED BY: fmt_resource_tim_fat_remove*
	this->content->seekp(TIM_FILECOUNT_OFFSET, stream::start);
	*this->content << u16le(iNewCount);
	return;
}

} // namespace gamearchive
} // namespace camoto
