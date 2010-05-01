/*
 * fmt-epf-lionking.cpp - Implementation of reader/writer for East Point
 *   Software's .EPF file format, used in The Lion King among other games.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/EPF_Format
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

#include "fmt-epf-lionking.hpp"
#include "iostream_helpers.hpp"
#include "debug.hpp"

#define EPF_HEADER_LEN               8   // "EPFS" header + u32le FAT offset
#define EPF_MAX_FILENAME_LEN         12
#define EPF_FILENAME_FIELD_LEN       13
#define EPF_FIRST_FILE_OFFSET        EPF_HEADER_LEN

#define EPF_FAT_FILENAME_OFFSET      0
#define EPF_FAT_ISCOMPRESSED_OFFSET  13
#define EPF_FAT_FILESIZE_OFFSET      14
#define EPF_FAT_DECOMP_SIZE_OFFSET   18
#define EPF_FAT_ENTRY_LEN            22

namespace camoto {
namespace gamearchive {

refcount_declclass(EPFType);

EPFType::EPFType()
	throw ()
{
	refcount_qenterclass(EPFType);
}

EPFType::~EPFType()
	throw ()
{
	refcount_qexitclass(EPFType);
}

std::string EPFType::getArchiveCode() const
	throw ()
{
	return "epf-lionking";
}

std::string EPFType::getFriendlyName() const
	throw ()
{
	return "East Point Software EPFS File";
}

// Get a list of the known file extensions for this format.
std::vector<std::string> EPFType::getFileExtensions() const
	throw ()
{
	std::vector<std::string> vcExtensions;
	vcExtensions.push_back("epf");
	return vcExtensions;
}

std::vector<std::string> EPFType::getGameList() const
	throw ()
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Alien Breed Tower Assault");
	vcGames.push_back("Arcade Pool");
	vcGames.push_back("Jungle Book, The");
	vcGames.push_back("Lion King, The");
	vcGames.push_back("Project X");
	vcGames.push_back("Overdrive");
	vcGames.push_back("Sensible Golf");
	vcGames.push_back("Smurfs, The");
	vcGames.push_back("Spirou");
	vcGames.push_back("Tin Tin in Tibet");
	vcGames.push_back("Universe");
	return vcGames;
}

E_CERTAINTY EPFType::isInstance(iostream_sptr psArchive) const
	throw (std::ios::failure)
{
	psArchive->seekg(0, std::ios::end);
	io::stream_offset lenArchive = psArchive->tellg();

	// TESTED BY: fmt_epf_lionking_isinstance_c02
	if (lenArchive < EPF_HEADER_LEN) return EC_DEFINITELY_NO; // too short

	char sig[4];
	psArchive->seekg(0, std::ios::beg);
	psArchive->read(sig, 4);

	// TESTED BY: fmt_epf_lionking_isinstance_c00
	if (strncmp(sig, "EPFS", 4) == 0) return EC_DEFINITELY_YES;

	// TESTED BY: fmt_epf_lionking_isinstance_c01
	return EC_DEFINITELY_NO;
}

ArchivePtr EPFType::newArchive(iostream_sptr psArchive, MP_SUPPDATA& suppData) const
	throw (std::ios::failure)
{
	psArchive->seekp(0, std::ios::beg);
	psArchive
		<< zeroPad("EPFS", 4)
		<< u32le(8); // FAT offset
	return ArchivePtr(new EPFArchive(psArchive));
}

// Preconditions: isInstance() has returned > EC_DEFINITELY_NO
ArchivePtr EPFType::open(iostream_sptr psArchive, MP_SUPPDATA& suppData) const
	throw (std::ios::failure)
{
	return ArchivePtr(new EPFArchive(psArchive));
}

MP_SUPPLIST EPFType::getRequiredSupps(const std::string& filenameArchive) const
	throw ()
{
	// No supplemental types/empty list
	return MP_SUPPLIST();
}


refcount_declclass(EPFArchive);

EPFArchive::EPFArchive(iostream_sptr psArchive)
	throw (std::ios::failure) :
		FATArchive(psArchive, EPF_FIRST_FILE_OFFSET)
{
	psArchive->seekg(0, std::ios::end);
	io::stream_offset lenArchive = psArchive->tellg();

	psArchive->seekg(4, std::ios::beg); // skip "EPFS" sig

	// We still have to perform sanity checks in case the user forced an archive
	// to open even though it failed the signature check.
	if (psArchive->tellg() != 4) throw std::ios::failure("File too short");

	psArchive >> u32le(this->offFAT);
	psArchive->seekg(this->offFAT, std::ios::beg);

	io::stream_offset lenFAT = lenArchive - offFAT;
	unsigned long numFiles = lenFAT / EPF_FAT_ENTRY_LEN;

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
	psArchive->read((char *)pFATBuf.get(), lenFAT);
	if (psArchive->gcount() != lenFAT) {
		throw std::ios::failure("short read loading FAT");
	}

	io::stream_offset offNext = EPF_FIRST_FILE_OFFSET;
	for (int i = 0; i < numFiles; i++) {
		EPFEntry *pEntry = new EPFEntry();
		pEntry->iIndex = i;
		pEntry->strName = string_from_buf(&pFATBuf[i*EPF_FAT_ENTRY_LEN + EPF_FAT_FILENAME_OFFSET], EPF_MAX_FILENAME_LEN);
		pEntry->iOffset = offNext;
		pEntry->iSize = u32le_from_buf(&pFATBuf[i*EPF_FAT_ENTRY_LEN + EPF_FAT_FILESIZE_OFFSET]);
		pEntry->lenHeader = 0;
		pEntry->eType = EFT_USEFILENAME;
		pEntry->fAttr = 0;
		pEntry->bValid = true;

		pEntry->decompressedSize = u32le_from_buf(&pFATBuf[i*EPF_FAT_ENTRY_LEN + EPF_FAT_DECOMP_SIZE_OFFSET]);
		pEntry->isCompressed = u32le_from_buf(&pFATBuf[i*EPF_FAT_ENTRY_LEN + EPF_FAT_ISCOMPRESSED_OFFSET]);

		this->vcFAT.push_back(EntryPtr(pEntry));
		offNext += pEntry->iSize;
	}
	refcount_qenterclass(EPFArchive);
}

EPFArchive::~EPFArchive()
	throw ()
{
	refcount_qexitclass(EPFArchive);
}

// Does not invalidate existing EntryPtrs
void EPFArchive::rename(EntryPtr& id, const std::string& strNewName)
	throw (std::ios_base::failure)
{
	// TESTED BY: fmt_epf_lionking_rename
	assert(this->isValid(id));
	FATEntry *pEntry = dynamic_cast<FATEntry *>(id.get());

	if (strNewName.length() > EPF_MAX_FILENAME_LEN) {
		throw std::ios_base::failure("new filename too long, max is "
			TOSTRING(EPF_MAX_FILENAME_LEN) " chars");
	}

	this->psArchive->seekp(this->offFAT + pEntry->iIndex * EPF_FAT_ENTRY_LEN);
	this->psArchive << zeroPad(strNewName, EPF_FILENAME_FIELD_LEN);
	pEntry->strName = strNewName;

	return;
}

VC_METADATA_ITEMS EPFArchive::getMetadataList() const
	throw ()
{
	// TESTED BY: fmt_epf_lionking_get_metadata_description
	VC_METADATA_ITEMS m;
	m.push_back(EM_DESCRIPTION);
	return m;
}

std::string EPFArchive::getMetadata(E_METADATA item) const
	throw (std::ios::failure)
{
	// TESTED BY: fmt_epf_lionking_get_metadata_description
	switch (item) {
		case EM_DESCRIPTION: {
			io::stream_offset offDesc = this->getDescOffset();
			std::streamsize sizeDesc = this->offFAT - offDesc;
			std::string strDesc;
			if (sizeDesc) {
				this->psArchive->seekg(offDesc, std::ios::beg);
				this->psArchive >> fixedLength(strDesc, sizeDesc);
				//psArchive->read(strDesc.c_str(), sizeDesc);
			}
			return strDesc;
		}
		default:
			assert(false);
			throw std::ios::failure("unsupported metadata item");
	}
}

void EPFArchive::setMetadata(E_METADATA item, const std::string& value)
	throw (std::ios::failure)
{
	// TESTED BY: fmt_epf_lionking_set_metadata_description
	// TESTED BY: fmt_epf_lionking_new_to_initialstate
	switch (item) {
		case EM_DESCRIPTION: {
			io::stream_offset offDesc = this->getDescOffset();
			std::streamsize sizeDesc = this->offFAT - offDesc;
			std::streamsize sizeDelta = value.length() - sizeDesc;
			this->psArchive->seekp(offDesc, std::ios::beg);
			if (sizeDelta < 0) {
				// TESTED BY: ?
				this->psArchive->remove(-sizeDelta);
			} else {
				// TESTED BY: ?
				this->psArchive->insert(sizeDelta);
			}
			this->psArchive << value; // TODO: confirm no terminating null
			this->offFAT += sizeDelta;
			this->updateFATOffset();
			break;
		}
		default:
			assert(false);
			throw std::ios::failure("unsupported metadata item");
	}
	return;
}

void EPFArchive::updateFileOffset(const FATEntry *pid, std::streamsize offDelta)
	throw (std::ios::failure)
{
	return;
}

void EPFArchive::updateFileSize(const FATEntry *pid, std::streamsize sizeDelta)
	throw (std::ios::failure)
{
	// TESTED BY: fmt_epf_lionking_insert*
	// TESTED BY: fmt_epf_lionking_resize*

	this->offFAT += sizeDelta;

	this->psArchive->seekp(this->offFAT + pid->iIndex * EPF_FAT_ENTRY_LEN + EPF_FAT_FILESIZE_OFFSET);
	this->psArchive
		<< u32le(pid->iSize)    // compressed size
		<< u32le(pid->iSize);   // decompressed size

	this->updateFATOffset();

	return;
}

void EPFArchive::preInsertFile(const FATEntry *idBeforeThis, FATEntry *pNewEntry)
	throw (std::ios::failure)
{
	// TESTED BY: fmt_epf_lionking_insert*
	if (pNewEntry->strName.length() > EPF_MAX_FILENAME_LEN) {
		throw std::ios::failure("maximum filename length is "
			TOSTRING(EPF_MAX_FILENAME_LEN) " chars");
	}

	// Set the format-specific variables
	pNewEntry->lenHeader = 0;

	return;
}

void EPFArchive::postInsertFile(FATEntry *pNewEntry)
	throw (std::ios::failure)
{
	this->offFAT += pNewEntry->iSize;

	this->psArchive->seekp(this->offFAT + pNewEntry->iIndex * EPF_FAT_ENTRY_LEN);
	this->psArchive->insert(EPF_FAT_ENTRY_LEN);
	boost::to_upper(pNewEntry->strName);
	this->psArchive
		<< zeroPad(pNewEntry->strName, EPF_FILENAME_FIELD_LEN)
		<< (uint8_t)0  // 0 == uncompressed, 1 == compressed
		<< u32le(pNewEntry->iSize)  // compressed
		<< u32le(pNewEntry->iSize); // decompressed

	this->updateFATOffset();

	return;
}

void EPFArchive::preRemoveFile(const FATEntry *pid)
	throw (std::ios::failure)
{
	// TESTED BY: fmt_epf_lionking_remove*

	this->psArchive->seekp(this->offFAT + pid->iIndex * EPF_FAT_ENTRY_LEN);
	this->psArchive->remove(EPF_FAT_ENTRY_LEN);

	this->offFAT -= pid->iSize;
	this->updateFATOffset();

	return;
}

void EPFArchive::updateFATOffset()
	throw (std::ios_base::failure)
{
	// TESTED BY: fmt_epf_lionking_insert*
	// TESTED BY: fmt_epf_lionking_remove*

	this->psArchive->seekp(4);
	this->psArchive << u32le(this->offFAT);

	return;
}

io::stream_offset EPFArchive::getDescOffset() const
	throw (std::ios::failure)
{
	io::stream_offset offDesc;
	if (this->vcFAT.size()) {
		EntryPtr lastFile = this->vcFAT.back();
		assert(lastFile);
		FATEntry *lastFATEntry = dynamic_cast<FATEntry *>(lastFile.get());
		offDesc = lastFATEntry->iOffset + lastFATEntry->iSize;
	} else {
		offDesc = EPF_FIRST_FILE_OFFSET;
	}
	return offDesc;
}

} // namespace gamearchive
} // namespace camoto
