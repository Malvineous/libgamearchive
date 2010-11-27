/**
 * @file   fmt-rff-blood.cpp
 * @brief  Implementation of reader/writer for Blood's .RFF format.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/RFF_Format
 *
 * Copyright (C) 2010 Adam Nielsen <malvineous@shikadi.net>
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
#include <boost/iostreams/invert.hpp>
#include <boost/bind.hpp>
#include <iostream>
#include <exception>
#include <string.h>

#include <camoto/iostream_helpers.hpp>
#include <camoto/filteredstream.hpp>
#include "fmt-rff-blood.hpp"
#include "filter-xor-blood.hpp"
#include "debug.hpp"

#define RFF_FATOFFSET_OFFSET         8
#define RFF_FILECOUNT_OFFSET         12
#define RFF_HEADER_LEN               32
#define RFF_FILENAME_FIELD_LEN       11
#define RFF_FIRST_FILE_OFFSET        RFF_HEADER_LEN

#define RFF_FAT_ENTRY_LEN            48

#define RFF_SAFETY_MAX_FILECOUNT     8192 // Maximum value we will load

#define RFF_FATENTRY_OFFSET(e)   ((e)->iIndex * RFF_FAT_ENTRY_LEN)
#define RFF_FILEOFFSET_OFFSET(e) (RFF_FATENTRY_OFFSET(e) + 16)
#define RFF_FILESIZE_OFFSET(e)   (RFF_FATENTRY_OFFSET(e) + 20)
#define RFF_FILENAME_OFFSET(e)   (RFF_FATENTRY_OFFSET(e) + 33)

// FAT flags
#define RFF_FILE_ENCRYPTED           1

namespace camoto {
namespace gamearchive {

RFFType::RFFType()
	throw ()
{
}

RFFType::~RFFType()
	throw ()
{
}

std::string RFFType::getArchiveCode() const
	throw ()
{
	return "rff-blood";
}

std::string RFFType::getFriendlyName() const
	throw ()
{
	return "Monolith Resource File Format";
}

// Get a list of the known file extensions for this format.
std::vector<std::string> RFFType::getFileExtensions() const
	throw ()
{
	std::vector<std::string> vcExtensions;
	vcExtensions.push_back("rff");
	return vcExtensions;
}

std::vector<std::string> RFFType::getGameList() const
	throw ()
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Blood");
	return vcGames;
}

E_CERTAINTY RFFType::isInstance(iostream_sptr psArchive) const
	throw (std::ios::failure)
{
	psArchive->seekg(0, std::ios::end);
	io::stream_offset lenArchive = psArchive->tellg();

	// TESTED BY: fmt_rff_blood_isinstance_c02
	if (lenArchive < RFF_HEADER_LEN) return EC_DEFINITELY_NO; // too short

	char sig[4];
	psArchive->seekg(0, std::ios::beg);
	psArchive->read(sig, 4);

	// TESTED BY: fmt_rff_blood_isinstance_c00
	if (strncmp(sig, "RFF\x1A", 4) == 0) return EC_DEFINITELY_YES;

	// TESTED BY: fmt_rff_blood_isinstance_c01
	return EC_DEFINITELY_NO;
}

ArchivePtr RFFType::newArchive(iostream_sptr psArchive, MP_SUPPDATA& suppData) const
	throw (std::ios::failure)
{
	psArchive->seekp(0, std::ios::beg);
	psArchive
		<< "RFF\x1A"
		<< u32le(0x0200)        // Version - TODO: 0x301, user-settable
		<< u32le(RFF_HEADER_LEN)  // FAT offset
		<< u32le(0)               // Number of files
		<< u32le(0)               // Unknown
		<< u32le(0)               // Unknown
		<< u32le(0)               // Unknown
		<< u32le(0);              // Unknown
	return ArchivePtr(new RFFArchive(psArchive));
}

// Preconditions: isInstance() has returned > EC_DEFINITELY_NO
ArchivePtr RFFType::open(iostream_sptr psArchive, MP_SUPPDATA& suppData) const
	throw (std::ios::failure)
{
	return ArchivePtr(new RFFArchive(psArchive));
}

MP_SUPPLIST RFFType::getRequiredSupps(const std::string& filenameArchive) const
	throw ()
{
	// No supplemental types/empty list
	return MP_SUPPLIST();
}


RFFArchive::RFFArchive(iostream_sptr psArchive)
	throw (std::ios::failure) :
		FATArchive(psArchive, RFF_FIRST_FILE_OFFSET)
{
	this->psArchive->seekg(0, std::ios::end);
	io::stream_offset lenArchive = this->psArchive->tellg();

	if (lenArchive < 16) throw std::ios::failure("File too short");

	this->psArchive->seekg(4, std::ios::beg); // skip "RFF\x1A" sig

	uint32_t offFAT, numFiles;

	this->psArchive
		>> u32le(this->version)
		>> u32le(offFAT)
		>> u32le(numFiles);

	if (numFiles >= RFF_SAFETY_MAX_FILECOUNT) {
		throw std::ios::failure("too many files or corrupted archive");
	}

	this->fatSubStream.reset(
		new substream(
			this->psArchive,
			offFAT,
			numFiles * RFF_FAT_ENTRY_LEN
		)
	);
	this->fatSubStream->exceptions(std::ios::badbit | std::ios::failbit);
	this->fatSubStream->seekg(0, std::ios::beg);

	filtering_istream_sptr fatFilter(new io::filtering_istream());
	iostream_sptr baseFATStream;
	if (this->version >= 0x301) {
		// The FAT is encrypted in this version
		fatFilter->push(rff_crypt_filter(0, offFAT & 0xFF));
	}
	fatFilter->push(*this->fatSubStream);

	for (int i = 0; i < numFiles; i++) {
		FATEntry *fatEntry = new FATEntry();
		EntryPtr ep(fatEntry);

		fatEntry->iIndex = i;
		fatEntry->lenHeader = 0;
		fatEntry->type = FILETYPE_GENERIC;
		fatEntry->fAttr = 0;
		fatEntry->bValid = true;

		uint32_t lastModified, unknown2, unknown3;
		uint8_t flags;
		std::string dummy;
		std::string filename;
		//fatFilter->seekg(16, std::ios::cur);
		fatFilter
			>> fixedLength(dummy, 16)
			>> u32le(fatEntry->iOffset)
			>> u32le(fatEntry->iSize)
			>> u32le(unknown2)
			>> u32le(lastModified)
			>> u8(flags)
			>> fixedLength(filename, 11) // TODO convert to 8.3
			>> u32le(unknown3);

		if (flags & RFF_FILE_ENCRYPTED) {
			fatEntry->fAttr |= EA_ENCRYPTED;
			fatEntry->filter = "xor-blood"; // decryption algorithm
		}

		int lenExt = 0, lenBase = 0;
		while (lenExt  < 3) { if (!filename[    lenExt ]) break; lenExt++; }
		while (lenBase < 8) { if (!filename[3 + lenBase]) break; lenBase++; }
		fatEntry->strName = filename.substr(3, lenBase) + "." + filename.substr(0, lenExt);
		// TODO: test this with 1) <8 char filename, 2) 8 char filename, 3) <3 char file ext
		this->vcFAT.push_back(ep);
		// TODO: What happens when reading files past EOF?
	}
}

RFFArchive::~RFFArchive()
	throw ()
{
}

// Does not invalidate existing EntryPtrs
void RFFArchive::rename(EntryPtr& id, const std::string& strNewName)
	throw (std::ios_base::failure)
{
	// TESTED BY: fmt_rff_blood_rename
	assert(this->isValid(id));

	// See if the filename is valid
	std::string base, ext;
	this->splitFilename(strNewName, &base, &ext);

	// If we reach here the filename was OK

	id->strName = strNewName;

	return;
}

RFFArchive::MetadataTypes RFFArchive::getMetadataList() const
	throw ()
{
	// TESTED BY: fmt_rff_blood_get_metadata_version
	MetadataTypes m;
	m.push_back(Version);
	return m;
}

std::string RFFArchive::getMetadata(MetadataType item) const
	throw (std::ios::failure)
{
	// TESTED BY: fmt_rff_blood_get_metadata_version
	switch (item) {
		case Version: {
			std::stringstream ss;
			ss << (this->version >> 8) << '.' << (this->version & 0xFF);
			return ss.str();
		}
		default:
			assert(false);
			throw std::ios::failure("unsupported metadata item");
	}
}

void RFFArchive::setMetadata(MetadataType item, const std::string& value)
	throw (std::ios::failure)
{
	// TESTED BY: fmt_rff_blood_set_metadata_version
	// TESTED BY: fmt_rff_blood_new_to_initialstate
	switch (item) {
		case Version: {
			int dotPos = value.find_first_of('.');
			std::string strMajor = value.substr(0, dotPos);
			std::string strMinor = value.substr(dotPos + 1);
			int iMajor = strtoul(strMajor.c_str(), NULL, 10);
			int iMinor = strtoul(strMinor.c_str(), NULL, 10);
			int newVersion = (iMajor << 8) | iMinor;
			if (
				(newVersion != 0x200) &&
				(newVersion != 0x301)
			) {
				throw std::ios::failure("only versions 2.0 and 3.1 are supported");
			}
			if (newVersion < 0x301) {
				// Moving from a version that supports encryption to one that doesn't,
				// so make sure there are no encrypted files.
				for (VC_ENTRYPTR::iterator i = this->vcFAT.begin(); i != this->vcFAT.end(); i++) {
					FATEntry *pFAT = dynamic_cast<FATEntry *>(i->get());
					if (pFAT->fAttr & EA_ENCRYPTED) {
						throw std::ios::failure("cannot change to this version while the "
							"archive contains encrypted files (the target version does not "
							"support encryption)");
					}
				}
			}
			this->version = newVersion;

			// Write new version number into file header
			this->psArchive->seekg(4, std::ios::beg);
			this->psArchive << u32le(this->version);
			break;
		}
		default:
			assert(false);
			throw std::ios::failure("unsupported metadata item");
	}
	return;
}

void RFFArchive::flush()
	throw (std::ios::failure)
{
	io::stream_offset offFAT = this->fatSubStream->getOffset();
	this->fatSubStream->seekp(0, std::ios::beg);

	// Cut off any leftover data after where the FAT will go
	this->psArchive->seekp(0, std::ios::end);
	io::stream_offset lenArchive = this->psArchive->tellp();
	io::stream_offset offEndFAT = offFAT + this->vcFAT.size() * RFF_FAT_ENTRY_LEN;
	std::streamsize lenDelta = offEndFAT - lenArchive;
	if (lenDelta < 0) {
		this->psArchive->seekp(offEndFAT, std::ios::beg);
		this->psArchive->remove(-lenDelta);
	} else if (lenDelta > 0) {
		this->psArchive->seekp(offEndFAT, std::ios::beg);
		this->psArchive->insert(lenDelta);
	} // else lenDelete == 0, no change required
	this->fatSubStream->setSize(this->vcFAT.size() * RFF_FAT_ENTRY_LEN);

	// Create an encryption stream around the FAT, so that whenever we write into
	// this stream encrypted data will end up in the underlying archive file.
	filtering_ostream_sptr fatFilter(new io::filtering_ostream());
	if (this->version >= 0x301) {
		// The FAT is encrypted in this version
		fatFilter->push(io::invert(rff_crypt_filter(0, offFAT & 0xFF)));
	}
	fatFilter->push(*this->fatSubStream);

	// Rewrite the entire FAT into the FAT stream (so it gets encrypted and
	// written into the archive.)
	for (VC_ENTRYPTR::iterator i = this->vcFAT.begin(); i != this->vcFAT.end(); i++) {
		FATEntry *fatEntry = dynamic_cast<FATEntry *>(i->get()); // RFF entry?

		// Prepare filename field
		std::string base, ext;
		this->splitFilename(fatEntry->strName, &base, &ext);

		// Prepare flags field
		uint8_t flags = 0;
		if (fatEntry->fAttr & EA_ENCRYPTED) flags |= RFF_FILE_ENCRYPTED;

		// Write FAT entry
		fatFilter
			<< nullPadded("", 16)
			<< u32le(fatEntry->iOffset)
			<< u32le(fatEntry->iSize)
			<< u32le(0) // unknown2
			<< u32le(0) // TODO: lastModified
			<< u8(flags)
			<< nullPadded(ext, 3)
			<< nullPadded(base, 8)
			<< u32le(0) // unknown3
		;
	}

	// Commit this->psArchive
	this->FATArchive::flush();
	return;
}

void RFFArchive::updateFileOffset(const FATEntry *pid, std::streamsize offDelta)
	throw (std::ios::failure)
{
	// Don't need to do anything here as we won't be writing out any offsets
	// until flush() is called (easier to handle the FAT encryption that way.)
	return;
}

void RFFArchive::updateFileSize(const FATEntry *pid, std::streamsize sizeDelta)
	throw (std::ios::failure)
{
	// TESTED BY: fmt_rff_blood_insert*
	// TESTED BY: fmt_rff_blood_resize*

	this->updateFATOffset(sizeDelta);
	return;
}

FATArchive::FATEntry *RFFArchive::preInsertFile(const FATEntry *idBeforeThis,
	FATEntry *pNewEntry)
	throw (std::ios::failure)
{
	// TESTED BY: fmt_rff_blood_insert*

	boost::to_upper(pNewEntry->strName);
	std::string base, ext;
	this->splitFilename(pNewEntry->strName, &base, &ext);
	pNewEntry->lenHeader = 0;
	if (pNewEntry->fAttr & EA_ENCRYPTED) {
		if (this->version >= 0x301) {
			pNewEntry->filter = "xor-blood";
		} else {
			// This version doesn't support encryption, remove the attribute
			pNewEntry->fAttr &= ~EA_ENCRYPTED;
		}
	}

	return pNewEntry;
}

void RFFArchive::postInsertFile(FATEntry *pNewEntry)
	throw (std::ios::failure)
{
	this->updateFileCount(this->vcFAT.size());
	this->updateFATOffset(pNewEntry->iSize);
	return;
}

void RFFArchive::postRemoveFile(const FATEntry *pid)
	throw (std::ios::failure)
{
	this->updateFATOffset(-pid->iSize);
	this->updateFileCount(this->vcFAT.size());
	return;
}

void RFFArchive::updateFileCount(uint32_t newCount)
	throw (std::ios::failure)
{
	this->psArchive->seekp(RFF_FILECOUNT_OFFSET);
	this->psArchive << u32le(newCount);
	return;
}

void RFFArchive::updateFATOffset(std::streamsize delta)
	throw (std::ios_base::failure)
{
	// TESTED BY: fmt_rff_blood_insert*
	// TESTED BY: fmt_rff_blood_remove*

	this->fatSubStream->relocate(delta);

	// Write the new FAT offset into the file header
	this->psArchive->seekp(RFF_FATOFFSET_OFFSET);
	this->psArchive << u32le(this->fatSubStream->getOffset());

	return;
}

io::stream_offset RFFArchive::getDescOffset() const
	throw (std::ios::failure)
{
	io::stream_offset offDesc;
	if (this->vcFAT.size()) {
		EntryPtr lastFile = this->vcFAT.back();
		assert(lastFile);
		FATEntry *lastFATEntry = dynamic_cast<FATEntry *>(lastFile.get());
		offDesc = lastFATEntry->iOffset + lastFATEntry->iSize;
	} else {
		offDesc = RFF_FIRST_FILE_OFFSET;
	}
	return offDesc;
}

void RFFArchive::splitFilename(const std::string& full, std::string *base, std::string *ext)
	throw (std::ios::failure)
{
	std::string::size_type posDot = full.find_last_of('.');
	if (
		(
			// If no dot, base name must be <= 8 chars
			// TESTED BY: fmt_rff_blood_insert_long_nodot
			(posDot == std::string::npos) &&
			(full.length() > 8)
		) || (
			// Extension must be <= 3 chars
			// TESTED BY: fmt_rff_blood_insert_long_ext
			full.length() - posDot > 4   // 4 == '.' + 3 chars
		) || (
			// Base name (without extension) must be <= 8 chars
			// TESTED BY: fmt_rff_blood_insert_long_base
			posDot > 8
		)
	) {
		throw std::ios::failure("maximum filename length is 8.3 chars");
	}

	if (posDot != std::string::npos) {
		ext->assign(full, posDot + 1, 3);
	} else {
		ext->clear();
	}
	base->assign(full, 0, posDot);

	return;
}

} // namespace gamearchive
} // namespace camoto
