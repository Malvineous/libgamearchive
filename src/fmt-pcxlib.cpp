/**
 * @file  fmt-pcxlib.cpp
 * @brief Implementation of PCX Library reader/writer.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/PCX_Format
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

#include "fmt-pcxlib.hpp"

#define PCX_MAX_FILES         65535
#define PCX_FAT_OFFSET        (2+50+42+2+32)
#define PCX_FILECOUNT_OFFSET  (2+50+42)
#define PCX_FAT_ENTRY_LEN     (1+13+4+4+2+2)
#define PCX_MAX_FILENAME_LEN  12
#define PCX_FIRST_FILE_OFFSET PCX_FAT_OFFSET

#define PCX_FATENTRY_OFFSET(e)   (PCX_FAT_OFFSET + (e)->iIndex * PCX_FAT_ENTRY_LEN)
#define PCX_FILENAME_OFFSET(e)   (PCX_FATENTRY_OFFSET(e) + 1)
#define PCX_FILEOFFSET_OFFSET(e) (PCX_FATENTRY_OFFSET(e) + 14)
#define PCX_FILESIZE_OFFSET(e)   (PCX_FATENTRY_OFFSET(e) + 18)

namespace camoto {
namespace gamearchive {

ArchiveType_PCXLib::ArchiveType_PCXLib()
{
}

ArchiveType_PCXLib::~ArchiveType_PCXLib()
{
}

std::string ArchiveType_PCXLib::code() const
{
	return "pcxlib";
}

std::string ArchiveType_PCXLib::friendlyName() const
{
	return "PCX Library (v2)";
}

std::vector<std::string> ArchiveType_PCXLib::fileExtensions() const
{
	return {
		"pcl",
	};
}

std::vector<std::string> ArchiveType_PCXLib::games() const
{
	return {
		"Word Rescue",
	};
}

ArchiveType::Certainty ArchiveType_PCXLib::isInstance(
	stream::input& content) const
{
	stream::pos lenArchive = content.size();
	// File too short to hold header
	// TESTED BY: fmt_pcxlib_isinstance_c01
	if (lenArchive < PCX_FAT_OFFSET) return DefinitelyNo;

	content.seekg(0, stream::start);
	uint32_t version;
	content >> u16le(version);

	// Only accept known versions
	// TESTED BY: fmt_pcxlib_isinstance_c02
	if (version != 0xCA01) return DefinitelyNo;

	content.seekg(PCX_FILECOUNT_OFFSET, stream::start);
	uint32_t numFiles;
	content >> u16le(numFiles);

	// File too short to hold FAT
	// TESTED BY: fmt_pcxlib_isinstance_c03
	if (lenArchive < PCX_FAT_OFFSET + numFiles * PCX_FAT_ENTRY_LEN) {
		return DefinitelyNo;
	}

	content.seekg(PCX_FAT_OFFSET, stream::start);
	for (unsigned int i = 0; i < numFiles; i++) {
		uint8_t sync;
		std::string name, ext;
		uint16_t date, time;
		uint32_t offset, size;
		content
			>> u8(sync)
			>> nullPadded(name, 8)
			>> nullPadded(ext, 5)
			>> u32le(offset)
			>> u32le(size)
			>> u16le(date)
			>> u16le(time)
		;

		// No/invalid sync byte
		// TESTED BY: fmt_pcxlib_isinstance_c04
		if (sync != 0x00) return DefinitelyNo;

		// Bad filename
		// TESTED BY: fmt_pcxlib_isinstance_c05
		if (ext[0] != '.') return DefinitelyNo;

		// File inside FAT
		// TESTED BY: fmt_pcxlib_isinstance_c06
		if (offset <= PCX_FAT_OFFSET + PCX_FAT_ENTRY_LEN) return DefinitelyNo;

		// Truncated file
		// TESTED BY: fmt_pcxlib_isinstance_c07
		if (offset + size > lenArchive) return DefinitelyNo;
	}

	// TESTED BY: fmt_pcxlib_isinstance_c00
	return DefinitelyYes;
}

std::shared_ptr<Archive> ArchiveType_PCXLib::create(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	content->seekp(0, stream::start);
	content->write(
		"\x01\xCA" "Copyright (c) Genus Microprogramming, Inc. 1988-90"
		"\x00\x00\x00\x00\x00\x00\x00\x00"
		"\x00\x00\x00\x00\x00\x00\x00\x00"
		"\x00\x00\x00\x00\x00\x00\x00\x00"
		"\x00\x00\x00\x00\x00\x00\x00\x00"
		"\x00\x00\x00\x00\x00\x00\x00\x00"
		"\x00\x00"
		"\x00\x00" // file count
		"\x00\x00\x00\x00\x00\x00\x00\x00"
		"\x00\x00\x00\x00\x00\x00\x00\x00"
		"\x00\x00\x00\x00\x00\x00\x00\x00"
		"\x00\x00\x00\x00\x00\x00\x00\x00",
		128);

	return std::make_shared<Archive_PCXLib>(std::move(content));
}

std::shared_ptr<Archive> ArchiveType_PCXLib::open(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	return std::make_shared<Archive_PCXLib>(std::move(content));
}

SuppFilenames ArchiveType_PCXLib::getRequiredSupps(stream::input& content,
	const std::string& filename) const
{
	return {};
}


Archive_PCXLib::Archive_PCXLib(std::unique_ptr<stream::inout> content)
	:	Archive_FAT(std::move(content), PCX_FIRST_FILE_OFFSET, PCX_MAX_FILENAME_LEN)
{
	stream::pos lenArchive = this->content->size();

	if (lenArchive < PCX_FAT_OFFSET) {
		throw stream::error("Truncated file");
	}

	this->content->seekg(PCX_FILECOUNT_OFFSET, stream::start);
	uint16_t numFiles;
	*this->content >> u16le(numFiles);
	this->vcFAT.reserve(numFiles);

	// Skip over remaining header
	this->content->seekg(32, stream::cur);

	for (int i = 0; i < numFiles; i++) {
		auto f = this->createNewFATEntry();

		uint8_t sync;
		std::string name, ext;
		uint16_t date, time;
		*this->content
			>> u8(sync)
			>> nullPadded(name, 8)
			>> nullPadded(ext, 5)
			>> u32le(f->iOffset)
			>> u32le(f->storedSize)
			>> u16le(date)
			>> u16le(time)
		;
		f->strName = name.substr(0, name.find_first_of(' ')) + ext.substr(0, ext.find_first_of(' '));

		f->iIndex = i;
		f->lenHeader = 0;
		f->type = FILETYPE_GENERIC;
		f->fAttr = File::Attribute::Default;
		f->bValid = true;
		f->realSize = f->storedSize;
		this->vcFAT.push_back(std::move(f));
	}
}

Archive_PCXLib::~Archive_PCXLib()
{
}

void Archive_PCXLib::updateFileName(const FATEntry *pid, const std::string& strNewName)
{
	// TESTED BY: fmt_pcxlib_rename
	assert(strNewName.length() <= PCX_MAX_FILENAME_LEN);
	this->content->seekp(PCX_FILENAME_OFFSET(pid), stream::start);
	int pos = strNewName.find_last_of('.');
	std::string name = strNewName.substr(0, pos);
	while (name.length() < 8) name += ' ';
	std::string ext = strNewName.substr(pos);
	if (ext.length() > 4) {
		throw stream::error("Filename extension too long - three letters max.");
	}
	while (ext.length() < 4) ext += ' ';
	*this->content
		<< nullPadded(name, 8)
		<< nullPadded(ext, 5)
	;
	return;
}

void Archive_PCXLib::updateFileOffset(const FATEntry *pid, stream::delta offDelta)
{
	// TESTED BY: fmt_pcxlib_insert*
	// TESTED BY: fmt_pcxlib_resize*
	this->content->seekp(PCX_FILEOFFSET_OFFSET(pid), stream::start);
	*this->content << u32le(pid->iOffset);
	return;
}

void Archive_PCXLib::updateFileSize(const FATEntry *pid, stream::delta sizeDelta)
{
	// TESTED BY: fmt_pcxlib_insert*
	// TESTED BY: fmt_pcxlib_resize*
	this->content->seekp(PCX_FILESIZE_OFFSET(pid), stream::start);
	*this->content << u32le(pid->storedSize);
	return;
}

void Archive_PCXLib::preInsertFile(const FATEntry *idBeforeThis,
	FATEntry *pNewEntry)
{
	// TESTED BY: fmt_pcxlib_insert*
	assert(pNewEntry->strName.length() <= PCX_MAX_FILENAME_LEN);

	// Set the format-specific variables
	pNewEntry->lenHeader = 0;

	// Because the new entry isn't in the vector yet we need to shift it manually
	pNewEntry->iOffset += PCX_FAT_ENTRY_LEN;

	if (this->vcFAT.size() >= PCX_MAX_FILES) {
		throw stream::error("too many files, maximum is " TOSTRING(PCX_MAX_FILES));
	}
	this->content->seekp(PCX_FATENTRY_OFFSET(pNewEntry), stream::start);
	this->content->insert(PCX_FAT_ENTRY_LEN);
	boost::to_upper(pNewEntry->strName);

	int pos = pNewEntry->strName.find_last_of('.');
	std::string name = pNewEntry->strName.substr(0, pos);
	while (name.length() < 8) name += ' ';
	std::string ext = pNewEntry->strName.substr(pos);
	if (ext.length() > 4) {
		throw stream::error("Filename extension too long - three letters max.");
	}
	while (ext.length() < 4) ext += ' ';

	uint16_t date = 0, time = 0; // TODO

	// Write out the entry
	*this->content
		<< u8(0) // sync byte
		<< nullPadded(name, 8)
		<< nullPadded(ext, 5)
		<< u32le(pNewEntry->iOffset)
		<< u32le(pNewEntry->storedSize)
		<< u16le(date)
		<< u16le(time)
	;

	// Update the offsets now there's a new FAT entry taking up space.
	this->shiftFiles(
		NULL,
		PCX_FAT_OFFSET + this->vcFAT.size() * PCX_FAT_ENTRY_LEN,
		PCX_FAT_ENTRY_LEN,
		0
	);

	this->updateFileCount(this->vcFAT.size() + 1);

	return;
}

void Archive_PCXLib::preRemoveFile(const FATEntry *pid)
{
	// TESTED BY: fmt_pcxlib_remove*

	// Update the offsets now there's one less FAT entry taking up space.  This
	// must be called before the FAT is altered, because it will write a new
	// offset into the FAT entry we're about to erase (and if we erase it first
	// it'll overwrite something else.)
	this->shiftFiles(
		NULL,
		PCX_FAT_OFFSET + this->vcFAT.size() * PCX_FAT_ENTRY_LEN,
		-PCX_FAT_ENTRY_LEN,
		0
	);

	// Remove the FAT entry
	this->content->seekp(PCX_FATENTRY_OFFSET(pid), stream::start);
	this->content->remove(PCX_FAT_ENTRY_LEN);

	this->updateFileCount(this->vcFAT.size() - 1);

	return;
}

void Archive_PCXLib::updateFileCount(uint32_t iNewCount)
{
	// TESTED BY: fmt_pcxlib_insert*
	// TESTED BY: fmt_pcxlib_remove*
	this->content->seekp(PCX_FILECOUNT_OFFSET, stream::start);
	*this->content << u32le(iNewCount);
	return;
}

} // namespace gamearchive
} // namespace camoto
