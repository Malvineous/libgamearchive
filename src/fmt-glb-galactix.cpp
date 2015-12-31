/**
 * @file  fmt-glb-galactix.cpp
 * @brief Implementation of Galactix .GLB file reader/writer.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/GLB_Format_%28Galactix%29
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
#include "fmt-glb-galactix.hpp"

#define GLB_FILECOUNT_OFFSET    0
#define GLB_HEADER_LEN          28  // first FAT entry
#define GLB_FAT_OFFSET          GLB_HEADER_LEN
#define GLB_FILENAME_FIELD_LEN  22
#define GLB_MAX_FILENAME_LEN    (GLB_FILENAME_FIELD_LEN-1)
#define GLB_FAT_ENTRY_LEN       28
#define GLB_FIRST_FILE_OFFSET   GLB_FAT_OFFSET  // empty archive only

#define GLB_SAFETY_MAX_FILECOUNT  8192 // Maximum value we will load

#define GLB_FATENTRY_OFFSET(e) (GLB_HEADER_LEN + e->iIndex * GLB_FAT_ENTRY_LEN)

#define GLB_FILENAME_OFFSET(e)   (GLB_FATENTRY_OFFSET(e) + 4)
#define GLB_FILESIZE_OFFSET(e)   (GLB_FATENTRY_OFFSET(e) + 26)
#define GLB_FILEOFFSET_OFFSET(e) (GLB_FATENTRY_OFFSET(e) + 0)

namespace camoto {
namespace gamearchive {

ArchiveType_GLB_Galactix::ArchiveType_GLB_Galactix()
{
}

ArchiveType_GLB_Galactix::~ArchiveType_GLB_Galactix()
{
}

std::string ArchiveType_GLB_Galactix::code() const
{
	return "glb-galactix";
}

std::string ArchiveType_GLB_Galactix::friendlyName() const
{
	return "Galactix GLB File";
}

std::vector<std::string> ArchiveType_GLB_Galactix::fileExtensions() const
{
	return {
		"glb",
	};
}

std::vector<std::string> ArchiveType_GLB_Galactix::games() const
{
	return {
		"Galactix",
	};
}

ArchiveType::Certainty ArchiveType_GLB_Galactix::isInstance(
	stream::input& content) const
{
	std::string sig;
	content.seekg(4, stream::start);
	content >> nullTerminated(sig, 22);
	if (sig.compare("GLIB FILE") != 0) return DefinitelyNo;

	// Don't really need to bother checking offset validity and other things,
	// the signature is good enough.

	// TESTED BY: fmt_glb_galactix_isinstance_c00
	return DefinitelyYes;
}

std::shared_ptr<Archive> ArchiveType_GLB_Galactix::create(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	content->seekp(0, stream::start);
	*content << nullPadded(std::string(
		"\x00\x00\x00\x00"
		"GLIB FILE\x00\x00\x00"
		"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
		"\x00\x00", GLB_HEADER_LEN)
		, GLB_HEADER_LEN);
	return std::make_shared<Archive_GLB_Galactix>(std::move(content));
}

std::shared_ptr<Archive> ArchiveType_GLB_Galactix::open(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	return std::make_shared<Archive_GLB_Galactix>(std::move(content));
}

SuppFilenames ArchiveType_GLB_Galactix::getRequiredSupps(stream::input& content,
	const std::string& filename) const
{
	return {};
}


Archive_GLB_Galactix::Archive_GLB_Galactix(std::unique_ptr<stream::inout> content)
	:	Archive_FAT(std::move(content), GLB_FIRST_FILE_OFFSET, GLB_MAX_FILENAME_LEN)
{
	uint32_t numFiles;
	this->content->seekg(0, stream::start);
	*this->content >> u32le(numFiles);

	if (numFiles >= GLB_SAFETY_MAX_FILECOUNT) {
		throw stream::error("too many files or corrupted archive");
	}

	this->content->seekg(GLB_FAT_OFFSET, stream::start);
	for (unsigned int i = 0; i < numFiles; i++) {
		auto f = this->createNewFATEntry();

		f->iIndex = i;
		f->lenHeader = 0;
		f->type = FILETYPE_GENERIC;
		f->fAttr = File::Attribute::Default;
		f->bValid = true;

		// Read the data in from the FAT entry in the file
		*this->content
			>> u32le(f->iOffset)
			>> nullPadded(f->strName, GLB_FILENAME_FIELD_LEN)
			>> u16le(f->storedSize)
		;
		f->realSize = f->storedSize;
		this->vcFAT.push_back(std::move(f));
	}
}

Archive_GLB_Galactix::~Archive_GLB_Galactix()
{
}

void Archive_GLB_Galactix::updateFileName(const FATEntry *pid, const std::string& strNewName)
{
	// TESTED BY: fmt_glb_galactix_rename
	assert(strNewName.length() <= GLB_MAX_FILENAME_LEN);
	this->content->seekp(GLB_FILENAME_OFFSET(pid), stream::start);
	*this->content << nullPadded(strNewName, GLB_FILENAME_FIELD_LEN);
	return;
}

void Archive_GLB_Galactix::updateFileOffset(const FATEntry *pid, stream::delta offDelta)
{
	// TESTED BY: fmt_glb_galactix_insert*
	// TESTED BY: fmt_glb_galactix_resize*
	this->content->seekp(GLB_FILEOFFSET_OFFSET(pid), stream::start);
	*this->content << u32le(pid->iOffset);
	return;
}

void Archive_GLB_Galactix::updateFileSize(const FATEntry *pid, stream::delta sizeDelta)
{
	// TESTED BY: fmt_glb_galactix_insert*
	// TESTED BY: fmt_glb_galactix_resize*
	this->content->seekp(GLB_FILESIZE_OFFSET(pid), stream::start);
	*this->content << u16le(pid->storedSize);
	return;
}

void Archive_GLB_Galactix::preInsertFile(const FATEntry *idBeforeThis, FATEntry *pNewEntry)
{
	// TESTED BY: fmt_glb_galactix_insert*
	assert(pNewEntry->strName.length() <= GLB_MAX_FILENAME_LEN);

	// Set the format-specific variables
	pNewEntry->lenHeader = 0;

	// Because the new entry isn't in the vector yet we need to shift it manually
	pNewEntry->iOffset += GLB_FAT_ENTRY_LEN;

	this->content->seekp(GLB_FATENTRY_OFFSET(pNewEntry), stream::start);
	this->content->insert(GLB_FAT_ENTRY_LEN);

	boost::to_upper(pNewEntry->strName);

	*this->content
		<< u32le(pNewEntry->iOffset)
		<< nullPadded(pNewEntry->strName, GLB_FILENAME_FIELD_LEN)
		<< u16le(pNewEntry->storedSize)
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

void Archive_GLB_Galactix::preRemoveFile(const FATEntry *pid)
{
	// TESTED BY: fmt_glb_galactix_remove*

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

	this->content->seekp(GLB_FATENTRY_OFFSET(pid), stream::start);
	this->content->remove(GLB_FAT_ENTRY_LEN);

	this->updateFileCount(this->vcFAT.size() - 1);
	return;
}

void Archive_GLB_Galactix::updateFileCount(uint32_t iNewCount)
{
	// TESTED BY: fmt_glb_galactix_insert*
	// TESTED BY: fmt_glb_galactix_remove*
	this->content->seekp(GLB_FILECOUNT_OFFSET, stream::start);
	*this->content << u32le(iNewCount);
	return;
}

} // namespace gamearchive
} // namespace camoto
