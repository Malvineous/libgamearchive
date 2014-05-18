/**
 * @file   fmt-dat-lostvikings.cpp
 * @brief  Implementation of The Lost Vikings .DAT file reader/writer.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/DAT_Format_%28The_Lost_Vikings%29
 *
 * Copyright (C) 2010-2014 Adam Nielsen <malvineous@shikadi.net>
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

DAT_LostVikingsType::DAT_LostVikingsType()
{
}

DAT_LostVikingsType::~DAT_LostVikingsType()
{
}

std::string DAT_LostVikingsType::getArchiveCode() const
{
	return "dat-lostvikings";
}

std::string DAT_LostVikingsType::getFriendlyName() const
{
	return "The Lost Vikings Data File";
}

std::vector<std::string> DAT_LostVikingsType::getFileExtensions() const
{
	std::vector<std::string> vcExtensions;
	vcExtensions.push_back("dat");
	return vcExtensions;
}

std::vector<std::string> DAT_LostVikingsType::getGameList() const
{
	std::vector<std::string> vcGames;
	vcGames.push_back("The Lost Vikings");
	return vcGames;
}

ArchiveType::Certainty DAT_LostVikingsType::isInstance(stream::input_sptr psArchive) const
{
	stream::pos lenArchive = psArchive->size();

	// Empty files could be empty archives
	// TESTED BY: fmt_dat_lostvikings_isinstance_c01
	if (lenArchive == 0) return PossiblyYes;

	// If the archive is smaller than a single entry then it's not a valid file.
	// TESTED BY: fmt_dat_lostvikings_isinstance_c02
	if (lenArchive < DAT_FAT_ENTRY_LEN) return DefinitelyNo;

	psArchive->seekg(0, stream::start);
	uint32_t offEntry;
	psArchive >> u32le(offEntry);

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
			psArchive >> u32le(offEntry);
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

ArchivePtr DAT_LostVikingsType::newArchive(stream::inout_sptr psArchive, SuppData& suppData) const
{
	return ArchivePtr(new DAT_LostVikingsArchive(psArchive));
}

ArchivePtr DAT_LostVikingsType::open(stream::inout_sptr psArchive, SuppData& suppData) const
{
	return ArchivePtr(new DAT_LostVikingsArchive(psArchive));
}

SuppFilenames DAT_LostVikingsType::getRequiredSupps(stream::input_sptr data,
	const std::string& filenameArchive) const
{
	// No supplemental types/empty list
	return SuppFilenames();
}


DAT_LostVikingsArchive::DAT_LostVikingsArchive(stream::inout_sptr psArchive)
	:	FATArchive(psArchive, DAT_FIRST_FILE_OFFSET, 0)
{
	stream::pos lenArchive = this->psArchive->size();
	if (lenArchive > 0) {
		this->psArchive->seekg(0, stream::start);
		uint32_t offNext;
		this->psArchive
			>> u32le(offNext)
		;
		uint32_t numFiles = offNext / DAT_FAT_ENTRY_LEN;
		this->vcFAT.reserve(numFiles);
		for (unsigned int i = 0; i < numFiles; i++) {
			FATEntry *fatEntry = new FATEntry();
			EntryPtr ep(fatEntry);

			fatEntry->iOffset = offNext;
			if (i == numFiles - 1) {
				offNext = lenArchive;
			} else {
				this->psArchive
					>> u32le(offNext)
				;
			}

			fatEntry->iIndex = i;
			fatEntry->lenHeader = 0;
			fatEntry->type = FILETYPE_GENERIC;
			fatEntry->fAttr = 0;
			fatEntry->storedSize = offNext - fatEntry->iOffset;
			fatEntry->realSize = fatEntry->storedSize;
			fatEntry->bValid = true;
			this->vcFAT.push_back(ep);
		}
	} // else empty archive
}

DAT_LostVikingsArchive::~DAT_LostVikingsArchive()
{
}

void DAT_LostVikingsArchive::updateFileName(const FATEntry *pid,
	const std::string& strNewName)
{
	throw stream::error("This archive format has no filenames to rename!");
}

void DAT_LostVikingsArchive::updateFileOffset(const FATEntry *pid, stream::delta offDelta)
{
	// TESTED BY: fmt_dat_lostvikings_insert*
	// TESTED BY: fmt_dat_lostvikings_resize*
	this->psArchive->seekp(pid->iIndex * DAT_FAT_ENTRY_LEN, stream::start);
	this->psArchive << u32le(pid->iOffset);
	return;
}

void DAT_LostVikingsArchive::updateFileSize(const FATEntry *pid, stream::delta sizeDelta)
{
	// No file sizes
	return;
}

FATArchive::FATEntry *DAT_LostVikingsArchive::preInsertFile(const FATEntry *idBeforeThis,
	FATEntry *pNewEntry)
{
	// TESTED BY: fmt_dat_lostvikings_insert*

	// Set the format-specific variables
	pNewEntry->lenHeader = 0;

	// Because the new entry isn't in the vector yet we need to shift it manually
	pNewEntry->iOffset += DAT_FAT_ENTRY_LEN;

	this->psArchive->seekp(pNewEntry->iIndex * DAT_FAT_ENTRY_LEN, stream::start);
	this->psArchive->insert(DAT_FAT_ENTRY_LEN);

	// Write out the entry
	this->psArchive
		<< u32le(pNewEntry->iOffset)
	;

	// Update the offsets now there's a new FAT entry taking up space.
	this->shiftFiles(
		NULL,
		this->vcFAT.size() * DAT_FAT_ENTRY_LEN,
		DAT_FAT_ENTRY_LEN,
		0
	);

	return pNewEntry;
}

void DAT_LostVikingsArchive::preRemoveFile(const FATEntry *pid)
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
	this->psArchive->seekp(pid->iIndex * DAT_FAT_ENTRY_LEN, stream::start);
	this->psArchive->remove(DAT_FAT_ENTRY_LEN);

	return;
}

} // namespace gamearchive
} // namespace camoto
