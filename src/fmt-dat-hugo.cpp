/**
 * @file   fmt-dat-hugo.cpp
 * @brief  Implementation of Hugo scenery .DAT file reader/writer.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/DAT_Format_%28Hugo%29
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

#include <boost/algorithm/string.hpp>

#include <camoto/iostream_helpers.hpp>
#include <camoto/debug.hpp>
#include "fmt-dat-hugo.hpp"

#define DAT_FAT_ENTRY_LEN        8  // u32le offset + u32le size
#define DAT_FIRST_FILE_OFFSET    0

#define DAT_FATENTRY_OFFSET(e)   (e->iIndex * DAT_FAT_ENTRY_LEN)

#define DAT_FILESIZE_OFFSET(e)   (DAT_FATENTRY_OFFSET(e) + 4)
#define DAT_FILEOFFSET_OFFSET(e)  DAT_FATENTRY_OFFSET(e)

namespace camoto {
namespace gamearchive {

DAT_HugoType::DAT_HugoType()
	throw ()
{
}

DAT_HugoType::~DAT_HugoType()
	throw ()
{
}

std::string DAT_HugoType::getArchiveCode() const
	throw ()
{
	return "dat-hugo";
}

std::string DAT_HugoType::getFriendlyName() const
	throw ()
{
	return "Hugo DAT File";
}

std::vector<std::string> DAT_HugoType::getFileExtensions() const
	throw ()
{
	std::vector<std::string> vcExtensions;
	vcExtensions.push_back("dat");
	return vcExtensions;
}

std::vector<std::string> DAT_HugoType::getGameList() const
	throw ()
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Hugo II, Whodunit?");
	vcGames.push_back("Hugo III, Jungle of Doom!");
	return vcGames;
}

ArchiveType::Certainty DAT_HugoType::isInstance(stream::inout_sptr psArchive) const
	throw (stream::error)
{
	stream::pos lenArchive = psArchive->size();

	// Because there's no header, an empty file could be in this format.
	// TESTED BY: fmt_dat_hugo_isinstance_c04
	if (lenArchive == 0) return PossiblyYes;

	// TESTED BY: fmt_dat_hugo_isinstance_c02
	if (lenArchive < DAT_FAT_ENTRY_LEN) return DefinitelyNo; // too short

	psArchive->seekg(0, stream::start);

	uint32_t fatEnd, firstLen;
	psArchive >> u32le(fatEnd) >> u32le(firstLen);

	// TESTED BY: fmt_dat_hugo_isinstance_c03
	if (fatEnd + firstLen > lenArchive)
		return DefinitelyNo; // first file finishes after EOF

	uint32_t numFiles = fatEnd / DAT_FAT_ENTRY_LEN;

	uint32_t offEntry, lenEntry;
	for (int i = 1; i < numFiles; i++) {
		psArchive
			>> u32le(offEntry)
			>> u32le(lenEntry)
		;

		// If a file entry points past the end of the archive then it's an invalid
		// format.
		// TESTED BY: fmt_dat_hugo_isinstance_c01
		if (offEntry + lenEntry > lenArchive) return DefinitelyNo;
	}

	// If we've made it this far, this is almost certainly a DAT file.

	// TESTED BY: fmt_dat_hugo_isinstance_c00
	return DefinitelyYes;
}

ArchivePtr DAT_HugoType::newArchive(stream::inout_sptr psArchive, SuppData& suppData) const
	throw (stream::error)
{
	// Return an empty file
	return ArchivePtr(new DAT_HugoArchive(psArchive, stream::inout_sptr()));
}

ArchivePtr DAT_HugoType::open(stream::inout_sptr psArchive, SuppData& suppData) const
	throw (stream::error)
{
	if (suppData.find(SuppItem::FAT) != suppData.end()) {
		return ArchivePtr(new DAT_HugoArchive(psArchive, suppData[SuppItem::FAT]));
	}
	return ArchivePtr(new DAT_HugoArchive(psArchive, stream::inout_sptr()));
}

SuppFilenames DAT_HugoType::getRequiredSupps(const std::string& filenameArchive) const
	throw ()
{
	// If this is 'scenery1.dat' then the rest of its data is in 'scenery2.dat'
	// so we will include it as a supp.
	SuppFilenames supps;
	std::string::size_type slash = filenameArchive.find_last_of('/');
	std::string filenameBase;
	if (slash != std::string::npos) {
		filenameBase = filenameArchive.substr(slash+1);
	} else {
		filenameBase = filenameArchive;
	}
	if (boost::iequals(filenameBase, "scenery2.dat")) {
		std::string firstFilename = filenameArchive;
		firstFilename[firstFilename.length() - 5] = '1';
		supps[SuppItem::FAT] = firstFilename;
	}
	return supps;
}


DAT_HugoArchive::DAT_HugoArchive(stream::inout_sptr psArchive, stream::inout_sptr psFAT)
	throw (stream::error) :
		FATArchive(psArchive, DAT_FIRST_FILE_OFFSET, 0)
{
	stream::inout_sptr fatStream;
	if (psFAT) {
		this->psFAT.reset(new stream::seg());
		this->psFAT->open(psFAT);
		fatStream = this->psFAT;
	} else fatStream = this->psArchive;

	stream::pos lenFAT = fatStream->size();

	stream::pos lenArchive = this->psArchive->size();

	// Empty files could be empty archives, so only attempt to read if the file
	// is non-empty.
	if ((lenFAT != 0) || (lenArchive != 0)) {

		if (lenFAT < DAT_FAT_ENTRY_LEN) {
			throw stream::error("Archive too short - no FAT terminator!");
		}

		uint32_t fatEnd;
		fatStream->seekg(0, stream::start);
		fatStream >> u32le(fatEnd);
		if (fatEnd >= lenFAT) {
			throw stream::error("Archive corrupt - FAT truncated!");
		}
		uint32_t numFiles = fatEnd / DAT_FAT_ENTRY_LEN;
		this->vcFAT.reserve(numFiles);

		stream::pos lastOffset = 0;
		int curFile = 1;
		int firstIndexInSecondArch = 0;
		fatStream->seekg(0, stream::start);
		for (int i = 0; i < numFiles; i++) {
			DAT_HugoEntry *fatEntry = new DAT_HugoEntry();
			EntryPtr ep(fatEntry);

			// Read the data in from the FAT entry in the file
			fatStream
				>> u32le(fatEntry->iOffset)
				>> u32le(fatEntry->iSize)
			;

			// If suddenly the offsets revert back to zero, it means we've reached
			// the second file (scenery2.dat)
			if ((fatEntry->iOffset != 0) || (fatEntry->iSize != 0)) {
				if (fatEntry->iOffset < lastOffset) {
					curFile++;
					firstIndexInSecondArch = i;
				}
				lastOffset = fatEntry->iOffset;
			} else {
				// TODO: mark as spare/unused FAT entry
			}

			fatEntry->iIndex = i - firstIndexInSecondArch;
			fatEntry->lenHeader = 0;
			fatEntry->type = FILETYPE_GENERIC;
			fatEntry->fAttr = 0;
			fatEntry->bValid = true;
			fatEntry->strName = std::string();

			fatEntry->file = curFile;

			if (
				(psFAT && (curFile == 2)) ||
				(!psFAT && (curFile == 1))
			) {
				// If we have a FAT we're only interested in files in the second FAT,
				// otherwise we want the first FAT.
				this->vcFAT.push_back(ep);
			}
		}
	} // else file is blank, treat as empty archive

}

DAT_HugoArchive::~DAT_HugoArchive()
	throw ()
{
}

void DAT_HugoArchive::updateFileName(const FATEntry *pid, const std::string& strNewName)
	throw (stream::error)
{
	throw stream::error("This archive format has no filenames to rename!");
}

void DAT_HugoArchive::updateFileOffset(const FATEntry *pid,
	stream::delta offDelta
)
	throw (stream::error)
{
	// TESTED BY: fmt_dat_hugo_insert*
	// TESTED BY: fmt_dat_hugo_resize*

	this->psArchive->seekp(DAT_FILEOFFSET_OFFSET(pid), stream::start);
	this->psArchive << u32le(pid->iOffset);
	return;
}

void DAT_HugoArchive::updateFileSize(const FATEntry *pid,
	stream::delta sizeDelta
)
	throw (stream::error)
{
	// TESTED BY: fmt_dat_hugo_insert*
	// TESTED BY: fmt_dat_hugo_resize*
	this->psArchive->seekp(DAT_FILESIZE_OFFSET(pid), stream::start);
	this->psArchive << u32le(pid->iSize);
	return;
}

FATArchive::FATEntry *DAT_HugoArchive::preInsertFile(
	const FATEntry *idBeforeThis, FATEntry *pNewEntry
)
	throw (stream::error)
{
	// Set the format-specific variables
	pNewEntry->lenHeader = 0;

	// Because the new entry isn't in the vector yet we need to shift it manually
	pNewEntry->iOffset += DAT_FAT_ENTRY_LEN;

	this->psArchive->seekp(DAT_FATENTRY_OFFSET(pNewEntry), stream::start);
	this->psArchive->insert(DAT_FAT_ENTRY_LEN);

	// Write out the entry
	this->psArchive
		<< u32le(pNewEntry->iOffset)
		<< u32le(pNewEntry->iSize)
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

void DAT_HugoArchive::preRemoveFile(const FATEntry *pid)
	throw (stream::error)
{
	// TESTED BY: fmt_dat_hugo_remove*

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

	this->psArchive->seekp(DAT_FATENTRY_OFFSET(pid), stream::start);
	this->psArchive->remove(DAT_FAT_ENTRY_LEN);

	return;
}

FATArchive::FATEntry *DAT_HugoArchive::createNewFATEntry()
	throw ()
{
	return new DAT_HugoEntry();
}

} // namespace gamearchive
} // namespace camoto
