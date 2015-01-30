/**
 * @file  fmt-dat-mystic.cpp
 * @brief Implementation of Mystic Towers .DAT format.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/DAT_Format_%28Mystic_Towers%29
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
#include "fmt-dat-mystic.hpp"

#define DAT_FATOFFSET_OFFSET          8
#define DAT_FILECOUNT_OFFSET_END     -2 // bytes from EOF
#define DAT_FILENAME_FIELD_LEN       12
#define DAT_MAX_FILENAME_LEN         12
#define DAT_FIRST_FILE_OFFSET         0

#define DAT_FAT_ENTRY_LEN            (1+12+4+4)

#define DAT_SAFETY_MAX_FILECOUNT     8192 // Maximum value we will recognise

#define DAT_FATENTRY_OFFSET_END(e)   (DAT_FILECOUNT_OFFSET_END - ((signed long)this->vcFAT.size() + this->uncommittedFiles - (signed long)((e)->iIndex)) * DAT_FAT_ENTRY_LEN)
#define DAT_FILEOFFSET_OFFSET_END(e) (DAT_FATENTRY_OFFSET_END(e) + 13)
#define DAT_FILESIZE_OFFSET_END(e)   (DAT_FATENTRY_OFFSET_END(e) + 17)
#define DAT_FILENAME_OFFSET_END(e)   (DAT_FATENTRY_OFFSET_END(e) + 0)

namespace camoto {
namespace gamearchive {

DAT_MysticType::DAT_MysticType()
{
}

DAT_MysticType::~DAT_MysticType()
{
}

std::string DAT_MysticType::getArchiveCode() const
{
	return "dat-mystic";
}

std::string DAT_MysticType::getFriendlyName() const
{
	return "Mystic Towers DAT File";
}

std::vector<std::string> DAT_MysticType::getFileExtensions() const
{
	std::vector<std::string> vcExtensions;
	vcExtensions.push_back("dat");
	return vcExtensions;
}

std::vector<std::string> DAT_MysticType::getGameList() const
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Mystic Towers");
	return vcGames;
}

ArchiveType::Certainty DAT_MysticType::isInstance(stream::input_sptr psArchive) const
{
	stream::len lenArchive = psArchive->size();

	// File too short
	// TESTED BY: fmt_dat_mystic_isinstance_c01
	if (lenArchive < 2) return DefinitelyNo; // too short

	uint16_t fileCount;
	psArchive->seekg(DAT_FILECOUNT_OFFSET_END, stream::end);
	psArchive >> u16le(fileCount);
	// Too many files
	// TESTED BY: fmt_dat_mystic_isinstance_c02
	if (fileCount >= DAT_SAFETY_MAX_FILECOUNT) return DefinitelyNo;

	// Too small to contain FAT
	// TESTED BY: fmt_dat_mystic_isinstance_c03
	if ((stream::len)(2 + fileCount * DAT_FAT_ENTRY_LEN) > lenArchive) return DefinitelyNo;

	// Don't count the FAT in the rest of the calculations using the archive size,
	// just count the actual data storage space.
	lenArchive -= 2 + fileCount * DAT_FAT_ENTRY_LEN;

	stream::len totalDataSize = 0;

	psArchive->seekg(DAT_FILECOUNT_OFFSET_END - fileCount * DAT_FAT_ENTRY_LEN, stream::end);
	for (unsigned int i = 0; i < fileCount; i++) {
		uint8_t lenFilename;
		psArchive >> u8(lenFilename);
		// Filename length longer than field size
		// TESTED BY: fmt_dat_mystic_isinstance_c04
		if (lenFilename > 12) return DefinitelyNo;
		psArchive->seekg(12, stream::cur);
		uint32_t off, len;
		psArchive >> u32le(off) >> u32le(len);
		// File starts or ends past archive EOF
		// TESTED BY: fmt_dat_mystic_isinstance_c05
		if (off + len > lenArchive) return DefinitelyNo;
		totalDataSize += len;
	}

	// File contains extra data beyond what is recorded in the FAT
	// TESTED BY: fmt_dat_mystic_isinstance_c06
	if (totalDataSize != lenArchive) return DefinitelyNo;

	// TESTED BY: fmt_dat_mystic_isinstance_c00
	return DefinitelyYes;
}

ArchivePtr DAT_MysticType::newArchive(stream::inout_sptr psArchive, SuppData& suppData) const
{
	psArchive->seekp(0, stream::start);
	psArchive << u16le(0);
	return ArchivePtr(new DAT_MysticArchive(psArchive));
}

ArchivePtr DAT_MysticType::open(stream::inout_sptr psArchive, SuppData& suppData) const
{
	return ArchivePtr(new DAT_MysticArchive(psArchive));
}

SuppFilenames DAT_MysticType::getRequiredSupps(stream::input_sptr data,
	const std::string& filenameArchive) const
{
	// No supplemental types/empty list
	return SuppFilenames();
}


DAT_MysticArchive::DAT_MysticArchive(stream::inout_sptr psArchive)
	:	FATArchive(psArchive, DAT_FIRST_FILE_OFFSET, ARCH_STD_DOS_FILENAMES),
		uncommittedFiles(0)
{
	stream::pos lenArchive = this->psArchive->size();
	if (lenArchive < 2) throw stream::error("File too short");

	uint16_t fileCount;
	this->psArchive->seekg(DAT_FILECOUNT_OFFSET_END, stream::end);
	this->psArchive >> u16le(fileCount);

	this->psArchive->seekg(DAT_FILECOUNT_OFFSET_END - fileCount * DAT_FAT_ENTRY_LEN, stream::end);
	for (unsigned int i = 0; i < fileCount; i++) {
		FATEntry *fatEntry = new FATEntry();
		EntryPtr ep(fatEntry);

		fatEntry->iIndex = i;
		fatEntry->lenHeader = 0;
		fatEntry->type = FILETYPE_GENERIC;
		fatEntry->fAttr = 0;
		fatEntry->bValid = true;

		uint8_t lenFilename;
		this->psArchive
			>> u8(lenFilename)
		;
		if (lenFilename > 12) lenFilename = 12;
		this->psArchive
			>> fixedLength(fatEntry->strName, lenFilename)
		;
		// Skip over padding chars
		this->psArchive->seekg(DAT_FILENAME_FIELD_LEN - lenFilename, stream::cur);
		this->psArchive
			>> u32le(fatEntry->iOffset)
			>> u32le(fatEntry->storedSize)
		;

		fatEntry->realSize = fatEntry->storedSize;
		this->vcFAT.push_back(ep);
	}
}

DAT_MysticArchive::~DAT_MysticArchive()
{
}

void DAT_MysticArchive::updateFileName(const FATEntry *pid, const std::string& strNewName)
{
	// TESTED BY: fmt_dat_mystic_rename
	assert(strNewName.length() <= DAT_MAX_FILENAME_LEN);
	this->psArchive->seekp(DAT_FILENAME_OFFSET_END(pid), stream::end);
	this->psArchive
		<< u8(strNewName.length())
		<< nullPadded(strNewName, DAT_FILENAME_FIELD_LEN)
	;
	return;
}

void DAT_MysticArchive::updateFileOffset(const FATEntry *pid, stream::delta offDelta)
{
	// TESTED BY: fmt_dat_mystic_insert*
	// TESTED BY: fmt_dat_mystic_resize*
	this->psArchive->seekp(DAT_FILEOFFSET_OFFSET_END(pid), stream::end);
	this->psArchive << u32le(pid->iOffset);
	return;
}

void DAT_MysticArchive::updateFileSize(const FATEntry *pid, stream::delta sizeDelta)
{
	// TESTED BY: fmt_dat_mystic_insert*
	// TESTED BY: fmt_dat_mystic_resize*
	this->psArchive->seekp(DAT_FILESIZE_OFFSET_END(pid), stream::end);
	this->psArchive << u32le(pid->storedSize);
	return;
}

FATArchive::FATEntry *DAT_MysticArchive::preInsertFile(const FATEntry *idBeforeThis,
	FATEntry *pNewEntry)
{
	// TESTED BY: fmt_dat_mystic_insert*

	pNewEntry->lenHeader = 0;

	// Prepare filename field
	boost::to_upper(pNewEntry->strName);

	// Add the new entry into the on-disk FAT.  This has to happen here (rather
	// than in postInsertFile()) because on return FATArchive will update the
	// offsets of all FAT entries following this one.  If we don't insert a new
	// entry now, all the offset changes will be applied to the wrong files.
	this->psArchive->seekp(DAT_FATENTRY_OFFSET_END(pNewEntry), stream::end);
	this->psArchive->insert(DAT_FAT_ENTRY_LEN);
	this->uncommittedFiles++;

	this->psArchive
		<< u8(pNewEntry->strName.length())
		<< nullPadded(pNewEntry->strName, DAT_FILENAME_FIELD_LEN)
		<< u32le(pNewEntry->iOffset)
		<< u32le(pNewEntry->storedSize)
	;
	return pNewEntry;
}

void DAT_MysticArchive::postInsertFile(FATEntry *pNewEntry)
{
	this->uncommittedFiles--;
	this->updateFileCount(this->vcFAT.size());
	return;
}

void DAT_MysticArchive::preRemoveFile(const FATEntry *pid)
{
	this->psArchive->seekp(DAT_FATENTRY_OFFSET_END(pid), stream::end);
	this->psArchive->remove(DAT_FAT_ENTRY_LEN);
	return;
}

void DAT_MysticArchive::postRemoveFile(const FATEntry *pid)
{
	this->updateFileCount(this->vcFAT.size());
	return;
}

void DAT_MysticArchive::updateFileCount(uint32_t newCount)
{
	this->psArchive->seekp(DAT_FILECOUNT_OFFSET_END, stream::end);
	this->psArchive << u16le(newCount);
	return;
}

} // namespace gamearchive
} // namespace camoto
