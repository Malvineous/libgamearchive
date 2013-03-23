/**
 * @file   fmt-dlt-stargunner.cpp
 * @brief  Implementation of Star Gunner .DLT file reader/writer.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/DLT_Format
 *
 * Copyright (C) 2010-2013 Adam Nielsen <malvineous@shikadi.net>
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

#include "fmt-dlt-stargunner.hpp"

#define DLT_FILECOUNT_OFFSET    6
#define DLT_HEADER_LEN          8  // "DAVE" header + u16le unk + u16le file count
#define DLT_FAT_OFFSET          DLT_HEADER_LEN
#define DLT_FILENAME_FIELD_LEN  32
#define DLT_MAX_FILENAME_LEN    DLT_FILENAME_FIELD_LEN
#define DLT_EFAT_ENTRY_LEN      (32+8)  // filename + u32le unk + u32le size
#define DLT_FIRST_FILE_OFFSET   DLT_FAT_OFFSET  // empty archive only

// TODO: not needed as max will be 64k files anyway
#define DLT_SAFETY_MAX_FILECOUNT  8192 // Maximum value we will load

#define DLT_FATENTRY_OFFSET(e) (e->iOffset)

#define DLT_FILENAME_OFFSET(e) DLT_FATENTRY_OFFSET(e)
#define DLT_FILESIZE_OFFSET(e) (DLT_FATENTRY_OFFSET(e) + DLT_FILENAME_FIELD_LEN + 4)

namespace camoto {
namespace gamearchive {

DLTType::DLTType()
{
}

DLTType::~DLTType()
{
}

std::string DLTType::getArchiveCode() const
{
	return "dlt-stargunner";
}

std::string DLTType::getFriendlyName() const
{
	return "Star Gunner DLT File";
}

std::vector<std::string> DLTType::getFileExtensions() const
{
	std::vector<std::string> vcExtensions;
	vcExtensions.push_back("dlt");
	return vcExtensions;
}

std::vector<std::string> DLTType::getGameList() const
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Star Gunner");
	return vcGames;
}

ArchiveType::Certainty DLTType::isInstance(stream::input_sptr psArchive) const
{
	stream::pos lenArchive = psArchive->size();

	// TESTED BY: fmt_dlt_stargunner_isinstance_c02
	if (lenArchive < DLT_HEADER_LEN) return DefinitelyNo; // too short

	char sig[4];
	psArchive->seekg(0, stream::start);
	psArchive->read(sig, 4);

	// TESTED BY: fmt_dlt_stargunner_isinstance_c00
	if (strncmp(sig, "DAVE", 4) == 0) return DefinitelyYes;

	// TESTED BY: fmt_dlt_stargunner_isinstance_c01
	return DefinitelyNo;
}

ArchivePtr DLTType::newArchive(stream::inout_sptr psArchive, SuppData& suppData) const
{
	psArchive->seekp(0, stream::start);
	psArchive->write("DAVE\x00\x01\x00\x00", 8);
	return ArchivePtr(new DLTArchive(psArchive));
}

ArchivePtr DLTType::open(stream::inout_sptr psArchive, SuppData& suppData) const
{
	return ArchivePtr(new DLTArchive(psArchive));
}

SuppFilenames DLTType::getRequiredSupps(stream::input_sptr data,
	const std::string& filenameArchive) const
{
	// No supplemental types/empty list
	return SuppFilenames();
}


DLTArchive::DLTArchive(stream::inout_sptr psArchive)
	:	FATArchive(psArchive, DLT_FIRST_FILE_OFFSET, DLT_MAX_FILENAME_LEN)
{
	this->psArchive->seekg(4, stream::start); // skip "DAVE" sig

	// We still have to perform sanity checks in case the user forced an archive
	// to open even though it failed the signature check.
	if (this->psArchive->tellg() != 4) throw stream::error("file too short");

	uint16_t unk;
	uint16_t numFiles;
	this->psArchive
		>> u16le(unk)
		>> u16le(numFiles)
	;

	if (numFiles >= DLT_SAFETY_MAX_FILECOUNT) {
		throw stream::error("too many files or corrupted archive");
	}

	stream::pos offNext = DLT_HEADER_LEN;
	for (unsigned int i = 0; i < numFiles; i++) {
		FATEntry *fatEntry = new FATEntry();
		EntryPtr ep(fatEntry);

		fatEntry->iIndex = i;
		fatEntry->iOffset = offNext;
		fatEntry->lenHeader = DLT_EFAT_ENTRY_LEN;
		fatEntry->type = FILETYPE_GENERIC;
		fatEntry->fAttr = 0;
		fatEntry->bValid = true;

		uint32_t unk;

		// Read the data in from the FAT entry in the file
		uint8_t name[DLT_FILENAME_FIELD_LEN + 1];
		this->psArchive->read(name, DLT_FILENAME_FIELD_LEN);
		this->psArchive
			>> u32le(unk)
			>> u32le(fatEntry->storedSize)
		;

		// Decrypt the filename
		for (int i = 1; i < DLT_FILENAME_FIELD_LEN; i++) name[i] ^= name[i - 1] + i;
		name[DLT_FILENAME_FIELD_LEN] = 0; // just in case
		fatEntry->strName = (char *)name;

		fatEntry->realSize = fatEntry->storedSize;
		this->vcFAT.push_back(ep);
		offNext += fatEntry->storedSize + DLT_EFAT_ENTRY_LEN;
		this->psArchive->seekg(fatEntry->storedSize, stream::cur);
	}
}

DLTArchive::~DLTArchive()
{
}

void DLTArchive::updateFileName(const FATEntry *pid, const std::string& strNewName)
{
	int lenName = strNewName.length();
	// TESTED BY: fmt_dlt_stargunner_rename
	assert(lenName <= DLT_MAX_FILENAME_LEN);

	// Encrypt the filename
	uint8_t encName[DLT_FILENAME_FIELD_LEN];
	const char *clearName = strNewName.c_str();
	encName[0] = clearName[0];
	int i;
	if (lenName < DLT_FILENAME_FIELD_LEN) lenName++; // write a terminating null
	for (i = 1; i < lenName; i++) encName[i] = clearName[i] ^ (clearName[i - 1] + i);
	for (; i < DLT_FILENAME_FIELD_LEN; i++) encName[i] = i; // decodes to '\0'

	this->psArchive->seekp(DLT_FILENAME_OFFSET(pid), stream::start);
	this->psArchive->write(encName, DLT_FILENAME_FIELD_LEN);
	return;
}

void DLTArchive::updateFileOffset(const FATEntry *pid, stream::delta offDelta)
{
	// This format doesn't have any offsets that need updating.  As this function
	// is only called when removing a file, the "offsets" will be sorted out
	// when the FAT entry is removed later.
	return;
}

void DLTArchive::updateFileSize(const FATEntry *pid, stream::delta sizeDelta)
{
	// TESTED BY: fmt_dlt_stargunner_insert*
	// TESTED BY: fmt_dlt_stargunner_resize*
	this->psArchive->seekp(DLT_FILESIZE_OFFSET(pid), stream::start);
	this->psArchive << u32le(pid->storedSize);
	return;
}

FATArchive::FATEntry *DLTArchive::preInsertFile(const FATEntry *idBeforeThis, FATEntry *pNewEntry)
{
	int lenName = pNewEntry->strName.length();
	// TESTED BY: fmt_dlt_stargunner_insert*
	assert(lenName <= DLT_MAX_FILENAME_LEN);

	// Encrypt the filename
	uint8_t encName[DLT_FILENAME_FIELD_LEN];
	const char *clearName = pNewEntry->strName.c_str();
	encName[0] = clearName[0];
	int i;
	if (lenName < DLT_FILENAME_FIELD_LEN) lenName++; // write a terminating null
	for (i = 1; i < lenName; i++) encName[i] = clearName[i] ^ (clearName[i - 1] + i);
	for (; i < DLT_FILENAME_FIELD_LEN; i++) encName[i] = i; // decodes to '\0'

	// Set the format-specific variables
	pNewEntry->lenHeader = DLT_EFAT_ENTRY_LEN;

	this->psArchive->seekp(pNewEntry->iOffset, stream::start);
	this->psArchive->insert(DLT_EFAT_ENTRY_LEN);
	this->psArchive->write(encName, DLT_FILENAME_FIELD_LEN);
	this->psArchive
		<< u32le(0) // unknown
		<< u32le(pNewEntry->storedSize);

	// Since we've inserted some data for the embedded header, we need to update
	// the other file offsets accordingly.  This call updates the offset of the
	// files, then calls updateFileOffset() on them, using the *new* offset, so
	// we need to do this after the insert() call above to make sure the extra
	// data has been inserted.  Then when updateFileOffset() writes data out it
	// will go into the correct spot.
	this->shiftFiles(NULL, pNewEntry->iOffset, pNewEntry->lenHeader, 0);

	this->updateFileCount(this->vcFAT.size() + 1);
	return pNewEntry;
}

void DLTArchive::preRemoveFile(const FATEntry *pid)
{
	// TESTED BY: fmt_dlt_stargunner_remove*

	this->updateFileCount(this->vcFAT.size() - 1);
	return;
}

void DLTArchive::updateFileCount(uint16_t iNewCount)
{
	// TESTED BY: fmt_dlt_stargunner_insert*
	// TESTED BY: fmt_dlt_stargunner_remove*
	this->psArchive->seekp(DLT_FILECOUNT_OFFSET, stream::start);
	this->psArchive << u16le(iNewCount);
	return;
}

} // namespace gamearchive
} // namespace camoto
