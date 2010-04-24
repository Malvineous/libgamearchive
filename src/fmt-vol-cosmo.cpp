/*
 * fmt-vol-cosmo.cpp - Implementation of Cosmo .VOL file reader/writer.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/VOL_Format
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

#include "fmt-vol-cosmo.hpp"
#include "iostream_helpers.hpp"
#include "debug.hpp"

#define VOL_FAT_LENGTH        4000
#define VOL_MAX_FILENAME_LEN  12
#define VOL_FAT_ENTRY_LEN     20  // filename + u32le offset + u32le size
#define VOL_FIRST_FILE_OFFSET VOL_FAT_LENGTH

namespace camoto {
namespace gamearchive {

refcount_declclass(VOLType);

VOLType::VOLType()
	throw ()
{
	refcount_qenterclass(VOLType);
}

VOLType::~VOLType()
	throw ()
{
	refcount_qexitclass(VOLType);
}

std::string VOLType::getArchiveCode() const
	throw ()
{
	return "vol-cosmo";
}

std::string VOLType::getFriendlyName() const
	throw ()
{
	return "Cosmo Volume File";
}

// Get a list of the known file extensions for this format.
std::vector<std::string> VOLType::getFileExtensions() const
	throw ()
{
	std::vector<std::string> vcExtensions;
	vcExtensions.push_back("vol");
	vcExtensions.push_back("stn");
	vcExtensions.push_back("cmp");
	vcExtensions.push_back("ms1");
	vcExtensions.push_back("ms2");
	vcExtensions.push_back("ms3");
	return vcExtensions;
}

std::vector<std::string> VOLType::getGameList() const
	throw ()
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Cosmo's Cosmic Adventure");
	vcGames.push_back("Duke Nukem II");
	vcGames.push_back("Major Stryker");
	return vcGames;
}

E_CERTAINTY VOLType::isInstance(iostream_sptr psArchive) const
	throw (std::ios::failure)
{
	psArchive->seekg(0, std::ios::end);
	io::stream_offset lenArchive = psArchive->tellg();

	// An empty file could just be an archive with no files in it
	if (lenArchive == 0) return EC_POSSIBLY_YES;

	psArchive->seekg(12, std::ios::beg);
	uint32_t lenFAT = read_u32le(psArchive);

	// If the FAT is larger than the entire archive then it's not a VOL file
	if (lenFAT > lenArchive) return EC_DEFINITELY_NO;

	// If the FAT is smaller than a single entry then it's not a VOL file
	// TESTED BY: fmt_vol_cosmo_isinstance_c02
	if (lenFAT < VOL_FAT_ENTRY_LEN) return EC_DEFINITELY_NO;

	// The FAT is not an even multiple of FAT entries.  Not sure whether this
	// is a requirement.
	//if (lenFAT % VOL_FAT_ENTRY_LEN) return EC_POSSIBLY_YES;

	// The FAT is not 4000 bytes.  Not sure whether this is a requirement.
	//if (lenFAT != 4000) return EC_DEFINITELY_NO;

	// Check each FAT entry
	char fn[VOL_MAX_FILENAME_LEN];
	psArchive->seekg(0, std::ios::beg);
	for (int i = 0; i < lenFAT / VOL_FAT_ENTRY_LEN; i++) {
		psArchive->read(fn, VOL_MAX_FILENAME_LEN);
		// Make sure there aren't any invalid characters in the filename
		for (int j = 0; j < VOL_MAX_FILENAME_LEN; j++) {
			if (!fn[j]) break; // stop on terminating null

			// Fail on control characters in the filename
			if (fn[j] < 32) return EC_DEFINITELY_NO; // TESTED BY: fmt_vol_cosmo_isinstance_c01
		}

		uint32_t offEntry = read_u32le(psArchive);
		uint32_t lenEntry = read_u32le(psArchive);
		// If a file entry points past the end of the archive then it's an invalid
		// format.
		// TESTED BY: fmt_vol_cosmo_isinstance_c03
		if (offEntry + lenEntry > lenArchive) return EC_DEFINITELY_NO;
	}

	// If we've made it this far, this is almost certainly a VOL file.
	// TESTED BY: fmt_vol_cosmo_isinstance_c00
	return EC_DEFINITELY_YES;
}

ArchivePtr VOLType::newArchive(iostream_sptr psArchive, MP_SUPPDATA& suppData) const
	throw (std::ios::failure)
{
	char emptyFAT[VOL_FAT_LENGTH];
	memset(emptyFAT, 0, VOL_FAT_LENGTH);
	psArchive->seekg(0, std::ios::beg);
	psArchive->write(emptyFAT, VOL_FAT_LENGTH);
	return ArchivePtr(new VOLArchive(psArchive));
}

// Preconditions: isInstance() has returned > EC_DEFINITELY_NO
ArchivePtr VOLType::open(iostream_sptr psArchive, MP_SUPPDATA& suppData) const
	throw (std::ios::failure)
{
	return ArchivePtr(new VOLArchive(psArchive));
}

MP_SUPPLIST VOLType::getRequiredSupps(const std::string& filenameArchive) const
	throw ()
{
	// No supplemental types/empty list
	return MP_SUPPLIST();
}


refcount_declclass(VOLArchive);

VOLArchive::VOLArchive(iostream_sptr psArchive)
	throw (std::ios::failure) :
		FATArchive(psArchive)
{
	psArchive->seekg(12, std::ios::beg); // skip to offset of first filesize

	// We still have to perform sanity checks in case the user forced an archive
	// to open even though it failed the signature check.
	// TODO: This could be allowed if the VOL file is empty
	if (psArchive->tellg() != 12) throw std::ios::failure("File too short");

	// TODO: Do we assume the files are in order and read up until the first one,
	// or do we read 4000 chars, or do we somehow scan the probable files and
	// read up until the first one in case they're out of order...?
	// I guess it depends on what works with the games.
	uint32_t lenFAT = read_u32le(psArchive);
	uint32_t numFiles = lenFAT / VOL_FAT_ENTRY_LEN;
	boost::shared_array<uint8_t> pFATBuf;
	try {
		pFATBuf.reset(new uint8_t[lenFAT]);
		this->vcFAT.reserve(numFiles);
	} catch (std::bad_alloc) {
		std::cerr << "Unable to allocate enough memory for " << numFiles
			<< " files." << std::endl;
		throw std::ios::failure("Memory allocation failure (archive corrupted?)");
	}

	// Read in all the FAT in one operation
	psArchive->seekg(0, std::ios::beg);
	psArchive->read((char *)pFATBuf.get(), lenFAT);
	if (psArchive->gcount() != lenFAT) {
		std::cerr << "VOL file only " << psArchive->gcount()
			<< " bytes long (FAT is meant to be first " << lenFAT
			<< " bytes.)" << std::endl;
		throw std::ios::failure("File has been truncated, it stops in the middle "
			"of the FAT!");
	}

	for (int i = 0; i < numFiles; i++) {
		FATEntry *pEntry = new FATEntry();
		pEntry->iIndex = i;
		pEntry->strName = string_from_buf(&pFATBuf[i*VOL_FAT_ENTRY_LEN], VOL_MAX_FILENAME_LEN);
		pEntry->iOffset = u32le_from_buf(&pFATBuf[i*VOL_FAT_ENTRY_LEN + VOL_MAX_FILENAME_LEN]);
		pEntry->iSize = u32le_from_buf(&pFATBuf[i*VOL_FAT_ENTRY_LEN + VOL_MAX_FILENAME_LEN + 4]);
		pEntry->lenHeader = 0;
		pEntry->eType = EFT_USEFILENAME;
		pEntry->fAttr = 0;
		pEntry->bValid = true;
		// Blank FAT entries have an offset of zero
		if (pEntry->iOffset > 0) {
			this->vcFAT.push_back(EntryPtr(pEntry));
		} else {
			delete pEntry;  // ergh, inefficient
		}
	}
	refcount_qenterclass(VOLArchive);
}

VOLArchive::~VOLArchive()
	throw ()
{
	refcount_qexitclass(VOLArchive);
}

void VOLArchive::rename(EntryPtr& id, const std::string& strNewName)
	throw (std::ios_base::failure)
{
	// TESTED BY: fmt_vol_cosmo_rename
	assert(this->isValid(id));
	FATEntry *pEntry = dynamic_cast<FATEntry *>(id.get());

	int iLen = strNewName.length();
	if (iLen > VOL_MAX_FILENAME_LEN) throw std::ios_base::failure("name too long");

	// Pad out the filename with NULLs
	char cBuffer[VOL_MAX_FILENAME_LEN];
	const char *p = strNewName.c_str();
	for (int i = 0; i < iLen; i++) cBuffer[i] = p[i];
	for (int i = iLen; i < VOL_MAX_FILENAME_LEN; i++) cBuffer[i] = 0;

	this->psArchive->seekp(pEntry->iIndex * VOL_FAT_ENTRY_LEN);
	this->psArchive->rdbuf()->sputn(cBuffer, VOL_MAX_FILENAME_LEN);
	pEntry->strName = strNewName;

	return;
}

void VOLArchive::updateFileOffset(const FATEntry *pid)
	throw (std::ios::failure)
{
	// TESTED BY: fmt_vol_cosmo_insert*
	// TESTED BY: fmt_vol_cosmo_resize*
	this->psArchive->seekp(pid->iIndex * VOL_FAT_ENTRY_LEN + 12);
	write_u32le(this->psArchive, pid->iOffset);
	return;
}

void VOLArchive::updateFileSize(const FATEntry *pid)
	throw (std::ios::failure)
{
	// TESTED BY: fmt_vol_cosmo_insert*
	// TESTED BY: fmt_vol_cosmo_resize*
	this->psArchive->seekp(pid->iIndex * VOL_FAT_ENTRY_LEN + 16);
	write_u32le(this->psArchive, pid->iSize);
	return;
}

void VOLArchive::insertFATEntry(const FATEntry *idBeforeThis, FATEntry *pNewEntry)
	throw (std::ios::failure)
{
	// TESTED BY: fmt_vol_cosmo_insert*
	if (pNewEntry->strName.length() > VOL_MAX_FILENAME_LEN) {
		throw std::ios::failure("maximum filename length is 12 chars");
	}

	// Set the format-specific variables
	pNewEntry->lenHeader = 0;

	// TODO: See if this is in fact a hard limitation
	if (this->vcFAT.size() >= 200) {
		throw std::ios::failure("too many files, maximum is 200");
	}
	this->psArchive->seekp(pNewEntry->iIndex * VOL_FAT_ENTRY_LEN);
	this->psArchive->insert(VOL_FAT_ENTRY_LEN);
	boost::to_upper(pNewEntry->strName);
	this->psArchive->write(pNewEntry->strName.c_str(), pNewEntry->strName.length());

	// Pad out to VOL_MAX_FILENAME_LEN chars, the null string must be at least
	// this long.
	char blank[VOL_FAT_ENTRY_LEN];
	memset(blank, 0, VOL_FAT_ENTRY_LEN);
	this->psArchive->write(blank, VOL_MAX_FILENAME_LEN - pNewEntry->strName.length());

	// Write out the offset and size
	write_u32le(this->psArchive, pNewEntry->iOffset);
	write_u32le(this->psArchive, pNewEntry->iSize);

	// Because the FAT is a fixed size we have to remove a blank entry to
	// compensate for the entry we just added.
	int indexLast = 200;
	for (VC_ENTRYPTR::reverse_iterator i = this->vcFAT.rbegin(); i != this->vcFAT.rend(); i++) {
		FATEntry *pFAT = dynamic_cast<FATEntry *>(i->get());
		if (pFAT->iIndex != indexLast) {
			// The previous slot is free, so delete it
			this->psArchive->seekp(indexLast * VOL_FAT_ENTRY_LEN);
			this->psArchive->remove(VOL_FAT_ENTRY_LEN);
			break;
		} else {
			indexLast = pFAT->iIndex - 1;
		}
	}

	// Make sure an entry was removed.  This should never fail as failure would
	// indicate there were 200+ files, which means an exception should've been
	// thrown at the start of this function.
	assert(indexLast >= 0);

	return;
}

void VOLArchive::removeFATEntry(const FATEntry *pid)
	throw (std::ios::failure)
{
	// TESTED BY: fmt_vol_cosmo_remove*

	// Remove the FAT entry
	this->psArchive->seekp(pid->iIndex * VOL_FAT_ENTRY_LEN);
	this->psArchive->remove(VOL_FAT_ENTRY_LEN);

	// Add an empty FAT entry onto the end to keep the FAT the same size
	const FATEntry *pFAT = dynamic_cast<const FATEntry *>(this->vcFAT.back().get());
	this->psArchive->seekp((pFAT->iIndex + 1) * VOL_FAT_ENTRY_LEN);
	this->psArchive->insert(VOL_FAT_ENTRY_LEN);

	return;
}

} // namespace gamearchive
} // namespace camoto
