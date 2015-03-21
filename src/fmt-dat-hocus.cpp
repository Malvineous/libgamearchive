/**
 * @file  fmt-dat-hocus.cpp
 * @brief Hocus Pocus .DAT file reader/writer.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/DAT_Format_%28Hocus Pocus%29
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

#include <cassert>
#include <camoto/iostream_helpers.hpp>
#include <camoto/util.hpp>
#include <camoto/gamearchive/util.hpp>
#include "fmt-dat-hocus.hpp"

#define DAT_FIRST_FILE_OFFSET     0

#define DAT_FAT_FILEOFFSET_OFFSET 0
#define DAT_FAT_FILESIZE_OFFSET   4
#define DAT_FAT_ENTRY_LEN         8  // uint32 offset + size

namespace camoto {
namespace gamearchive {

ArchiveType_DAT_Hocus::ArchiveType_DAT_Hocus()
{
}

ArchiveType_DAT_Hocus::~ArchiveType_DAT_Hocus()
{
}

std::string ArchiveType_DAT_Hocus::code() const
{
	return "dat-hocus";
}

std::string ArchiveType_DAT_Hocus::friendlyName() const
{
	return "Hocus Pocus DAT File";
}

std::vector<std::string> ArchiveType_DAT_Hocus::fileExtensions() const
{
	std::vector<std::string> vcExtensions;
	vcExtensions.push_back("dat");
	return vcExtensions;
}

std::vector<std::string> ArchiveType_DAT_Hocus::games() const
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Hocus Pocus");
	return vcGames;
}

ArchiveType::Certainty ArchiveType_DAT_Hocus::isInstance(
	stream::input& content) const
{
	// There is literally no identifying information in this archive format!
	// TESTED BY: fmt_dat_hocus_isinstance_c00
	return Unsure;
}

std::shared_ptr<Archive> ArchiveType_DAT_Hocus::open(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	assert(suppData.find(SuppItem::FAT) != suppData.end());
	stream::pos lenEXE = suppData[SuppItem::FAT]->size();
	stream::pos offFAT, lenFAT;
	switch (lenEXE) {
		case 218096: // leaked beta
			offFAT = 0x01AD74;
			lenFAT = 8 * 236;
			break;
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
		case 8 * 16: // test code
			offFAT = 0;
			lenFAT = 8 * 16;
			break;
		default:
			throw stream::error("Unknown file version");
	}
	return std::make_shared<Archive_DAT_Hocus>(
		std::move(content),
		std::make_unique<stream::sub>(
			std::move(suppData[SuppItem::FAT]), offFAT, lenFAT, preventResize
		)
	);
}

std::shared_ptr<Archive> ArchiveType_DAT_Hocus::create(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	// We can't create new archives because the FAT has to go inside a
	// specific version of an .EXE file, and we wouldn't know where that is!
	throw stream::error("Cannot create archives from scratch in this format!");
}

SuppFilenames ArchiveType_DAT_Hocus::getRequiredSupps(stream::input& content,
	const std::string& filenameArchive) const
{
	// No supplemental types/empty list
	SuppFilenames supps;
	std::string filenameBase = filenameArchive.substr(0,
		filenameArchive.find_last_of('.'));
	supps[SuppItem::FAT] = filenameBase + ".exe";
	return supps;
}


Archive_DAT_Hocus::Archive_DAT_Hocus(std::unique_ptr<stream::inout> content,
	std::unique_ptr<stream::inout> psFAT)
	:	Archive_FAT(std::move(content), DAT_FIRST_FILE_OFFSET, 0),
		psFAT(std::make_unique<stream::seg>(std::move(psFAT))),
		numFiles(0)
{
	stream::pos lenArchive = this->content->size();

	this->maxFiles = this->psFAT->size() / DAT_FAT_ENTRY_LEN;
	this->psFAT->seekg(0, stream::start);

	for (unsigned int i = 0; i < this->maxFiles; i++) {
		auto f = this->createNewFATEntry();
		f->iIndex = i;
		*this->psFAT
			>> u32le(f->iOffset)
			>> u32le(f->storedSize);
		;
		if ((f->iOffset == 0) && (f->storedSize == 0)) continue;
		f->lenHeader = 0;
		f->type = FILETYPE_GENERIC;
		f->fAttr = File::Attribute::Default;
		f->bValid = true;
		f->realSize = f->storedSize;

		if (f->iOffset + f->storedSize > lenArchive) {
			std::cerr << "DAT file has been truncated, file @" << i
				<< " ends at offset " << f->iOffset + f->storedSize
				<< " but the DAT file is only " << lenArchive
				<< " bytes long." << std::endl;
			throw stream::error("archive has been truncated or FAT is corrupt");
		}
		this->vcFAT.push_back(std::move(f));
		this->numFiles++;
	}
}

Archive_DAT_Hocus::~Archive_DAT_Hocus()
{
}

void Archive_DAT_Hocus::flush()
{
	this->Archive_FAT::flush();

	// Write out to the underlying stream for the supplemental files
	this->psFAT->flush();

	return;
}

void Archive_DAT_Hocus::updateFileOffset(const FATEntry *pid, stream::delta offDelta)
{
	// Only the external FAT file has offsets, not the embedded FAT
	this->psFAT->seekp(pid->iIndex * DAT_FAT_ENTRY_LEN + DAT_FAT_FILEOFFSET_OFFSET, stream::start);
	*this->psFAT << u32le(pid->iOffset);
	return;
}

void Archive_DAT_Hocus::updateFileSize(const FATEntry *pid, stream::delta sizeDelta)
{
	// Update external FAT
	this->psFAT->seekp(pid->iIndex * DAT_FAT_ENTRY_LEN + DAT_FAT_FILESIZE_OFFSET, stream::start);
	*this->psFAT << u32le(pid->storedSize);
	return;
}

void Archive_DAT_Hocus::preInsertFile(const FATEntry *idBeforeThis, FATEntry *pNewEntry)
{
	// Make sure FAT hasn't reached maximum size
	if (this->numFiles + 1 >= this->maxFiles) {
		throw stream::error("Maximum number of files reached in this archive format.");
	}

	// Set the format-specific variables
	pNewEntry->lenHeader = 0;

	// Remove the last (empty) entry in the FAT to keep the size fixed
	this->psFAT->seekp(-DAT_FAT_ENTRY_LEN, stream::end);
	this->psFAT->remove(DAT_FAT_ENTRY_LEN);

	// Insert the new FAT entry
	this->psFAT->seekp(pNewEntry->iIndex * DAT_FAT_ENTRY_LEN, stream::start);
	this->psFAT->insert(DAT_FAT_ENTRY_LEN);

	// Write out the file size
	*this->psFAT
		<< u32le(pNewEntry->iOffset)
		<< u32le(pNewEntry->storedSize)
	;

	this->numFiles++;
	return;
}

void Archive_DAT_Hocus::preRemoveFile(const FATEntry *pid)
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
