/*
 * fmt-dat-hocus.cpp - Hocus Pocus .DAT file reader/writer.
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
#include <boost/pointer_cast.hpp>
#include <iostream>
#include <exception>

#include "fmt-dat-hocus.hpp"
#include <camoto/iostream_helpers.hpp>
#include <camoto/debug.hpp>

#define BNK_FIRST_FILE_OFFSET     0

#define DAT_FAT_FILEOFFSET_OFFSET 0
#define DAT_FAT_FILESIZE_OFFSET   4
#define DAT_FAT_ENTRY_LEN         8  // uint32 offset + size

namespace camoto {
namespace gamearchive {

// Truncate function that doesn't do anything (since the FAT is inside the EXE,
// and a fixed size, so there's no need to alter the size of the .exe)
void dummyTrunc(unsigned long len)
{
}

DAT_HocusType::DAT_HocusType()
	throw ()
{
}

DAT_HocusType::~DAT_HocusType()
	throw ()
{
}

std::string DAT_HocusType::getArchiveCode() const
	throw ()
{
	return "dat-hocus";
}

std::string DAT_HocusType::getFriendlyName() const
	throw ()
{
	return "Hocus Pocus DAT File";
}

// Get a list of the known file extensions for this format.
std::vector<std::string> DAT_HocusType::getFileExtensions() const
	throw ()
{
	std::vector<std::string> vcExtensions;
	vcExtensions.push_back("dat");
	return vcExtensions;
}

std::vector<std::string> DAT_HocusType::getGameList() const
	throw ()
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Hocus Pocus");
	return vcGames;
}

E_CERTAINTY DAT_HocusType::isInstance(iostream_sptr psArchive) const
	throw (std::ios::failure)
{
	// There is literally no identifying information in this archive format!
	// TESTED BY: fmt_dat_hocus_isinstance_c00
	return EC_UNSURE;
}

ArchivePtr DAT_HocusType::open(iostream_sptr psArchive, MP_SUPPDATA& suppData) const
	throw (std::ios::failure)
{
	assert(suppData.find(EST_FAT) != suppData.end());
	SuppItem& si = suppData[EST_FAT];
	si.stream->seekg(0, std::ios::end);
	io::stream_offset lenEXE = si.stream->tellg();
	io::stream_offset offFAT, lenFAT;
	switch (lenEXE) {
		case 178592: // shareware v1.0 (untested!)
			offFAT = 0x01EE04;
			lenFAT = 8 * 651;
			break;
		case 179360: // shareware v1.1
			offFAT = 0x01F0E4;
			lenFAT = 8 * 253;
			break;
		case 181872: // registered v1.0
			offFAT = 0x01EEB4;
			lenFAT = 8 * 651;
			break;
		case 182656: // registered v1.1
			offFAT = 0x01F1A4;
			lenFAT = 8 * 652;
			break;
		default:
			throw std::ios::failure("Unknown file version");
	}
	substream_sptr fat(new substream(si.stream, offFAT, lenFAT));
	return ArchivePtr(new DAT_HocusArchive(psArchive, fat));
}

ArchivePtr DAT_HocusType::newArchive(iostream_sptr psArchive, MP_SUPPDATA& suppData) const
	throw (std::ios::failure)
{
	// We can't create new archives because the FAT has to go inside a
	// specific version of an .EXE file, and we wouldn't know where that is!
	throw std::ios::failure("Cannot create archives from scratch in this format!");
}

MP_SUPPLIST DAT_HocusType::getRequiredSupps(const std::string& filenameArchive) const
	throw ()
{
	// No supplemental types/empty list
	MP_SUPPLIST supps;
	std::string filenameBase = filenameArchive.substr(0, filenameArchive.find_last_of('.'));
	supps[EST_FAT] = filenameBase + ".Exe"; // TODO: case sensitivity?
	return supps;
}


DAT_HocusArchive::DAT_HocusArchive(iostream_sptr psArchive, iostream_sptr psFAT)
	throw (std::ios::failure) :
		FATArchive(psArchive, BNK_FIRST_FILE_OFFSET),
		psFAT(new segmented_stream(psFAT)),
		numFiles(0)
{
	this->psArchive->seekg(0, std::ios::end);
	io::stream_offset lenArchive = this->psArchive->tellg();

	this->psFAT->seekg(0, std::ios::end);
	this->maxFiles = this->psFAT->tellg() / DAT_FAT_ENTRY_LEN;
	this->psFAT->seekg(0, std::ios::beg);

	for (int i = 0; i < this->maxFiles; i++) {
		FATEntry *pEntry = new FATEntry();
		pEntry->iIndex = i;
		this->psFAT
			>> u32le(pEntry->iOffset)
			>> u32le(pEntry->iSize);
		;
		pEntry->lenHeader = 0;
		pEntry->type = FILETYPE_GENERIC;
		pEntry->fAttr = 0;
		pEntry->bValid = true;
		this->vcFAT.push_back(EntryPtr(pEntry));

		if (pEntry->iOffset + pEntry->iSize > lenArchive) {
			std::cerr << "DAT file has been truncated, file @" << i
				<< " ends at offset " << pEntry->iOffset + pEntry->iSize
				<< " but the DAT file is only " << lenArchive
				<< " bytes long." << std::endl;
			throw std::ios::failure("archive has been truncated or FAT is corrupt");
		}
		this->numFiles++;
	}
}

DAT_HocusArchive::~DAT_HocusArchive()
	throw ()
{
}

void DAT_HocusArchive::rename(EntryPtr& id, const std::string& strNewName)
	throw (std::ios::failure)
{
	throw std::ios::failure("This archive format does not support filenames.");
}

void DAT_HocusArchive::flush()
	throw (std::ios::failure)
{
	this->FATArchive::flush();

	// Write out to the underlying stream for the supplemental files
	this->psFAT->commit(dummyTrunc);

	return;
}

void DAT_HocusArchive::updateFileOffset(const FATEntry *pid, std::streamsize offDelta)
	throw (std::ios::failure)
{
	// Only the external FAT file has offsets, not the embedded FAT
	this->psFAT->seekp(pid->iIndex * DAT_FAT_ENTRY_LEN + DAT_FAT_FILEOFFSET_OFFSET);
	this->psFAT << u32le(pid->iOffset);
	return;
}

void DAT_HocusArchive::updateFileSize(const FATEntry *pid, std::streamsize sizeDelta)
	throw (std::ios::failure)
{
	// Update external FAT
	this->psFAT->seekp(pid->iIndex * DAT_FAT_ENTRY_LEN + DAT_FAT_FILESIZE_OFFSET);
	this->psFAT << u32le(pid->iSize);

	return;
}

FATArchive::FATEntry *DAT_HocusArchive::preInsertFile(const FATEntry *idBeforeThis, FATEntry *pNewEntry)
	throw (std::ios::failure)
{
	// Make sure FAT hasn't reached maximum size
	if (this->numFiles + 1 >= this->maxFiles) {
		throw std::ios::failure("Maximum number of files reached in this archive format.");
	}

	// Set the format-specific variables
	pNewEntry->lenHeader = 0;

	// Remove the last (empty) entry in the FAT to keep the size fixed
	this->psFAT->seekp(DAT_FAT_ENTRY_LEN, std::ios::end);
	this->psFAT->remove(DAT_FAT_ENTRY_LEN);

	// Insert the new FAT entry
	this->psFAT->seekp(pNewEntry->iIndex * DAT_FAT_ENTRY_LEN);
	this->psFAT->insert(DAT_FAT_ENTRY_LEN);

	// Write out the file size
	this->psFAT << u32le(pNewEntry->iOffset);
	this->psFAT << u32le(pNewEntry->iSize);

	this->numFiles++;

	return pNewEntry;
}

void DAT_HocusArchive::preRemoveFile(const FATEntry *pid)
	throw (std::ios::failure)
{
	// Remove the FAT entry
	this->psFAT->seekp(pid->iIndex * DAT_FAT_ENTRY_LEN);
	this->psFAT->remove(DAT_FAT_ENTRY_LEN);

	// And add space at the end to keep the FAT length fixed
	this->psFAT->seekp(0, std::ios::end);
	this->psFAT->insert(DAT_FAT_ENTRY_LEN);

	this->numFiles--;

	return;
}

} // namespace gamearchive
} // namespace camoto
