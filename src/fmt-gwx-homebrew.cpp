/**
 * @file  fmt-gwx-homebrew.cpp
 * @brief HomeBrew File Folder format.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/HomeBrew_File_Folder_Format
 *
 * Copyright (C) 2010-2016 Adam Nielsen <malvineous@shikadi.net>
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

#include "fmt-gwx-homebrew.hpp"

#define GWx_FAT_OFFSET            0x40
#define GWx_FAT_ENTRY_LEN         0x20  // filename + u32le offset + u32le size
#define GWx_MAX_FILENAME_LEN      12
#define GWx_FIRST_FILE_OFFSET     GWx_FAT_OFFSET

#define GWx_FATENTRY_OFFSET(e)   (GWx_FAT_OFFSET + (e)->iIndex * GWx_FAT_ENTRY_LEN)
#define GWx_FILENAME_OFFSET(e)    GWx_FATENTRY_OFFSET(e)
#define GWx_FILEOFFSET_OFFSET(e) (GWx_FATENTRY_OFFSET(e) + GWx_MAX_FILENAME_LEN + 4)
#define GWx_FILESIZE_OFFSET(e)   (GWx_FILEOFFSET_OFFSET(e) + 4)

namespace camoto {
namespace gamearchive {

ArchiveType_GWx_HomeBrew::ArchiveType_GWx_HomeBrew()
{
}

ArchiveType_GWx_HomeBrew::~ArchiveType_GWx_HomeBrew()
{
}

std::string ArchiveType_GWx_HomeBrew::code() const
{
	return "gwx-homebrew";
}

std::string ArchiveType_GWx_HomeBrew::friendlyName() const
{
	return "HomeBrew File Folder";
}

std::vector<std::string> ArchiveType_GWx_HomeBrew::fileExtensions() const
{
	return {
		"gw1",
		"gw2",
		"gw3",
	};
}

std::vector<std::string> ArchiveType_GWx_HomeBrew::games() const
{
	return {
		"Gateworld",
	};
}

ArchiveType::Certainty ArchiveType_GWx_HomeBrew::isInstance(
	stream::input& content) const
{
	stream::pos lenArchive = content.size();

	// Must have signature + file count
	// TESTED BY: fmt_gwx_homebrew_isinstance_c01
	if (lenArchive < GWx_FAT_OFFSET) return Certainty::DefinitelyNo;

	content.seekg(0, stream::start);
	std::string sig;
	content >> nullTerminated(sig, 21);

	// Validate signature
	// TESTED BY: fmt_gwx_homebrew_isinstance_c02
	if (sig.compare("HomeBrew File Folder\x1A") != 0) return Certainty::DefinitelyNo;

	// TESTED BY: fmt_gwx_homebrew_isinstance_c00
	return Certainty::DefinitelyYes;
}

std::shared_ptr<Archive> ArchiveType_GWx_HomeBrew::create(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	content->seekp(0, stream::start);
	*content
		<< nullPadded("HomeBrew File Folder\x1A", 32)
		<< u16le(0x100) // Version?
		<< u32le(0) // file count
		<< nullPadded("", 32 - 2 - 4)
	;
	return std::make_shared<Archive_GWx_HomeBrew>(std::move(content));
}

std::shared_ptr<Archive> ArchiveType_GWx_HomeBrew::open(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	return std::make_shared<Archive_GWx_HomeBrew>(std::move(content));
}

SuppFilenames ArchiveType_GWx_HomeBrew::getRequiredSupps(stream::input& content,
	const std::string& filename) const
{
	return {};
}


Archive_GWx_HomeBrew::Archive_GWx_HomeBrew(std::unique_ptr<stream::inout> content)
	:	Archive_FAT(std::move(content), GWx_FIRST_FILE_OFFSET, GWx_MAX_FILENAME_LEN)
{
	this->content->seekg(0x22, stream::start);
	uint32_t numFiles;
	*this->content >> u32le(numFiles);
	this->vcFAT.reserve(numFiles);

	this->content->seekg(GWx_FAT_OFFSET, stream::start);

	for (unsigned int i = 0; i < numFiles; i++) {
		auto f = this->createNewFATEntry();
		f->iIndex = i;
		*this->content
			>> nullPadded(f->strName, GWx_MAX_FILENAME_LEN)
		;
		this->content->seekg(4, stream::cur);
		*this->content
			>> u32le(f->iOffset)
			>> u32le(f->storedSize)
		;
		this->content->seekg(8, stream::cur);
		f->lenHeader = 0;
		f->type = FILETYPE_GENERIC;
		f->fAttr = File::Attribute::Default;
		f->bValid = true;
		f->realSize = f->storedSize;
		this->vcFAT.push_back(std::move(f));
	}
}

Archive_GWx_HomeBrew::~Archive_GWx_HomeBrew()
{
}

void Archive_GWx_HomeBrew::updateFileName(const FATEntry *pid, const std::string& strNewName)
{
	// TESTED BY: fmt_gwx_homebrew_rename
	assert(strNewName.length() <= GWx_MAX_FILENAME_LEN);
	this->content->seekp(GWx_FILENAME_OFFSET(pid), stream::start);
	*this->content << nullPadded(strNewName, GWx_MAX_FILENAME_LEN);
	return;
}

void Archive_GWx_HomeBrew::updateFileOffset(const FATEntry *pid, stream::delta offDelta)
{
	// TESTED BY: fmt_gwx_homebrew_insert*
	// TESTED BY: fmt_gwx_homebrew_resize*
	this->content->seekp(GWx_FILEOFFSET_OFFSET(pid), stream::start);
	*this->content << u32le(pid->iOffset);
	return;
}

void Archive_GWx_HomeBrew::updateFileSize(const FATEntry *pid, stream::delta sizeDelta)
{
	// TESTED BY: fmt_gwx_homebrew_insert*
	// TESTED BY: fmt_gwx_homebrew_resize*
	this->content->seekp(GWx_FILESIZE_OFFSET(pid), stream::start);
	*this->content << u32le(pid->storedSize);
	return;
}

void Archive_GWx_HomeBrew::preInsertFile(const FATEntry *idBeforeThis, FATEntry *pNewEntry)
{
	// TESTED BY: fmt_gwx_homebrew_insert*
	assert(pNewEntry->strName.length() <= GWx_MAX_FILENAME_LEN);

	// Set the format-specific variables
	pNewEntry->lenHeader = 0;

	// Because the new entry isn't in the vector yet we need to shift it manually
	pNewEntry->iOffset += GWx_FAT_ENTRY_LEN;

	this->content->seekp(GWx_FATENTRY_OFFSET(pNewEntry), stream::start);
	this->content->insert(GWx_FAT_ENTRY_LEN);
	boost::to_upper(pNewEntry->strName);

	// Write out the entry
	*this->content
		<< nullPadded(pNewEntry->strName, GWx_MAX_FILENAME_LEN)
		<< u32le(0) // padding
		<< u32le(pNewEntry->iOffset)
		<< u32le(pNewEntry->storedSize)
		<< u32le(0) // padding
		<< u32le(0) // padding
	;

	// Update the offsets now there's a new FAT entry taking up space.
	this->shiftFiles(
		NULL,
		GWx_FAT_OFFSET + this->vcFAT.size() * GWx_FAT_ENTRY_LEN,
		GWx_FAT_ENTRY_LEN,
		0
	);

	this->updateFileCount(this->vcFAT.size() + 1);

	return;
}

void Archive_GWx_HomeBrew::preRemoveFile(const FATEntry *pid)
{
	// TESTED BY: fmt_gwx_homebrew_remove*

	// Update the offsets now there's one less FAT entry taking up space.  This
	// must be called before the FAT is altered, because it will write a new
	// offset into the FAT entry we're about to erase (and if we erase it first
	// it'll overwrite something else.)
	this->shiftFiles(
		NULL,
		GWx_FAT_OFFSET + this->vcFAT.size() * GWx_FAT_ENTRY_LEN,
		-GWx_FAT_ENTRY_LEN,
		0
	);

	// Remove the FAT entry
	this->content->seekp(GWx_FATENTRY_OFFSET(pid), stream::start);
	this->content->remove(GWx_FAT_ENTRY_LEN);

	this->updateFileCount(this->vcFAT.size() - 1);

	return;
}

void Archive_GWx_HomeBrew::updateFileCount(uint32_t newCount)
{
	// TESTED BY: fmt_gwx_homebrew_insert*
	// TESTED BY: fmt_gwx_homebrew_remove*
	this->content->seekp(0x22, stream::start);
	*this->content << u32le(newCount);
	return;
}

} // namespace gamearchive
} // namespace camoto
