/*
 * fmt-pod-tv.cpp - Terminal Velocity .POD file reader/writer.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/POD_Format
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

#include "fmt-pod-tv.hpp"
#include "iostream_helpers.hpp"
#include "debug.hpp"

#define POD_DESCRIPTION_LEN       80
#define POD_FAT_OFFSET            84
#define POD_FAT_ENTRY_LEN         40  // filename + u32le offset + u32le size
#define POD_MAX_FILENAME_LEN      32
#define POD_FAT_ENTRY_SIZE_POS    (POD_MAX_FILENAME_LEN)
#define POD_FAT_ENTRY_OFFSET_POS  (POD_MAX_FILENAME_LEN + 4)

namespace camoto {
namespace gamearchive {

refcount_declclass(PODType);

PODType::PODType()
	throw ()
{
	refcount_qenterclass(PODType);
}

PODType::~PODType()
	throw ()
{
	refcount_qexitclass(PODType);
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

// Get a list of the known file extensions for this format.
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

E_CERTAINTY PODType::isInstance(iostream_sptr psArchive) const
	throw (std::ios::failure)
{
	psArchive->seekg(0, std::ios::end);
	io::stream_offset lenArchive = psArchive->tellg();

	// Must have filecount + description
	if (lenArchive < 84) return EC_DEFINITELY_NO;

	psArchive->seekg(0, std::ios::beg);
	uint32_t numFiles = read_u32le(psArchive);

	char description[POD_DESCRIPTION_LEN + 1];
	psArchive->read(description, POD_DESCRIPTION_LEN);
	description[POD_DESCRIPTION_LEN] = 0;

	for (int j = 0; j < POD_DESCRIPTION_LEN; j++) {
		// Fail on control characters in the description
		if ((description[j]) && (description[j] < 32)) {
			return EC_DEFINITELY_NO; // TESTED BY: fmt_pod_tv_isinstance_c04
		}
	}

	// Make sure the FAT fits inside the archive
	if (POD_FAT_OFFSET + numFiles * POD_FAT_ENTRY_LEN > lenArchive) {
		return EC_DEFINITELY_NO;
	}

	// Check each FAT entry
	char fn[POD_MAX_FILENAME_LEN];
	psArchive->seekg(POD_FAT_OFFSET, std::ios::beg);
	for (int i = 0; i < numFiles; i++) {
		psArchive->read(fn, POD_MAX_FILENAME_LEN);
		// Make sure there aren't any invalid characters in the filename
		for (int j = 0; j < POD_MAX_FILENAME_LEN; j++) {
			if (!fn[j]) break; // stop on terminating null

			// Fail on control characters in the filename
			if (fn[j] < 32) return EC_DEFINITELY_NO; // TESTED BY: fmt_pod_tv_isinstance_c01
		}

		uint32_t offEntry = read_u32le(psArchive);
		uint32_t lenEntry = read_u32le(psArchive);
		// If a file entry points past the end of the archive then it's an invalid
		// format.
		// TESTED BY: fmt_pod_tv_isinstance_c0[23]
		if (offEntry + lenEntry > lenArchive) return EC_DEFINITELY_NO;
	}

	// If we've made it this far, this is almost certainly a POD file.
	// TESTED BY: fmt_pod_tv_isinstance_c00
	return EC_DEFINITELY_YES;
}

Archive *PODType::newArchive(iostream_sptr psArchive, MP_SUPPDATA& suppData) const
	throw (std::ios::failure)
{
#define fmt_pod_empty \
	"\x00\x00\x00\x00" \
	"Empty POD file\0\0" \
	"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
	"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
	"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
	"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	psArchive->seekg(0, std::ios::beg);
	psArchive->write(fmt_pod_empty, strlen(fmt_pod_empty));
	return new PODArchive(psArchive);
}

Archive *PODType::open(iostream_sptr psArchive, MP_SUPPDATA& suppData) const
	throw (std::ios::failure)
{
	return new PODArchive(psArchive);
}

MP_SUPPLIST PODType::getRequiredSupps(const std::string& filenameArchive) const
	throw ()
{
	// No supplemental types/empty list
	return MP_SUPPLIST();
}


refcount_declclass(PODArchive);

PODArchive::PODArchive(iostream_sptr psArchive)
	throw (std::ios::failure) :
		FATArchive(psArchive)
{
	psArchive->seekg(0, std::ios::beg);

	uint32_t numFiles = read_u32le(psArchive);
	unsigned long lenFAT = numFiles * POD_FAT_ENTRY_LEN;

	char description[POD_DESCRIPTION_LEN + 1];
	psArchive->read(description, POD_DESCRIPTION_LEN);
	description[POD_DESCRIPTION_LEN] = 0;
	// TODO: set description as metadata
	// TODO: see whether description can span two file entries (80 bytes) or
	// whether the offsets have to be null

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
	//psArchive->seekg(0, std::ios::beg);
	psArchive->read((char *)pFATBuf.get(), lenFAT);
	if (psArchive->gcount() != lenFAT) {
		std::cerr << "POD file only " << psArchive->gcount()
			<< " bytes long (FAT is meant to be first " << lenFAT
			<< " bytes.)" << std::endl;
		throw std::ios::failure("File has been truncated, it stops in the middle "
			"of the FAT!");
	}

	for (int i = 0; i < numFiles; i++) {
		FATEntry *pEntry = new FATEntry();
		pEntry->iIndex = i;
		pEntry->strName = string_from_buf(&pFATBuf[i*POD_FAT_ENTRY_LEN], POD_MAX_FILENAME_LEN);
		pEntry->iSize = u32le_from_buf(&pFATBuf[i*POD_FAT_ENTRY_LEN + POD_FAT_ENTRY_SIZE_POS]);
		pEntry->iOffset = u32le_from_buf(&pFATBuf[i*POD_FAT_ENTRY_LEN + POD_FAT_ENTRY_OFFSET_POS]);
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
	refcount_qenterclass(PODArchive);
}

PODArchive::~PODArchive()
	throw ()
{
	refcount_qexitclass(PODArchive);
}

void PODArchive::rename(EntryPtr& id, const std::string& strNewName)
	throw (std::ios_base::failure)
{
	// TESTED BY: fmt_pod_tv_rename
	assert(this->isValid(id));
	FATEntry *pEntry = FATEntryPtr_from_EntryPtr(id);

	int iLen = strNewName.length();
	if (iLen > POD_MAX_FILENAME_LEN) throw std::ios_base::failure("name too long");

	// Pad out the filename with NULLs
	char cBuffer[POD_MAX_FILENAME_LEN];
	const char *p = strNewName.c_str();
	for (int i = 0; i < iLen; i++) cBuffer[i] = p[i];
	for (int i = iLen; i < POD_MAX_FILENAME_LEN; i++) cBuffer[i] = 0;

	this->psArchive->seekp(POD_FAT_OFFSET + pEntry->iIndex * POD_FAT_ENTRY_LEN);
	this->psArchive->rdbuf()->sputn(cBuffer, POD_MAX_FILENAME_LEN);
	pEntry->strName = strNewName;

	return;
}

void PODArchive::updateFileOffset(const FATEntry *pid)
	throw (std::ios::failure)
{
	// TESTED BY: fmt_pod_tv_insert*
	// TESTED BY: fmt_pod_tv_resize*
	this->psArchive->seekp(POD_FAT_OFFSET + pid->iIndex * POD_FAT_ENTRY_LEN + POD_FAT_ENTRY_OFFSET_POS);
	write_u32le(this->psArchive, pid->iOffset);
	return;
}

void PODArchive::updateFileSize(const FATEntry *pid)
	throw (std::ios::failure)
{
	// TESTED BY: fmt_pod_tv_insert*
	// TESTED BY: fmt_pod_tv_resize*
	this->psArchive->seekp(POD_FAT_OFFSET + pid->iIndex * POD_FAT_ENTRY_LEN + POD_FAT_ENTRY_SIZE_POS);
	write_u32le(this->psArchive, pid->iSize);
	return;
}

void PODArchive::insertFATEntry(const FATEntry *idBeforeThis, FATEntry *pNewEntry)
	throw (std::ios::failure)
{
	// TESTED BY: fmt_pod_tv_insert*
	if (pNewEntry->strName.length() > POD_MAX_FILENAME_LEN) {
		throw std::ios::failure("maximum filename/path length is 32 chars");
	}

	// Because the new entry isn't in the vector yet we need to shift it manually
	pNewEntry->iOffset += POD_FAT_ENTRY_LEN;

	this->psArchive->seekp(POD_FAT_OFFSET + pNewEntry->iIndex * POD_FAT_ENTRY_LEN);
	this->psArchive->insert(POD_FAT_ENTRY_LEN);
	boost::to_upper(pNewEntry->strName);
	this->psArchive->write(pNewEntry->strName.c_str(), pNewEntry->strName.length());

	// Pad out to POD_MAX_FILENAME_LEN chars, the null string must be at least
	// this long.
	char blank[POD_FAT_ENTRY_LEN];
	memset(blank, 0, POD_FAT_ENTRY_LEN);
	this->psArchive->write(blank, POD_MAX_FILENAME_LEN - pNewEntry->strName.length());

	// Write out the offset and size
	write_u32le(this->psArchive, pNewEntry->iSize);
	write_u32le(this->psArchive, pNewEntry->iOffset);

	// Update the offsets now there's a new FAT entry taking up space.  We need
	// to +1 to the vector size to take into account the KenSilverman header.
	this->shiftFiles(POD_FAT_OFFSET + this->vcFAT.size() * POD_FAT_ENTRY_LEN, POD_FAT_ENTRY_LEN, 0);

	this->updateFileCount(this->vcFAT.size() + 1);

	return;
}

void PODArchive::removeFATEntry(const FATEntry *pid)
	throw (std::ios::failure)
{
	// TESTED BY: fmt_pod_tv_remove*

	// Update the offsets now there's one less FAT entry taking up space.  This
	// must be called before the FAT is altered, because it will write a new
	// offset into the FAT entry we're about to erase (and if we erase it first
	// it'll overwrite something else.)
	this->shiftFiles(POD_FAT_OFFSET + this->vcFAT.size() * POD_FAT_ENTRY_LEN, -POD_FAT_ENTRY_LEN, 0);

	// Remove the FAT entry
	this->psArchive->seekp(POD_FAT_OFFSET + pid->iIndex * POD_FAT_ENTRY_LEN);
	this->psArchive->remove(POD_FAT_ENTRY_LEN);

	this->updateFileCount(this->vcFAT.size() - 1);

	return;
}

void PODArchive::updateFileCount(uint32_t iNewCount)
	throw (std::ios_base::failure)
{
	// TESTED BY: fmt_pod_tv_insert*
	// TESTED BY: fmt_pod_tv_remove*
	this->psArchive->seekp(0);
	write_u32le(this->psArchive, iNewCount);
	return;
}

} // namespace gamearchive
} // namespace camoto
