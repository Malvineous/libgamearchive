/**
 * @file  fmt-roads-skyroads.cpp
 * @brief Implementation of Skyroads roads.lzs file reader/writer.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/SkyRoads_level_format
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
#include <camoto/util.hpp> // std::make_unique
#include "fmt-roads-skyroads.hpp"

#define SRR_FAT_ENTRY_LEN     4  // u16le offset + u16le size
#define SRR_FIRST_FILE_OFFSET 0

namespace camoto {
namespace gamearchive {

ArchiveType_Roads_SkyRoads::ArchiveType_Roads_SkyRoads()
{
}

ArchiveType_Roads_SkyRoads::~ArchiveType_Roads_SkyRoads()
{
}

std::string ArchiveType_Roads_SkyRoads::code() const
{
	return "roads-skyroads";
}

std::string ArchiveType_Roads_SkyRoads::friendlyName() const
{
	return "SkyRoads Roads File";
}

std::vector<std::string> ArchiveType_Roads_SkyRoads::fileExtensions() const
{
	std::vector<std::string> vcExtensions;
	vcExtensions.push_back("lzs");
	return vcExtensions;
}

std::vector<std::string> ArchiveType_Roads_SkyRoads::games() const
{
	std::vector<std::string> vcGames;
	vcGames.push_back("SkyRoads");
	return vcGames;
}

ArchiveType::Certainty ArchiveType_Roads_SkyRoads::isInstance(
	stream::input& content) const
{
	stream::pos lenArchive = content.size();
	// An empty file is valid as an archive with no files (since this format
	// lacks a header.)
	// TESTED BY: fmt_skyroads_roads_isinstance_c01
	if (lenArchive == 0) return DefinitelyYes;

	content.seekg(0, stream::start);
	uint16_t lenFAT;
	content >> u16le(lenFAT);

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
	content.seekg(0, stream::start);
	uint16_t offPrev = 0;
	for (int i = 0; i < lenFAT / SRR_FAT_ENTRY_LEN; i++) {

		uint16_t offEntry, lenDecomp;
		content >> u16le(offEntry) >> u16le(lenDecomp);

		// If a file entry points past the end of the archive then it's an invalid
		// format.
		// TESTED BY: fmt_skyroads_roads_isinstance_c05
		if (offEntry > lenArchive) return DefinitelyNo;

		// Offsets must increase or we'll get a negative file size.
		// TESTED BY: fmt_skyroads_roads_isinstance_c06
		if (offEntry < offPrev) return DefinitelyNo;

		// Assume files cannot be zero length.  This helps avoid false positives
		// with Sango .dat files.
		if (lenDecomp == 0) return DefinitelyNo;

		offPrev = offEntry;
	}

	// TODO: Make trailing data available as hidden data or metadata?
	// TODO: What about data in between files?

	// TESTED BY: fmt_skyroads_roads_isinstance_c00
	return DefinitelyYes;
}

std::unique_ptr<Archive> ArchiveType_Roads_SkyRoads::create(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	return std::make_unique<Archive_Roads_SkyRoads>(std::move(content));
}

std::unique_ptr<Archive> ArchiveType_Roads_SkyRoads::open(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	return std::make_unique<Archive_Roads_SkyRoads>(std::move(content));
}

SuppFilenames ArchiveType_Roads_SkyRoads::getRequiredSupps(stream::input& content,
	const std::string& filenameArchive) const
{
	// No supplemental types/empty list
	return SuppFilenames();
}


Archive_Roads_SkyRoads::Archive_Roads_SkyRoads(std::shared_ptr<stream::inout> content)
	:	FATArchive(content, SRR_FIRST_FILE_OFFSET, 0)
{
	stream::pos lenArchive = this->content->size();
	if (lenArchive > 0) {
		this->content->seekg(0, stream::start);
		uint16_t offCur;
		*this->content >> u16le(offCur);

		int numFiles = offCur / SRR_FAT_ENTRY_LEN;
		this->vcFAT.reserve(numFiles);

		for (int i = 0; i < numFiles; i++) {
			auto f = this->createNewFATEntry();

			uint16_t lenDecomp, offNext;
			*this->content >> u16le(lenDecomp);
			if (i < numFiles - 1) {
				*this->content >> u16le(offNext);
			} else {
				offNext = lenArchive;
			}

			f->iOffset = offCur;
			f->storedSize = offNext - offCur;
			f->realSize = lenDecomp;
			f->iIndex = i;
			f->lenHeader = 0;
			f->type = "map/skyroads";
			f->fAttr = 0;
			f->filter = "";
			f->bValid = true;
			this->vcFAT.push_back(std::move(f));

			offCur = offNext;
		}
	} // else empty archive
}

Archive_Roads_SkyRoads::~Archive_Roads_SkyRoads()
{
}

void Archive_Roads_SkyRoads::updateFileName(const FATEntry *pid, const std::string& strNewName)
{
	throw stream::error("This format does not have any filenames.");
}

void Archive_Roads_SkyRoads::updateFileOffset(const FATEntry *pid, stream::delta offDelta)
{
	// TESTED BY: fmt_skyroads_roads_insert*
	// TESTED BY: fmt_skyroads_roads_resize*
	this->content->seekp(pid->iIndex * SRR_FAT_ENTRY_LEN, stream::start);
	*this->content << u16le(pid->iOffset);
	return;
}

void Archive_Roads_SkyRoads::updateFileSize(const FATEntry *pid, stream::delta sizeDelta)
{
	// TESTED BY: fmt_skyroads_roads_insert*
	// TESTED BY: fmt_skyroads_roads_resize*
	this->content->seekp(pid->iIndex * SRR_FAT_ENTRY_LEN + 2, stream::start);
	*this->content << u16le(pid->storedSize);
	return;
}

void Archive_Roads_SkyRoads::preInsertFile(const FATEntry *idBeforeThis, FATEntry *pNewEntry)
{
	// TESTED BY: fmt_skyroads_roads_insert*

	// Set the format-specific variables
	pNewEntry->lenHeader = 0;

	// Because the new entry isn't in the vector yet we need to shift it manually
	pNewEntry->iOffset += SRR_FAT_ENTRY_LEN;

	this->content->seekp(pNewEntry->iIndex * SRR_FAT_ENTRY_LEN, stream::start);
	this->content->insert(SRR_FAT_ENTRY_LEN);

	// Write out the entry
	*this->content
		<< u16le(pNewEntry->iOffset)
		<< u16le(pNewEntry->storedSize)
	;

	// Update the offsets now there's a new FAT entry taking up space.
	this->shiftFiles(
		NULL,
		this->vcFAT.size() * SRR_FAT_ENTRY_LEN,
		SRR_FAT_ENTRY_LEN,
		0
	);

	return;
}

void Archive_Roads_SkyRoads::preRemoveFile(const FATEntry *pid)
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

	this->content->seekp(pid->iIndex * SRR_FAT_ENTRY_LEN, stream::start);
	this->content->remove(SRR_FAT_ENTRY_LEN);
	return;
}

} // namespace gamearchive
} // namespace camoto
