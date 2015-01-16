/**
 * @file   fmt-wad-doom.cpp
 * @brief  Implementation of Doom .WAD file reader/writer.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/WAD_Format
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

#include "fmt-wad-doom.hpp"

#define WAD_FILECOUNT_OFFSET    4
#define WAD_HEADER_LEN          12
#define WAD_FAT_OFFSET          WAD_HEADER_LEN // assuming no extra data after header
#define WAD_FILENAME_FIELD_LEN  8
#define WAD_MAX_FILENAME_LEN    WAD_FILENAME_FIELD_LEN
#define WAD_FAT_ENTRY_LEN       16
#define WAD_FIRST_FILE_OFFSET   WAD_HEADER_LEN  // empty archive only

#define WAD_SAFETY_MAX_FILECOUNT  8192 // Maximum value we will load

#define WAD_FATENTRY_OFFSET(e) (WAD_HEADER_LEN + e->iIndex * WAD_FAT_ENTRY_LEN)

#define WAD_FILEOFFSET_OFFSET(e) WAD_FATENTRY_OFFSET(e)
#define WAD_FILESIZE_OFFSET(e) (WAD_FATENTRY_OFFSET(e) + 4)
#define WAD_FILENAME_OFFSET(e) (WAD_FATENTRY_OFFSET(e) + 8)

namespace camoto {
namespace gamearchive {

WADType::WADType()
{
}

WADType::~WADType()
{
}

std::string WADType::getArchiveCode() const
{
	return "wad-doom";
}

std::string WADType::getFriendlyName() const
{
	return "Doom WAD File";
}

std::vector<std::string> WADType::getFileExtensions() const
{
	std::vector<std::string> vcExtensions;
	vcExtensions.push_back("wad");
	vcExtensions.push_back("rts");
	return vcExtensions;
}

std::vector<std::string> WADType::getGameList() const
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Doom");
	vcGames.push_back("Duke Nukem 3D");
	vcGames.push_back("Heretic");
	vcGames.push_back("Hexen");
	vcGames.push_back("Redneck Rampage");
	vcGames.push_back("Rise of the Triad");
	vcGames.push_back("Shadow Warrior");
	return vcGames;
}

ArchiveType::Certainty WADType::isInstance(stream::input_sptr psArchive) const
{
	stream::pos lenArchive = psArchive->size();

	// TESTED BY: fmt_wad_doom_isinstance_c03
	if (lenArchive < WAD_HEADER_LEN) return DefinitelyNo; // too short

	char sig[4];
	psArchive->seekg(0, stream::start);
	psArchive->read(sig, 4);

	// TESTED BY: fmt_wad_doom_isinstance_c00
	if (strncmp(sig, "IWAD", 4) == 0) return DefinitelyYes;

	// TESTED BY: fmt_wad_doom_isinstance_c01
	if (strncmp(sig, "PWAD", 4) == 0) return DefinitelyYes;

	// TESTED BY: fmt_wad_doom_isinstance_c02
	return DefinitelyNo;
}

ArchivePtr WADType::newArchive(stream::inout_sptr psArchive, SuppData& suppData) const
{
	psArchive->seekp(0, stream::start);
	psArchive->write("IWAD\x00\x00\x00\x00\x0c\x00\x00\x00", WAD_HEADER_LEN);
	return ArchivePtr(new WADArchive(psArchive));
}

ArchivePtr WADType::open(stream::inout_sptr psArchive, SuppData& suppData) const
{
	return ArchivePtr(new WADArchive(psArchive));
}

SuppFilenames WADType::getRequiredSupps(stream::input_sptr data,
	const std::string& filenameArchive) const
{
	// No supplemental types/empty list
	return SuppFilenames();
}


WADArchive::WADArchive(stream::inout_sptr psArchive)
	:	FATArchive(psArchive, WAD_FIRST_FILE_OFFSET, WAD_MAX_FILENAME_LEN)
{
	this->psArchive->seekg(4, stream::start); // skip sig

	// We still have to perform sanity checks in case the user forced an archive
	// to open even though it failed the signature check.
	if (this->psArchive->tellg() != 4) throw stream::error("file too short");

	uint32_t numFiles, offFAT;
	this->psArchive
		>> u32le(numFiles)
		>> u32le(offFAT)
	;

	if (numFiles >= WAD_SAFETY_MAX_FILECOUNT) {
		throw stream::error("too many files or corrupted archive");
	}

	this->psArchive->seekg(offFAT, stream::start);
	for (unsigned int i = 0; i < numFiles; i++) {
		FATEntry *fatEntry = new FATEntry();
		EntryPtr ep(fatEntry);

		fatEntry->iIndex = i;
		fatEntry->lenHeader = 0;
		fatEntry->type = FILETYPE_GENERIC;
		fatEntry->fAttr = 0;
		fatEntry->bValid = true;

		// Read the data in from the FAT entry in the file
		this->psArchive
			>> u32le(fatEntry->iOffset)
			>> u32le(fatEntry->storedSize)
			>> nullPadded(fatEntry->strName, WAD_FILENAME_FIELD_LEN)
		;

		fatEntry->realSize = fatEntry->storedSize;
		this->vcFAT.push_back(ep);
	}
}

WADArchive::~WADArchive()
{
}

WADArchive::MetadataTypes WADArchive::getMetadataList() const
{
	// TESTED BY: fmt_wad_doom::test_metadata_get_ver
	MetadataTypes m;
	m.push_back(Version);
	return m;
}

std::string WADArchive::getMetadata(MetadataType item) const
{
	// TESTED BY: fmt_wad_doom::test_metadata_get_ver
	switch (item) {
		case Version: {
			psArchive->seekg(0, stream::start);
			char wadtype;
			psArchive->read(&wadtype, 1);
			return std::string(1, wadtype);
		}
		default:
			assert(false);
			throw stream::error("unsupported metadata item");
	}
}

void WADArchive::setMetadata(MetadataType item, const std::string& value)
{
	// TESTED BY: test_wad_doom_changemetadata_c01
	// TESTED BY: fmt_wad_doom_new_to_initialstate
	switch (item) {
		case Version:
			if (
				(value.compare("I") != 0) &&
				(value.compare("P") != 0)
			) {
				throw stream::error("Version can only be set to I or P for IWAD or PWAD");
			}
			this->psArchive->seekp(0, stream::start);
			this->psArchive->write(&(value[0]), 1);
			break;
		default:
			assert(false);
			throw stream::error("unsupported metadata item");
	}
	return;
}

void WADArchive::updateFileName(const FATEntry *pid, const std::string& strNewName)
{
	// TESTED BY: fmt_wad_doom_rename
	assert(strNewName.length() <= WAD_MAX_FILENAME_LEN);
	this->psArchive->seekp(WAD_FILENAME_OFFSET(pid), stream::start);
	this->psArchive << nullPadded(strNewName, WAD_FILENAME_FIELD_LEN);
	return;
}

void WADArchive::updateFileOffset(const FATEntry *pid, stream::delta offDelta)
{
	// TESTED BY: fmt_wad_doom_insert*
	// TESTED BY: fmt_wad_doom_resize*
	this->psArchive->seekp(WAD_FILEOFFSET_OFFSET(pid), stream::start);
	this->psArchive << u32le(pid->iOffset);
	return;
}

void WADArchive::updateFileSize(const FATEntry *pid, stream::delta sizeDelta)
{
	// TESTED BY: fmt_wad_doom_insert*
	// TESTED BY: fmt_wad_doom_resize*
	this->psArchive->seekp(WAD_FILESIZE_OFFSET(pid), stream::start);
	this->psArchive << u32le(pid->storedSize);
	return;
}

FATArchive::FATEntry *WADArchive::preInsertFile(const FATEntry *idBeforeThis, FATEntry *pNewEntry)
{
	// TESTED BY: fmt_wad_doom_insert*
	assert(pNewEntry->strName.length() <= WAD_MAX_FILENAME_LEN);

	// Set the format-specific variables
	pNewEntry->lenHeader = 0;

	// Because the new entry isn't in the vector yet we need to shift it manually
	pNewEntry->iOffset += WAD_FAT_ENTRY_LEN;

	this->psArchive->seekp(WAD_FATENTRY_OFFSET(pNewEntry), stream::start);
	this->psArchive->insert(WAD_FAT_ENTRY_LEN);
	boost::to_upper(pNewEntry->strName);

	this->psArchive
		<< u32le(pNewEntry->iOffset)
		<< u32le(pNewEntry->storedSize)
		<< nullPadded(pNewEntry->strName, WAD_FILENAME_FIELD_LEN)
	;

	// Update the offsets now there's a new FAT entry taking up space.
	this->shiftFiles(
		NULL,
		WAD_FAT_OFFSET + this->vcFAT.size() * WAD_FAT_ENTRY_LEN,
		WAD_FAT_ENTRY_LEN,
		0
	);

	this->updateFileCount(this->vcFAT.size() + 1);
	return pNewEntry;
}

void WADArchive::preRemoveFile(const FATEntry *pid)
{
	// TESTED BY: fmt_wad_doom_remove*

	// Update the offsets now there's one less FAT entry taking up space.  This
	// must be called before the FAT is altered, because it will write a new
	// offset into the FAT entry we're about to erase (and if we erase it first
	// it'll overwrite something else.)
	this->shiftFiles(
		NULL,
		WAD_FAT_OFFSET + this->vcFAT.size() * WAD_FAT_ENTRY_LEN,
		-WAD_FAT_ENTRY_LEN,
		0
	);

	this->psArchive->seekp(WAD_FATENTRY_OFFSET(pid), stream::start);
	this->psArchive->remove(WAD_FAT_ENTRY_LEN);

	this->updateFileCount(this->vcFAT.size() - 1);
	return;
}

void WADArchive::updateFileCount(uint32_t iNewCount)
{
	// TESTED BY: fmt_wad_doom_insert*
	// TESTED BY: fmt_wad_doom_remove*
	this->psArchive->seekp(WAD_FILECOUNT_OFFSET, stream::start);
	this->psArchive << u32le(iNewCount);
	return;
}

} // namespace gamearchive
} // namespace camoto
