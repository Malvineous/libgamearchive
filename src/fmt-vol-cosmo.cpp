/**
 * @file  fmt-vol-cosmo.cpp
 * @brief Implementation of Cosmo .VOL file reader/writer.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/VOL_Format
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
#include "fmt-vol-cosmo.hpp"

#define VOL_MAX_FILES         200
#define VOL_FAT_ENTRY_LEN     20  // filename + u32le offset + u32le size
#define VOL_FAT_LENGTH        (VOL_MAX_FILES * VOL_FAT_ENTRY_LEN)
#define VOL_MAX_FILENAME_LEN  12
#define VOL_FIRST_FILE_OFFSET VOL_FAT_LENGTH

namespace camoto {
namespace gamearchive {

ArchiveType_VOL_Cosmo::ArchiveType_VOL_Cosmo()
{
}

ArchiveType_VOL_Cosmo::~ArchiveType_VOL_Cosmo()
{
}

std::string ArchiveType_VOL_Cosmo::code() const
{
	return "vol-cosmo";
}

std::string ArchiveType_VOL_Cosmo::friendlyName() const
{
	return "Cosmo Volume File";
}

std::vector<std::string> ArchiveType_VOL_Cosmo::fileExtensions() const
{
	std::vector<std::string> vcExtensions;
	vcExtensions.push_back("vol");
	vcExtensions.push_back("stn");
	vcExtensions.push_back("cmp");
	vcExtensions.push_back("ms1");
	vcExtensions.push_back("ms2");
	vcExtensions.push_back("ms3");
	return vcExtensions;
}

std::vector<std::string> ArchiveType_VOL_Cosmo::games() const
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Cosmo's Cosmic Adventure");
	vcGames.push_back("Duke Nukem II");
	vcGames.push_back("Major Stryker");
	return vcGames;
}

ArchiveType::Certainty ArchiveType_VOL_Cosmo::isInstance(
	stream::input& content) const
{
	stream::pos lenArchive = content.size();
	if (lenArchive < VOL_FAT_ENTRY_LEN) return DefinitelyNo; // too short

	content.seekg(12, stream::start);
	uint32_t lenFAT;
	content >> u32le(lenFAT);

	// If the FAT is larger than the entire archive then it's not a VOL file
	// TESTED BY: fmt_vol_cosmo_isinstance_c04
	if (lenFAT > lenArchive) return DefinitelyNo;

	// If the FAT is smaller than a single entry then it's not a VOL file, but
	// allow a zero-length FAT in the case of an empty archive.
	// TESTED BY: fmt_vol_cosmo_isinstance_c02
	if ((lenFAT > 0) && (lenFAT < VOL_FAT_ENTRY_LEN)) return DefinitelyNo;

	// Check each FAT entry
	char fn[VOL_MAX_FILENAME_LEN];
	content.seekg(0, stream::start);
	for (unsigned int i = 0; i < lenFAT / VOL_FAT_ENTRY_LEN; i++) {
		content.read(fn, VOL_MAX_FILENAME_LEN);
		// Make sure there aren't any invalid characters in the filename
		for (unsigned int j = 0; j < VOL_MAX_FILENAME_LEN; j++) {
			if (!fn[j]) break; // stop on terminating null

			// Fail on control characters in the filename
			// TESTED BY: fmt_vol_cosmo_isinstance_c01
			if (fn[j] < 32) return DefinitelyNo;
		}

		uint32_t offEntry, lenEntry;
		content >> u32le(offEntry) >> u32le(lenEntry);

		// If a file entry points past the end of the archive then it's an invalid
		// format.
		// TESTED BY: fmt_vol_cosmo_isinstance_c03
		if (offEntry + lenEntry > lenArchive) return DefinitelyNo;
	}

	// If we've made it this far, this is almost certainly a VOL file.

	// The FAT is not an even multiple of FAT entries.  Not sure whether this
	// is a requirement.
	//if (lenFAT % VOL_FAT_ENTRY_LEN) return PossiblyYes;

	if (lenArchive < VOL_FAT_LENGTH) return PossiblyYes; // too short though

	// The FAT is not 4000 bytes.  Not sure whether this is a requirement.
	if ((lenFAT != 0) && (lenFAT != 4000)) return PossiblyYes;

	// TESTED BY: fmt_vol_cosmo_isinstance_c00
	return DefinitelyYes;
}

std::unique_ptr<Archive> ArchiveType_VOL_Cosmo::create(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	content->seekp(0, stream::start);
	content->write(std::string(VOL_FAT_LENGTH, '\0'));
	return std::make_unique<Archive_VOL_Cosmo>(std::move(content));
}

std::unique_ptr<Archive> ArchiveType_VOL_Cosmo::open(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	return std::make_unique<Archive_VOL_Cosmo>(std::move(content));
}

SuppFilenames ArchiveType_VOL_Cosmo::getRequiredSupps(stream::input& content,
	const std::string& filenameArchive) const
{
	// No supplemental types/empty list
	return SuppFilenames();
}


Archive_VOL_Cosmo::Archive_VOL_Cosmo(std::shared_ptr<stream::inout> content)
	:	FATArchive(content, VOL_FIRST_FILE_OFFSET, VOL_MAX_FILENAME_LEN)
{
	stream::pos lenArchive = this->content->size();
	if (lenArchive > 0) {

		this->content->seekg(12, stream::start); // skip to offset of first filesize

		// TODO: Do we assume the files are in order and read up until the first one,
		// or do we read 4000 chars, or do we somehow scan the probable files and
		// read up until the first one in case they're out of order...?
		// I guess it depends on what works with the games.
		uint32_t lenFAT;
		*this->content >> u32le(lenFAT);

		uint32_t numFiles = lenFAT / VOL_FAT_ENTRY_LEN;
		this->vcFAT.reserve(numFiles);

		this->content->seekg(0, stream::start);
		for (unsigned int i = 0; i < numFiles; i++) {
			auto f = this->createNewFATEntry();

			*this->content
				>> nullPadded(f->strName, VOL_MAX_FILENAME_LEN)
				>> u32le(f->iOffset)
				>> u32le(f->storedSize)
			;

			f->iIndex = i;
			f->lenHeader = 0;
			f->type = FILETYPE_GENERIC;
			f->fAttr = 0;
			f->bValid = true;
			f->realSize = f->storedSize;
			// Blank FAT entries have an offset of zero
			if (f->iOffset > 0) {
				this->vcFAT.push_back(std::move(f));
			}
		}
	} // else empty archive
}

Archive_VOL_Cosmo::~Archive_VOL_Cosmo()
{
}

void Archive_VOL_Cosmo::updateFileName(const FATEntry *pid, const std::string& strNewName)
{
	// TESTED BY: fmt_vol_cosmo_rename
	assert(strNewName.length() <= VOL_MAX_FILENAME_LEN);
	this->content->seekp(pid->iIndex * VOL_FAT_ENTRY_LEN, stream::start);
	*this->content << nullPadded(strNewName, VOL_MAX_FILENAME_LEN);
	return;
}

void Archive_VOL_Cosmo::updateFileOffset(const FATEntry *pid, stream::delta offDelta)
{
	// TESTED BY: fmt_vol_cosmo_insert*
	// TESTED BY: fmt_vol_cosmo_resize*
	this->content->seekp(pid->iIndex * VOL_FAT_ENTRY_LEN + 12, stream::start);
	*this->content << u32le(pid->iOffset);
	return;
}

void Archive_VOL_Cosmo::updateFileSize(const FATEntry *pid, stream::delta sizeDelta)
{
	// TESTED BY: fmt_vol_cosmo_insert*
	// TESTED BY: fmt_vol_cosmo_resize*
	this->content->seekp(pid->iIndex * VOL_FAT_ENTRY_LEN + 16, stream::start);
	*this->content << u32le(pid->storedSize);
	return;
}

void Archive_VOL_Cosmo::preInsertFile(const FATEntry *idBeforeThis,
	FATEntry *pNewEntry)
{
	// TESTED BY: fmt_vol_cosmo_insert*
	assert(pNewEntry->strName.length() <= VOL_MAX_FILENAME_LEN);

	// Set the format-specific variables
	pNewEntry->lenHeader = 0;

	// TODO: See if this is in fact a hard limitation
	if (this->vcFAT.size() >= VOL_MAX_FILES) {
		throw stream::error("too many files, maximum is " TOSTRING(VOL_MAX_FILES));
	}
	this->content->seekp(pNewEntry->iIndex * VOL_FAT_ENTRY_LEN, stream::start);
	this->content->insert(VOL_FAT_ENTRY_LEN);
	boost::to_upper(pNewEntry->strName);

	// Write out the entry
	*this->content
		<< nullPadded(pNewEntry->strName, VOL_MAX_FILENAME_LEN)
		<< u32le(pNewEntry->iOffset)
		<< u32le(pNewEntry->storedSize);

	// Because the FAT is a fixed size we have to remove a blank entry to
	// compensate for the entry we just added.
	if (this->vcFAT.size() > 0) {
		unsigned int indexLast = VOL_MAX_FILES - 1;
		for (auto i = this->vcFAT.rbegin(); i != this->vcFAT.rend(); i++) {
			auto pFAT = dynamic_cast<const FATEntry *>(i->get());
			if (pFAT->iIndex != indexLast) {
				// The previous slot is free, so delete it
				this->content->seekp(indexLast * VOL_FAT_ENTRY_LEN, stream::start);
				this->content->remove(VOL_FAT_ENTRY_LEN);
				break;
			} else {
				indexLast = pFAT->iIndex - 1;
			}
		}

		// Make sure an entry was removed.  This should never fail as failure would
		// indicate there were 200+ files, which means an exception should've been
		// thrown at the start of this function.
		//assert(indexLast >= 0);
		assert(indexLast < VOL_MAX_FILES); // unsigned version of previous line
	} else {
		// No files so just remove the following entry
		this->content->seekp(1 * VOL_FAT_ENTRY_LEN, stream::start);
		this->content->remove(VOL_FAT_ENTRY_LEN);
	}

	return;
}

void Archive_VOL_Cosmo::preRemoveFile(const FATEntry *pid)
{
	// TESTED BY: fmt_vol_cosmo_remove*

	// Remove the FAT entry
	this->content->seekp(pid->iIndex * VOL_FAT_ENTRY_LEN, stream::start);
	this->content->remove(VOL_FAT_ENTRY_LEN);

	// Add an empty FAT entry onto the end to keep the FAT the same size
	const FATEntry *pFAT = dynamic_cast<const FATEntry *>(this->vcFAT.back().get());
	this->content->seekp((pFAT->iIndex + 1) * VOL_FAT_ENTRY_LEN, stream::start);
	this->content->insert(VOL_FAT_ENTRY_LEN);

	return;
}

} // namespace gamearchive
} // namespace camoto
