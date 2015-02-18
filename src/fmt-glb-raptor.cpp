/**
 * @file  fmt-glb-raptor.cpp
 * @brief Implementation of Raptor .GLB file reader/writer.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/GLB_Format
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
#include <boost/bind.hpp>
#include <boost/weak_ptr.hpp>
#include <camoto/iostream_helpers.hpp>
#include <camoto/stream_memory.hpp>
#include <camoto/util.hpp>
#include "filter-glb-raptor.hpp"
#include "fmt-glb-raptor.hpp"

#define GLB_FILECOUNT_OFFSET    4
#define GLB_HEADER_LEN          28  // first FAT entry
#define GLB_FAT_OFFSET          GLB_HEADER_LEN
#define GLB_FILENAME_FIELD_LEN  16
#define GLB_MAX_FILENAME_LEN    (GLB_FILENAME_FIELD_LEN-1)
#define GLB_FAT_ENTRY_LEN       28
#define GLB_FIRST_FILE_OFFSET   GLB_FAT_OFFSET  // empty archive only

#define GLB_SAFETY_MAX_FILECOUNT  8192 // Maximum value we will load

#define GLB_FATENTRY_OFFSET(e) (GLB_HEADER_LEN + e->iIndex * GLB_FAT_ENTRY_LEN)

#define GLB_FILENAME_OFFSET(e)   (GLB_FATENTRY_OFFSET(e) + 12)
#define GLB_FILESIZE_OFFSET(e)   (GLB_FATENTRY_OFFSET(e) + 8)
#define GLB_FILEOFFSET_OFFSET(e) (GLB_FATENTRY_OFFSET(e) + 4)

// Uncomment to temporarily disable FAT encryption (for debugging)
//#define GLB_CLEARTEXT

namespace camoto {
namespace gamearchive {

ArchiveType_GLB_Raptor::ArchiveType_GLB_Raptor()
{
}

ArchiveType_GLB_Raptor::~ArchiveType_GLB_Raptor()
{
}

std::string ArchiveType_GLB_Raptor::code() const
{
	return "glb-raptor";
}

std::string ArchiveType_GLB_Raptor::friendlyName() const
{
	return "Raptor GLB File";
}

std::vector<std::string> ArchiveType_GLB_Raptor::fileExtensions() const
{
	std::vector<std::string> vcExtensions;
	vcExtensions.push_back("glb");
	return vcExtensions;
}

std::vector<std::string> ArchiveType_GLB_Raptor::games() const
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Raptor");
	return vcGames;
}

ArchiveType::Certainty ArchiveType_GLB_Raptor::isInstance(
	stream::input& content) const
{
	uint8_t sig[4];
	content.seekg(0, stream::start);
	content.read(sig, 4);
	uint8_t sig_match[] = {0x64, 0x9B, 0xD1, 0x09};
	for (unsigned int i = 0; i < sizeof(sig_match); i++) {
		if (sig[i] != sig_match[i]) {
			// TESTED BY: fmt_glb_raptor_isinstance_c01
			return DefinitelyNo;
		}
	}

	// Don't really need to bother checking offset validity and other things,
	// the signature is good enough.

	// TESTED BY: fmt_glb_raptor_isinstance_c00
	return DefinitelyYes;
}

std::shared_ptr<Archive> ArchiveType_GLB_Raptor::create(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	content->seekp(0, stream::start);
	*content << nullPadded(
#ifdef GLB_CLEARTEXT
		"\x00\x00\x00\x00\x00\x00\x00\x00"
		"\x00\x00\x00\x00\x00\x00\x00\x00"
		"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
#else
		"\x64\x9B\xD1\x09\x50\x9C\xDE\x11"
		"\x43\x7A\xB0\xE8\x2F\x7B\xBD\xF0"
		"\x22\x59\x8F\xC7\x0E\x5A\x9C\xCF\x01\x38\x6E\xA6"
#endif
		, GLB_HEADER_LEN);
	return std::make_shared<Archive_GLB_Raptor>(std::move(content));
}

std::shared_ptr<Archive> ArchiveType_GLB_Raptor::open(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	return std::make_shared<Archive_GLB_Raptor>(std::move(content));
}

SuppFilenames ArchiveType_GLB_Raptor::getRequiredSupps(stream::input& content,
	const std::string& filenameArchive) const
{
	// No supplemental types/empty list
	return SuppFilenames();
}


Archive_GLB_Raptor::Archive_GLB_Raptor(std::unique_ptr<stream::inout> content)
	:	FATArchive(std::move(content), GLB_FIRST_FILE_OFFSET, GLB_MAX_FILENAME_LEN)
{
	FilterType_GLB_Raptor_FAT glbFilterType;
	uint32_t numFiles;
	{
		// Decode just enough of the FAT to get the file count, so we know the size
		// of the FAT
		auto substrFAT = std::make_unique<stream::input_sub>(
			this->content, 0, GLB_HEADER_LEN
		);
#ifdef GLB_CLEARTEXT
		auto preFAT = std::move(substrFAT);
#else
		auto preFAT = glbFilterType.apply(std::move(substrFAT));
#endif
		preFAT->seekg(4, stream::start);
		*preFAT >> u32le(numFiles);
	}

	// Copy the FAT into memory and decode it
	auto substrFAT = std::make_unique<stream::input_sub>(
		this->content, 0, GLB_HEADER_LEN + numFiles * GLB_FAT_ENTRY_LEN
	);
#ifdef GLB_CLEARTEXT
		auto preFAT = std::move(substrFAT);
#else
		auto preFAT = glbFilterType.apply(std::move(substrFAT));
#endif
	auto mem = std::make_unique<stream::memory>();
	stream::copy(*mem, *preFAT);
	this->fat = std::make_unique<stream::seg>(std::move(mem));

	if (numFiles >= GLB_SAFETY_MAX_FILECOUNT) {
		throw stream::error("too many files or corrupted archive");
	}

	this->fat->seekg(GLB_FAT_OFFSET, stream::start);
	for (unsigned int i = 0; i < numFiles; i++) {
		auto f = this->createNewFATEntry();

		f->iIndex = i;
		f->lenHeader = 0;
		f->type = FILETYPE_GENERIC;
		f->fAttr = EA_NONE;
		f->bValid = true;

		uint32_t glbFlags;

		// Read the data in from the FAT entry in the file
		*this->fat
			>> u32le(glbFlags)
			>> u32le(f->iOffset)
			>> u32le(f->storedSize)
			>> nullPadded(f->strName, GLB_FILENAME_FIELD_LEN)
		;
		if (glbFlags == 0x01) {
			f->fAttr = EA_ENCRYPTED;
			f->filter = "glb-raptor";
		}
		f->realSize = f->storedSize;
		this->vcFAT.push_back(std::move(f));
	}
}

Archive_GLB_Raptor::~Archive_GLB_Raptor()
{
}

void Archive_GLB_Raptor::flush()
{
	FilterType_GLB_Raptor_FAT glbFilterType;
	auto substrFAT = std::make_unique<stream::output_sub>(
		this->content, 0,
		GLB_HEADER_LEN + this->vcFAT.size() * GLB_FAT_ENTRY_LEN,
		[](stream::output_sub* sub, stream::len newSize) {
			/// Resize the substream without regard to the underlying data
			/**
			 * This is possible because every time we expand or shrink the FAT, we also make
			 * extra room, so the underlying data is already of the correct size.  We just
			 * have to tell the substream it can now use this extra data.
			 */
			sub->resize(newSize);
			return;
		}
	);
#ifdef GLB_CLEARTEXT
	auto bareCrypt = std::move(substrFAT);
#else
	auto bareCrypt = glbFilterType.apply(
		std::move(substrFAT),
		[](stream::output*, stream::len) {
			// Dummy resize function - don't do anything
			return;
		}
	);
#endif
	this->fat->seekg(0, stream::start);
	bareCrypt->seekp(0, stream::start);
	stream::copy(*bareCrypt, *this->fat);
	bareCrypt->flush();

	this->FATArchive::flush();
	return;
}

void Archive_GLB_Raptor::updateFileName(const FATEntry *pid, const std::string& strNewName)
{
	// TESTED BY: fmt_glb_raptor_rename
	assert(strNewName.length() <= GLB_MAX_FILENAME_LEN);
	this->fat->seekp(GLB_FILENAME_OFFSET(pid), stream::start);
	*this->fat << nullPadded(strNewName, GLB_FILENAME_FIELD_LEN);
	return;
}

void Archive_GLB_Raptor::updateFileOffset(const FATEntry *pid, stream::delta offDelta)
{
	// TESTED BY: fmt_glb_raptor_insert*
	// TESTED BY: fmt_glb_raptor_resize*
	this->fat->seekp(GLB_FILEOFFSET_OFFSET(pid), stream::start);
	*this->fat << u32le(pid->iOffset);
	return;
}

void Archive_GLB_Raptor::updateFileSize(const FATEntry *pid, stream::delta sizeDelta)
{
	// TESTED BY: fmt_glb_raptor_insert*
	// TESTED BY: fmt_glb_raptor_resize*
	this->fat->seekp(GLB_FILESIZE_OFFSET(pid), stream::start);
	*this->fat << u32le(pid->storedSize);
	return;
}

void Archive_GLB_Raptor::preInsertFile(const FATEntry *idBeforeThis, FATEntry *pNewEntry)
{
	// TESTED BY: fmt_glb_raptor_insert*
	assert(pNewEntry->strName.length() <= GLB_MAX_FILENAME_LEN);

	// Set the format-specific variables
	pNewEntry->lenHeader = 0;

	// Because the new entry isn't in the vector yet we need to shift it manually
	pNewEntry->iOffset += GLB_FAT_ENTRY_LEN;

	this->fat->seekp(GLB_FATENTRY_OFFSET(pNewEntry), stream::start);
	this->fat->insert(GLB_FAT_ENTRY_LEN);
	this->content->seekp(GLB_FATENTRY_OFFSET(pNewEntry), stream::start);
	this->content->insert(GLB_FAT_ENTRY_LEN);

	boost::to_upper(pNewEntry->strName);

	uint32_t flags = 0;
	if (pNewEntry->fAttr & EA_COMPRESSED) flags = 1;

	*this->fat
		<< u32le(flags)
		<< u32le(pNewEntry->iOffset)
		<< u32le(pNewEntry->storedSize)
		<< nullPadded(pNewEntry->strName, GLB_FILENAME_FIELD_LEN)
	;

	// Update the offsets now there's a new FAT entry taking up space.
	this->shiftFiles(
		NULL,
		GLB_FAT_OFFSET + this->vcFAT.size() * GLB_FAT_ENTRY_LEN,
		GLB_FAT_ENTRY_LEN,
		0
	);

	this->updateFileCount(this->vcFAT.size() + 1);
	return;
}

void Archive_GLB_Raptor::preRemoveFile(const FATEntry *pid)
{
	// TESTED BY: fmt_glb_raptor_remove*

	// Update the offsets now there's one less FAT entry taking up space.  This
	// must be called before the FAT is altered, because it will write a new
	// offset into the FAT entry we're about to erase (and if we erase it first
	// it'll overwrite something else.)
	this->shiftFiles(
		NULL,
		GLB_FAT_OFFSET + this->vcFAT.size() * GLB_FAT_ENTRY_LEN,
		-GLB_FAT_ENTRY_LEN,
		0
	);

	this->fat->seekp(GLB_FATENTRY_OFFSET(pid), stream::start);
	this->fat->remove(GLB_FAT_ENTRY_LEN);

	this->content->seekp(GLB_FATENTRY_OFFSET(pid), stream::start);
	this->content->remove(GLB_FAT_ENTRY_LEN);

	this->updateFileCount(this->vcFAT.size() - 1);
	return;
}

void Archive_GLB_Raptor::updateFileCount(uint32_t iNewCount)
{
	// TESTED BY: fmt_glb_raptor_insert*
	// TESTED BY: fmt_glb_raptor_remove*
	this->fat->seekp(GLB_FILECOUNT_OFFSET, stream::start);
	*this->fat << u32le(iNewCount);
	return;
}

} // namespace gamearchive
} // namespace camoto
