/**
 * @file   fmt-pod-tv.cpp
 * @brief  Terminal Velocity .POD file reader/writer.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/POD_Format
 *
 * Copyright (C) 2010-2011 Adam Nielsen <malvineous@shikadi.net>
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

PODType::PODType()
	throw ()
{
}

PODType::~PODType()
	throw ()
{
}

std::string PODType::getArchiveCode() const
	throw ()
{
	return "pod-tv";
}

std::string PODType::getFriendlyName() const
	throw ()
{
	return "Terminal Velocity POD File";
}

std::vector<std::string> PODType::getFileExtensions() const
	throw ()
{
	std::vector<std::string> vcExtensions;
	vcExtensions.push_back("pod");
	return vcExtensions;
}

std::vector<std::string> PODType::getGameList() const
	throw ()
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Terminal Velocity");
	return vcGames;
}

ArchiveType::Certainty PODType::isInstance(stream::input_sptr psArchive) const
	throw (stream::error)
{
	stream::pos lenArchive = psArchive->size();

	// Must have filecount + description
	if (lenArchive < 84) return DefinitelyNo;

	psArchive->seekg(0, stream::start);
	uint32_t numFiles;
	psArchive >> u32le(numFiles);

	char description[POD_DESCRIPTION_LEN + 1];
	psArchive->read(description, POD_DESCRIPTION_LEN);
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
	psArchive->seekg(POD_FAT_OFFSET, stream::start);
	for (unsigned int i = 0; i < numFiles; i++) {
		psArchive->read(fn, POD_MAX_FILENAME_LEN);
		// Make sure there aren't any invalid characters in the filename
		for (int j = 0; j < POD_MAX_FILENAME_LEN; j++) {
			if (!fn[j]) break; // stop on terminating null

			// Fail on control characters in the filename
			if (fn[j] < 32) return DefinitelyNo; // TESTED BY: fmt_pod_tv_isinstance_c01
		}

		uint32_t offEntry, lenEntry;
		psArchive >> u32le(offEntry) >> u32le(lenEntry);

		// If a file entry points past the end of the archive then it's an invalid
		// format.
		// TESTED BY: fmt_pod_tv_isinstance_c0[23]
		if (offEntry + lenEntry > lenArchive) return DefinitelyNo;
	}

	// If we've made it this far, this is almost certainly a POD file.
	// TESTED BY: fmt_pod_tv_isinstance_c00
	return DefinitelyYes;
}

ArchivePtr PODType::newArchive(stream::inout_sptr psArchive, SuppData& suppData) const
	throw (stream::error)
{
	psArchive->seekp(0, stream::start);
	psArchive
		<< u32le(0) // File count
		<< nullPadded("Empty POD file", POD_DESCRIPTION_LEN);
	return ArchivePtr(new PODArchive(psArchive));
}

ArchivePtr PODType::open(stream::inout_sptr psArchive, SuppData& suppData) const
	throw (stream::error)
{
	return ArchivePtr(new PODArchive(psArchive));
}

SuppFilenames PODType::getRequiredSupps(stream::input_sptr data,
	const std::string& filenameArchive) const
	throw ()
{
	// No supplemental types/empty list
	return SuppFilenames();
}


PODArchive::PODArchive(stream::inout_sptr psArchive)
	throw (stream::error) :
		FATArchive(psArchive, POD_FIRST_FILE_OFFSET, POD_MAX_FILENAME_LEN)
{
	this->psArchive->seekg(0, stream::start);
	uint32_t numFiles;
	this->psArchive >> u32le(numFiles);
	this->vcFAT.reserve(numFiles);

	this->psArchive->seekg(POD_FAT_OFFSET, stream::start);

	for (unsigned int i = 0; i < numFiles; i++) {
		FATEntry *pEntry = new FATEntry();
		pEntry->iIndex = i;
		this->psArchive
			>> nullPadded(pEntry->strName, POD_MAX_FILENAME_LEN)
			>> u32le(pEntry->iSize)
			>> u32le(pEntry->iOffset)
		;
		pEntry->lenHeader = 0;
		pEntry->type = FILETYPE_GENERIC;
		pEntry->fAttr = 0;
		pEntry->bValid = true;
		this->vcFAT.push_back(EntryPtr(pEntry));
	}
}

PODArchive::~PODArchive()
	throw ()
{
}

PODArchive::MetadataTypes PODArchive::getMetadataList() const
	throw ()
{
	// TESTED BY: fmt_pod_tv_get_metadata_description
	MetadataTypes m;
	m.push_back(Description);
	return m;
}

std::string PODArchive::getMetadata(MetadataType item) const
	throw (stream::error)
{
	// TESTED BY: fmt_pod_tv_get_metadata_description
	switch (item) {
		case Description: {
			// TODO: see whether description can span two file entries (80 bytes) or
			// whether the offsets have to be null
			psArchive->seekg(POD_DESCRIPTION_OFFSET, stream::start);
			char description[POD_DESCRIPTION_LEN + 1];
			psArchive->read(description, POD_DESCRIPTION_LEN);
			description[POD_DESCRIPTION_LEN] = 0;
			return std::string(description);
		}
		default:
			assert(false);
			throw stream::error("unsupported metadata item");
	}
}

void PODArchive::setMetadata(MetadataType item, const std::string& value)
	throw (stream::error)
{
	// TESTED BY: fmt_pod_tv_set_metadata_description
	// TESTED BY: fmt_pod_tv_new_to_initialstate
	switch (item) {
		case Description:
			if (value.length() > POD_DESCRIPTION_LEN) {
				throw stream::error("description too long");
			}
			this->psArchive->seekp(POD_DESCRIPTION_OFFSET, stream::start);
			this->psArchive << nullPadded(value, POD_DESCRIPTION_LEN);
			break;
		default:
			assert(false);
			throw stream::error("unsupported metadata item");
	}
	return;
}

void PODArchive::updateFileName(const FATEntry *pid, const std::string& strNewName)
	throw (stream::error)
{
	// TESTED BY: fmt_pod_tv_rename
	assert(strNewName.length() <= POD_MAX_FILENAME_LEN);
	this->psArchive->seekp(POD_FILENAME_OFFSET(pid), stream::start);
	this->psArchive << nullPadded(strNewName, POD_MAX_FILENAME_LEN);
	return;
}

void PODArchive::updateFileOffset(const FATEntry *pid, stream::delta offDelta)
	throw (stream::error)
{
	// TESTED BY: fmt_pod_tv_insert*
	// TESTED BY: fmt_pod_tv_resize*
	this->psArchive->seekp(POD_FILEOFFSET_OFFSET(pid), stream::start);
	this->psArchive << u32le(pid->iOffset);
	return;
}

void PODArchive::updateFileSize(const FATEntry *pid, stream::delta sizeDelta)
	throw (stream::error)
{
	// TESTED BY: fmt_pod_tv_insert*
	// TESTED BY: fmt_pod_tv_resize*
	this->psArchive->seekp(POD_FILESIZE_OFFSET(pid), stream::start);
	this->psArchive << u32le(pid->iSize);
	return;
}

FATArchive::FATEntry *PODArchive::preInsertFile(const FATEntry *idBeforeThis, FATEntry *pNewEntry)
	throw (stream::error)
{
	// TESTED BY: fmt_pod_tv_insert*
	assert(pNewEntry->strName.length() <= POD_MAX_FILENAME_LEN);

	// Set the format-specific variables
	pNewEntry->lenHeader = 0;

	// Because the new entry isn't in the vector yet we need to shift it manually
	pNewEntry->iOffset += POD_FAT_ENTRY_LEN;

	this->psArchive->seekp(POD_FATENTRY_OFFSET(pNewEntry), stream::start);
	this->psArchive->insert(POD_FAT_ENTRY_LEN);
	boost::to_upper(pNewEntry->strName);

	// Write out the entry
	this->psArchive
		<< nullPadded(pNewEntry->strName, POD_MAX_FILENAME_LEN)
		<< u32le(pNewEntry->iSize)
		<< u32le(pNewEntry->iOffset);

	// Update the offsets now there's a new FAT entry taking up space.
	this->shiftFiles(
		NULL,
		POD_FAT_OFFSET + this->vcFAT.size() * POD_FAT_ENTRY_LEN,
		POD_FAT_ENTRY_LEN,
		0
	);

	this->updateFileCount(this->vcFAT.size() + 1);

	return pNewEntry;
}

void PODArchive::preRemoveFile(const FATEntry *pid)
	throw (stream::error)
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
	this->psArchive->seekp(POD_FATENTRY_OFFSET(pid), stream::start);
	this->psArchive->remove(POD_FAT_ENTRY_LEN);

	this->updateFileCount(this->vcFAT.size() - 1);

	return;
}

void PODArchive::updateFileCount(uint32_t iNewCount)
	throw (stream::error)
{
	// TESTED BY: fmt_pod_tv_insert*
	// TESTED BY: fmt_pod_tv_remove*
	this->psArchive->seekp(0, stream::start);
	this->psArchive << u32le(iNewCount);
	return;
}

} // namespace gamearchive
} // namespace camoto
