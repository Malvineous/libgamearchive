/**
 * @file  fmt-grp-duke3d.cpp
 * @brief Implementation of Duke3D .GRP file reader/writer.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/GRP_Format
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

#include "fmt-grp-duke3d.hpp"

#define GRP_FILECOUNT_OFFSET    12
#define GRP_HEADER_LEN          16  // "KenSilverman" header + u32le file count
#define GRP_FAT_OFFSET          GRP_HEADER_LEN
#define GRP_FILENAME_FIELD_LEN  12
#define GRP_MAX_FILENAME_LEN    GRP_FILENAME_FIELD_LEN
#define GRP_FAT_ENTRY_LEN       16  // filename + u32le size
#define GRP_FIRST_FILE_OFFSET   GRP_FAT_OFFSET  // empty archive only

#define GRP_SAFETY_MAX_FILECOUNT  8192 // Maximum value we will load

#define GRP_FATENTRY_OFFSET(e) (GRP_HEADER_LEN + e->iIndex * GRP_FAT_ENTRY_LEN)

#define GRP_FILENAME_OFFSET(e) GRP_FATENTRY_OFFSET(e)
#define GRP_FILESIZE_OFFSET(e) (GRP_FATENTRY_OFFSET(e) + GRP_FILENAME_FIELD_LEN)

namespace camoto {
namespace gamearchive {

ArchiveType_GRP_Duke3D::ArchiveType_GRP_Duke3D()
{
}

ArchiveType_GRP_Duke3D::~ArchiveType_GRP_Duke3D()
{
}

std::string ArchiveType_GRP_Duke3D::code() const
{
	return "grp-duke3d";
}

std::string ArchiveType_GRP_Duke3D::friendlyName() const
{
	return "Duke Nukem 3D Group File";
}

std::vector<std::string> ArchiveType_GRP_Duke3D::fileExtensions() const
{
	std::vector<std::string> vcExtensions;
	vcExtensions.push_back("grp");
	return vcExtensions;
}

std::vector<std::string> ArchiveType_GRP_Duke3D::games() const
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Duke Nukem 3D");
	vcGames.push_back("Redneck Rampage");
	vcGames.push_back("Shadow Warrior");
	return vcGames;
}

ArchiveType::Certainty ArchiveType_GRP_Duke3D::isInstance(
	stream::input& content) const
{
	stream::pos lenArchive = content.size();

	// File too short
	// TESTED BY: fmt_grp_duke3d_isinstance_c02
	if (lenArchive < GRP_FAT_ENTRY_LEN) return DefinitelyNo; // too short

	char sig[12];
	content.seekg(0, stream::start);
	content.read(sig, 12);

	// Bad signature
	// TESTED BY: fmt_grp_duke3d_isinstance_c01
	if (strncmp(sig, "KenSilverman", 12) != 0) return DefinitelyNo;

	// TESTED BY: fmt_grp_duke3d_isinstance_c00
	return DefinitelyYes;
}

std::unique_ptr<Archive> ArchiveType_GRP_Duke3D::create(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	content->seekp(0, stream::start);
	content->write("KenSilverman\0\0\0\0", 16);
	return std::make_unique<Archive_GRP_Duke3D>(std::move(content));
}

std::unique_ptr<Archive> ArchiveType_GRP_Duke3D::open(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	return std::make_unique<Archive_GRP_Duke3D>(std::move(content));
}

SuppFilenames ArchiveType_GRP_Duke3D::getRequiredSupps(stream::input& data,
	const std::string& filenameArchive) const
{
	// No supplemental types/empty list
	return SuppFilenames();
}


Archive_GRP_Duke3D::Archive_GRP_Duke3D(std::shared_ptr<stream::inout> content)
	:	FATArchive(content, GRP_FIRST_FILE_OFFSET, GRP_MAX_FILENAME_LEN)
{
	this->content->seekg(GRP_FILECOUNT_OFFSET, stream::start); // skip "KenSilverman" sig

	// We still have to perform sanity checks in case the user forced an archive
	// to open even though it failed the signature check.
	if (this->content->tellg() != GRP_FILECOUNT_OFFSET) throw stream::error("file too short");

	uint32_t numFiles;
	*this->content >> u32le(numFiles);

	if (numFiles >= GRP_SAFETY_MAX_FILECOUNT) {
		throw stream::error("too many files or corrupted archive");
	}

	stream::pos offNext = GRP_HEADER_LEN + (numFiles * GRP_FAT_ENTRY_LEN);
	for (unsigned int i = 0; i < numFiles; i++) {
		auto f = std::make_unique<FATEntry>();

		f->iIndex = i;
		f->iOffset = offNext;
		f->lenHeader = 0;
		f->type = FILETYPE_GENERIC;
		f->fAttr = 0;
		f->bValid = true;

		// Read the data in from the FAT entry in the file
		*this->content
			>> nullPadded(f->strName, GRP_FILENAME_FIELD_LEN)
			>> u32le(f->storedSize);

		f->realSize = f->storedSize;
		offNext += f->storedSize;
		this->vcFAT.push_back(std::move(f));
	}
}

Archive_GRP_Duke3D::~Archive_GRP_Duke3D()
{
}

void Archive_GRP_Duke3D::updateFileName(const FATEntry *pid,
	const std::string& strNewName)
{
	// TESTED BY: fmt_grp_duke3d_rename
	assert(strNewName.length() <= GRP_MAX_FILENAME_LEN);
	this->content->seekp(GRP_FILENAME_OFFSET(pid), stream::start);
	*this->content << nullPadded(strNewName, GRP_FILENAME_FIELD_LEN);
	return;
}

void Archive_GRP_Duke3D::updateFileOffset(const FATEntry *pid,
	stream::delta offDelta)
{
	// This format doesn't have any offsets that need updating.  As this function
	// is only called when removing a file, the "offsets" will be sorted out
	// when the FAT entry is removed later.
	return;
}

void Archive_GRP_Duke3D::updateFileSize(const FATEntry *pid,
	stream::delta sizeDelta)
{
	// TESTED BY: fmt_grp_duke3d_insert*
	// TESTED BY: fmt_grp_duke3d_resize*
	this->content->seekp(GRP_FILESIZE_OFFSET(pid), stream::start);
	*this->content << u32le(pid->storedSize);
	return;
}

void Archive_GRP_Duke3D::preInsertFile(const FATEntry *idBeforeThis,
	FATEntry *pNewEntry)
{
	// TESTED BY: fmt_grp_duke3d_insert*
	assert(pNewEntry->strName.length() <= GRP_MAX_FILENAME_LEN);

	// Set the format-specific variables
	pNewEntry->lenHeader = 0;

	// Because the new entry isn't in the vector yet we need to shift it manually
	pNewEntry->iOffset += GRP_FAT_ENTRY_LEN;

	this->content->seekp(GRP_FATENTRY_OFFSET(pNewEntry), stream::start);
	this->content->insert(GRP_FAT_ENTRY_LEN);
	boost::to_upper(pNewEntry->strName);

	*this->content
		<< nullPadded(pNewEntry->strName, GRP_FILENAME_FIELD_LEN)
		<< u32le(pNewEntry->storedSize);

	// Update the offsets now there's a new FAT entry taking up space.
	this->shiftFiles(
		NULL,
		GRP_FAT_OFFSET + this->vcFAT.size() * GRP_FAT_ENTRY_LEN,
		GRP_FAT_ENTRY_LEN,
		0
	);

	this->updateFileCount(this->vcFAT.size() + 1);
	return;
}

void Archive_GRP_Duke3D::preRemoveFile(const FATEntry *pid)
{
	// TESTED BY: fmt_grp_duke3d_remove*

	// Update the offsets now there's one less FAT entry taking up space.  This
	// must be called before the FAT is altered, because it will write a new
	// offset into the FAT entry we're about to erase (and if we erase it first
	// it'll overwrite something else.)
	this->shiftFiles(
		NULL,
		GRP_FAT_OFFSET + this->vcFAT.size() * GRP_FAT_ENTRY_LEN,
		-GRP_FAT_ENTRY_LEN,
		0
	);

	this->content->seekp(GRP_FATENTRY_OFFSET(pid), stream::start);
	this->content->remove(GRP_FAT_ENTRY_LEN);

	this->updateFileCount(this->vcFAT.size() - 1);
	return;
}

void Archive_GRP_Duke3D::updateFileCount(uint32_t iNewCount)
{
	// TESTED BY: fmt_grp_duke3d_insert*
	// TESTED BY: fmt_grp_duke3d_remove*
	this->content->seekp(GRP_FILECOUNT_OFFSET, stream::start);
	*this->content << u32le(iNewCount);
	return;
}

} // namespace gamearchive
} // namespace camoto
