/**
 * @file   fmt-dat-hocus.cpp
 * @brief  Hocus Pocus .DAT file reader/writer.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/BNK_Format
 *
 * Copyright (C) 2010-2011 Adam Nielsen <malvineous@shikadi.net>
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

#include <camoto/iostream_helpers.hpp>
#include "fmt-dat-hocus.hpp"

#define DAT_FIRST_FILE_OFFSET     0

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

ArchiveType::Certainty DAT_HocusType::isInstance(stream::input_sptr psArchive) const
	throw (stream::error)
{
	// There is literally no identifying information in this archive format!
	// TESTED BY: fmt_dat_hocus_isinstance_c00
	return Unsure;
}

ArchivePtr DAT_HocusType::open(stream::inout_sptr psArchive, SuppData& suppData) const
	throw (stream::error)
{
	assert(suppData.find(SuppItem::FAT) != suppData.end());
	stream::pos lenEXE = suppData[SuppItem::FAT]->size();
	stream::pos offFAT, lenFAT;
	switch (lenEXE) {
		case 178592: // shareware v1.0
			offFAT = 0x01EE04;
			lenFAT = 8 * 252;
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
			throw stream::error("Unknown file version");
	}
	stream::sub_sptr fat(new stream::sub());
	fat->open(suppData[SuppItem::FAT], offFAT, lenFAT, preventResize);
	return ArchivePtr(new DAT_HocusArchive(psArchive, fat));
}

ArchivePtr DAT_HocusType::newArchive(stream::inout_sptr psArchive, SuppData& suppData) const
	throw (stream::error)
{
	// We can't create new archives because the FAT has to go inside a
	// specific version of an .EXE file, and we wouldn't know where that is!
	throw stream::error("Cannot create archives from scratch in this format!");
}

SuppFilenames DAT_HocusType::getRequiredSupps(stream::input_sptr data,
	const std::string& filenameArchive) const
	throw ()
{
	// No supplemental types/empty list
	SuppFilenames supps;
	std::string filenameBase = filenameArchive.substr(0, filenameArchive.find_last_of('.'));
	supps[SuppItem::FAT] = filenameBase + ".exe";
	return supps;
}


DAT_HocusArchive::DAT_HocusArchive(stream::inout_sptr psArchive, stream::inout_sptr psFAT)
	throw (stream::error) :
		FATArchive(psArchive, DAT_FIRST_FILE_OFFSET, 0),
		psFAT(new stream::seg()),
		numFiles(0)
{
	assert(psFAT);
	this->psFAT->open(psFAT);

	stream::pos lenArchive = this->psArchive->size();

	this->psFAT->seekg(0, stream::end);
	this->maxFiles = this->psFAT->tellg() / DAT_FAT_ENTRY_LEN;
	this->psFAT->seekg(0, stream::start);

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
			throw stream::error("archive has been truncated or FAT is corrupt");
		}
		this->numFiles++;
	}
}

DAT_HocusArchive::~DAT_HocusArchive()
	throw ()
{
}

void DAT_HocusArchive::flush()
	throw (stream::error)
{
	this->FATArchive::flush();

	// Write out to the underlying stream for the supplemental files
	this->psFAT->flush();

	return;
}

void DAT_HocusArchive::updateFileName(const FATEntry *pid, const std::string& strNewName)
	throw (stream::error)
{
	throw stream::error("This archive format does not support filenames.");
}

void DAT_HocusArchive::updateFileOffset(const FATEntry *pid, stream::delta offDelta)
	throw (stream::error)
{
	// Only the external FAT file has offsets, not the embedded FAT
	this->psFAT->seekp(pid->iIndex * DAT_FAT_ENTRY_LEN + DAT_FAT_FILEOFFSET_OFFSET, stream::start);
	this->psFAT << u32le(pid->iOffset);
	return;
}

void DAT_HocusArchive::updateFileSize(const FATEntry *pid, stream::delta sizeDelta)
	throw (stream::error)
{
	// Update external FAT
	this->psFAT->seekp(pid->iIndex * DAT_FAT_ENTRY_LEN + DAT_FAT_FILESIZE_OFFSET, stream::start);
	this->psFAT << u32le(pid->iSize);

	return;
}

FATArchive::FATEntry *DAT_HocusArchive::preInsertFile(const FATEntry *idBeforeThis, FATEntry *pNewEntry)
	throw (stream::error)
{
	// Make sure FAT hasn't reached maximum size
	if (this->numFiles + 1 >= this->maxFiles) {
		throw stream::error("Maximum number of files reached in this archive format.");
	}

	// Set the format-specific variables
	pNewEntry->lenHeader = 0;

	// Remove the last (empty) entry in the FAT to keep the size fixed
	this->psFAT->seekp(DAT_FAT_ENTRY_LEN, stream::end);
	this->psFAT->remove(DAT_FAT_ENTRY_LEN);

	// Insert the new FAT entry
	this->psFAT->seekp(pNewEntry->iIndex * DAT_FAT_ENTRY_LEN, stream::start);
	this->psFAT->insert(DAT_FAT_ENTRY_LEN);

	// Write out the file size
	this->psFAT << u32le(pNewEntry->iOffset);
	this->psFAT << u32le(pNewEntry->iSize);

	this->numFiles++;

	return pNewEntry;
}

void DAT_HocusArchive::preRemoveFile(const FATEntry *pid)
	throw (stream::error)
{
	// Remove the FAT entry
	this->psFAT->seekp(pid->iIndex * DAT_FAT_ENTRY_LEN, stream::start);
	this->psFAT->remove(DAT_FAT_ENTRY_LEN);

	// And add space at the end to keep the FAT length fixed
	this->psFAT->seekp(0, stream::end);
	this->psFAT->insert(DAT_FAT_ENTRY_LEN);

	this->numFiles--;

	return;
}

} // namespace gamearchive
} // namespace camoto
