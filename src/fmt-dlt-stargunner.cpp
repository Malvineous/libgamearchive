/**
 * @file  fmt-dlt-stargunner.cpp
 * @brief Implementation of Star Gunner .DLT file reader/writer.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/DLT_Format
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

#include "fmt-dlt-stargunner.hpp"

#define DLT_FILECOUNT_OFFSET    6
#define DLT_HEADER_LEN          8  // "DAVE" header + u16le unk + u16le file count
#define DLT_FAT_OFFSET          DLT_HEADER_LEN
#define DLT_FILENAME_FIELD_LEN  32
#define DLT_MAX_FILENAME_LEN    DLT_FILENAME_FIELD_LEN
#define DLT_EFAT_ENTRY_LEN      (32+8)  // filename + u32le unk + u32le size
#define DLT_FIRST_FILE_OFFSET   DLT_FAT_OFFSET  // empty archive only

// TODO: not needed as max will be 64k files anyway
#define DLT_SAFETY_MAX_FILECOUNT  8192 // Maximum value we will load

#define DLT_FATENTRY_OFFSET(e) (e->iOffset)

#define DLT_FILENAME_OFFSET(e) DLT_FATENTRY_OFFSET(e)
#define DLT_FILESIZE_OFFSET(e) (DLT_FATENTRY_OFFSET(e) + DLT_FILENAME_FIELD_LEN + 4)

namespace camoto {
namespace gamearchive {

ArchiveType_DLT_Stargunner::ArchiveType_DLT_Stargunner()
{
}

ArchiveType_DLT_Stargunner::~ArchiveType_DLT_Stargunner()
{
}

std::string ArchiveType_DLT_Stargunner::code() const
{
	return "dlt-stargunner";
}

std::string ArchiveType_DLT_Stargunner::friendlyName() const
{
	return "Stargunner DLT File";
}

std::vector<std::string> ArchiveType_DLT_Stargunner::fileExtensions() const
{
	std::vector<std::string> vcExtensions;
	vcExtensions.push_back("dlt");
	return vcExtensions;
}

std::vector<std::string> ArchiveType_DLT_Stargunner::games() const
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Stargunner");
	return vcGames;
}

ArchiveType::Certainty ArchiveType_DLT_Stargunner::isInstance(
	stream::input& content) const
{
	stream::pos lenArchive = content.size();

	// TESTED BY: fmt_dlt_stargunner_isinstance_c02
	if (lenArchive < DLT_HEADER_LEN) return DefinitelyNo; // too short

	char sig[4];
	content.seekg(0, stream::start);
	content.read(sig, 4);

	// TESTED BY: fmt_dlt_stargunner_isinstance_c00
	if (strncmp(sig, "DAVE", 4) == 0) return DefinitelyYes;

	// TESTED BY: fmt_dlt_stargunner_isinstance_c01
	return DefinitelyNo;
}

std::shared_ptr<Archive> ArchiveType_DLT_Stargunner::create(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	content->seekp(0, stream::start);
	content->write("DAVE\x00\x01\x00\x00", 8);
	return std::make_shared<Archive_DLT_Stargunner>(std::move(content));
}

std::shared_ptr<Archive> ArchiveType_DLT_Stargunner::open(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	return std::make_shared<Archive_DLT_Stargunner>(std::move(content));
}

SuppFilenames ArchiveType_DLT_Stargunner::getRequiredSupps(stream::input& content,
	const std::string& filenameArchive) const
{
	// No supplemental types/empty list
	return SuppFilenames();
}


Archive_DLT_Stargunner::Archive_DLT_Stargunner(std::unique_ptr<stream::inout> content)
	:	Archive_FAT(std::move(content), DLT_FIRST_FILE_OFFSET, DLT_MAX_FILENAME_LEN)
{
	this->content->seekg(4, stream::start); // skip "DAVE" sig

	// We still have to perform sanity checks in case the user forced an archive
	// to open even though it failed the signature check.
	if (this->content->tellg() != 4) throw stream::error("file too short");

	uint16_t unk;
	uint16_t numFiles;
	*this->content
		>> u16le(unk)
		>> u16le(numFiles)
	;

	if (numFiles >= DLT_SAFETY_MAX_FILECOUNT) {
		throw stream::error("too many files or corrupted archive");
	}

	stream::pos offNext = DLT_HEADER_LEN;
	for (unsigned int i = 0; i < numFiles; i++) {
		auto f = this->createNewFATEntry();

		f->iIndex = i;
		f->iOffset = offNext;
		f->lenHeader = DLT_EFAT_ENTRY_LEN;
		f->type = FILETYPE_GENERIC;
		f->fAttr = File::Attribute::Default;
		f->bValid = true;

		uint32_t unk;

		// Read the data in from the FAT entry in the file
		uint8_t name[DLT_FILENAME_FIELD_LEN + 1];
		this->content->read(name, DLT_FILENAME_FIELD_LEN);
		*this->content
			>> u32le(unk)
			>> u32le(f->storedSize)
		;

		// Decrypt the filename
		for (int i = 1; i < DLT_FILENAME_FIELD_LEN; i++) name[i] ^= name[i - 1] + i;
		name[DLT_FILENAME_FIELD_LEN] = 0; // just in case
		f->strName = (char *)name;

		f->realSize = f->storedSize;
		offNext += f->storedSize + DLT_EFAT_ENTRY_LEN;
		this->content->seekg(f->storedSize, stream::cur);
		this->vcFAT.push_back(std::move(f));
	}
}

Archive_DLT_Stargunner::~Archive_DLT_Stargunner()
{
}

void Archive_DLT_Stargunner::updateFileName(const FATEntry *pid, const std::string& strNewName)
{
	int lenName = strNewName.length();
	// TESTED BY: fmt_dlt_stargunner_rename
	assert(lenName <= DLT_MAX_FILENAME_LEN);

	// Encrypt the filename
	uint8_t encName[DLT_FILENAME_FIELD_LEN];
	const char *clearName = strNewName.c_str();
	encName[0] = clearName[0];
	int i;
	if (lenName < DLT_FILENAME_FIELD_LEN) lenName++; // write a terminating null
	for (i = 1; i < lenName; i++) encName[i] = clearName[i] ^ (clearName[i - 1] + i);
	for (; i < DLT_FILENAME_FIELD_LEN; i++) encName[i] = i; // decodes to '\0'

	this->content->seekp(DLT_FILENAME_OFFSET(pid), stream::start);
	this->content->write(encName, DLT_FILENAME_FIELD_LEN);
	return;
}

void Archive_DLT_Stargunner::updateFileSize(const FATEntry *pid, stream::delta sizeDelta)
{
	// TESTED BY: fmt_dlt_stargunner_insert*
	// TESTED BY: fmt_dlt_stargunner_resize*
	this->content->seekp(DLT_FILESIZE_OFFSET(pid), stream::start);
	*this->content << u32le(pid->storedSize);
	return;
}

void Archive_DLT_Stargunner::preInsertFile(const FATEntry *idBeforeThis, FATEntry *pNewEntry)
{
	int lenName = pNewEntry->strName.length();
	// TESTED BY: fmt_dlt_stargunner_insert*
	assert(lenName <= DLT_MAX_FILENAME_LEN);

	// Encrypt the filename
	uint8_t encName[DLT_FILENAME_FIELD_LEN];
	const char *clearName = pNewEntry->strName.c_str();
	encName[0] = clearName[0];
	int i;
	if (lenName < DLT_FILENAME_FIELD_LEN) lenName++; // write a terminating null
	for (i = 1; i < lenName; i++) encName[i] = clearName[i] ^ (clearName[i - 1] + i);
	for (; i < DLT_FILENAME_FIELD_LEN; i++) encName[i] = i; // decodes to '\0'

	// Set the format-specific variables
	pNewEntry->lenHeader = DLT_EFAT_ENTRY_LEN;

	this->content->seekp(pNewEntry->iOffset, stream::start);
	this->content->insert(DLT_EFAT_ENTRY_LEN);
	this->content->write(encName, DLT_FILENAME_FIELD_LEN);
	*this->content
		<< u32le(0) // unknown
		<< u32le(pNewEntry->storedSize);

	// Since we've inserted some data for the embedded header, we need to update
	// the other file offsets accordingly.  This call updates the offset of the
	// files, then calls updateFileOffset() on them, using the *new* offset, so
	// we need to do this after the insert() call above to make sure the extra
	// data has been inserted.  Then when updateFileOffset() writes data out it
	// will go into the correct spot.
	this->shiftFiles(NULL, pNewEntry->iOffset, pNewEntry->lenHeader, 0);

	this->updateFileCount(this->vcFAT.size() + 1);
	return;
}

void Archive_DLT_Stargunner::preRemoveFile(const FATEntry *pid)
{
	// TESTED BY: fmt_dlt_stargunner_remove*

	this->updateFileCount(this->vcFAT.size() - 1);
	return;
}

void Archive_DLT_Stargunner::updateFileCount(uint16_t iNewCount)
{
	// TESTED BY: fmt_dlt_stargunner_insert*
	// TESTED BY: fmt_dlt_stargunner_remove*
	this->content->seekp(DLT_FILECOUNT_OFFSET, stream::start);
	*this->content << u16le(iNewCount);
	return;
}

} // namespace gamearchive
} // namespace camoto
