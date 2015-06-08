/**
 * @file  fmt-pod-tv.cpp
 * @brief Terminal Velocity .POD file reader/writer.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/POD_Format
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

#include "fmt-pod-tv.hpp"

#define POD_DESCRIPTION_OFFSET    4
#define POD_DESCRIPTION_LEN       80
#define POD_FAT_OFFSET            84
#define POD_FAT_ENTRY_LEN         40  // filename + u32le offset + u32le size
#define POD_MAX_FILENAME_LEN      32
#define POD_FIRST_FILE_OFFSET     POD_FAT_OFFSET

#define POD_FATENTRY_OFFSET(e)   (POD_FAT_OFFSET + (e)->iIndex * POD_FAT_ENTRY_LEN)
#define POD_FILENAME_OFFSET(e)    POD_FATENTRY_OFFSET(e)
#define POD_FILESIZE_OFFSET(e)   (POD_FATENTRY_OFFSET(e) + POD_MAX_FILENAME_LEN)
#define POD_FILEOFFSET_OFFSET(e) (POD_FILESIZE_OFFSET(e) + 4)

namespace camoto {
namespace gamearchive {

ArchiveType_POD_TV::ArchiveType_POD_TV()
{
}

ArchiveType_POD_TV::~ArchiveType_POD_TV()
{
}

std::string ArchiveType_POD_TV::code() const
{
	return "pod-tv";
}

std::string ArchiveType_POD_TV::friendlyName() const
{
	return "Terminal Velocity POD File";
}

std::vector<std::string> ArchiveType_POD_TV::fileExtensions() const
{
	std::vector<std::string> vcExtensions;
	vcExtensions.push_back("pod");
	return vcExtensions;
}

std::vector<std::string> ArchiveType_POD_TV::games() const
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Terminal Velocity");
	return vcGames;
}

ArchiveType::Certainty ArchiveType_POD_TV::isInstance(
	stream::input& content) const
{
	stream::pos lenArchive = content.size();

	// Must have filecount + description
	if (lenArchive < POD_FAT_OFFSET) return DefinitelyNo;

	content.seekg(0, stream::start);
	uint32_t numFiles;
	content >> u32le(numFiles);

	char description[POD_DESCRIPTION_LEN + 1];
	content.read(description, POD_DESCRIPTION_LEN);
	description[POD_DESCRIPTION_LEN] = 0;

	for (int j = 0; j < POD_DESCRIPTION_LEN; j++) {
		// Fail on control characters in the description
		if ((description[j]) && (description[j] < 32)) {
			return DefinitelyNo; // TESTED BY: fmt_pod_tv_isinstance_c04
		}
	}

	// Make sure the FAT fits inside the archive
	if (POD_FAT_OFFSET + numFiles * POD_FAT_ENTRY_LEN > lenArchive) {
		return DefinitelyNo;
	}

	// Check each FAT entry
	char fn[POD_MAX_FILENAME_LEN];
	content.seekg(POD_FAT_OFFSET, stream::start);
	for (unsigned int i = 0; i < numFiles; i++) {
		content.read(fn, POD_MAX_FILENAME_LEN);
		// Make sure there aren't any invalid characters in the filename
		for (int j = 0; j < POD_MAX_FILENAME_LEN; j++) {
			if (!fn[j]) break; // stop on terminating null

			// Fail on control characters in the filename
			if (fn[j] < 32) return DefinitelyNo; // TESTED BY: fmt_pod_tv_isinstance_c01
		}

		uint32_t offEntry, lenEntry;
		content >> u32le(offEntry) >> u32le(lenEntry);

		// If a file entry points past the end of the archive then it's an invalid
		// format.
		// TESTED BY: fmt_pod_tv_isinstance_c0[23]
		if (offEntry + lenEntry > lenArchive) return DefinitelyNo;
	}

	// If we've made it this far, this is almost certainly a POD file.
	// TESTED BY: fmt_pod_tv_isinstance_c00
	return DefinitelyYes;
}

std::shared_ptr<Archive> ArchiveType_POD_TV::create(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	content->seekp(0, stream::start);
	*content
		<< u32le(0) // File count
		<< nullPadded("Empty POD file", POD_DESCRIPTION_LEN);
	return std::make_shared<Archive_POD_TV>(std::move(content));
}

std::shared_ptr<Archive> ArchiveType_POD_TV::open(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	return std::make_shared<Archive_POD_TV>(std::move(content));
}

SuppFilenames ArchiveType_POD_TV::getRequiredSupps(stream::input& content,
	const std::string& filename) const
{
	// No supplemental types/empty list
	return SuppFilenames();
}


Archive_POD_TV::Archive_POD_TV(std::unique_ptr<stream::inout> content)
	:	Archive_FAT(std::move(content), POD_FIRST_FILE_OFFSET, POD_MAX_FILENAME_LEN)
{
	this->content->seekg(0, stream::start);
	uint32_t numFiles;
	*this->content >> u32le(numFiles);
	this->vcFAT.reserve(numFiles);

	this->content->seekg(POD_FAT_OFFSET, stream::start);

	for (unsigned int i = 0; i < numFiles; i++) {
		auto f = this->createNewFATEntry();
		f->iIndex = i;
		*this->content
			>> nullPadded(f->strName, POD_MAX_FILENAME_LEN)
			>> u32le(f->storedSize)
			>> u32le(f->iOffset)
		;
		f->lenHeader = 0;
		f->type = FILETYPE_GENERIC;
		f->fAttr = File::Attribute::Default;
		f->bValid = true;
		f->realSize = f->storedSize;
		this->vcFAT.push_back(std::move(f));
	}
}

Archive_POD_TV::~Archive_POD_TV()
{
}

Archive_POD_TV::MetadataTypes Archive_POD_TV::getMetadataList() const
{
	// TESTED BY: fmt_pod_tv_get_metadata_description
	MetadataTypes m;
	m.push_back(Description);
	return m;
}

std::string Archive_POD_TV::getMetadata(MetadataType item) const
{
	// TESTED BY: fmt_pod_tv_get_metadata_description
	switch (item) {
		case Description: {
			// TODO: see whether description can span two file entries (80 bytes) or
			// whether the offsets have to be null
			this->content->seekg(POD_DESCRIPTION_OFFSET, stream::start);
			char description[POD_DESCRIPTION_LEN + 1];
			this->content->read(description, POD_DESCRIPTION_LEN);
			description[POD_DESCRIPTION_LEN] = 0;
			return std::string(description);
		}
		default:
			assert(false);
			throw stream::error("unsupported metadata item");
	}
}

void Archive_POD_TV::setMetadata(MetadataType item, const std::string& value)
{
	// TESTED BY: fmt_pod_tv_set_metadata_description
	// TESTED BY: fmt_pod_tv_new_to_initialstate
	switch (item) {
		case Description:
			if (value.length() > POD_DESCRIPTION_LEN) {
				throw stream::error("description too long");
			}
			this->content->seekp(POD_DESCRIPTION_OFFSET, stream::start);
			*this->content << nullPadded(value, POD_DESCRIPTION_LEN);
			break;
		default:
			assert(false);
			throw stream::error("unsupported metadata item");
	}
	return;
}

void Archive_POD_TV::updateFileName(const FATEntry *pid, const std::string& strNewName)
{
	// TESTED BY: fmt_pod_tv_rename
	assert(strNewName.length() <= POD_MAX_FILENAME_LEN);
	this->content->seekp(POD_FILENAME_OFFSET(pid), stream::start);
	*this->content << nullPadded(strNewName, POD_MAX_FILENAME_LEN);
	return;
}

void Archive_POD_TV::updateFileOffset(const FATEntry *pid, stream::delta offDelta)
{
	// TESTED BY: fmt_pod_tv_insert*
	// TESTED BY: fmt_pod_tv_resize*
	this->content->seekp(POD_FILEOFFSET_OFFSET(pid), stream::start);
	*this->content << u32le(pid->iOffset);
	return;
}

void Archive_POD_TV::updateFileSize(const FATEntry *pid, stream::delta sizeDelta)
{
	// TESTED BY: fmt_pod_tv_insert*
	// TESTED BY: fmt_pod_tv_resize*
	this->content->seekp(POD_FILESIZE_OFFSET(pid), stream::start);
	*this->content << u32le(pid->storedSize);
	return;
}

void Archive_POD_TV::preInsertFile(const FATEntry *idBeforeThis, FATEntry *pNewEntry)
{
	// TESTED BY: fmt_pod_tv_insert*
	assert(pNewEntry->strName.length() <= POD_MAX_FILENAME_LEN);

	// Set the format-specific variables
	pNewEntry->lenHeader = 0;

	// Because the new entry isn't in the vector yet we need to shift it manually
	pNewEntry->iOffset += POD_FAT_ENTRY_LEN;

	this->content->seekp(POD_FATENTRY_OFFSET(pNewEntry), stream::start);
	this->content->insert(POD_FAT_ENTRY_LEN);
	boost::to_upper(pNewEntry->strName);

	// Write out the entry
	*this->content
		<< nullPadded(pNewEntry->strName, POD_MAX_FILENAME_LEN)
		<< u32le(pNewEntry->storedSize)
		<< u32le(pNewEntry->iOffset);

	// Update the offsets now there's a new FAT entry taking up space.
	this->shiftFiles(
		NULL,
		POD_FAT_OFFSET + this->vcFAT.size() * POD_FAT_ENTRY_LEN,
		POD_FAT_ENTRY_LEN,
		0
	);

	this->updateFileCount(this->vcFAT.size() + 1);

	return;
}

void Archive_POD_TV::preRemoveFile(const FATEntry *pid)
{
	// TESTED BY: fmt_pod_tv_remove*

	// Update the offsets now there's one less FAT entry taking up space.  This
	// must be called before the FAT is altered, because it will write a new
	// offset into the FAT entry we're about to erase (and if we erase it first
	// it'll overwrite something else.)
	this->shiftFiles(
		NULL,
		POD_FAT_OFFSET + this->vcFAT.size() * POD_FAT_ENTRY_LEN,
		-POD_FAT_ENTRY_LEN,
		0
	);

	// Remove the FAT entry
	this->content->seekp(POD_FATENTRY_OFFSET(pid), stream::start);
	this->content->remove(POD_FAT_ENTRY_LEN);

	this->updateFileCount(this->vcFAT.size() - 1);

	return;
}

void Archive_POD_TV::updateFileCount(uint32_t iNewCount)
{
	// TESTED BY: fmt_pod_tv_insert*
	// TESTED BY: fmt_pod_tv_remove*
	this->content->seekp(0, stream::start);
	*this->content << u32le(iNewCount);
	return;
}

} // namespace gamearchive
} // namespace camoto
