/*
 * fmt-bnk-harry.cpp - Halloween Harry .BNK file reader/writer.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/BNK_Format
 *
 * Copyright (C) 2010 Adam Nielsen <malvineous@shikadi.net>
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

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/progress.hpp>
#include <boost/shared_array.hpp>
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <exception>
#include <string.h>

#include "fmt-bnk-harry.hpp"
#include "iostream_helpers.hpp"
#include "debug.hpp"

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

refcount_declclass(BNKType);

BNKType::BNKType()
	throw ()
{
	refcount_qenterclass(BNKType);
}

BNKType::~BNKType()
	throw ()
{
	refcount_qexitclass(BNKType);
}

std::string BNKType::getArchiveCode() const
	throw ()
{
	return "bnk-harry";
}

std::string BNKType::getFriendlyName() const
	throw ()
{
	return "Halloween Harry BNK File";
}

// Get a list of the known file extensions for this format.
std::vector<std::string> BNKType::getFileExtensions() const
	throw ()
{
	std::vector<std::string> vcExtensions;
	vcExtensions.push_back("bnk");
	vcExtensions.push_back("-0");
	return vcExtensions;
}

std::vector<std::string> BNKType::getGameList() const
	throw ()
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Halloween Harry");
	return vcGames;
}

E_CERTAINTY BNKType::isInstance(iostream_sptr psArchive) const
	throw (std::ios::failure)
{
	psArchive->seekg(0, std::ios::end);
	io::stream_offset lenArchive = psArchive->tellg();
	if (lenArchive == 0) return EC_DEFINITELY_YES; // empty archive
	if (lenArchive < BNK_HH_EFAT_ENTRY_LEN) return EC_DEFINITELY_NO; // too short

	char sig[5];
	psArchive->seekg(0, std::ios::beg);
	psArchive->read(sig, 5);

	// TESTED BY: fmt_bnk_harry_isinstance_c01
	if (strncmp(sig, "\x04-ID-", 5)) return EC_DEFINITELY_NO;

	// If we've made it this far, this is almost certainly a BNK file.
	// TESTED BY: fmt_bnk_harry_isinstance_c00
	return EC_DEFINITELY_YES;
}

ArchivePtr BNKType::open(iostream_sptr psArchive, MP_SUPPDATA& suppData) const
	throw (std::ios::failure)
{
	assert(suppData.find(EST_FAT) != suppData.end());
	return ArchivePtr(new BNKArchive(psArchive, suppData.find(EST_FAT)->second));
}

MP_SUPPLIST BNKType::getRequiredSupps(const std::string& filenameArchive) const
	throw ()
{
	// No supplemental types/empty list
	MP_SUPPLIST supps;
	std::string filenameBase = filenameArchive.substr(0, filenameArchive.find_last_of('.'));
	supps[EST_FAT] = filenameBase + ".Fat"; // TODO: case sensitivity?
	return supps;
}


refcount_declclass(BNKArchive);

BNKArchive::BNKArchive(iostream_sptr psArchive, iostream_sptr psFAT)
	throw (std::ios::failure) :
		FATArchive(psArchive, BNK_FIRST_FILE_OFFSET),
		psFAT(new segmented_stream(psFAT)),
		isAC(false) // TODO: detect and set this
{
	psArchive->seekg(0, std::ios::end);
	io::stream_offset lenArchive = psArchive->tellg();

	psFAT->seekg(0, std::ios::end);
	io::stream_offset lenFAT = psFAT->tellg();
	unsigned long numFiles = lenFAT / BNK_FAT_ENTRY_LEN;

	boost::shared_array<uint8_t> pFATBuf;
	try {
		pFATBuf.reset(new uint8_t[lenFAT]);
		this->vcFAT.reserve(numFiles);
	} catch (std::bad_alloc) {
		std::cerr << "Unable to allocate enough memory for " << numFiles
			<< " files." << std::endl;
		throw std::ios::failure("Memory allocation failure (archive file corrupted?)");
	}

	// Read in all the FAT in one operation
	psFAT->seekg(0, std::ios::beg);
	psFAT->read((char *)pFATBuf.get(), lenFAT);

	for (int i = 0; i < numFiles; i++) {
		FATEntry *pEntry = new FATEntry();
		pEntry->iIndex = i;
		uint8_t lenName = pFATBuf[i * BNK_FAT_ENTRY_LEN + BNK_FAT_FILENAME_OFFSET];
		pEntry->strName = string_from_buf(&pFATBuf[i * BNK_FAT_ENTRY_LEN + BNK_FAT_FILENAME_OFFSET + 1], lenName);
		// The offsets are of the start of the file data (skipping over the
		// embedded header) so we need to subtract a bit to include the
		// header.
		pEntry->iOffset = u32le_from_buf(&pFATBuf[i * BNK_FAT_ENTRY_LEN + BNK_FAT_FILEOFFSET_OFFSET])
			- BNK_EFAT_ENTRY_LEN;
		pEntry->iSize = u32le_from_buf(&pFATBuf[i * BNK_FAT_ENTRY_LEN + BNK_FAT_FILESIZE_OFFSET]);
		pEntry->lenHeader = BNK_EFAT_ENTRY_LEN;
		pEntry->eType = EFT_USEFILENAME;
		pEntry->fAttr = 0;
		pEntry->bValid = true;
		if (pEntry->strName[0] == '\0') {
			// Invalid entry, or empty archive
			delete pEntry;
			break;
		}
		this->vcFAT.push_back(EntryPtr(pEntry));

		if (pEntry->iOffset + pEntry->iSize > lenArchive) {
			std::cerr << "BNK file has been truncated, file @" << i
				<< " ends at offset " << pEntry->iOffset + pEntry->iSize
				<< " but the BNK file is only " << lenArchive
				<< " bytes long." << std::endl;
			throw std::ios::failure("archive has been truncated or FAT is corrupt");
		}
	}
	refcount_qenterclass(BNKArchive);
}

BNKArchive::~BNKArchive()
	throw ()
{
	refcount_qexitclass(BNKArchive);
}

void BNKArchive::rename(EntryPtr& id, const std::string& strNewName)
	throw (std::ios_base::failure)
{
	// TESTED BY: fmt_bnk_harry_rename
	assert(this->isValid(id));
	FATEntry *pEntry = dynamic_cast<FATEntry *>(id.get());

	int len = strNewName.length();
	if (len > BNK_MAX_FILENAME_LEN) {
		throw std::ios_base::failure("new filename too long, max is "
			TOSTRING(BNK_MAX_FILENAME_LEN) " chars");
	}

	uint8_t lenByte = len;
	this->psFAT->seekp(pEntry->iIndex * BNK_FAT_ENTRY_LEN + BNK_FAT_FILENAME_OFFSET);
	this->psFAT->rdbuf()->sputn((char *)&lenByte, 1);
	this->psFAT << zeroPad(strNewName, BNK_MAX_FILENAME_LEN);

	this->psArchive->seekp(pEntry->iOffset + BNK_EFAT_FILENAME_OFFSET);
	this->psArchive->rdbuf()->sputn((char *)&lenByte, 1);
	this->psArchive << zeroPad(strNewName, BNK_MAX_FILENAME_LEN);

	pEntry->strName = strNewName;

	return;
}

void BNKArchive::flush()
	throw (std::ios::failure)
{
	this->FATArchive::flush();

	// Write out to the underlying stream for the supplemental files
	this->psFAT->commit();

	return;
}

void BNKArchive::updateFileOffset(const FATEntry *pid)
	throw (std::ios::failure)
{
	// TESTED BY: fmt_bnk_harry_insert*
	// TESTED BY: fmt_bnk_harry_resize*

	// Only the external FAT file has offsets, not the embedded FAT
	this->psFAT->seekp(pid->iIndex * BNK_FAT_ENTRY_LEN + BNK_FAT_FILEOFFSET_OFFSET);
	this->psFAT << u32le(pid->iOffset + BNK_EFAT_ENTRY_LEN);
	return;
}

void BNKArchive::updateFileSize(const FATEntry *pid)
	throw (std::ios::failure)
{
	// TESTED BY: fmt_bnk_harry_insert*
	// TESTED BY: fmt_bnk_harry_resize*

	// Update external FAT
	this->psFAT->seekp(pid->iIndex * BNK_FAT_ENTRY_LEN + BNK_FAT_FILESIZE_OFFSET);
	this->psFAT << u32le(pid->iSize);

	// Update embedded FAT
	this->psArchive->seekp(pid->iOffset + BNK_EFAT_FILESIZE_OFFSET);
	this->psArchive << u32le(pid->iSize);

	return;
}

void BNKArchive::insertFATEntry(const FATEntry *idBeforeThis, FATEntry *pNewEntry)
	throw (std::ios::failure)
{
	// TESTED BY: fmt_bnk_harry_insert*
	int len = pNewEntry->strName.length();
	if (len > BNK_MAX_FILENAME_LEN) {
		throw std::ios::failure("maximum filename length is "
			TOSTRING(BNK_MAX_FILENAME_LEN) " chars");
	}

	// Set the format-specific variables
	pNewEntry->lenHeader = BNK_EFAT_ENTRY_LEN;

	uint8_t lenByte = len;
	boost::to_upper(pNewEntry->strName);

	// Write out the new embedded FAT entry
	// Insert some space for the embedded header
/*	if (this->vcFAT.size() == 0) {
		// This is the first file, so overwrite the signature
		this->psArchive->seekp(pNewEntry->iOffset + BNK_EFAT_FILENAME_OFFSET);
		this->psArchive->insert(BNK_EFAT_ENTRY_LEN - BNK_EFAT_FILENAME_OFFSET);
		this->psArchive->seekp(pNewEntry->iOffset);
	} else {*/
		this->psArchive->seekp(pNewEntry->iOffset);
		this->psArchive->insert(BNK_EFAT_ENTRY_LEN);
	//}
	// Write the header
	this->psArchive->rdbuf()->sputn("\x04-ID-", 5);
	this->psArchive->rdbuf()->sputn((char *)&lenByte, 1);
	this->psArchive << zeroPad(pNewEntry->strName, BNK_MAX_FILENAME_LEN);
	this->psArchive << u32le(pNewEntry->iSize);

	// Since we've inserted some data for the embedded header, we need to update
	// the other file offsets accordingly.  This call updates the offset of the
	// files, then calls updateFileOffset() on them, using the *new* offset, so
	// we need to do this after the insert() call above to make sure the extra
	// data has been inserted.  Then when updateFileOffset() writes data out it
	// will go into the correct spot.
	this->shiftFiles(pNewEntry->iOffset, pNewEntry->lenHeader, 0);

	// Write out same again but into the BNK file's external FAT
	this->psFAT->seekp(pNewEntry->iIndex * BNK_FAT_ENTRY_LEN);
	this->psFAT->insert(BNK_FAT_ENTRY_LEN);
	this->psFAT->rdbuf()->sputn((char *)&lenByte, 1);
	this->psFAT << zeroPad(pNewEntry->strName, BNK_MAX_FILENAME_LEN);

	// Write out the file size
	this->psFAT << u32le(pNewEntry->iOffset + BNK_EFAT_ENTRY_LEN);
	this->psFAT << u32le(pNewEntry->iSize);

	return;
}

void BNKArchive::removeFATEntry(const FATEntry *pid)
	throw (std::ios::failure)
{
	// TESTED BY: fmt_bnk_harry_remove*

	// Remove the FAT entry
	/*if (this->vcFAT.size() == 1) {
		// This is the last file, so don't remove the signature (otherwise we
		// can't tell it's a BNK file.)
		std::cerr << "last file" << std::endl;
		this->psFAT->seekp(pid->iIndex * BNK_FAT_ENTRY_LEN + BNK_EFAT_FILENAME_OFFSET);
		this->psFAT->remove(BNK_FAT_ENTRY_LEN - BNK_EFAT_FILENAME_OFFSET);
	} else {*/
		this->psFAT->seekp(pid->iIndex * BNK_FAT_ENTRY_LEN);
		this->psFAT->remove(BNK_FAT_ENTRY_LEN);
	//}

	return;
}

} // namespace gamearchive
} // namespace camoto
