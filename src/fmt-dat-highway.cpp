/**
 * @file   fmt-dat-highway.cpp
 * @brief  Implementation of Highway Hunder .DAT file reader/writer.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/DAT_Format_%28Highway_Hunter%29
 *
 * Copyright (C) 2010-2014 Adam Nielsen <malvineous@shikadi.net>
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

#include "fmt-dat-highway.hpp"

#define DATHH_FATLEN_OFFSET       0
#define DATHH_HEADER_LEN          2  // FAT len field
#define DATHH_FAT_OFFSET          DATHH_HEADER_LEN
#define DATHH_FILENAME_FIELD_LEN  13
#define DATHH_MAX_FILENAME_LEN    12
#define DATHH_FAT_ENTRY_LEN       17  // u32le offset + filename
#define DATHH_FIRST_FILE_OFFSET   (DATHH_HEADER_LEN + DATHH_FAT_ENTRY_LEN)  // empty archive only

#define DATHH_EFAT_ENTRY_LEN      4 // u32le len

#define DATHH_FATENTRY_OFFSET(e) (DATHH_HEADER_LEN + e->iIndex * DATHH_FAT_ENTRY_LEN)

#define DATHH_FILEOFFSET_OFFSET(e)  DATHH_FATENTRY_OFFSET(e)
#define DATHH_FILENAME_OFFSET(e)   (DATHH_FATENTRY_OFFSET(e) + 4)
#define DATHH_FILESIZE_OFFSET(e)   ((e)->iOffset + 0)

namespace camoto {
namespace gamearchive {

DAT_HighwayType::DAT_HighwayType()
{
}

DAT_HighwayType::~DAT_HighwayType()
{
}

std::string DAT_HighwayType::getArchiveCode() const
{
	return "dat-highway";
}

std::string DAT_HighwayType::getFriendlyName() const
{
	return "Highway Hunter DAT Archive";
}

std::vector<std::string> DAT_HighwayType::getFileExtensions() const
{
	std::vector<std::string> vcExtensions;
	vcExtensions.push_back("dat");
	return vcExtensions;
}

std::vector<std::string> DAT_HighwayType::getGameList() const
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Highway Hunter");
	return vcGames;
}

ArchiveType::Certainty DAT_HighwayType::isInstance(stream::input_sptr psArchive) const
{
	stream::pos lenArchive = psArchive->size();

	// File too short
	// TESTED BY: fmt_dat_highway_isinstance_c01
	if (lenArchive < DATHH_FIRST_FILE_OFFSET) return DefinitelyNo;

	uint16_t lenFAT;
	psArchive->seekg(0, stream::start);
	psArchive >> u16le(lenFAT);

	// FAT is not a multiple of the FAT entry length
	// TESTED BY: fmt_dat_highway_isinstance_c02
	if (lenFAT % DATHH_FAT_ENTRY_LEN) return DefinitelyNo;

	// FAT length too small to hold final null entry
	// TESTED BY: fmt_dat_highway_isinstance_c07
	if (lenFAT < DATHH_FAT_ENTRY_LEN) return DefinitelyNo;

	unsigned int numFiles = lenFAT / DATHH_FAT_ENTRY_LEN;
	uint32_t offFile;
	for (unsigned int i = 0; i < numFiles; i++) {
		uint8_t c;
		psArchive >> u32le(offFile);
		psArchive->seekg(DATHH_FILENAME_FIELD_LEN - 1, stream::cur);
		psArchive >> u8(c);

		// Offset past EOF
		// TESTED BY: fmt_dat_highway_isinstance_c03
		if (offFile > lenArchive) return DefinitelyNo;

		// File starts inside FAT
		// TESTED BY: fmt_dat_highway_isinstance_c04
		if ((offFile != 0) && (offFile < lenFAT + 2u)) return DefinitelyNo;

		// Filename isn't null terminated
		// TESTED BY: fmt_dat_highway_isinstance_c05
		if (c != 0) return DefinitelyNo;
	}

	// Final file must be empty
	// TESTED BY: fmt_dat_highway_isinstance_c06
	if (offFile != 0) return DefinitelyNo;

	// TESTED BY: fmt_dat_highway_isinstance_c00
	return DefinitelyYes;
}

ArchivePtr DAT_HighwayType::newArchive(stream::inout_sptr psArchive, SuppData& suppData) const
{
	psArchive->seekp(0, stream::start);
	psArchive->write("\x11\x00" "\x00\x00\x00\x00" "\0\0\0\0\0\0\0\0\0\0\0\0\0", 2+4+13);
	return ArchivePtr(new DAT_HighwayArchive(psArchive));
}

ArchivePtr DAT_HighwayType::open(stream::inout_sptr psArchive, SuppData& suppData) const
{
	return ArchivePtr(new DAT_HighwayArchive(psArchive));
}

SuppFilenames DAT_HighwayType::getRequiredSupps(stream::input_sptr data,
	const std::string& filenameArchive) const
{
	// No supplemental types/empty list
	return SuppFilenames();
}


DAT_HighwayArchive::DAT_HighwayArchive(stream::inout_sptr psArchive)
	:	FATArchive(psArchive, DATHH_FIRST_FILE_OFFSET, DATHH_MAX_FILENAME_LEN)
{
	uint16_t lenFAT;
	this->psArchive->seekg(DATHH_FATLEN_OFFSET, stream::start);
	this->psArchive >> u16le(lenFAT);

	unsigned int numFiles = (lenFAT / DATHH_FAT_ENTRY_LEN) - 1;
	FATEntry *lastFATEntry = NULL;
	for (unsigned int i = 0; i < numFiles; i++) {
		this->psArchive->seekg(DATHH_HEADER_LEN + i * DATHH_FAT_ENTRY_LEN, stream::start);
		FATEntry *fatEntry = new FATEntry();
		EntryPtr ep(fatEntry);

		fatEntry->iIndex = i;
		fatEntry->lenHeader = DATHH_EFAT_ENTRY_LEN;;
		fatEntry->type = FILETYPE_GENERIC;
		fatEntry->fAttr = 0;
		fatEntry->bValid = true;
		this->psArchive
			>> u32le(fatEntry->iOffset)
			>> nullPadded(fatEntry->strName, DATHH_FILENAME_FIELD_LEN)
		;
		this->psArchive->seekg(fatEntry->iOffset, stream::start);
		this->psArchive
			>> u32le(fatEntry->realSize)
		;
		if (lastFATEntry) {
			lastFATEntry->storedSize = fatEntry->iOffset - lastFATEntry->iOffset - DATHH_EFAT_ENTRY_LEN;
		}
		lastFATEntry = fatEntry;

		this->vcFAT.push_back(ep);
	}
	if (lastFATEntry) {
		stream::pos lenArchive = this->psArchive->size();
		lastFATEntry->storedSize = lenArchive - lastFATEntry->iOffset - DATHH_EFAT_ENTRY_LEN;
	}
}

DAT_HighwayArchive::~DAT_HighwayArchive()
{
}

void DAT_HighwayArchive::updateFileName(const FATEntry *pid, const std::string& strNewName)
{
	// TESTED BY: fmt_dat_highway_rename
	assert(strNewName.length() <= DATHH_MAX_FILENAME_LEN);
	this->psArchive->seekp(DATHH_FILENAME_OFFSET(pid), stream::start);
	this->psArchive << nullPadded(strNewName, DATHH_FILENAME_FIELD_LEN);
	return;
}

void DAT_HighwayArchive::updateFileOffset(const FATEntry *pid, stream::delta offDelta)
{
	// TESTED BY: fmt_dat_highway_insert*
	// TESTED BY: fmt_dat_highway_resize*
	this->psArchive->seekp(DATHH_FILEOFFSET_OFFSET(pid), stream::start);
	this->psArchive << u32le(pid->iOffset);
	return;
}

void DAT_HighwayArchive::updateFileSize(const FATEntry *pid, stream::delta sizeDelta)
{
	// This format doesn't have any sizes that need updating.
	// TESTED BY: fmt_dat_highway_insert*
	// TESTED BY: fmt_dat_highway_resize*
	this->psArchive->seekp(DATHH_FILESIZE_OFFSET(pid), stream::start);
	this->psArchive << u32le(pid->realSize);
	return;
}

FATArchive::FATEntry *DAT_HighwayArchive::preInsertFile(const FATEntry *idBeforeThis, FATEntry *pNewEntry)
{
	// TESTED BY: fmt_dat_highway_insert*
	assert(pNewEntry->strName.length() <= DATHH_MAX_FILENAME_LEN);

	if (this->vcFAT.size() >= 65535 / DATHH_FAT_ENTRY_LEN) {
		throw stream::error("Maximum number of files in this archive has been reached.");
	}

	this->psArchive->seekp(DATHH_FATENTRY_OFFSET(pNewEntry), stream::start);
	this->psArchive->insert(DATHH_FAT_ENTRY_LEN);
	boost::to_lower(pNewEntry->strName);

	// Update the offsets now there's a new FAT entry taking up space.
	this->shiftFiles(
		NULL,
		DATHH_FAT_OFFSET + this->vcFAT.size() * DATHH_FAT_ENTRY_LEN,
		DATHH_FAT_ENTRY_LEN,
		0
	);

	// Because the new entry isn't in the vector yet we need to shift it manually
	pNewEntry->iOffset += DATHH_FAT_ENTRY_LEN;

	pNewEntry->lenHeader = DATHH_EFAT_ENTRY_LEN;
	this->psArchive->seekp(pNewEntry->iOffset, stream::start);
	this->psArchive->insert(DATHH_EFAT_ENTRY_LEN);

	// Since we've inserted some data for the embedded header, we need to update
	// the other file offsets accordingly.  This call updates the offset of the
	// files, then calls updateFileOffset() on them, using the *new* offset, so
	// we need to do this after the insert() call above to make sure the extra
	// data has been inserted.  Then when updateFileOffset() writes data out it
	// will go into the correct spot.
	this->shiftFiles(NULL, pNewEntry->iOffset, pNewEntry->lenHeader, 0);

	// Now write all the fields in.  We can't do this earlier like normal, because
	// the calls to shiftFiles() overwrite anything we have written, because this
	// file entry isn't in the FAT vector yet.
	this->psArchive->seekp(DATHH_FATENTRY_OFFSET(pNewEntry), stream::start);
	this->psArchive
		<< u32le(pNewEntry->iOffset)
		<< nullPadded(pNewEntry->strName, DATHH_FILENAME_FIELD_LEN)
	;

	this->psArchive->seekp(pNewEntry->iOffset, stream::start);
	this->psArchive
		<< u32le(pNewEntry->realSize)
	;

	// Set the format-specific variables
	this->updateFileCount(this->vcFAT.size() + 1);
	return pNewEntry;
}

void DAT_HighwayArchive::preRemoveFile(const FATEntry *pid)
{
	// TESTED BY: fmt_dat_highway_remove*

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

	this->psArchive->seekp(DATHH_FATENTRY_OFFSET(pid), stream::start);
	this->psArchive->remove(DATHH_FAT_ENTRY_LEN);

	this->updateFileCount(this->vcFAT.size() - 1);
	return;
}

void DAT_HighwayArchive::updateFileCount(uint32_t iNewCount)
{
	// TESTED BY: fmt_dat_highway_insert*
	// TESTED BY: fmt_dat_highway_remove*
	uint32_t lenFAT = (iNewCount + 1) * DATHH_FAT_ENTRY_LEN;
	assert(lenFAT < 65536);
	this->psArchive->seekp(DATHH_FATLEN_OFFSET, stream::start);
	this->psArchive << u16le(lenFAT);
	return;
}

} // namespace gamearchive
} // namespace camoto
