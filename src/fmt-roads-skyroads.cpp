/**
 * @file   fmt-roads-skyroads.cpp
 * @brief  Implementation of Skyroads roads.lzs file reader/writer.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/SkyRoads_level_format
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

#include "fmt-roads-skyroads.hpp"

#define SRR_FAT_ENTRY_LEN     4  // u16le offset + u16le size
#define SRR_FIRST_FILE_OFFSET 0

namespace camoto {
namespace gamearchive {

SkyRoadsRoadsType::SkyRoadsRoadsType()
	throw ()
{
}

SkyRoadsRoadsType::~SkyRoadsRoadsType()
	throw ()
{
}

std::string SkyRoadsRoadsType::getArchiveCode() const
	throw ()
{
	return "roads-skyroads";
}

std::string SkyRoadsRoadsType::getFriendlyName() const
	throw ()
{
	return "SkyRoads Roads File";
}

std::vector<std::string> SkyRoadsRoadsType::getFileExtensions() const
	throw ()
{
	std::vector<std::string> vcExtensions;
	vcExtensions.push_back("lzs");
	return vcExtensions;
}

std::vector<std::string> SkyRoadsRoadsType::getGameList() const
	throw ()
{
	std::vector<std::string> vcGames;
	vcGames.push_back("SkyRoads");
	return vcGames;
}

ArchiveType::Certainty SkyRoadsRoadsType::isInstance(stream::inout_sptr psArchive) const
	throw (stream::error)
{
	stream::pos lenArchive = psArchive->size();
	// An empty file is valid as an archive with no files (since this format
	// lacks a header.)
	// TESTED BY: fmt_skyroads_roads_isinstance_c01
	if (lenArchive == 0) return DefinitelyYes;

	psArchive->seekg(0, stream::start);
	uint16_t lenFAT;
	psArchive >> u16le(lenFAT);

	// If the FAT is larger than the entire archive then it's not a SkyRoads roads file
	// TESTED BY: fmt_skyroads_roads_isinstance_c02
	if (lenFAT > lenArchive) return DefinitelyNo;

	// If the FAT is smaller than a single entry then it's not a SkyRoads roads file
	// TESTED BY: fmt_skyroads_roads_isinstance_c03
	if (lenFAT < SRR_FAT_ENTRY_LEN) return DefinitelyNo;

	// The FAT is not an even multiple of FAT entries.
	// TESTED BY: fmt_skyroads_roads_isinstance_c04
	if (lenFAT % SRR_FAT_ENTRY_LEN) return DefinitelyNo;

	// Check each FAT entry
	psArchive->seekg(0, stream::start);
	uint16_t offPrev = 0;
	for (int i = 0; i < lenFAT / SRR_FAT_ENTRY_LEN; i++) {

		uint16_t offEntry, lenDecomp;
		psArchive >> u16le(offEntry) >> u16le(lenDecomp);

		// If a file entry points past the end of the archive then it's an invalid
		// format.
		// TESTED BY: fmt_skyroads_roads_isinstance_c05
		if (offEntry > lenArchive) return DefinitelyNo;

		// Offsets must increase or we'll get a negative file size.
		// TESTED BY: fmt_skyroads_roads_isinstance_c06
		if (offEntry < offPrev) return DefinitelyNo;

		offPrev = offEntry;
	}

	// TODO: Make trailing data available as hidden data or metadata?
	// TODO: What about data in between files?

	// TESTED BY: fmt_skyroads_roads_isinstance_c00
	return DefinitelyYes;
}

ArchivePtr SkyRoadsRoadsType::newArchive(stream::inout_sptr psArchive, SuppData& suppData) const
	throw (stream::error)
{
	return ArchivePtr(new SkyRoadsRoadsArchive(psArchive));
}

ArchivePtr SkyRoadsRoadsType::open(stream::inout_sptr psArchive, SuppData& suppData) const
	throw (stream::error)
{
	return ArchivePtr(new SkyRoadsRoadsArchive(psArchive));
}

SuppFilenames SkyRoadsRoadsType::getRequiredSupps(const std::string& filenameArchive) const
	throw ()
{
	// No supplemental types/empty list
	return SuppFilenames();
}


SkyRoadsRoadsArchive::SkyRoadsRoadsArchive(stream::inout_sptr psArchive)
	throw (stream::error) :
		FATArchive(psArchive, SRR_FIRST_FILE_OFFSET, 0)
{
	stream::pos lenArchive = this->psArchive->size();
	if (lenArchive > 0) {
		this->psArchive->seekg(0, stream::start);
		uint16_t offCur;
		this->psArchive >> u16le(offCur);

		int numFiles = offCur / SRR_FAT_ENTRY_LEN;
		this->vcFAT.reserve(numFiles);

		for (int i = 0; i < numFiles; i++) {
			FATEntry *fatEntry = new FATEntry();
			EntryPtr ep(fatEntry);

			uint16_t lenDecomp, offNext;
			this->psArchive >> u16le(lenDecomp);
			if (i < numFiles - 1) {
				this->psArchive >> u16le(offNext);
			} else {
				offNext = lenArchive;
			}

			fatEntry->iOffset = offCur;
			fatEntry->iSize = offNext - offCur;
			fatEntry->iIndex = i;
			fatEntry->lenHeader = 0;
			fatEntry->type = "map/skyroads";
			fatEntry->fAttr = 0;
			fatEntry->bValid = true;
			this->vcFAT.push_back(ep);

			offCur = offNext;
		}
	} // else empty archive
}

SkyRoadsRoadsArchive::~SkyRoadsRoadsArchive()
	throw ()
{
}

void SkyRoadsRoadsArchive::updateFileName(const FATEntry *pid, const std::string& strNewName)
	throw (stream::error)
{
	throw stream::error("This format does not have any filenames.");
}

void SkyRoadsRoadsArchive::updateFileOffset(const FATEntry *pid, stream::delta offDelta)
	throw (stream::error)
{
	// TESTED BY: fmt_skyroads_roads_insert*
	// TESTED BY: fmt_skyroads_roads_resize*
	this->psArchive->seekp(pid->iIndex * SRR_FAT_ENTRY_LEN, stream::start);
	this->psArchive << u16le(pid->iOffset);
	return;
}

void SkyRoadsRoadsArchive::updateFileSize(const FATEntry *pid, stream::delta sizeDelta)
	throw (stream::error)
{
	// TESTED BY: fmt_skyroads_roads_insert*
	// TESTED BY: fmt_skyroads_roads_resize*
	this->psArchive->seekp(pid->iIndex * SRR_FAT_ENTRY_LEN + 2, stream::start);
	this->psArchive << u16le(pid->iSize);
	return;
}

FATArchive::FATEntry *SkyRoadsRoadsArchive::preInsertFile(const FATEntry *idBeforeThis, FATEntry *pNewEntry)
	throw (stream::error)
{
	// TESTED BY: fmt_skyroads_roads_insert*

	// Set the format-specific variables
	pNewEntry->lenHeader = 0;

	// Because the new entry isn't in the vector yet we need to shift it manually
	pNewEntry->iOffset += SRR_FAT_ENTRY_LEN;

	this->psArchive->seekp(pNewEntry->iIndex * SRR_FAT_ENTRY_LEN, stream::start);
	this->psArchive->insert(SRR_FAT_ENTRY_LEN);

	// Write out the entry
	this->psArchive
		<< u16le(pNewEntry->iOffset)
		<< u16le(pNewEntry->iSize)
	;

	// Update the offsets now there's a new FAT entry taking up space.
	this->shiftFiles(
		NULL,
		this->vcFAT.size() * SRR_FAT_ENTRY_LEN,
		SRR_FAT_ENTRY_LEN,
		0
	);

	return pNewEntry;
}

void SkyRoadsRoadsArchive::preRemoveFile(const FATEntry *pid)
	throw (stream::error)
{
	// TESTED BY: fmt_skyroads_roads_remove*

	// Update the offsets now there's one less FAT entry taking up space.  This
	// must be called before the FAT is altered, because it will write a new
	// offset into the FAT entry we're about to erase (and if we erase it first
	// it'll overwrite something else.)
	this->shiftFiles(
		NULL,
		this->vcFAT.size() * SRR_FAT_ENTRY_LEN,
		-SRR_FAT_ENTRY_LEN,
		0
	);

	this->psArchive->seekp(pid->iIndex * SRR_FAT_ENTRY_LEN, stream::start);
	this->psArchive->remove(SRR_FAT_ENTRY_LEN);
	return;
}

} // namespace gamearchive
} // namespace camoto
