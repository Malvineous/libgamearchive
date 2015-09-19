/**
 * @file  fmt-dat-riptide.cpp
 * @brief Implementation of Dr. Riptide .DAT file reader/writer.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/DAT_Format_%28Dr._Riptide%29
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

#include "fmt-dat-riptide.hpp"

#define DATHH_FILECOUNT_OFFSET    0
#define DATHH_HEADER_LEN          2  // FAT len field
#define DATHH_FAT_OFFSET          DATHH_HEADER_LEN
#define DATHH_FILENAME_FIELD_LEN  13
#define DATHH_MAX_FILENAME_LEN    12
#define DATHH_FAT_ENTRY_LEN       (4+4+4+13)  // u32le size + timestamp + offset + filename
#define DATHH_FIRST_FILE_OFFSET   DATHH_HEADER_LEN

#define DATHH_FATENTRY_OFFSET(e) (DATHH_HEADER_LEN + e->iIndex * DATHH_FAT_ENTRY_LEN)

#define DATHH_FILESIZE_OFFSET(e)    DATHH_FATENTRY_OFFSET(e)
#define DATHH_FILEOFFSET_OFFSET(e) (DATHH_FATENTRY_OFFSET(e) + 8)
#define DATHH_FILENAME_OFFSET(e)   (DATHH_FATENTRY_OFFSET(e) + 12)

namespace camoto {
namespace gamearchive {

ArchiveType_DAT_Riptide::ArchiveType_DAT_Riptide()
{
}

ArchiveType_DAT_Riptide::~ArchiveType_DAT_Riptide()
{
}

std::string ArchiveType_DAT_Riptide::code() const
{
	return "dat-riptide";
}

std::string ArchiveType_DAT_Riptide::friendlyName() const
{
	return "Dr. Riptide DAT Archive";
}

std::vector<std::string> ArchiveType_DAT_Riptide::fileExtensions() const
{
	return {
		"dat",
	};
}

std::vector<std::string> ArchiveType_DAT_Riptide::games() const
{
	return {
		"In Search of Dr. Riptide",
	};
}

ArchiveType::Certainty ArchiveType_DAT_Riptide::isInstance(
	stream::input& content) const
{
	stream::pos lenArchive = content.size();

	// File too short
	// TESTED BY: fmt_dat_riptide_isinstance_c01
	if (lenArchive < DATHH_FIRST_FILE_OFFSET) return DefinitelyNo;

	uint16_t numFiles;
	content.seekg(0, stream::start);
	content >> u16le(numFiles);

	// If the file count is zero, the archive must be only two bytes long
	// TESTED BY: fmt_dat_riptide_isinstance_c02
	if (numFiles == 0) {
		if (lenArchive == 2) return DefinitelyYes;
		return DefinitelyNo;
	}

	// FAT too short
	// TESTED BY: fmt_dat_riptide_isinstance_c03
	unsigned long lenFAT = numFiles * (4+4+4+13);
	if (lenArchive < lenFAT) return DefinitelyNo;

	uint32_t offFile, lenFile;
	char name[13];
	for (unsigned int i = 0; i < numFiles; i++) {
		content >> u32le(lenFile);
		content.seekg(4, stream::cur);
		content >> u32le(offFile);
		content.read(name, 13);

		// Offset past EOF
		// TESTED BY: fmt_dat_riptide_isinstance_c04
		if (offFile + lenFile > lenArchive) return DefinitelyNo;

		// File starts inside FAT
		// TESTED BY: fmt_dat_riptide_isinstance_c05
		if ((offFile != 0) && (offFile < lenFAT + 2u)) return DefinitelyNo;

		// Filename isn't null terminated
		// TESTED BY: fmt_dat_riptide_isinstance_c06
		bool foundNull = false;
		for (int j = 0; j < 13; j++) {
			if (name[j] == 0) {
				foundNull = true;
				break;
			}
		}
		if (!foundNull) return DefinitelyNo;
	}

	// TESTED BY: fmt_dat_riptide_isinstance_c00
	return DefinitelyYes;
}

std::shared_ptr<Archive> ArchiveType_DAT_Riptide::create(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	content->seekp(0, stream::start);
	*content << u16le(0);
	return std::make_shared<Archive_DAT_Riptide>(std::move(content));
}

std::shared_ptr<Archive> ArchiveType_DAT_Riptide::open(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	return std::make_shared<Archive_DAT_Riptide>(std::move(content));
}

SuppFilenames ArchiveType_DAT_Riptide::getRequiredSupps(stream::input& content,
	const std::string& filename) const
{
	return {};
}


Archive_DAT_Riptide::Archive_DAT_Riptide(std::unique_ptr<stream::inout> content)
	:	Archive_FAT(std::move(content), DATHH_FIRST_FILE_OFFSET, DATHH_MAX_FILENAME_LEN)
{
	uint16_t numFiles;
	this->content->seekg(DATHH_FILECOUNT_OFFSET, stream::start);
	*this->content >> u16le(numFiles);

	for (unsigned int i = 0; i < numFiles; i++) {
		auto f = this->createNewFATEntry();

		uint16_t lastModified;
		f->iIndex = i;
		f->lenHeader = 0;
		f->type = FILETYPE_GENERIC;
		f->fAttr = File::Attribute::Default;
		f->bValid = true;
		*this->content
			>> u32le(f->storedSize)
			>> u32le(lastModified)
			>> u32le(f->iOffset)
			>> nullPadded(f->strName, DATHH_FILENAME_FIELD_LEN)
		;
		f->realSize = f->storedSize;

		this->vcFAT.push_back(std::move(f));
	}
}

Archive_DAT_Riptide::~Archive_DAT_Riptide()
{
}

void Archive_DAT_Riptide::updateFileName(const FATEntry *pid, const std::string& strNewName)
{
	// TESTED BY: fmt_dat_riptide_rename
	assert(strNewName.length() <= DATHH_MAX_FILENAME_LEN);
	this->content->seekp(DATHH_FILENAME_OFFSET(pid), stream::start);
	*this->content << nullPadded(strNewName, DATHH_FILENAME_FIELD_LEN);
	return;
}

void Archive_DAT_Riptide::updateFileOffset(const FATEntry *pid, stream::delta offDelta)
{
	// TESTED BY: fmt_dat_riptide_insert*
	// TESTED BY: fmt_dat_riptide_resize*
	this->content->seekp(DATHH_FILEOFFSET_OFFSET(pid), stream::start);
	*this->content << u32le(pid->iOffset);
	return;
}

void Archive_DAT_Riptide::updateFileSize(const FATEntry *pid, stream::delta sizeDelta)
{
	// TESTED BY: fmt_dat_riptide_insert*
	// TESTED BY: fmt_dat_riptide_resize*
	this->content->seekp(DATHH_FILESIZE_OFFSET(pid), stream::start);
	*this->content << u32le(pid->storedSize);
	return;
}

void Archive_DAT_Riptide::preInsertFile(const FATEntry *idBeforeThis, FATEntry *pNewEntry)
{
	// TESTED BY: fmt_dat_riptide_insert*
	assert(pNewEntry->strName.length() <= DATHH_MAX_FILENAME_LEN);

	if (this->vcFAT.size() >= 65535) {
		throw stream::error("Maximum number of files in this archive has been reached.");
	}

	this->content->seekp(DATHH_FATENTRY_OFFSET(pNewEntry), stream::start);
	this->content->insert(DATHH_FAT_ENTRY_LEN);
	boost::to_upper(pNewEntry->strName);

	// Update the offsets now there's a new FAT entry taking up space.
	this->shiftFiles(
		NULL,
		DATHH_FAT_OFFSET + this->vcFAT.size() * DATHH_FAT_ENTRY_LEN,
		DATHH_FAT_ENTRY_LEN,
		0
	);

	// Because the new entry isn't in the vector yet we need to shift it manually
	pNewEntry->iOffset += DATHH_FAT_ENTRY_LEN;

	pNewEntry->lenHeader = 0;

	// Now write all the fields in.  We can't do this earlier like normal, because
	// the calls to shiftFiles() overwrite anything we have written, because this
	// file entry isn't in the FAT vector yet.
	this->content->seekp(DATHH_FATENTRY_OFFSET(pNewEntry), stream::start);
	*this->content
		<< u32le(pNewEntry->storedSize)

#warning TODO: Write last-updated time
		<< u32le(0)

		<< u32le(pNewEntry->iOffset)
		<< nullPadded(pNewEntry->strName, DATHH_FILENAME_FIELD_LEN)
	;

	// Set the format-specific variables
	this->updateFileCount(this->vcFAT.size() + 1);
	return;
}

void Archive_DAT_Riptide::preRemoveFile(const FATEntry *pid)
{
	// TESTED BY: fmt_dat_riptide_remove*

	// Update the offsets now there's one less FAT entry taking up space.  This
	// must be called before the FAT is altered, because it will write a new
	// offset into the FAT entry we're about to erase (and if we erase it first
	// it'll overwrite something else.)
	this->shiftFiles(
		NULL,
		DATHH_FAT_OFFSET + this->vcFAT.size() * DATHH_FAT_ENTRY_LEN,
		-DATHH_FAT_ENTRY_LEN,
		0
	);

	this->content->seekp(DATHH_FATENTRY_OFFSET(pid), stream::start);
	this->content->remove(DATHH_FAT_ENTRY_LEN);

	this->updateFileCount(this->vcFAT.size() - 1);
	return;
}

void Archive_DAT_Riptide::updateFileCount(uint32_t iNewCount)
{
	// TESTED BY: fmt_dat_riptide_insert*
	// TESTED BY: fmt_dat_riptide_remove*
	assert(iNewCount < 65536);
	this->content->seekp(DATHH_FILECOUNT_OFFSET, stream::start);
	*this->content << u16le(iNewCount);
	return;
}

} // namespace gamearchive
} // namespace camoto
