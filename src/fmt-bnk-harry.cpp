/**
 * @file   fmt-bnk-harry.cpp
 * @brief  Halloween Harry .BNK file reader/writer.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/BNK_Format_%28Halloween_Harry%29
 *
 * Copyright (C) 2010-2012 Adam Nielsen <malvineous@shikadi.net>
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

#include "fmt-bnk-harry.hpp"

#define BNK_FIRST_FILE_OFFSET     0
#define BNK_MAX_FILENAME_LEN      12

// Embedded FAT (no offset, has sig)
#define BNK_EFAT_FILENAME_OFFSET  5   // First byte is filename length
#define BNK_EFAT_FILESIZE_OFFSET  (BNK_EFAT_FILENAME_OFFSET + 1 + BNK_MAX_FILENAME_LEN)

#define BNK_HH_EFAT_ENTRY_LEN     22  // sig + filename + u32le size
#define BNK_AC_EFAT_ENTRY_LEN     (BNK_HH_EFAT_ENTRY_LEN + 4)  // +uint32 (decompressed size)

#define BNK_EFAT_ENTRY_LEN        (this->isAC ? BNK_AC_EFAT_ENTRY_LEN : BNK_HH_EFAT_ENTRY_LEN)

// FAT file (no sig, has offset)
#define BNK_FAT_FILENAME_OFFSET   0   // First byte is filename length
#define BNK_FAT_FILEOFFSET_OFFSET (BNK_FAT_FILENAME_OFFSET + 1 + BNK_MAX_FILENAME_LEN)
#define BNK_FAT_FILESIZE_OFFSET   (BNK_FAT_FILEOFFSET_OFFSET + 4)

#define BNK_HH_FAT_ENTRY_LEN      21  // nosig, filename + u32le offset + u32le size
#define BNK_AC_FAT_ENTRY_LEN      (BNK_HH_FAT_ENTRY_LEN + 4)  // +uint32 (decompressed size)

#define BNK_FAT_ENTRY_LEN         (this->isAC ? BNK_AC_FAT_ENTRY_LEN : BNK_HH_FAT_ENTRY_LEN)

namespace camoto {
namespace gamearchive {

BNKType::BNKType()
{
}

BNKType::~BNKType()
{
}

std::string BNKType::getArchiveCode() const
{
	return "bnk-harry";
}

std::string BNKType::getFriendlyName() const
{
	return "Halloween Harry BNK File";
}

std::vector<std::string> BNKType::getFileExtensions() const
{
	std::vector<std::string> vcExtensions;
	vcExtensions.push_back("bnk");
	vcExtensions.push_back("-0");
	return vcExtensions;
}

std::vector<std::string> BNKType::getGameList() const
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Halloween Harry");
	return vcGames;
}

ArchiveType::Certainty BNKType::isInstance(stream::input_sptr psArchive) const
{
	stream::pos lenArchive = psArchive->size();
	if (lenArchive == 0) return DefinitelyYes; // empty archive
	if (lenArchive < BNK_HH_EFAT_ENTRY_LEN) return DefinitelyNo; // too short

	char sig[5];
	psArchive->seekg(0, stream::start);
	psArchive->read(sig, 5);

	// TESTED BY: fmt_bnk_harry_isinstance_c01
	if (strncmp(sig, "\x04-ID-", 5)) return DefinitelyNo;

	// If we've made it this far, this is almost certainly a BNK file.
	// TESTED BY: fmt_bnk_harry_isinstance_c00
	return DefinitelyYes;
}

ArchivePtr BNKType::open(stream::inout_sptr psArchive, SuppData& suppData) const
{
	assert(suppData.find(SuppItem::FAT) != suppData.end());
	return ArchivePtr(new BNKArchive(psArchive, suppData[SuppItem::FAT]));
}

SuppFilenames BNKType::getRequiredSupps(stream::input_sptr data,
	const std::string& filenameArchive) const
{
	// No supplemental types/empty list
	SuppFilenames supps;
	std::string filenameBase = filenameArchive.substr(0, filenameArchive.find_last_of('.'));
	supps[SuppItem::FAT] = filenameBase + ".fat";
	return supps;
}


BNKArchive::BNKArchive(stream::inout_sptr psArchive, stream::inout_sptr psFAT)
	:	FATArchive(psArchive, BNK_FIRST_FILE_OFFSET, BNK_MAX_FILENAME_LEN),
		psFAT(new stream::seg()),
		isAC(false) // TODO: detect and set this
{
	this->psFAT->open(psFAT);

	stream::pos lenFAT = this->psFAT->size();
	unsigned long numFiles = lenFAT / BNK_FAT_ENTRY_LEN;
	this->vcFAT.reserve(numFiles);

	this->psFAT->seekg(0, stream::start);

	for (unsigned int i = 0; i < numFiles; i++) {
		FATEntry *fatEntry = new FATEntry();
		EntryPtr ep(fatEntry);

		uint8_t lenName;
		this->psFAT
			>> u8(lenName)
			>> nullPadded(fatEntry->strName, BNK_MAX_FILENAME_LEN)
			>> u32le(fatEntry->iOffset)
			>> u32le(fatEntry->storedSize)
		;
		fatEntry->strName = fatEntry->strName.substr(0, lenName);

		// The offsets are of the start of the file data (skipping over the
		// embedded header) so we need to subtract a bit to include the
		// header.
		fatEntry->iOffset -= BNK_EFAT_ENTRY_LEN;

		fatEntry->iIndex = i;
		fatEntry->lenHeader = BNK_EFAT_ENTRY_LEN;
		fatEntry->type = FILETYPE_GENERIC;
		fatEntry->fAttr = 0;
		fatEntry->bValid = true;
		fatEntry->realSize = fatEntry->storedSize;

		if (fatEntry->strName[0] == '\0') fatEntry->fAttr = EA_EMPTY;

		this->vcFAT.push_back(ep);
	}
}

BNKArchive::~BNKArchive()
{
}

void BNKArchive::flush()
{
	this->FATArchive::flush();

	// Write out to the underlying stream for the supplemental files
	this->psFAT->flush();

	return;
}

void BNKArchive::updateFileName(const FATEntry *pid, const std::string& strNewName)
{
	// TESTED BY: fmt_bnk_harry_rename
	assert(strNewName.length() <= BNK_MAX_FILENAME_LEN);

	uint8_t lenByte = strNewName.length();
	this->psFAT->seekp(pid->iIndex * BNK_FAT_ENTRY_LEN + BNK_FAT_FILENAME_OFFSET, stream::start);
	this->psFAT->write(&lenByte, 1);
	this->psFAT << nullPadded(strNewName, BNK_MAX_FILENAME_LEN);

	this->psArchive->seekp(pid->iOffset + BNK_EFAT_FILENAME_OFFSET, stream::start);
	this->psArchive->write(&lenByte, 1);
	this->psArchive << nullPadded(strNewName, BNK_MAX_FILENAME_LEN);

	return;
}

void BNKArchive::updateFileOffset(const FATEntry *pid, stream::delta offDelta)
{
	// TESTED BY: fmt_bnk_harry_insert*
	// TESTED BY: fmt_bnk_harry_resize*

	// Only the external FAT file has offsets, not the embedded FAT
	this->psFAT->seekp(pid->iIndex * BNK_FAT_ENTRY_LEN + BNK_FAT_FILEOFFSET_OFFSET, stream::start);
	this->psFAT << u32le(pid->iOffset + BNK_EFAT_ENTRY_LEN);
	return;
}

void BNKArchive::updateFileSize(const FATEntry *pid, stream::delta sizeDelta)
{
	// TESTED BY: fmt_bnk_harry_insert*
	// TESTED BY: fmt_bnk_harry_resize*

	// Update external FAT
	this->psFAT->seekp(pid->iIndex * BNK_FAT_ENTRY_LEN + BNK_FAT_FILESIZE_OFFSET, stream::start);
	this->psFAT << u32le(pid->storedSize);

	// Update embedded FAT
	this->psArchive->seekp(pid->iOffset + BNK_EFAT_FILESIZE_OFFSET, stream::start);
	this->psArchive << u32le(pid->storedSize);

	return;
}

FATArchive::FATEntry *BNKArchive::preInsertFile(const FATEntry *idBeforeThis, FATEntry *pNewEntry)
{
	// TESTED BY: fmt_bnk_harry_insert*
	int len = pNewEntry->strName.length();
	assert(len <= BNK_MAX_FILENAME_LEN);

	// Set the format-specific variables
	pNewEntry->lenHeader = BNK_EFAT_ENTRY_LEN;

	uint8_t lenByte = len;
	boost::to_upper(pNewEntry->strName);

	// Write out the new embedded FAT entry
	this->psArchive->seekp(pNewEntry->iOffset, stream::start);
	this->psArchive->insert(BNK_EFAT_ENTRY_LEN);

	// Write the header
	this->psArchive->write("\x04-ID-", 5);
	this->psArchive
		<< u8(lenByte)
		<< nullPadded(pNewEntry->strName, BNK_MAX_FILENAME_LEN)
		<< u32le(pNewEntry->storedSize)
	;

	// Since we've inserted some data for the embedded header, we need to update
	// the other file offsets accordingly.  This call updates the offset of the
	// files, then calls updateFileOffset() on them, using the *new* offset, so
	// we need to do this after the insert() call above to make sure the extra
	// data has been inserted.  Then when updateFileOffset() writes data out it
	// will go into the correct spot.
	this->shiftFiles(NULL, pNewEntry->iOffset, pNewEntry->lenHeader, 0);

	// Write out same again but into the BNK file's external FAT
	this->psFAT->seekp(pNewEntry->iIndex * BNK_FAT_ENTRY_LEN, stream::start);
	this->psFAT->insert(BNK_FAT_ENTRY_LEN);
	this->psFAT
		<< u8(lenByte)
		<< nullPadded(pNewEntry->strName, BNK_MAX_FILENAME_LEN)
		<< u32le(pNewEntry->iOffset + BNK_EFAT_ENTRY_LEN)
		<< u32le(pNewEntry->storedSize)
	;

	return pNewEntry;
}

void BNKArchive::preRemoveFile(const FATEntry *pid)
{
	// TESTED BY: fmt_bnk_harry_remove*

	// Remove the FAT entry
	this->psFAT->seekp(pid->iIndex * BNK_FAT_ENTRY_LEN, stream::start);
	this->psFAT->remove(BNK_FAT_ENTRY_LEN);

	return;
}

} // namespace gamearchive
} // namespace camoto
