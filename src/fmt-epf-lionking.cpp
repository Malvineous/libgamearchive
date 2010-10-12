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
#include <boost/iostreams/filtering_stream.hpp>
#include <iostream>
#include <exception>
#include <string.h>

#include "fmt-epf-lionking.hpp"
#include <camoto/iostream_helpers.hpp>
#include <camoto/debug.hpp>
#include <camoto/lzw.hpp>
#include <camoto/filteredstream.hpp>

#define EPF_HEADER_LEN               11
#define EPF_FAT_OFFSET_POS           4
#define EPF_FILECOUNT_POS            9
#define EPF_FIRST_FILE_OFFSET        EPF_HEADER_LEN

#define EPF_FAT_FILENAME_OFFSET      0
#define EPF_MAX_FILENAME_LEN         12
#define EPF_FILENAME_FIELD_LEN       13
#define EPF_FAT_ISCOMPRESSED_OFFSET  13
#define EPF_FAT_FILESIZE_OFFSET      14
#define EPF_FAT_DECOMP_SIZE_OFFSET   18
#define EPF_FAT_ENTRY_LEN            22

#define EPF_FAT_FLAG_COMPRESSED      1

namespace camoto {
namespace gamearchive {

EPFType::EPFType()
	throw ()
{
}

EPFType::~EPFType()
	throw ()
{
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
		<< nullPadded("EPFS", 4)
		<< u32le(11) // FAT offset
		<< u8(0)     // Unknown/flags?
		<< u16le(0); // File count
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


EPFArchive::EPFArchive(iostream_sptr psArchive)
	throw (std::ios::failure) :
		FATArchive(psArchive, EPF_FIRST_FILE_OFFSET)
{
	this->psArchive->seekg(0, std::ios::end);
	io::stream_offset lenArchive = this->psArchive->tellg();

	// We still have to perform sanity checks in case the user forced an archive
	// to open even though it failed the signature check.
	if (lenArchive < EPF_HEADER_LEN) throw std::ios::failure("file too short");

	this->psArchive->seekg(4, std::ios::beg); // skip "EPFS" sig

	uint8_t unknown;
	uint16_t numFiles;
	this->psArchive
		>> u32le(this->offFAT)
		>> u8(unknown)
		>> u16le(numFiles);

	if (
		// These two comparisons are sort of redundant, but we need the first
		// one in case the values are so large they wrap and the second one
		// returns an incorrect result.

		// TESTED BY: fmt_epf_lionking_invaliddata_c01
		(this->offFAT > lenArchive) ||

		// TESTED BY: fmt_epf_lionking_invaliddata_c02, when io::stream_offset <= 32bit
		((this->offFAT + numFiles * EPF_FAT_ENTRY_LEN) > lenArchive)
	) {
		throw std::ios::failure("header corrupted or file truncated");
	}
	this->psArchive->seekg(this->offFAT, std::ios::beg);

	io::stream_offset offNext = EPF_FIRST_FILE_OFFSET;
	for (int i = 0; i < numFiles; i++) {
		EPFEntry *fatEntry = new EPFEntry();
		EntryPtr ep(fatEntry);

		fatEntry->iIndex = i;
		fatEntry->iOffset = offNext;
		fatEntry->lenHeader = 0;
		fatEntry->type = FILETYPE_GENERIC;
		fatEntry->fAttr = 0;
		fatEntry->bValid = true;

		uint8_t flags;

		// Read the data in from the FAT entry in the file
		this->psArchive
			>> nullPadded(fatEntry->strName, EPF_FILENAME_FIELD_LEN)
			>> u8(flags)
			>> u32le(fatEntry->iSize)
			>> u32le(fatEntry->decompressedSize);

		if (flags & EPF_FAT_FLAG_COMPRESSED) {
			fatEntry->fAttr |= EA_COMPRESSED;
		}

		this->vcFAT.push_back(ep);
		offNext += fatEntry->iSize;
	}
	// TODO: hidden data after FAT until EOF?
}

EPFArchive::~EPFArchive()
	throw ()
{
}

boost::shared_ptr<std::iostream> EPFArchive::open(const EntryPtr& id)
	throw ()
{
	// TESTED BY: fmt_epf_lionking_open
	assert(this->isValid(id));
	EPFEntry *pEntry = dynamic_cast<EPFEntry *>(id.get());
	assert(pEntry);

	iostream_sptr sub = this->FATArchive::open(id);

	if (pEntry->fAttr & EA_COMPRESSED) {
		try {
			// File needs to be decompressed
			filtering_istream_sptr pinf(new io::filtering_istream());
			pinf->push(lzw_decompress_filter(
				9,   // initial codeword length (in bits)
				14,  // maximum codeword length (in bits)
				256, // first valid codeword
				0,   // EOF codeword is max codeword
				-1,  // reset codeword is max-1
				LZW_BIG_ENDIAN        | // bits are split into bytes in big-endian order
				LZW_NO_BITSIZE_RESET  | // bitsize doesn't go back to 9 after dict reset
				LZW_EOF_PARAM_VALID   | // Has codeword reserved for EOF
				LZW_RESET_PARAM_VALID   // Has codeword reserved for dict reset
			));
			filtering_ostream_sptr poutf(new io::filtering_ostream());
			iostream_sptr dec(new filteredstream(sub, pinf, poutf));
			return dec;
		} catch (ECorruptedData& e) {
			std::cerr << e.what() << "\nReturning file in compressed state."
				<< std::endl;
			sub->seekg(0);
			// continue on to return sub below
		}
	}

	// Else file is not compressed
	return sub;
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
	this->psArchive << nullPadded(strNewName, EPF_FILENAME_FIELD_LEN);
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

FATArchive::FATEntry *EPFArchive::preInsertFile(const FATEntry *idBeforeThis, FATEntry *pNewEntry)
	throw (std::ios::failure)
{
	// TESTED BY: fmt_epf_lionking_insert*
	if (pNewEntry->strName.length() > EPF_MAX_FILENAME_LEN) {
		throw std::ios::failure("maximum filename length is "
			TOSTRING(EPF_MAX_FILENAME_LEN) " chars");
	}

	// Set the format-specific variables
	pNewEntry->lenHeader = 0;

	return pNewEntry;
}

void EPFArchive::postInsertFile(FATEntry *pNewEntry)
	throw (std::ios::failure)
{
	this->offFAT += pNewEntry->iSize;

	this->psArchive->seekp(this->offFAT + pNewEntry->iIndex * EPF_FAT_ENTRY_LEN);
	this->psArchive->insert(EPF_FAT_ENTRY_LEN);
	boost::to_upper(pNewEntry->strName);
	this->psArchive
		<< nullPadded(pNewEntry->strName, EPF_FILENAME_FIELD_LEN)
		<< (uint8_t)0  // 0 == uncompressed, 1 == compressed
		<< u32le(pNewEntry->iSize)  // compressed
		<< u32le(pNewEntry->iSize); // decompressed

	this->updateFATOffset();
	this->updateFileCount(this->vcFAT.size());

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
	this->updateFileCount(this->vcFAT.size() - 1);

	return;
}

FATArchive::FATEntry *EPFArchive::createNewFATEntry()
	throw ()
{
	return new EPFEntry();
}

void EPFArchive::updateFileCount(uint16_t iNewCount)
	throw (std::ios::failure)
{
	// TESTED BY: fmt_epf_lionking_insert*
	// TESTED BY: fmt_epf_lionking_remove*
	this->psArchive->seekp(EPF_FILECOUNT_POS);
	this->psArchive << u16le(iNewCount);
	return;
}

void EPFArchive::updateFATOffset()
	throw (std::ios_base::failure)
{
	// TESTED BY: fmt_epf_lionking_insert*
	// TESTED BY: fmt_epf_lionking_remove*

	this->psArchive->seekp(EPF_FAT_OFFSET_POS);
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
