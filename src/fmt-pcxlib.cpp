/**
 * @file   fmt-pcxlib.cpp
 * @brief  Implementation of PCX Library reader/writer.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/PCX_Format
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
#include <camoto/util.hpp>

#include "fmt-pcxlib.hpp"

#define PCX_MAX_FILES         65535
#define PCX_FAT_OFFSET        (2+50+42+2+32)
#define PCX_FILECOUNT_OFFSET  (2+50+42)
#define PCX_FAT_ENTRY_LEN     (1+13+4+4+2+2)
#define PCX_MAX_FILENAME_LEN  12
#define PCX_FIRST_FILE_OFFSET PCX_FAT_OFFSET

#define PCX_FATENTRY_OFFSET(e)   (PCX_FAT_OFFSET + (e)->iIndex * PCX_FAT_ENTRY_LEN)
#define PCX_FILENAME_OFFSET(e)   (PCX_FATENTRY_OFFSET(e) + 1)
#define PCX_FILEOFFSET_OFFSET(e) (PCX_FATENTRY_OFFSET(e) + 14)
#define PCX_FILESIZE_OFFSET(e)   (PCX_FATENTRY_OFFSET(e) + 18)

namespace camoto {
namespace gamearchive {

PCXLibType::PCXLibType()
	throw ()
{
}

PCXLibType::~PCXLibType()
	throw ()
{
}

std::string PCXLibType::getArchiveCode() const
	throw ()
{
	return "pcxlib";
}

std::string PCXLibType::getFriendlyName() const
	throw ()
{
	return "PCX Library (v2)";
}

std::vector<std::string> PCXLibType::getFileExtensions() const
	throw ()
{
	std::vector<std::string> vcExtensions;
	vcExtensions.push_back("pcl");
	return vcExtensions;
}

std::vector<std::string> PCXLibType::getGameList() const
	throw ()
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Word Rescue");
	return vcGames;
}

E_CERTAINTY PCXLibType::isInstance(iostream_sptr psArchive) const
	throw (std::ios::failure)
{
	psArchive->seekg(0, std::ios::end);
	io::stream_offset lenArchive = psArchive->tellg();
	// File too short to hold header
	// TESTED BY: fmt_pcxlib_isinstance_c01
	if (lenArchive < PCX_FAT_OFFSET) return EC_DEFINITELY_NO;

	psArchive->seekg(0, std::ios::beg);
	uint32_t version;
	psArchive >> u16le(version);

	// Only accept known versions
	// TESTED BY: fmt_pcxlib_isinstance_c02
	if (version != 0xCA01) return EC_DEFINITELY_NO;

	psArchive->seekg(PCX_FILECOUNT_OFFSET, std::ios::beg);
	uint32_t numFiles;
	psArchive >> u16le(numFiles);

	// File too short to hold FAT
	// TESTED BY: fmt_pcxlib_isinstance_c03
	if (lenArchive < PCX_FAT_OFFSET + numFiles * PCX_FAT_ENTRY_LEN) {
		return EC_DEFINITELY_NO;
	}

	psArchive->seekg(PCX_FAT_OFFSET, std::ios::beg);
	for (int i = 0; i < numFiles; i++) {
		uint8_t sync;
		std::string name, ext;
		uint16_t date, time;
		uint32_t offset, size;
		psArchive
			>> u8(sync)
			>> nullPadded(name, 8)
			>> nullPadded(ext, 5)
			>> u32le(offset)
			>> u32le(size)
			>> u16le(date)
			>> u16le(time)
		;

		// No/invalid sync byte
		// TESTED BY: fmt_pcxlib_isinstance_c04
		if (sync != 0x00) return EC_DEFINITELY_NO;

		// Bad filename
		// TESTED BY: fmt_pcxlib_isinstance_c05
		if (ext[0] != '.') return EC_DEFINITELY_NO;

		// File inside FAT
		// TESTED BY: fmt_pcxlib_isinstance_c06
		if (offset <= PCX_FAT_OFFSET + PCX_FAT_ENTRY_LEN) return EC_DEFINITELY_NO;

		// Truncated file
		// TESTED BY: fmt_pcxlib_isinstance_c07
		if (offset + size > lenArchive) return EC_DEFINITELY_NO;
	}

	// TESTED BY: fmt_pcxlib_isinstance_c00
	return EC_DEFINITELY_YES;
}

ArchivePtr PCXLibType::newArchive(iostream_sptr psArchive, SuppData& suppData) const
	throw (std::ios::failure)
{
	psArchive->seekp(0, std::ios::beg);
	psArchive->write(
		"\x01\xCA" "Copyright (c) Genus Microprogramming, Inc. 1988-90"
		"\x00\x00\x00\x00\x00\x00\x00\x00"
		"\x00\x00\x00\x00\x00\x00\x00\x00"
		"\x00\x00\x00\x00\x00\x00\x00\x00"
		"\x00\x00\x00\x00\x00\x00\x00\x00"
		"\x00\x00\x00\x00\x00\x00\x00\x00"
		"\x00\x00"
		"\x00\x00" // file count
		"\x00\x00\x00\x00\x00\x00\x00\x00"
		"\x00\x00\x00\x00\x00\x00\x00\x00"
		"\x00\x00\x00\x00\x00\x00\x00\x00"
		"\x00\x00\x00\x00\x00\x00\x00\x00",
		128);

	return ArchivePtr(new PCXLibArchive(psArchive));
}

ArchivePtr PCXLibType::open(iostream_sptr psArchive, SuppData& suppData) const
	throw (std::ios::failure)
{
	return ArchivePtr(new PCXLibArchive(psArchive));
}

SuppFilenames PCXLibType::getRequiredSupps(const std::string& filenameArchive) const
	throw ()
{
	// No supplemental types/empty list
	return SuppFilenames();
}


PCXLibArchive::PCXLibArchive(iostream_sptr psArchive)
	throw (std::ios::failure) :
		FATArchive(psArchive, PCX_FIRST_FILE_OFFSET, PCX_MAX_FILENAME_LEN)
{
	this->psArchive->seekg(0, std::ios::end);
	io::stream_offset lenArchive = this->psArchive->tellg();

	if (lenArchive < PCX_FAT_OFFSET) {
		throw std::ios::failure("Truncated file");
	}

	this->psArchive->seekg(PCX_FILECOUNT_OFFSET, std::ios::beg);
	uint16_t numFiles;
	this->psArchive >> u16le(numFiles);
	this->vcFAT.reserve(numFiles);

	// Skip over remaining header
	this->psArchive->seekg(32, std::ios::cur);

	for (int i = 0; i < numFiles; i++) {
		FATEntry *fatEntry = new FATEntry();
		EntryPtr ep(fatEntry);

		uint8_t sync;
		std::string name, ext;
		uint16_t date, time;
		this->psArchive
			>> u8(sync)
			>> nullPadded(name, 8)
			>> nullPadded(ext, 5)
			>> u32le(fatEntry->iOffset)
			>> u32le(fatEntry->iSize)
			>> u16le(date)
			>> u16le(time)
		;
		fatEntry->strName = name.substr(0, name.find_first_of(' ')) + ext.substr(0, ext.find_first_of(' '));

		fatEntry->iIndex = i;
		fatEntry->lenHeader = 0;
		fatEntry->type = FILETYPE_GENERIC;
		fatEntry->fAttr = 0;
		fatEntry->bValid = true;
		this->vcFAT.push_back(ep);
	}
}

PCXLibArchive::~PCXLibArchive()
	throw ()
{
}

void PCXLibArchive::updateFileName(const FATEntry *pid, const std::string& strNewName)
	throw (std::ios::failure)
{
	// TESTED BY: fmt_pcxlib_rename
	assert(strNewName.length() <= PCX_MAX_FILENAME_LEN);
	this->psArchive->seekp(PCX_FILENAME_OFFSET(pid));
	int pos = strNewName.find_last_of('.');
	std::string name = strNewName.substr(0, pos);
	while (name.length() < 8) name += ' ';
	std::string ext = strNewName.substr(pos);
	if (ext.length() > 4) {
		throw std::ios::failure("Filename extension too long - three letters max.");
	}
	while (ext.length() < 4) ext += ' ';
	this->psArchive
		<< nullPadded(name, 8)
		<< nullPadded(ext, 5)
	;
	return;
}

void PCXLibArchive::updateFileOffset(const FATEntry *pid, std::streamsize offDelta)
	throw (std::ios::failure)
{
	// TESTED BY: fmt_pcxlib_insert*
	// TESTED BY: fmt_pcxlib_resize*
	this->psArchive->seekp(PCX_FILEOFFSET_OFFSET(pid));
	this->psArchive << u32le(pid->iOffset);
	return;
}

void PCXLibArchive::updateFileSize(const FATEntry *pid, std::streamsize sizeDelta)
	throw (std::ios::failure)
{
	// TESTED BY: fmt_pcxlib_insert*
	// TESTED BY: fmt_pcxlib_resize*
	this->psArchive->seekp(PCX_FILESIZE_OFFSET(pid));
	this->psArchive << u32le(pid->iSize);
	return;
}

FATArchive::FATEntry *PCXLibArchive::preInsertFile(const FATEntry *idBeforeThis,
	FATEntry *pNewEntry)
	throw (std::ios::failure)
{
	// TESTED BY: fmt_pcxlib_insert*
	assert(pNewEntry->strName.length() <= PCX_MAX_FILENAME_LEN);

	// Set the format-specific variables
	pNewEntry->lenHeader = 0;

	// Because the new entry isn't in the vector yet we need to shift it manually
	pNewEntry->iOffset += PCX_FAT_ENTRY_LEN;

	if (this->vcFAT.size() >= PCX_MAX_FILES) {
		throw std::ios::failure("too many files, maximum is " TOSTRING(PCX_MAX_FILES));
	}
	this->psArchive->seekp(PCX_FATENTRY_OFFSET(pNewEntry));
	this->psArchive->insert(PCX_FAT_ENTRY_LEN);
	boost::to_upper(pNewEntry->strName);

	int pos = pNewEntry->strName.find_last_of('.');
	std::string name = pNewEntry->strName.substr(0, pos);
	while (name.length() < 8) name += ' ';
	std::string ext = pNewEntry->strName.substr(pos);
	if (ext.length() > 4) {
		throw std::ios::failure("Filename extension too long - three letters max.");
	}
	while (ext.length() < 4) ext += ' ';

	uint16_t date = 0, time = 0; // TODO

	// Write out the entry
	this->psArchive
		<< u8(0) // sync byte
		<< nullPadded(name, 8)
		<< nullPadded(ext, 5)
		<< u32le(pNewEntry->iOffset)
		<< u32le(pNewEntry->iSize)
		<< u16le(date)
		<< u16le(time)
	;

	// Update the offsets now there's a new FAT entry taking up space.
	this->shiftFiles(
		NULL,
		PCX_FAT_OFFSET + this->vcFAT.size() * PCX_FAT_ENTRY_LEN,
		PCX_FAT_ENTRY_LEN,
		0
	);

	this->updateFileCount(this->vcFAT.size() + 1);

	return pNewEntry;
}

void PCXLibArchive::preRemoveFile(const FATEntry *pid)
	throw (std::ios::failure)
{
	// TESTED BY: fmt_pcxlib_remove*

	// Update the offsets now there's one less FAT entry taking up space.  This
	// must be called before the FAT is altered, because it will write a new
	// offset into the FAT entry we're about to erase (and if we erase it first
	// it'll overwrite something else.)
	this->shiftFiles(
		NULL,
		PCX_FAT_OFFSET + this->vcFAT.size() * PCX_FAT_ENTRY_LEN,
		-PCX_FAT_ENTRY_LEN,
		0
	);

	// Remove the FAT entry
	this->psArchive->seekp(PCX_FATENTRY_OFFSET(pid));
	this->psArchive->remove(PCX_FAT_ENTRY_LEN);

	this->updateFileCount(this->vcFAT.size() - 1);

	return;
}

void PCXLibArchive::updateFileCount(uint32_t iNewCount)
	throw (std::ios::failure)
{
	// TESTED BY: fmt_pcxlib_insert*
	// TESTED BY: fmt_pcxlib_remove*
	this->psArchive->seekp(PCX_FILECOUNT_OFFSET);
	this->psArchive << u32le(iNewCount);
	return;
}

} // namespace gamearchive
} // namespace camoto
