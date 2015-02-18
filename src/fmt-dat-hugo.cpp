/**
 * @file  fmt-dat-hugo.cpp
 * @brief Implementation of Hugo scenery .DAT file reader/writer.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/DAT_Format_%28Hugo%29
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
#include <camoto/util.hpp> // std::make_unique
#include "fmt-dat-hugo.hpp"

#define DAT_FAT_ENTRY_LEN        8  // u32le offset + u32le size
#define DAT_FIRST_FILE_OFFSET    0

#define DAT_FATENTRY_OFFSET(e)   (e->iIndex * DAT_FAT_ENTRY_LEN)

#define DAT_FILESIZE_OFFSET(e)   (DAT_FATENTRY_OFFSET(e) + 4)
#define DAT_FILEOFFSET_OFFSET(e)  DAT_FATENTRY_OFFSET(e)

namespace camoto {
namespace gamearchive {

ArchiveType_DAT_Hugo::ArchiveType_DAT_Hugo()
{
}

ArchiveType_DAT_Hugo::~ArchiveType_DAT_Hugo()
{
}

std::string ArchiveType_DAT_Hugo::code() const
{
	return "dat-hugo";
}

std::string ArchiveType_DAT_Hugo::friendlyName() const
{
	return "Hugo DAT File";
}

std::vector<std::string> ArchiveType_DAT_Hugo::fileExtensions() const
{
	std::vector<std::string> vcExtensions;
	vcExtensions.push_back("dat");
	return vcExtensions;
}

std::vector<std::string> ArchiveType_DAT_Hugo::games() const
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Hugo II, Whodunit?");
	vcGames.push_back("Hugo III, Jungle of Doom!");
	return vcGames;
}

ArchiveType::Certainty ArchiveType_DAT_Hugo::isInstance(
	stream::input& content) const
{
	stream::pos lenArchive = content.size();

	// Because there's no header, an empty file could be in this format.
	// TESTED BY: fmt_dat_hugo_isinstance_c04
	if (lenArchive == 0) return PossiblyYes;

	// TESTED BY: fmt_dat_hugo_isinstance_c02
	if (lenArchive < DAT_FAT_ENTRY_LEN) return DefinitelyNo; // too short

	content.seekg(0, stream::start);

	uint32_t fatEnd, firstLen;
	content >> u32le(fatEnd) >> u32le(firstLen);

	// TESTED BY: fmt_dat_hugo_isinstance_c03
	if (fatEnd + firstLen > lenArchive)
		return DefinitelyNo; // first file finishes after EOF

	// Last FAT entry is truncated
	if (fatEnd % DAT_FAT_ENTRY_LEN != 0) return DefinitelyNo;

	uint32_t numFiles = fatEnd / DAT_FAT_ENTRY_LEN;

	uint32_t offEntry = 0, lenEntry = 0;
	for (unsigned int i = 1; i < numFiles; i++) {
		content
			>> u32le(offEntry)
			>> u32le(lenEntry)
		;

		// If a file entry points past the end of the archive then it's an invalid
		// format.
		// TESTED BY: fmt_dat_hugo_isinstance_c01
		if (offEntry + lenEntry > lenArchive) return DefinitelyNo;
	}

	if (offEntry + lenEntry != lenArchive) {
		// There's trailing data at the end of the format, so it could be one
		// of the other similar ones.
		return Unsure;
	}

	// If we've made it this far, this is almost certainly a DAT file.

	// TESTED BY: fmt_dat_hugo_isinstance_c00
	return DefinitelyYes;
}

std::shared_ptr<Archive> ArchiveType_DAT_Hugo::create(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	// Return an empty file
	return std::make_shared<Archive_DAT_Hugo>(std::move(content), nullptr);
}

std::shared_ptr<Archive> ArchiveType_DAT_Hugo::open(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	if (suppData.find(SuppItem::FAT) != suppData.end()) {
		return std::make_shared<Archive_DAT_Hugo>(
			std::move(content), std::move(suppData[SuppItem::FAT])
		);
	}
	return std::make_shared<Archive_DAT_Hugo>(std::move(content), nullptr);
}

SuppFilenames ArchiveType_DAT_Hugo::getRequiredSupps(stream::input& content,
	const std::string& filenameArchive) const
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


Archive_DAT_Hugo::Archive_DAT_Hugo(std::unique_ptr<stream::inout> content,
	std::unique_ptr<stream::inout> psFAT)
	:	FATArchive(std::move(content), DAT_FIRST_FILE_OFFSET, 0)
{
	if (psFAT) {
		this->psFAT = std::make_unique<stream::seg>(std::move(psFAT));
	} else {
		this->psFAT = this->content;
	}

	stream::pos lenFAT = this->psFAT->size();
	stream::pos lenArchive = this->content->size();

	// Empty files could be empty archives, so only attempt to read if the file
	// is non-empty.
	if ((lenFAT != 0) || (lenArchive != 0)) {

		if (lenFAT < DAT_FAT_ENTRY_LEN) {
			throw stream::error("Archive too short - no FAT terminator!");
		}

		uint32_t fatEnd;
		this->psFAT->seekg(0, stream::start);
		*this->psFAT >> u32le(fatEnd);
		if (fatEnd >= lenFAT) {
			throw stream::error("Archive corrupt - FAT truncated!");
		}
		uint32_t numFiles = fatEnd / DAT_FAT_ENTRY_LEN;
		this->vcFAT.reserve(numFiles);

		stream::pos lastOffset = 0;
		int curFile = 1;
		int firstIndexInSecondArch = 0;
		this->psFAT->seekg(0, stream::start);
		for (unsigned int i = 0; i < numFiles; i++) {
			auto f = this->createNewFATEntry();

			// Read the data in from the FAT entry in the file
			*this->psFAT
				>> u32le(f->iOffset)
				>> u32le(f->storedSize)
			;

			// If suddenly the offsets revert back to zero, it means we've reached
			// the second file (scenery2.dat)
			if ((f->iOffset != 0) || (f->storedSize != 0)) {
				if (f->iOffset < lastOffset) {
					curFile++;
					firstIndexInSecondArch = i;
				}
				lastOffset = f->iOffset;
			} else {
				// TODO: mark as spare/unused FAT entry
			}

			f->iIndex = i - firstIndexInSecondArch;
			f->lenHeader = 0;
			f->type = FILETYPE_GENERIC;
			f->fAttr = 0;
			f->bValid = true;
			f->strName = std::string();
			f->realSize = f->storedSize;

			auto fd = dynamic_cast<FATEntry_Hugo *>(&*f);
			fd->file = curFile;

			if (
				(psFAT && (curFile == 2)) ||
				(!psFAT && (curFile == 1))
			) {
				// If we have a FAT we're only interested in files in the second FAT,
				// otherwise we want the first FAT.
				this->vcFAT.push_back(std::move(f));
			}
		}
	} // else file is blank, treat as empty archive

}

Archive_DAT_Hugo::~Archive_DAT_Hugo()
{
}

void Archive_DAT_Hugo::updateFileName(const FATEntry *pid, const std::string& strNewName)
{
	throw stream::error("This archive format has no filenames to rename!");
}

void Archive_DAT_Hugo::updateFileOffset(const FATEntry *pid,
	stream::delta offDelta)
{
	// TESTED BY: fmt_dat_hugo_insert*
	// TESTED BY: fmt_dat_hugo_resize*

	this->content->seekp(DAT_FILEOFFSET_OFFSET(pid), stream::start);
	*this->content << u32le(pid->iOffset);
	return;
}

void Archive_DAT_Hugo::updateFileSize(const FATEntry *pid,
	stream::delta sizeDelta)
{
	// TESTED BY: fmt_dat_hugo_insert*
	// TESTED BY: fmt_dat_hugo_resize*
	this->content->seekp(DAT_FILESIZE_OFFSET(pid), stream::start);
	*this->content << u32le(pid->storedSize);
	return;
}

void Archive_DAT_Hugo::preInsertFile(
	const FATEntry *idBeforeThis, FATEntry *pNewEntry)
{
	// TESTED BY: fmt_dat_hugo_insert*

	// Set the format-specific variables
	pNewEntry->lenHeader = 0;

	// Because the new entry isn't in the vector yet we need to shift it manually
	pNewEntry->iOffset += DAT_FAT_ENTRY_LEN;

	this->content->seekp(DAT_FATENTRY_OFFSET(pNewEntry), stream::start);
	this->content->insert(DAT_FAT_ENTRY_LEN);

	// Write out the entry
	*this->content
		<< u32le(pNewEntry->iOffset)
		<< u32le(pNewEntry->storedSize)
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

void Archive_DAT_Hugo::preRemoveFile(const FATEntry *pid)
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

	this->content->seekp(DAT_FATENTRY_OFFSET(pid), stream::start);
	this->content->remove(DAT_FAT_ENTRY_LEN);

	return;
}

std::unique_ptr<FATArchive::FATEntry> Archive_DAT_Hugo::createNewFATEntry()
{
	return std::make_unique<FATEntry_Hugo>();
}

} // namespace gamearchive
} // namespace camoto
