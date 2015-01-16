/**
 * @file   fmt-dat-doofus.cpp
 * @brief  Doofus .G-D file reader/writer.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/Doofus_Game_Data_Format
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

#include <camoto/iostream_helpers.hpp>
#include "fmt-gd-doofus.hpp"

#define GD_FIRST_FILE_OFFSET     0

#define GD_FAT_FILESIZE_OFFSET   0
#define GD_FAT_ENTRY_LEN         8

namespace camoto {
namespace gamearchive {

GD_DoofusType::GD_DoofusType()
{
}

GD_DoofusType::~GD_DoofusType()
{
}

std::string GD_DoofusType::getArchiveCode() const
{
	return "gd-doofus";
}

std::string GD_DoofusType::getFriendlyName() const
{
	return "Doofus DAT File";
}

std::vector<std::string> GD_DoofusType::getFileExtensions() const
{
	std::vector<std::string> vcExtensions;
	vcExtensions.push_back("g-d");
	return vcExtensions;
}

std::vector<std::string> GD_DoofusType::getGameList() const
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Doofus");
	return vcGames;
}

ArchiveType::Certainty GD_DoofusType::isInstance(stream::input_sptr psArchive) const
{
	// There is literally no identifying information in this archive format!
	// TESTED BY: fmt_dat_doofus_isinstance_c00
	return Unsure;
}

ArchivePtr GD_DoofusType::open(stream::inout_sptr psArchive, SuppData& suppData) const
{
	assert(suppData.find(SuppItem::FAT) != suppData.end());
	stream::pos lenEXE = suppData[SuppItem::FAT]->size();
	stream::pos offFAT, lenFAT;
	switch (lenEXE) {
		case 580994:
			offFAT = 0x015372;
			lenFAT = 8 * 64;
			break;
		default:
			throw stream::error("Unknown file version");
	}
	stream::sub_sptr fat(new stream::sub());
	fat->open(suppData[SuppItem::FAT], offFAT, lenFAT, preventResize);
	return ArchivePtr(new GD_DoofusArchive(psArchive, fat));
}

ArchivePtr GD_DoofusType::newArchive(stream::inout_sptr psArchive, SuppData& suppData) const
{
	// We can't create new archives because the FAT has to go inside a
	// specific version of an .EXE file, and we wouldn't know where that is!
	throw stream::error("Cannot create archives from scratch in this format!");
}

SuppFilenames GD_DoofusType::getRequiredSupps(stream::input_sptr data,
	const std::string& filenameArchive) const
{
	// No supplemental types/empty list
	SuppFilenames supps;
	std::string filenameBase = filenameArchive.substr(0, filenameArchive.find_last_of('.'));
	supps[SuppItem::FAT] = "doofus.exe";
	return supps;
}


GD_DoofusArchive::GD_DoofusArchive(stream::inout_sptr psArchive, stream::inout_sptr psFAT)
	:	FATArchive(psArchive, GD_FIRST_FILE_OFFSET, 0),
		psFAT(new stream::seg()),
		numFiles(0)
{
	assert(psFAT);
	this->psFAT->open(psFAT);

	stream::pos lenArchive = this->psArchive->size();

	this->psFAT->seekg(0, stream::end);
	this->maxFiles = this->psFAT->tellg() / GD_FAT_ENTRY_LEN;
	this->psFAT->seekg(0, stream::start);

	stream::len off = 0;
	uint16_t type;
	for (unsigned int i = 0; i < this->maxFiles; i++) {
		FATEntry *pEntry = new FATEntry();
		pEntry->iIndex = i;
		this->psFAT
			>> u16le(pEntry->storedSize)
			>> u16le(type)
		;
		this->psFAT->seekg(4, stream::cur);
		pEntry->lenHeader = 0;
		switch (type) {
			case 0x1636: pEntry->type = "unknown/doofus-1636"; break;
			case 0x2376: pEntry->type = "unknown/doofus-2376"; break;
			case 0x3276: pEntry->type = "unknown/doofus-3276"; break;
			case 0x3F2E: pEntry->type = "unknown/doofus-3f2e"; break;
			case 0x3F64: pEntry->type = "unknown/doofus-3f64"; break;
			case 0x48BE: pEntry->type = "unknown/doofus-48be"; break;
			case 0x43EE: pEntry->type = "unknown/doofus-43ee"; break;
			case 0x59EE: pEntry->type = "music/tbsa"; break;
			default: pEntry->type = FILETYPE_GENERIC; break;
		}
		pEntry->fAttr = 0;
		pEntry->bValid = true;
		pEntry->realSize = pEntry->storedSize;
		pEntry->iOffset = off;
		off += pEntry->storedSize;
		this->vcFAT.push_back(EntryPtr(pEntry));

		if (pEntry->iOffset + pEntry->storedSize > lenArchive) {
			std::cerr << "G-D file has been truncated, file @" << i
				<< " ends at offset " << pEntry->iOffset + pEntry->storedSize
				<< " but the G-D file is only " << lenArchive
				<< " bytes long." << std::endl;
			throw stream::error("archive has been truncated or FAT is corrupt");
		}
		this->numFiles++;
	}
}

GD_DoofusArchive::~GD_DoofusArchive()
{
}

void GD_DoofusArchive::flush()
{
	this->FATArchive::flush();

	// Write out to the underlying stream for the supplemental files
	this->psFAT->flush();

	return;
}

void GD_DoofusArchive::updateFileName(const FATEntry *pid, const std::string& strNewName)
{
	throw stream::error("This archive format does not support filenames.");
}

void GD_DoofusArchive::updateFileOffset(const FATEntry *pid, stream::delta offDelta)
{
	// Nothing to do, offsets aren't stored
	return;
}

void GD_DoofusArchive::updateFileSize(const FATEntry *pid, stream::delta sizeDelta)
{
	// Update external FAT
	this->psFAT->seekp(pid->iIndex * GD_FAT_ENTRY_LEN + GD_FAT_FILESIZE_OFFSET, stream::start);
	this->psFAT << u32le(pid->storedSize);

	return;
}

FATArchive::FATEntry *GD_DoofusArchive::preInsertFile(const FATEntry *idBeforeThis, FATEntry *pNewEntry)
{
	// Make sure FAT hasn't reached maximum size
	if (this->numFiles + 1 >= this->maxFiles) {
		throw stream::error("Maximum number of files reached in this archive format.");
	}

	// Set the format-specific variables
	pNewEntry->lenHeader = 0;

	// Remove the last (empty) entry in the FAT to keep the size fixed
	this->psFAT->seekp(GD_FAT_ENTRY_LEN, stream::end);
	this->psFAT->remove(GD_FAT_ENTRY_LEN);

	// Insert the new FAT entry
	this->psFAT->seekp(pNewEntry->iIndex * GD_FAT_ENTRY_LEN, stream::start);
	this->psFAT->insert(GD_FAT_ENTRY_LEN);

	// Write out the file size
	this->psFAT << u16le(pNewEntry->storedSize);

	this->numFiles++;

	return pNewEntry;
}

void GD_DoofusArchive::preRemoveFile(const FATEntry *pid)
{
	// Remove the FAT entry
	this->psFAT->seekp(pid->iIndex * GD_FAT_ENTRY_LEN, stream::start);
	this->psFAT->remove(GD_FAT_ENTRY_LEN);

	// And add space at the end to keep the FAT length fixed
	this->psFAT->seekp(0, stream::end);
	this->psFAT->insert(GD_FAT_ENTRY_LEN);

	this->numFiles--;

	return;
}

} // namespace gamearchive
} // namespace camoto
