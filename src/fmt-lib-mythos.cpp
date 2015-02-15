/**
 * @file  fmt-lib-mythos.cpp
 * @brief Implementation of Mythos .LIB archive format.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/LIB_Format_%28Mythos_Software%29
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

#include "fmt-lib-mythos.hpp"

#define LIB_HEADER_LEN          4  // "LIB\x1A"
#define LIB_MAX_FILENAME_LEN    12
#define LIB_FILENAME_FIELD_LEN  13
#define LIB_FILECOUNT_OFFSET    LIB_HEADER_LEN
#define LIB_FAT_OFFSET          (LIB_FILECOUNT_OFFSET + 2)
#define LIB_FAT_ENTRY_LEN       (LIB_FILENAME_FIELD_LEN + 4) // name+offset
#define LIB_FIRST_FILE_OFFSET   (LIB_FAT_OFFSET + LIB_FAT_ENTRY_LEN)  // empty archive only

#define LIB_SAFETY_MAX_FILECOUNT  8192 // Maximum value we will load

#define LIB_FATENTRY_OFFSET(e)   (LIB_FAT_OFFSET + e->iIndex * LIB_FAT_ENTRY_LEN)
#define LIB_FILENAME_OFFSET(e)   LIB_FATENTRY_OFFSET(e)
#define LIB_FILEOFFSET_OFFSET(e) (LIB_FATENTRY_OFFSET(e) + LIB_FILENAME_FIELD_LEN)

namespace camoto {
namespace gamearchive {

ArchiveType_LIB_Mythos::ArchiveType_LIB_Mythos()
{
}

ArchiveType_LIB_Mythos::~ArchiveType_LIB_Mythos()
{
}

std::string ArchiveType_LIB_Mythos::code() const
{
	return "lib-mythos";
}

std::string ArchiveType_LIB_Mythos::friendlyName() const
{
	return "Mythos Library File";
}

std::vector<std::string> ArchiveType_LIB_Mythos::fileExtensions() const
{
	std::vector<std::string> vcExtensions;
	vcExtensions.push_back("lib");
	vcExtensions.push_back("snd");
	return vcExtensions;
}

std::vector<std::string> ArchiveType_LIB_Mythos::games() const
{
	std::vector<std::string> vcGames;
	vcGames.push_back("The Lost Files of Sherlock Holmes: The Case of the Serrated Scalpel");
	return vcGames;
}

ArchiveType::Certainty ArchiveType_LIB_Mythos::isInstance(
	stream::input& content) const
{
	stream::pos lenArchive = content.size();

	// TESTED BY: fmt_lib_mythos_isinstance_c02
	if (lenArchive < LIB_FAT_ENTRY_LEN) return DefinitelyNo; // too short

	char sig[4];
	content.seekg(0, stream::start);
	content.read(sig, 4);

	// TESTED BY: fmt_lib_mythos_isinstance_c01
	if (strncmp(sig, "LIB\x1A", 4) != 0) return DefinitelyNo;

	// TESTED BY: fmt_lib_mythos_isinstance_c00
	return DefinitelyYes;
}

std::unique_ptr<Archive> ArchiveType_LIB_Mythos::create(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	content->seekp(0, stream::start);
	content->write("LIB\x1A\x00\x00" "\0\0\0\0\0\0\0\0\0\0\0\0\0\x17\x00\x00\x00", 4+2+13+4);
	return std::make_unique<Archive_LIB_Mythos>(std::move(content));
}

std::unique_ptr<Archive> ArchiveType_LIB_Mythos::open(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	return std::make_unique<Archive_LIB_Mythos>(std::move(content));
}

SuppFilenames ArchiveType_LIB_Mythos::getRequiredSupps(stream::input& content,
	const std::string& filenameArchive) const
{
	// No supplemental types/empty list
	return SuppFilenames();
}


Archive_LIB_Mythos::Archive_LIB_Mythos(std::shared_ptr<stream::inout> content)
	:	FATArchive(content, LIB_FIRST_FILE_OFFSET, LIB_MAX_FILENAME_LEN)
{
	this->lenArchive = this->content->size();

	if (this->lenArchive < LIB_FIRST_FILE_OFFSET) {
		throw stream::error("file too short");
	}

	uint16_t numFiles;
	this->content->seekg(4, stream::start);
	*this->content >> u16le(numFiles);

	FATEntry *fatLast = NULL;
	for (unsigned int i = 0; i <= numFiles; i++) {
		if (i >= LIB_SAFETY_MAX_FILECOUNT) {
			throw stream::error("too many files or corrupted archive");
		}
		auto f = this->createNewFATEntry();

		f->iIndex = i;
		f->lenHeader = 0;
		f->type = FILETYPE_GENERIC;
		f->fAttr = 0;
		f->bValid = true;
		*this->content
			>> nullPadded(f->strName, LIB_FILENAME_FIELD_LEN)
			>> u32le(f->iOffset)
		;
		if (fatLast) {
			fatLast->storedSize = f->iOffset - fatLast->iOffset;
			fatLast->realSize = fatLast->storedSize;
		}
		fatLast = &*f;
		if (i != numFiles) { // skip the last spacer entry
			this->vcFAT.push_back(std::move(f));
		}
	}
}

Archive_LIB_Mythos::~Archive_LIB_Mythos()
{
}

void Archive_LIB_Mythos::updateFileName(const FATEntry *pid, const std::string& strNewName)
{
	// TESTED BY: fmt_lib_mythos_rename
	assert(strNewName.length() <= LIB_MAX_FILENAME_LEN);
	this->content->seekp(LIB_FILENAME_OFFSET(pid), stream::start);
	*this->content << nullPadded(strNewName, LIB_FILENAME_FIELD_LEN);
	return;
}

void Archive_LIB_Mythos::updateFileOffset(const FATEntry *pid, stream::delta offDelta)
{
	this->content->seekp(LIB_FILEOFFSET_OFFSET(pid), stream::start);
	*this->content << u32le(pid->iOffset);
	return;
}

void Archive_LIB_Mythos::updateFileSize(const FATEntry *pid, stream::delta sizeDelta)
{
	// Update the last FAT entry (the one that points to EOF.)
	this->updateLastEntry(sizeDelta);
	return;
}

void Archive_LIB_Mythos::preInsertFile(const FATEntry *idBeforeThis, FATEntry *pNewEntry)
{
	// TESTED BY: fmt_lib_mythos_insert*
	assert(pNewEntry->strName.length() <= LIB_MAX_FILENAME_LEN);

	// Set the format-specific variables
	pNewEntry->lenHeader = 0;

	// Because the new entry isn't in the vector yet we need to shift it manually
	pNewEntry->iOffset += LIB_FAT_ENTRY_LEN;

	// Update the last FAT entry (the one that points to EOF.)
	this->updateLastEntry(pNewEntry->storedSize + LIB_FAT_ENTRY_LEN);

	this->content->seekp(LIB_FATENTRY_OFFSET(pNewEntry), stream::start);
	this->content->insert(LIB_FAT_ENTRY_LEN);
	boost::to_upper(pNewEntry->strName);

	*this->content
		<< nullPadded(pNewEntry->strName, LIB_FILENAME_FIELD_LEN)
		<< u32le(pNewEntry->iOffset)
	;

	// Update the offsets now there's a new FAT entry taking up space.
	this->shiftFiles(
		NULL,
		LIB_FAT_OFFSET + (this->vcFAT.size() + 1) * LIB_FAT_ENTRY_LEN,
		LIB_FAT_ENTRY_LEN,
		0
	);

	this->updateFileCount(this->vcFAT.size() + 1);
	return;
}

void Archive_LIB_Mythos::preRemoveFile(const FATEntry *pid)
{
	// TESTED BY: fmt_lib_mythos_remove*

	// Update the last FAT entry (the one that points to EOF.)
	this->updateLastEntry(-((stream::delta)pid->storedSize + LIB_FAT_ENTRY_LEN));

	// Update the offsets now there's one less FAT entry taking up space.  This
	// must be called before the FAT is altered, because it will write a new
	// offset into the FAT entry we're about to erase (and if we erase it first
	// it'll overwrite something else.)
	this->shiftFiles(
		NULL,
		LIB_FAT_OFFSET + (this->vcFAT.size() + 1) * LIB_FAT_ENTRY_LEN,
		-LIB_FAT_ENTRY_LEN,
		0
	);

	this->content->seekp(LIB_FATENTRY_OFFSET(pid), stream::start);
	this->content->remove(LIB_FAT_ENTRY_LEN);

	this->updateFileCount(this->vcFAT.size() - 1);
	return;
}

void Archive_LIB_Mythos::updateLastEntry(stream::delta lenDelta)
{
	assert(this->lenArchive >= 0x17); // smallest valid size (sig+fat terminator)
	this->lenArchive += lenDelta;
	this->content->seekp(LIB_FAT_OFFSET + this->vcFAT.size() * LIB_FAT_ENTRY_LEN
		+ LIB_FILENAME_FIELD_LEN, stream::start);
	*this->content
		<< u32le(this->lenArchive);
	return;
}

void Archive_LIB_Mythos::updateFileCount(uint16_t iNewCount)
{
	// TESTED BY: fmt_lib_mythos_insert*
	// TESTED BY: fmt_lib_mythos_remove*
	this->content->seekp(LIB_FILECOUNT_OFFSET, stream::start);
	*this->content << u16le(iNewCount);
	return;
}

} // namespace gamearchive
} // namespace camoto
