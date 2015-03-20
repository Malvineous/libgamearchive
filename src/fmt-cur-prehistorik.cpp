/**
 * @file  fmt-cur-prehistorik.cpp
 * @brief Prehistorik .CUR/.VGA format.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/CUR_Format
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

#include "fmt-cur-prehistorik.hpp"

#define CUR_FATLEN_OFFSET       0
#define CUR_HEADER_LEN          6  // u16le file count + uint32le=0 terminator
#define CUR_FAT_OFFSET          2  // u16le file count
#define CUR_MAX_FILENAME_LEN    32 // Just because
#define CUR_FIRST_FILE_OFFSET   CUR_HEADER_LEN  // empty archive only

namespace camoto {
namespace gamearchive {

ArchiveType_CUR_Prehistorik::ArchiveType_CUR_Prehistorik()
{
}

ArchiveType_CUR_Prehistorik::~ArchiveType_CUR_Prehistorik()
{
}

std::string ArchiveType_CUR_Prehistorik::code() const
{
	return "cur-prehistorik";
}

std::string ArchiveType_CUR_Prehistorik::friendlyName() const
{
	return "Prehistorik CUR/VGA Archive";
}

std::vector<std::string> ArchiveType_CUR_Prehistorik::fileExtensions() const
{
	std::vector<std::string> vcExtensions;
	vcExtensions.push_back("cur");
	vcExtensions.push_back("vga");
	return vcExtensions;
}

std::vector<std::string> ArchiveType_CUR_Prehistorik::games() const
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Prehistorik");
	return vcGames;
}

ArchiveType::Certainty ArchiveType_CUR_Prehistorik::isInstance(
	stream::input& content) const
{
	stream::pos lenArchive = content.size();

	// File too short
	// TESTED BY: fmt_cur_prehistorik_isinstance_c01
	if (lenArchive < CUR_HEADER_LEN) return DefinitelyNo;

	uint16_t offEndFAT;
	content.seekg(0, stream::start);
	content >> u16le(offEndFAT);
	// FAT is too short
	// TESTED BY: fmt_cur_prehistorik_isinstance_c02
	if (offEndFAT < CUR_HEADER_LEN) return DefinitelyNo;
	// FAT ends past EOF
	// TESTED BY: fmt_cur_prehistorik_isinstance_c03
	if (offEndFAT > lenArchive) return DefinitelyNo;
	stream::pos offNext = offEndFAT;
	for (unsigned int i = 0; offEndFAT >= 4; i++) {
		uint32_t storedSize;
		content >> u32le(storedSize);
		offEndFAT -= 4;
		if (storedSize == 0) break;

		std::string filename;
		content >> nullTerminated(filename, CUR_MAX_FILENAME_LEN);
		unsigned int lenFilename = filename.length() + 1;
		// Filename too long
		// TESTED BY: fmt_cur_prehistorik_isinstance_c04
		if (lenFilename >= CUR_MAX_FILENAME_LEN) return DefinitelyNo;

		for (auto i : filename) {
			// Control char in filename
			// TESTED BY: fmt_cur_prehistorik_isinstance_c05
			if ((i < 32) || (i == 127)) return DefinitelyNo;
		}

		// FAT ends mid-filename
		// TESTED BY: fmt_cur_prehistorik_isinstance_c06
		if (offEndFAT < lenFilename) return DefinitelyNo;
		offEndFAT -= lenFilename;

		offNext += storedSize;
		// File goes past archive EOF
		// TESTED BY: fmt_cur_prehistorik_isinstance_c07
		if (offNext > lenArchive) return DefinitelyNo;
	}

	// Last file doesn't end at archive EOF
	// TESTED BY: fmt_cur_prehistorik_isinstance_c08
	if (offNext != lenArchive) return DefinitelyNo;

	// TESTED BY: fmt_cur_prehistorik_isinstance_c00
	return DefinitelyYes;
}

std::shared_ptr<Archive> ArchiveType_CUR_Prehistorik::create(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	content->seekp(0, stream::start);
	content->write("\x06\x00\0\0\0\0", 6);
	return std::make_shared<Archive_CUR_Prehistorik>(std::move(content));
}

std::shared_ptr<Archive> ArchiveType_CUR_Prehistorik::open(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	return std::make_shared<Archive_CUR_Prehistorik>(std::move(content));
}

SuppFilenames ArchiveType_CUR_Prehistorik::getRequiredSupps(stream::input& data,
	const std::string& filenameArchive) const
{
	// No supplemental types/empty list
	return SuppFilenames();
}


Archive_CUR_Prehistorik::Archive_CUR_Prehistorik(std::unique_ptr<stream::inout> content)
	:	Archive_FAT(std::move(content), CUR_FIRST_FILE_OFFSET, CUR_MAX_FILENAME_LEN)
{
	this->content->seekg(CUR_FATLEN_OFFSET, stream::start);

	uint16_t offEndFAT;
	*this->content >> u16le(offEndFAT);
	stream::pos offNext = offEndFAT;
	offEndFAT -= 2;

	for (unsigned int i = 0; offEndFAT >= 4; i++) {
		uint32_t storedSize;
		*this->content >> u32le(storedSize);
		offEndFAT -= 4;
#warning TODO: Does a file with 0 bytes terminate the FAT, or will the game read files past that?
		if (storedSize == 0) break;
		auto f = std::make_unique<FATEntry>();

		f->iIndex = i;
		f->iOffset = offNext;
		f->lenHeader = 0;
		f->type = FILETYPE_GENERIC;
		f->fAttr = EA_NONE;
		f->bValid = true;
		f->storedSize = storedSize;

		// Read the data in from the FAT entry in the file
		*this->content >> nullTerminated(f->strName, CUR_MAX_FILENAME_LEN);
		unsigned int lenFilename = f->strName.length() + 1;
		if (offEndFAT < lenFilename) break;
		offEndFAT -= lenFilename;

		f->realSize = f->storedSize;
		offNext += f->storedSize;
		this->vcFAT.push_back(std::move(f));
	}
}

Archive_CUR_Prehistorik::~Archive_CUR_Prehistorik()
{
}

void Archive_CUR_Prehistorik::updateFileName(const FATEntry *pid,
	const std::string& strNewName)
{
	// TESTED BY: fmt_cur_prehistorik_rename
	assert(strNewName.length() <= CUR_MAX_FILENAME_LEN);
	unsigned int lenEntry = 4 + pid->strName.length() + 1;
	unsigned int lenNewEntry = 4 + strNewName.length() + 1;
	auto offEntry = 4 + this->fatOffset(pid);
	this->content->seekp(offEntry, stream::start);
	bool resized = false;
	if (lenNewEntry < lenEntry) {
		this->content->remove(lenEntry - lenNewEntry);
		resized = true;
	} else if (lenNewEntry > lenEntry) {
		this->content->insert(lenNewEntry - lenEntry);
		resized = true;
	}
	*this->content << nullTerminated(strNewName, CUR_MAX_FILENAME_LEN);
	if (resized) this->updateFATLength(lenNewEntry - lenEntry);
	return;
}

void Archive_CUR_Prehistorik::updateFileOffset(const FATEntry *pid,
	stream::delta offDelta)
{
	// This format doesn't have any offsets that need updating.  As this function
	// is only called when removing a file, the "offsets" will be sorted out
	// when the FAT entry is removed later.
	return;
}

void Archive_CUR_Prehistorik::updateFileSize(const FATEntry *pid,
	stream::delta sizeDelta)
{
	// TESTED BY: fmt_cur_prehistorik_insert*
	// TESTED BY: fmt_cur_prehistorik_resize*
	auto offEntry = this->fatOffset(pid);
	this->content->seekp(offEntry, stream::start);
	*this->content << u32le(pid->storedSize);
	return;
}

void Archive_CUR_Prehistorik::preInsertFile(const FATEntry *idBeforeThis,
	FATEntry *pNewEntry)
{
	// TESTED BY: fmt_cur_prehistorik_insert*
	assert(pNewEntry->strName.length() <= CUR_MAX_FILENAME_LEN);

	unsigned int lenEntry = 4 + pNewEntry->strName.length() + 1;
	auto offEntry = this->fatOffset(idBeforeThis);

	// Set the format-specific variables
	pNewEntry->lenHeader = 0;

	// Because the new entry isn't in the vector yet we need to shift it manually
	pNewEntry->iOffset += lenEntry;

	this->content->seekp(offEntry, stream::start);
	this->content->insert(lenEntry);
	boost::to_upper(pNewEntry->strName);

	*this->content
		<< u32le(pNewEntry->storedSize)
		<< nullTerminated(pNewEntry->strName, CUR_MAX_FILENAME_LEN);

	// Update the offsets now there's a new FAT entry taking up space.
	this->shiftFiles(
		NULL,
		offEntry,
		lenEntry,
		0
	);

	return;
}

void Archive_CUR_Prehistorik::postInsertFile(FATEntry *pid)
{
	this->updateFATLength();
	return;
}

void Archive_CUR_Prehistorik::preRemoveFile(const FATEntry *pid)
{
	// TESTED BY: fmt_cur_prehistorik_remove*

	int lenEntry = 4 + pid->strName.length() + 1;
	auto offEntry = this->fatOffset(pid);

	// Update the offsets now there's one less FAT entry taking up space.  This
	// must be called before the FAT is altered, because it will write a new
	// offset into the FAT entry we're about to erase (and if we erase it first
	// it'll overwrite something else.)
	this->shiftFiles(
		NULL,
		offEntry,
		-lenEntry,
		0
	);

	this->content->seekp(offEntry, stream::start);
	this->content->remove(lenEntry);

	return;
}

void Archive_CUR_Prehistorik::postRemoveFile(const FATEntry *pid)
{
	this->updateFATLength();
	return;
}

void Archive_CUR_Prehistorik::updateFATLength(int extra)
{
	// TESTED BY: fmt_cur_prehistorik_insert*
	// TESTED BY: fmt_cur_prehistorik_remove*
	unsigned int lenFAT = 2 + 4;
	for (auto& i : this->vcFAT) {
		lenFAT += 4 + i->strName.length() + 1;
	}
	lenFAT += extra; // may be negative, so add last
	this->content->seekp(CUR_FATLEN_OFFSET, stream::start);
	*this->content << u16le(lenFAT);
	return;
}

stream::pos Archive_CUR_Prehistorik::fatOffset(const FATEntry *pid)
{
	stream::pos lenFAT = 2;
	bool valid = pid && (pid->bValid);
	for (auto& i : this->vcFAT) {
		auto fatEntry = dynamic_cast<const FATEntry *>(i.get());
		if (valid && (pid->iIndex == fatEntry->iIndex)) return lenFAT;
		lenFAT += 4 + i->strName.length() + 1;
	}
	if (!valid) return lenFAT; // insert at end of FAT
	throw stream::error("Unable to find FAT entry in vector");
}

} // namespace gamearchive
} // namespace camoto
