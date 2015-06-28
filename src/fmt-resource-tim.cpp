/**
 * @file  fmt-resource-tim.cpp
 * @brief File reader/writer for The Incredible Machine resource files.
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

#include "fmt-resource-tim.hpp"

#define TIM_FIRST_FILE_OFFSET     0
#define TIM_MAX_FILENAME_LEN      12
#define TIM_FILENAME_FIELD_LEN    13

// Embedded FAT (no offset, has sig)
#define TIM_EFAT_FILENAME_OFFSET  0
#define TIM_EFAT_FILESIZE_OFFSET  13
#define TIM_EFAT_ENTRY_LEN        17  // filename + u32le size

// FAT file (no sig, has offset)
#define TIM_FAT_FILEOFFSET_OFFSET 4
#define TIM_FAT_ENTRY_LEN         8   // 2x unknown + offset

namespace camoto {
namespace gamearchive {

ArchiveType_Resource_TIM::ArchiveType_Resource_TIM()
{
}

ArchiveType_Resource_TIM::~ArchiveType_Resource_TIM()
{
}

std::string ArchiveType_Resource_TIM::code() const
{
	return "resource-tim";
}

std::string ArchiveType_Resource_TIM::friendlyName() const
{
	return "The Incredible Machine Resource File";
}

std::vector<std::string> ArchiveType_Resource_TIM::fileExtensions() const
{
	return {
		"001",
		"002",
		"003",
		"004",
	};
}

std::vector<std::string> ArchiveType_Resource_TIM::games() const
{
	return {
		"The Incredible Machine",
	};
}

ArchiveType::Certainty ArchiveType_Resource_TIM::isInstance(
	stream::input& content) const
{
	try {
		stream::len lenArchive = content.size();
		// TESTED BY: fmt_resource_tim_new_isinstance
		if (lenArchive == 0) return DefinitelyYes; // empty archive

		// TESTED BY: fmt_resource_tim_isinstance_c01
		if (lenArchive < TIM_EFAT_ENTRY_LEN) return DefinitelyNo; // too short

		stream::pos step = 0;
		uint32_t fileSize;
		while (step < lenArchive) {
			content.seekg(step + TIM_FILENAME_FIELD_LEN, stream::start);
			content >> u32le(fileSize);
			step += TIM_FILENAME_FIELD_LEN + 4 + fileSize;
		}

		// TESTED BY: fmt_resource_tim_isinstance_c02
		if (step != lenArchive) return DefinitelyNo;
	} catch (const stream::incomplete_read&) {
		// TESTED BY: fmt_resource_tim_isinstance_c03
		return DefinitelyNo;
	}

	// If we've made it this far, this is almost certainly in the correct format.
	// TESTED BY: fmt_resource_tim_isinstance_c00
	return DefinitelyYes;
}

std::shared_ptr<Archive> ArchiveType_Resource_TIM::create(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	return this->open(std::move(content), suppData);
}

std::shared_ptr<Archive> ArchiveType_Resource_TIM::open(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	assert(suppData.find(SuppItem::FAT) != suppData.end());
	return std::make_shared<Archive_Resource_TIM>(
		std::move(content),
		std::move(suppData[SuppItem::FAT])
	);
}

SuppFilenames ArchiveType_Resource_TIM::getRequiredSupps(stream::input& content,
	const std::string& filename) const
{
	// No supplemental types/empty list
	SuppFilenames supps;
	supps[SuppItem::FAT] = "fat/" + filename;
	return supps;
}


Archive_Resource_TIM::Archive_Resource_TIM(
	std::unique_ptr<stream::inout> content, std::unique_ptr<stream::inout> psFAT)
	:	Archive_FAT(std::move(content), TIM_FIRST_FILE_OFFSET, TIM_MAX_FILENAME_LEN),
		psFAT(std::make_unique<stream::seg>(std::move(psFAT)))
{
	stream::len lenArchive = this->content->size();
	this->content->seekg(0, stream::start);

	stream::pos pos = 0;
	unsigned int index = 0;
	while (pos < lenArchive) {
		auto f = this->createNewFATEntry();
		*this->content
			>> nullPadded(f->strName, TIM_FILENAME_FIELD_LEN)
			>> u32le(f->storedSize)
		;
		f->iOffset = pos;
		f->iIndex = index++;
		f->lenHeader = TIM_EFAT_ENTRY_LEN;
		f->type = FILETYPE_GENERIC;
		f->fAttr = File::Attribute::Default;
		f->bValid = true;
		f->realSize = f->storedSize;

		this->content->seekg(f->storedSize, stream::cur);
		pos += TIM_EFAT_ENTRY_LEN + f->storedSize;

		this->vcFAT.push_back(std::move(f));
	}
}

Archive_Resource_TIM::~Archive_Resource_TIM()
{
}

void Archive_Resource_TIM::flush()
{
	this->psFAT->flush();
	this->Archive_FAT::flush();
	return;
}

void Archive_Resource_TIM::updateFileName(const FATEntry *pid,
	const std::string& strNewName)
{
	// TESTED BY: fmt_resource_tim_rename
	assert(strNewName.length() <= TIM_MAX_FILENAME_LEN);

	this->content->seekp(pid->iOffset + TIM_EFAT_FILENAME_OFFSET, stream::start);
	*this->content << nullPadded(strNewName, TIM_FILENAME_FIELD_LEN);

	return;
}

void Archive_Resource_TIM::updateFileOffset(const FATEntry *pid, stream::delta offDelta)
{
	// TESTED BY: fmt_resource_tim_insert*
	// TESTED BY: fmt_resource_tim_resize*

	// Only the external FAT file has offsets, not the embedded FAT
	this->psFAT->seekp(pid->iIndex * TIM_FAT_ENTRY_LEN + TIM_FAT_FILEOFFSET_OFFSET, stream::start);
	*this->psFAT << u32le(pid->iOffset);
	return;
}

void Archive_Resource_TIM::updateFileSize(const FATEntry *pid, stream::delta sizeDelta)
{
	// TESTED BY: fmt_resource_tim_insert*
	// TESTED BY: fmt_resource_tim_resize*

	// Update embedded FAT
	this->content->seekp(pid->iOffset + TIM_EFAT_FILESIZE_OFFSET, stream::start);
	*this->content << u32le(pid->storedSize);

	return;
}

void Archive_Resource_TIM::preInsertFile(const FATEntry *idBeforeThis, FATEntry *pNewEntry)
{
	// TESTED BY: fmt_resource_tim_insert*
	int len = pNewEntry->strName.length();
	assert(len <= TIM_MAX_FILENAME_LEN);

	// Set the format-specific variables
	pNewEntry->lenHeader = TIM_EFAT_ENTRY_LEN;

	boost::to_upper(pNewEntry->strName);

	// Write out the new embedded FAT entry
	this->content->seekp(pNewEntry->iOffset, stream::start);
	this->content->insert(TIM_EFAT_ENTRY_LEN);

	// Write the header
	*this->content
		<< nullPadded(pNewEntry->strName, TIM_FILENAME_FIELD_LEN)
		<< u32le(pNewEntry->storedSize)
	;

	// Since we've inserted some data for the embedded header, we need to update
	// the other file offsets accordingly.  This call updates the offset of the
	// files, then calls updateFileOffset() on them, using the *new* offset, so
	// we need to do this after the insert() call above to make sure the extra
	// data has been inserted.  Then when updateFileOffset() writes data out it
	// will go into the correct spot.
	this->shiftFiles(NULL, pNewEntry->iOffset, pNewEntry->lenHeader, 0);

	// Write out same info again but into the external FAT
	this->psFAT->seekp(pNewEntry->iIndex * TIM_FAT_ENTRY_LEN, stream::start);
	this->psFAT->insert(TIM_FAT_ENTRY_LEN);
	*this->psFAT
		<< u16le(0)
		<< u16le(0)
		<< u32le(pNewEntry->iOffset)
	;

	return;
}

void Archive_Resource_TIM::preRemoveFile(const FATEntry *pid)
{
	// TESTED BY: fmt_resource_tim_remove*

	// Remove the FAT entry
	this->psFAT->seekp(pid->iIndex * TIM_FAT_ENTRY_LEN, stream::start);
	this->psFAT->remove(TIM_FAT_ENTRY_LEN);

	return;
}

} // namespace gamearchive
} // namespace camoto
