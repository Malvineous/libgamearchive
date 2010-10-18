/**
 * @file   fmt-lbr-vinyl.cpp
 * @brief  Implementation of Vinyl Goddess From Mars .LBR file reader/writer.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/LBR_Format
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

#include "fmt-lbr-vinyl.hpp"
#include <camoto/iostream_helpers.hpp>
#include <camoto/debug.hpp>

#define LBR_FILECOUNT_OFFSET    0
#define LBR_HEADER_LEN          2  // u16le file count
#define LBR_FAT_OFFSET          LBR_HEADER_LEN
#define LBR_FAT_ENTRY_LEN       6  // u16le hash + u32le size
#define LBR_FIRST_FILE_OFFSET   LBR_FAT_OFFSET  // empty archive only

#define LBR_FATENTRY_OFFSET(e) (LBR_HEADER_LEN + e->iIndex * LBR_FAT_ENTRY_LEN)

#define LBR_FILEHASH_OFFSET(e)    LBR_FATENTRY_OFFSET(e)
#define LBR_FILEOFFSET_OFFSET(e) (LBR_FATENTRY_OFFSET(e) + 2)

namespace camoto {
namespace gamearchive {

LBRType::LBRType()
	throw ()
{
}

LBRType::~LBRType()
	throw ()
{
}

std::string LBRType::getArchiveCode() const
	throw ()
{
	return "lbr-vinyl";
}

std::string LBRType::getFriendlyName() const
	throw ()
{
	return "Vinyl Goddess From Mars Library";
}

// Get a list of the known file extensions for this format.
std::vector<std::string> LBRType::getFileExtensions() const
	throw ()
{
	std::vector<std::string> vcExtensions;
	vcExtensions.push_back("lbr");
	return vcExtensions;
}

std::vector<std::string> LBRType::getGameList() const
	throw ()
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Vinyl Goddess From Mars");
	return vcGames;
}

E_CERTAINTY LBRType::isInstance(iostream_sptr psArchive) const
	throw (std::ios::failure)
{
	psArchive->seekg(0, std::ios::end);
	io::stream_offset lenArchive = psArchive->tellg();

	// TESTED BY: fmt_lbr_vinyl_isinstance_c01
	if (lenArchive < LBR_HEADER_LEN) return EC_DEFINITELY_NO; // too short

	psArchive->seekg(0, std::ios::beg);

	uint32_t numFiles;
	psArchive >> u16le(numFiles);
	
	uint16_t hash;
	uint32_t offset;
	while (numFiles--) {
		psArchive
			>> u16le(hash)
			>> u32le(offset)
		;
		// TESTED BY: fmt_lbr_vinyl_isinstance_c02
		if (offset > lenArchive) return EC_DEFINITELY_NO;
	}

	// TESTED BY: fmt_lbr_vinyl_isinstance_c00
	return EC_DEFINITELY_YES;
}

ArchivePtr LBRType::newArchive(iostream_sptr psArchive, MP_SUPPDATA& suppData) const
	throw (std::ios::failure)
{
	psArchive->seekp(0, std::ios::beg);
	psArchive->write("\x00\x00", 2);
	return ArchivePtr(new LBRArchive(psArchive));
}

// Preconditions: isInstance() has returned > EC_DEFINITELY_NO
ArchivePtr LBRType::open(iostream_sptr psArchive, MP_SUPPDATA& suppData) const
	throw (std::ios::failure)
{
	return ArchivePtr(new LBRArchive(psArchive));
}

MP_SUPPLIST LBRType::getRequiredSupps(const std::string& filenameArchive) const
	throw ()
{
	// No supplemental types/empty list
	return MP_SUPPLIST();
}


LBRArchive::LBRArchive(iostream_sptr psArchive)
	throw (std::ios::failure) :
		FATArchive(psArchive, LBR_FIRST_FILE_OFFSET)
{
	psArchive->seekg(0, std::ios::end);
	io::stream_offset lenArchive = psArchive->tellg();

	if (lenArchive < LBR_HEADER_LEN) throw std::ios::failure("file too short");

	this->psArchive->seekg(0, std::ios::beg);

	uint32_t numFiles;
	this->psArchive >> u16le(numFiles);

	if (numFiles > 0) {
		uint32_t offNext, offCur;
		uint16_t hashNext, hashCur; // TODO: store in new LBREntry class
		this->psArchive
			>> u16le(hashCur)
			>> u32le(offCur)
		;
		for (int i = 0; i < numFiles; i++) {
			// Read the data in from the FAT entry in the file
			if (i == numFiles - 1) {
				// Last entry has no 'next' one, so fake it as if next entry is EOF
				offNext = lenArchive;
			} else {
				this->psArchive
					>> u16le(hashNext)
					>> u32le(offNext)
				;
			}

			FATEntry *fatEntry = new FATEntry();
			EntryPtr ep(fatEntry);

			fatEntry->iIndex = i;
			fatEntry->lenHeader = 0;
			fatEntry->iOffset = offCur;
			fatEntry->iSize = offNext - offCur;
			fatEntry->type = FILETYPE_GENERIC;
			fatEntry->fAttr = 0;
			fatEntry->bValid = true;


			this->vcFAT.push_back(ep);
			offCur = offNext;
			hashCur = hashNext;
		}
	}
}

LBRArchive::~LBRArchive()
	throw ()
{
}

// Does not invalidate existing EntryPtrs
void LBRArchive::rename(EntryPtr& id, const std::string& strNewName)
	throw (std::ios_base::failure)
{
	throw std::ios::failure("This archive format does not support filenames.");
}

void LBRArchive::updateFileOffset(const FATEntry *pid, std::streamsize offDelta)
	throw (std::ios::failure)
{
	this->psArchive->seekp(LBR_FILEOFFSET_OFFSET(pid));
	this->psArchive << u32le(pid->iOffset);
	return;
}

void LBRArchive::updateFileSize(const FATEntry *pid, std::streamsize sizeDelta)
	throw (std::ios::failure)
{
	return;
}

FATArchive::FATEntry *LBRArchive::preInsertFile(const FATEntry *idBeforeThis, FATEntry *pNewEntry)
	throw (std::ios::failure)
{
	// TESTED BY: fmt_lbr_vinyl_insert*

	// Set the format-specific variables
	pNewEntry->lenHeader = 0;

	// Because the new entry isn't in the vector yet we need to shift it manually
	pNewEntry->iOffset += LBR_FAT_ENTRY_LEN;

	this->psArchive->seekp(LBR_FATENTRY_OFFSET(pNewEntry));
	this->psArchive->insert(LBR_FAT_ENTRY_LEN);

	this->psArchive
		<< u16le(0) // TODO: hash
		<< u32le(pNewEntry->iOffset)
	;

	// Update the offsets now there's a new FAT entry taking up space.
	this->shiftFiles(LBR_FAT_OFFSET + this->vcFAT.size() * LBR_FAT_ENTRY_LEN, LBR_FAT_ENTRY_LEN, 0);

	this->updateFileCount(this->vcFAT.size() + 1);
	return pNewEntry;
}

void LBRArchive::preRemoveFile(const FATEntry *pid)
	throw (std::ios::failure)
{
	// TESTED BY: fmt_lbr_vinyl_remove*

	// Update the offsets now there's one less FAT entry taking up space.  This
	// must be called before the FAT is altered, because it will write a new
	// offset into the FAT entry we're about to erase (and if we erase it first
	// it'll overwrite something else.)
	this->shiftFiles(LBR_FAT_OFFSET + this->vcFAT.size() * LBR_FAT_ENTRY_LEN, -LBR_FAT_ENTRY_LEN, 0);

	this->psArchive->seekp(LBR_FATENTRY_OFFSET(pid));
	this->psArchive->remove(LBR_FAT_ENTRY_LEN);

	this->updateFileCount(this->vcFAT.size() - 1);
	return;
}

void LBRArchive::updateFileCount(uint32_t iNewCount)
	throw (std::ios_base::failure)
{
	// TESTED BY: fmt_lbr_vinyl_insert*
	// TESTED BY: fmt_lbr_vinyl_remove*
	this->psArchive->seekp(LBR_FILECOUNT_OFFSET);
	this->psArchive << u16le(iNewCount);
	return;
}

} // namespace gamearchive
} // namespace camoto
