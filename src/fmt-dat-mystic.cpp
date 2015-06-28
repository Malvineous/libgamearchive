/**
 * @file  fmt-dat-mystic.cpp
 * @brief Implementation of Mystic Towers .DAT format.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/DAT_Format_%28Mystic_Towers%29
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
#include <camoto/util.hpp> // std::make_unique
#include "fmt-dat-mystic.hpp"

#define DAT_FATOFFSET_OFFSET          8
#define DAT_FILECOUNT_OFFSET_END     -2 // bytes from EOF
#define DAT_FILENAME_FIELD_LEN       12
#define DAT_MAX_FILENAME_LEN         12
#define DAT_FIRST_FILE_OFFSET         0

#define DAT_FAT_ENTRY_LEN            (1+12+4+4)

#define DAT_SAFETY_MAX_FILECOUNT     8192 // Maximum value we will recognise

#define DAT_FATENTRY_OFFSET_END(e)   (DAT_FILECOUNT_OFFSET_END - ((signed long)this->vcFAT.size() + this->uncommittedFiles - (signed long)((e)->iIndex)) * DAT_FAT_ENTRY_LEN)
#define DAT_FILEOFFSET_OFFSET_END(e) (DAT_FATENTRY_OFFSET_END(e) + 13)
#define DAT_FILESIZE_OFFSET_END(e)   (DAT_FATENTRY_OFFSET_END(e) + 17)
#define DAT_FILENAME_OFFSET_END(e)   (DAT_FATENTRY_OFFSET_END(e) + 0)

namespace camoto {
namespace gamearchive {

ArchiveType_DAT_Mystic::ArchiveType_DAT_Mystic()
{
}

ArchiveType_DAT_Mystic::~ArchiveType_DAT_Mystic()
{
}

std::string ArchiveType_DAT_Mystic::code() const
{
	return "dat-mystic";
}

std::string ArchiveType_DAT_Mystic::friendlyName() const
{
	return "Mystic Towers DAT File";
}

std::vector<std::string> ArchiveType_DAT_Mystic::fileExtensions() const
{
	return {
		"dat",
	};
}

std::vector<std::string> ArchiveType_DAT_Mystic::games() const
{
	return {
		"Mystic Towers",
	};
}

ArchiveType::Certainty ArchiveType_DAT_Mystic::isInstance(
	stream::input& content) const
{
	stream::len lenArchive = content.size();

	// File too short
	// TESTED BY: fmt_dat_mystic_isinstance_c01
	if (lenArchive < 2) return DefinitelyNo; // too short

	uint16_t fileCount;
	content.seekg(DAT_FILECOUNT_OFFSET_END, stream::end);
	content >> u16le(fileCount);
	// Too many files
	// TESTED BY: fmt_dat_mystic_isinstance_c02
	if (fileCount >= DAT_SAFETY_MAX_FILECOUNT) return DefinitelyNo;

	// Too small to contain FAT
	// TESTED BY: fmt_dat_mystic_isinstance_c03
	if ((stream::len)(2 + fileCount * DAT_FAT_ENTRY_LEN) > lenArchive) return DefinitelyNo;

	// Don't count the FAT in the rest of the calculations using the archive size,
	// just count the actual data storage space.
	lenArchive -= 2 + fileCount * DAT_FAT_ENTRY_LEN;

	stream::len totalDataSize = 0;

	content.seekg(DAT_FILECOUNT_OFFSET_END - fileCount * DAT_FAT_ENTRY_LEN, stream::end);
	for (unsigned int i = 0; i < fileCount; i++) {
		uint8_t lenFilename;
		content >> u8(lenFilename);
		// Filename length longer than field size
		// TESTED BY: fmt_dat_mystic_isinstance_c04
		if (lenFilename > 12) return DefinitelyNo;
		content.seekg(12, stream::cur);
		uint32_t off, len;
		content >> u32le(off) >> u32le(len);
		// File starts or ends past archive EOF
		// TESTED BY: fmt_dat_mystic_isinstance_c05
		if (off + len > lenArchive) return DefinitelyNo;
		totalDataSize += len;
	}

	// File contains extra data beyond what is recorded in the FAT
	// TESTED BY: fmt_dat_mystic_isinstance_c06
	if (totalDataSize != lenArchive) return DefinitelyNo;

	// TESTED BY: fmt_dat_mystic_isinstance_c00
	return DefinitelyYes;
}

std::shared_ptr<Archive> ArchiveType_DAT_Mystic::create(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	content->seekp(0, stream::start);
	*content << u16le(0);
	return std::make_shared<Archive_DAT_Mystic>(std::move(content));
}

std::shared_ptr<Archive> ArchiveType_DAT_Mystic::open(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	return std::make_shared<Archive_DAT_Mystic>(std::move(content));
}

SuppFilenames ArchiveType_DAT_Mystic::getRequiredSupps(stream::input& content,
	const std::string& filename) const
{
	// No supplemental types/empty list
	return SuppFilenames();
}


Archive_DAT_Mystic::Archive_DAT_Mystic(std::unique_ptr<stream::inout> content)
	:	Archive_FAT(std::move(content), DAT_FIRST_FILE_OFFSET, ARCH_STD_DOS_FILENAMES),
		uncommittedFiles(0)
{
	stream::pos lenArchive = this->content->size();
	if (lenArchive < 2) throw stream::error("File too short");

	uint16_t fileCount;
	this->content->seekg(DAT_FILECOUNT_OFFSET_END, stream::end);
	*this->content >> u16le(fileCount);

	this->content->seekg(DAT_FILECOUNT_OFFSET_END - fileCount * DAT_FAT_ENTRY_LEN, stream::end);
	for (unsigned int i = 0; i < fileCount; i++) {
		auto f = this->createNewFATEntry();

		f->iIndex = i;
		f->lenHeader = 0;
		f->type = FILETYPE_GENERIC;
		f->fAttr = File::Attribute::Default;
		f->bValid = true;

		uint8_t lenFilename;
		*this->content
			>> u8(lenFilename)
		;
		if (lenFilename > 12) lenFilename = 12;
		*this->content
			>> fixedLength(f->strName, lenFilename)
		;
		// Skip over padding chars
		this->content->seekg(DAT_FILENAME_FIELD_LEN - lenFilename, stream::cur);
		*this->content
			>> u32le(f->iOffset)
			>> u32le(f->storedSize)
		;

		f->realSize = f->storedSize;
		this->vcFAT.push_back(std::move(f));
	}
}

Archive_DAT_Mystic::~Archive_DAT_Mystic()
{
}

void Archive_DAT_Mystic::updateFileName(const FATEntry *pid, const std::string& strNewName)
{
	// TESTED BY: fmt_dat_mystic_rename
	assert(strNewName.length() <= DAT_MAX_FILENAME_LEN);
	this->content->seekp(DAT_FILENAME_OFFSET_END(pid), stream::end);
	*this->content
		<< u8(strNewName.length())
		<< nullPadded(strNewName, DAT_FILENAME_FIELD_LEN)
	;
	return;
}

void Archive_DAT_Mystic::updateFileOffset(const FATEntry *pid, stream::delta offDelta)
{
	// TESTED BY: fmt_dat_mystic_insert*
	// TESTED BY: fmt_dat_mystic_resize*
	this->content->seekp(DAT_FILEOFFSET_OFFSET_END(pid), stream::end);
	*this->content << u32le(pid->iOffset);
	return;
}

void Archive_DAT_Mystic::updateFileSize(const FATEntry *pid, stream::delta sizeDelta)
{
	// TESTED BY: fmt_dat_mystic_insert*
	// TESTED BY: fmt_dat_mystic_resize*
	this->content->seekp(DAT_FILESIZE_OFFSET_END(pid), stream::end);
	*this->content << u32le(pid->storedSize);
	return;
}

void Archive_DAT_Mystic::preInsertFile(const FATEntry *idBeforeThis,
	FATEntry *pNewEntry)
{
	// TESTED BY: fmt_dat_mystic_insert*

	pNewEntry->lenHeader = 0;

	// Prepare filename field
	boost::to_upper(pNewEntry->strName);

	// Add the new entry into the on-disk FAT.  This has to happen here (rather
	// than in postInsertFile()) because on return Archive_FAT will update the
	// offsets of all FAT entries following this one.  If we don't insert a new
	// entry now, all the offset changes will be applied to the wrong files.
	this->content->seekp(DAT_FATENTRY_OFFSET_END(pNewEntry), stream::end);
	this->content->insert(DAT_FAT_ENTRY_LEN);
	this->uncommittedFiles++;

	*this->content
		<< u8(pNewEntry->strName.length())
		<< nullPadded(pNewEntry->strName, DAT_FILENAME_FIELD_LEN)
		<< u32le(pNewEntry->iOffset)
		<< u32le(pNewEntry->storedSize)
	;
	return;
}

void Archive_DAT_Mystic::postInsertFile(FATEntry *pNewEntry)
{
	this->uncommittedFiles--;
	this->updateFileCount(this->vcFAT.size());
	return;
}

void Archive_DAT_Mystic::preRemoveFile(const FATEntry *pid)
{
	this->content->seekp(DAT_FATENTRY_OFFSET_END(pid), stream::end);
	this->content->remove(DAT_FAT_ENTRY_LEN);
	return;
}

void Archive_DAT_Mystic::postRemoveFile(const FATEntry *pid)
{
	this->updateFileCount(this->vcFAT.size());
	return;
}

void Archive_DAT_Mystic::updateFileCount(uint32_t newCount)
{
	this->content->seekp(DAT_FILECOUNT_OFFSET_END, stream::end);
	*this->content << u16le(newCount);
	return;
}

} // namespace gamearchive
} // namespace camoto
