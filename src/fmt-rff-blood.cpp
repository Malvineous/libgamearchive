/**
 * @file   fmt-rff-blood.cpp
 * @brief  Implementation of reader/writer for Blood's .RFF format.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/RFF_Format
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
#include "fmt-rff-blood.hpp"
#include "filter-xor-blood.hpp"

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
#define RFF_FILE_ENCRYPTED           0x10

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

ArchiveType::Certainty RFFType::isInstance(stream::inout_sptr psArchive) const
	throw (stream::error)
{
	stream::pos lenArchive = psArchive->size();

	// TESTED BY: fmt_rff_blood_isinstance_c02
	if (lenArchive < RFF_HEADER_LEN) return DefinitelyNo; // too short

	char sig[4];
	psArchive->seekg(0, stream::start);
	psArchive->read(sig, 4);

	// TESTED BY: fmt_rff_blood_isinstance_c00
	if (strncmp(sig, "RFF\x1A", 4) == 0) return DefinitelyYes;

	// TESTED BY: fmt_rff_blood_isinstance_c01
	return DefinitelyNo;
}

ArchivePtr RFFType::newArchive(stream::inout_sptr psArchive, SuppData& suppData) const
	throw (stream::error)
{
	psArchive->seekp(0, stream::start);
	psArchive
		<< "RFF\x1A"
		<< u32le(0x0200)          // Default version
		<< u32le(RFF_HEADER_LEN)  // FAT offset
		<< u32le(0)               // Number of files
		<< u32le(0)               // Unknown
		<< u32le(0)               // Unknown
		<< u32le(0)               // Unknown
		<< u32le(0);              // Unknown
	return ArchivePtr(new RFFArchive(psArchive));
}

ArchivePtr RFFType::open(stream::inout_sptr psArchive, SuppData& suppData) const
	throw (stream::error)
{
	return ArchivePtr(new RFFArchive(psArchive));
}

SuppFilenames RFFType::getRequiredSupps(stream::input_sptr data,
	const std::string& filenameArchive) const
	throw ()
{
	// No supplemental types/empty list
	return SuppFilenames();
}


RFFArchive::RFFArchive(stream::inout_sptr psArchive)
	throw (stream::error) :
		FATArchive(psArchive, RFF_FIRST_FILE_OFFSET, ARCH_STD_DOS_FILENAMES),
		modifiedFAT(false)
{
	stream::pos lenArchive = this->psArchive->size();

	if (lenArchive < 16) throw stream::error("File too short");

	this->psArchive->seekg(4, stream::start); // skip "RFF\x1A" sig

	uint32_t offFAT, numFiles;

	this->psArchive
		>> u32le(this->version)
		>> u32le(offFAT)
		>> u32le(numFiles);

	if (numFiles >= RFF_SAFETY_MAX_FILECOUNT) {
		throw stream::error("too many files or corrupted archive");
	}

	// Create a substream to decrypt the FAT
	stream::sub_sptr fatSubStream(new stream::sub());
	fatSubStream->open(
		this->psArchive,
		offFAT,
		numFiles * RFF_FAT_ENTRY_LEN,
		NULL
	);

	// Decrypt the FAT if needed
	stream::input_sptr fatPlaintext;
	if (this->version >= 0x301) {
		// The FAT is encrypted in this version
		filter_sptr fatCrypt(new filter_rff_crypt(0, offFAT & 0xFF));
		stream::input_filtered_sptr filt(new stream::input_filtered());
		filt->open(fatSubStream, fatCrypt);
		fatPlaintext = filt;
	} else {
		fatPlaintext = fatSubStream;
	}

	// Copy the decrypted FAT into memory
	stream::string_sptr tempStorage(new stream::string());
	this->fatStream.reset(new stream::seg());
	this->fatStream->open(tempStorage);
	this->fatStream->seekp(0, stream::start);
	this->fatStream->insert(numFiles * RFF_FAT_ENTRY_LEN);
	stream::copy(this->fatStream, fatPlaintext);

	this->fatStream->seekg(0, stream::start);

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
		//fatFilter->seekg(16, stream::cur);
		this->fatStream
			>> fixedLength(dummy, 16)
			>> u32le(fatEntry->iOffset)
			>> u32le(fatEntry->iSize)
			>> u32le(unknown2)
			>> u32le(lastModified)
			>> u8(flags)
			>> fixedLength(filename, 11)
			>> u32le(unknown3);

		if (flags & RFF_FILE_ENCRYPTED) {
			fatEntry->fAttr |= EA_ENCRYPTED;
			fatEntry->filter = "xor-blood"; // decryption algorithm
		}

		int lenExt = 0, lenBase = 0;
		while (lenExt  < 3) { if (!filename[    lenExt ]) break; lenExt++; }
		while (lenBase < 8) { if (!filename[3 + lenBase]) break; lenBase++; }
		fatEntry->strName = filename.substr(3, lenBase) + "." + filename.substr(0, lenExt);
		this->vcFAT.push_back(ep);
	}
}

RFFArchive::~RFFArchive()
	throw ()
{
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
	throw (stream::error)
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
			throw stream::error("unsupported metadata item");
	}
}

void RFFArchive::setMetadata(MetadataType item, const std::string& value)
	throw (stream::error)
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
				throw stream::error("only versions 2.0 and 3.1 are supported");
			}
			if (newVersion < 0x301) {
				// Moving from a version that supports encryption to one that doesn't,
				// so make sure there are no encrypted files.
				for (VC_ENTRYPTR::iterator i = this->vcFAT.begin(); i != this->vcFAT.end(); i++) {
					FATEntry *pFAT = dynamic_cast<FATEntry *>(i->get());
					if (pFAT->fAttr & EA_ENCRYPTED) {
						throw stream::error("cannot change to this version while the "
							"archive contains encrypted files (the target version does not "
							"support encryption)");
					}
				}
			}
			this->version = newVersion;

			// Write new version number into file header
			this->psArchive->seekg(4, stream::start);
			this->psArchive << u32le(this->version);
			break;
		}
		default:
			assert(false);
			throw stream::error("unsupported metadata item");
	}
	return;
}

void RFFArchive::flush()
	throw (stream::error)
{
	if (this->modifiedFAT) {

		// Write the new FAT offset into the file header
		uint32_t offFAT;
		if (this->vcFAT.size() == 0) {
			// No files
			offFAT = RFF_FIRST_FILE_OFFSET;
		} else {
			const FATEntry *pLast = dynamic_cast<const FATEntry *>(this->vcFAT.back().get());
			assert(pLast);
			offFAT = pLast->iOffset + pLast->lenHeader + pLast->iSize;
		}
		this->psArchive->seekp(RFF_FATOFFSET_OFFSET, stream::start);
		this->psArchive << u32le(offFAT);

		// Work out how much to add to or remove from the end of the archive so that
		// it ends immediately following the FAT.
		stream::pos lenArchive = this->psArchive->size();
		stream::pos offEndFAT = offFAT + this->vcFAT.size() * RFF_FAT_ENTRY_LEN;
		stream::delta lenDelta = offEndFAT - lenArchive;

		// If we need to make room for a larger FAT, do that now so there's room to
		// commit it.  If we're removing data we'll do that later.
		if (lenDelta > 0) {
			//this->psArchive->seekp(offEndFAT, stream::start);
			this->psArchive->seekp(offFAT, stream::start);
			this->psArchive->insert(lenDelta);

		} else if (lenDelta < 0) {
			// If there's extra data in the archive following the FAT, remove that now.
			// This will remove data from the FAT but that's ok because we have it all
			// in memory and we're about to write it out.
			//this->psArchive->seekp(offEndFAT, stream::start);
			this->psArchive->seekp(offFAT, stream::start);
			this->psArchive->remove(-lenDelta);
		}

		// Write the FAT back out

		// Create a substream to decrypt the FAT
		stream::sub_sptr fatSubStream(new stream::sub());
		fatSubStream->open(
			this->psArchive,
			offFAT,
			this->vcFAT.size() * RFF_FAT_ENTRY_LEN,
			NULL
		);

		stream::output_sptr fatPlaintext;
		if (this->version >= 0x301) {
			// The FAT is encrypted in this version
			filter_sptr fatCrypt(new filter_rff_crypt(0, offFAT & 0xFF));
			stream::output_filtered_sptr filt(new stream::output_filtered());
			filt->open(fatSubStream, fatCrypt, NULL);
			fatPlaintext = filt;
		} else {
			fatPlaintext = fatSubStream;
		}

		this->fatStream->seekg(0, stream::start);
		stream::copy(fatPlaintext, this->fatStream);
		// Need to flush here because we're going to access the underlying stream
		fatPlaintext->flush();

		// Write the new FAT offset into the file header
		this->psArchive->seekp(RFF_FATOFFSET_OFFSET, stream::start);
		this->psArchive << u32le(offFAT);

		this->modifiedFAT = false;
	}

	// Commit this->psArchive
	this->FATArchive::flush();
	return;
}

void RFFArchive::updateFileName(const FATEntry *pid, const std::string& strNewName)
	throw (stream::error)
{
	// TESTED BY: fmt_rff_blood_rename

	// See if the filename is valid
	std::string base, ext;
	this->splitFilename(strNewName, &base, &ext);

	// If we reach here the filename was OK

	this->fatStream->seekp(RFF_FILENAME_OFFSET(pid), stream::start);
	this->fatStream
		<< nullPadded(ext, 3)
		<< nullPadded(base, 8);

	this->modifiedFAT = true;
	return;
}

void RFFArchive::updateFileOffset(const FATEntry *pid, stream::delta offDelta)
	throw (stream::error)
{
	// TESTED BY: fmt_rff_blood_insert*
	// TESTED BY: fmt_rff_blood_resize*
	this->fatStream->seekp(RFF_FILEOFFSET_OFFSET(pid), stream::start);
	this->fatStream << u32le(pid->iOffset);
	this->modifiedFAT = true;
	return;
}

void RFFArchive::updateFileSize(const FATEntry *pid, stream::delta sizeDelta)
	throw (stream::error)
{
	// TESTED BY: fmt_rff_blood_insert*
	// TESTED BY: fmt_rff_blood_resize*
	this->fatStream->seekp(RFF_FILESIZE_OFFSET(pid), stream::start);
	this->fatStream << u32le(pid->iSize);
	this->modifiedFAT = true;
	return;
}

FATArchive::FATEntry *RFFArchive::preInsertFile(const FATEntry *idBeforeThis,
	FATEntry *pNewEntry)
	throw (stream::error)
{
	// TESTED BY: fmt_rff_blood_insert*

	uint8_t flags = 0;
	pNewEntry->lenHeader = 0;
	if (pNewEntry->fAttr & EA_ENCRYPTED) {
		if (this->version >= 0x301) {
			pNewEntry->filter = "xor-blood";
			flags |= RFF_FILE_ENCRYPTED;
		} else {
			// This version doesn't support encryption, remove the attribute
			pNewEntry->fAttr &= ~EA_ENCRYPTED;
		}
	}

	// Prepare filename field
	std::string base, ext;
	boost::to_upper(pNewEntry->strName);
	this->splitFilename(pNewEntry->strName, &base, &ext);

	// Add the new entry into the on-disk FAT.  This has to happen here (rather
	// than in postInsertFile()) because on return FATArchive will update the
	// offsets of all FAT entries following this one.  If we don't insert a new
	// entry now, all the offset changes will be applied to the wrong files.
	this->fatStream->seekp(RFF_FATENTRY_OFFSET(pNewEntry), stream::start);
	this->fatStream->insert(RFF_FAT_ENTRY_LEN);

	this->fatStream
		<< nullPadded("", 16) // unknown
		<< u32le(pNewEntry->iOffset)
		<< u32le(pNewEntry->iSize)
		<< u32le(0) // unknown
		<< u32le(0) // last modified time
		<< u8(flags)
		<< nullPadded(ext, 3)
		<< nullPadded(base, 8)
		<< u32le(0); // unknown

	this->modifiedFAT = true;
	return pNewEntry;
}

void RFFArchive::postInsertFile(FATEntry *pNewEntry)
	throw (stream::error)
{
	this->updateFileCount(this->vcFAT.size());
	return;
}

void RFFArchive::preRemoveFile(const FATEntry *pid)
	throw (stream::error)
{
	this->fatStream->seekp(RFF_FATENTRY_OFFSET(pid), stream::start);
	this->fatStream->remove(RFF_FAT_ENTRY_LEN);
	this->modifiedFAT = true;
	return;
}

void RFFArchive::postRemoveFile(const FATEntry *pid)
	throw (stream::error)
{
	this->updateFileCount(this->vcFAT.size());
	return;
}

void RFFArchive::updateFileCount(uint32_t newCount)
	throw (stream::error)
{
	this->psArchive->seekp(RFF_FILECOUNT_OFFSET, stream::start);
	this->psArchive << u32le(newCount);
	return;
}

stream::pos RFFArchive::getDescOffset() const
	throw (stream::error)
{
	stream::pos offDesc;
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
	throw (stream::error)
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
		throw stream::error("maximum filename length is 8.3 chars");
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
