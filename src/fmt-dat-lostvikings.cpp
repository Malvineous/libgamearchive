/**
 * @file  fmt-dat-lostvikings.cpp
 * @brief Implementation of The Lost Vikings .DAT file reader/writer.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/DAT_Format_%28The_Lost_Vikings%29
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

#include <boost/algorithm/string.hpp>
#include <camoto/iostream_helpers.hpp>
#include <camoto/util.hpp>

#include "fmt-dat-lostvikings.hpp"

#define DAT_FAT_ENTRY_LEN     4  // u32le offset
#define DAT_FIRST_FILE_OFFSET 0

namespace camoto {
namespace gamearchive {

ArchiveType_DAT_LostVikings::ArchiveType_DAT_LostVikings()
{
}

ArchiveType_DAT_LostVikings::~ArchiveType_DAT_LostVikings()
{
}

std::string ArchiveType_DAT_LostVikings::code() const
{
	return "dat-lostvikings";
}

std::string ArchiveType_DAT_LostVikings::friendlyName() const
{
	return "The Lost Vikings Data File";
}

std::vector<std::string> ArchiveType_DAT_LostVikings::fileExtensions() const
{
	std::vector<std::string> vcExtensions;
	vcExtensions.push_back("dat");
	return vcExtensions;
}

std::vector<std::string> ArchiveType_DAT_LostVikings::games() const
{
	std::vector<std::string> vcGames;
	vcGames.push_back("BlackThorne");
	vcGames.push_back("The Lost Vikings");
	vcGames.push_back("WarCraft: Orcs & Humans");
	return vcGames;
}

ArchiveType::Certainty ArchiveType_DAT_LostVikings::isInstance(
	stream::input& content) const
{
	stream::pos lenArchive = content.size();

	// Empty files could be empty archives
	// TESTED BY: fmt_dat_lostvikings_isinstance_c01
	if (lenArchive == 0) return PossiblyYes;

	// If the archive is smaller than a single entry then it's not a valid file.
	// TESTED BY: fmt_dat_lostvikings_isinstance_c02
	if (lenArchive < DAT_FAT_ENTRY_LEN) return DefinitelyNo;

	content.seekg(0, stream::start);
	uint32_t offEntry;
	content >> u32le(offEntry);

	// If the FAT is smaller than a single entry then it's not a valid file.
	// TESTED BY: fmt_dat_lostvikings_isinstance_c03
	if (offEntry < DAT_FAT_ENTRY_LEN) return DefinitelyNo;

	// Check each FAT entry
	uint32_t offLast = 0;
	uint32_t numFiles = offEntry / DAT_FAT_ENTRY_LEN;
	for (unsigned int i = 0; i < numFiles; i++) {
		// If a file entry points past the end of the archive then it's an invalid
		// format.
		// TESTED BY: fmt_dat_lostvikings_isinstance_c04
		if (offEntry > lenArchive) return DefinitelyNo;

		// Files can't be negative size
		// TESTED BY: fmt_dat_lostvikings_isinstance_c05
		if (offEntry < offLast) return DefinitelyNo;

		// Files must have decompression indicator
//		if (offEntry - offLast < 2) return DefinitelyNo;

		offLast = offEntry;
		if (i < numFiles - 1) {
			// Don't want to read past EOF in case archive contents are a single
			// empty file.
			content >> u32le(offEntry);
		}
	}

	if (lenArchive - offLast == 0) {
		// Last file is empty, so this is probably a Sango Fighter file instead.
		return Unsure;
	}

	// If we've made it this far, this is almost certainly a DAT file.

	// TESTED BY: fmt_dat_lostvikings_isinstance_c00
	return DefinitelyYes;
}

std::shared_ptr<Archive> ArchiveType_DAT_LostVikings::create(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	return std::make_shared<Archive_DAT_LostVikings>(std::move(content));
}

std::shared_ptr<Archive> ArchiveType_DAT_LostVikings::open(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	return std::make_shared<Archive_DAT_LostVikings>(std::move(content));
}

SuppFilenames ArchiveType_DAT_LostVikings::getRequiredSupps(stream::input& content,
	const std::string& filenameArchive) const
{
	// No supplemental types/empty list
	return SuppFilenames();
}


Archive_DAT_LostVikings::Archive_DAT_LostVikings(std::unique_ptr<stream::inout> content)
	:	Archive_FAT(std::move(content), DAT_FIRST_FILE_OFFSET, 0)
{
	stream::pos lenArchive = this->content->size();
	if (lenArchive > 0) {
		this->content->seekg(0, stream::start);
		uint32_t offNext;
		*this->content
			>> u32le(offNext)
		;
		uint32_t numFiles = offNext / DAT_FAT_ENTRY_LEN;
		this->vcFAT.reserve(numFiles);
		for (unsigned int i = 0; i < numFiles; i++) {
			auto f = this->createNewFATEntry();

			f->iOffset = offNext;
			if (i == numFiles - 1) {
				offNext = lenArchive;
			} else {
				*this->content
					>> u32le(offNext)
				;
			}

			f->iIndex = i;
			f->lenHeader = 0;
			f->type = FILETYPE_GENERIC;
			f->fAttr = File::Attribute::Default;
			f->storedSize = offNext - f->iOffset;
			f->realSize = f->storedSize;
			f->bValid = true;
			this->vcFAT.push_back(std::move(f));
		}
	} // else empty archive
}

Archive_DAT_LostVikings::~Archive_DAT_LostVikings()
{
}

void Archive_DAT_LostVikings::updateFileOffset(const FATEntry *pid, stream::delta offDelta)
{
	// TESTED BY: fmt_dat_lostvikings_insert*
	// TESTED BY: fmt_dat_lostvikings_resize*
	this->content->seekp(pid->iIndex * DAT_FAT_ENTRY_LEN, stream::start);
	*this->content << u32le(pid->iOffset);
	return;
}

void Archive_DAT_LostVikings::preInsertFile(const FATEntry *idBeforeThis,
	FATEntry *pNewEntry)
{
	// TESTED BY: fmt_dat_lostvikings_insert*

	// Set the format-specific variables
	pNewEntry->lenHeader = 0;

	// Because the new entry isn't in the vector yet we need to shift it manually
	pNewEntry->iOffset += DAT_FAT_ENTRY_LEN;

	this->content->seekp(pNewEntry->iIndex * DAT_FAT_ENTRY_LEN, stream::start);
	this->content->insert(DAT_FAT_ENTRY_LEN);

	// Write out the entry
	*this->content
		<< u32le(pNewEntry->iOffset)
	;

	// Update the offsets now there's a new FAT entry taking up space.
	this->shiftFiles(
		NULL,
		this->vcFAT.size() * DAT_FAT_ENTRY_LEN,
		DAT_FAT_ENTRY_LEN,
		0
	);

	return;
}

void Archive_DAT_LostVikings::preRemoveFile(const FATEntry *pid)
{
	// TESTED BY: fmt_dat_lostvikings_remove*

	// Update the offsets now there's one less FAT entry taking up space.  This
	// must be called before the FAT is altered, because it will write a new
	// offset into the FAT entry we're about to erase (and if we erase it first
	// it'll overwrite something else.)
	this->shiftFiles(
		NULL,
		this->vcFAT.size() * DAT_FAT_ENTRY_LEN,
		-DAT_FAT_ENTRY_LEN,
		0
	);

	// Remove the FAT entry
	this->content->seekp(pid->iIndex * DAT_FAT_ENTRY_LEN, stream::start);
	this->content->remove(DAT_FAT_ENTRY_LEN);

	return;
}

} // namespace gamearchive
} // namespace camoto
