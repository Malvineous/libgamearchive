/**
 * @file  fmt-rff-blood.cpp
 * @brief Implementation of reader/writer for Blood's .RFF format.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/RFF_Format
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
#include <camoto/stream_memory.hpp>
#include <camoto/util.hpp> // std::make_unique
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

ArchiveType_RFF_Blood::ArchiveType_RFF_Blood()
{
}

ArchiveType_RFF_Blood::~ArchiveType_RFF_Blood()
{
}

std::string ArchiveType_RFF_Blood::code() const
{
	return "rff-blood";
}

std::string ArchiveType_RFF_Blood::friendlyName() const
{
	return "Monolith Resource File Format";
}

std::vector<std::string> ArchiveType_RFF_Blood::fileExtensions() const
{
	std::vector<std::string> vcExtensions;
	vcExtensions.push_back("rff");
	return vcExtensions;
}

std::vector<std::string> ArchiveType_RFF_Blood::games() const
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Blood");
	return vcGames;
}

ArchiveType::Certainty ArchiveType_RFF_Blood::isInstance(
	stream::input& content) const
{
	stream::pos lenArchive = content.size();

	// TESTED BY: fmt_rff_blood_isinstance_c02
	if (lenArchive < RFF_HEADER_LEN) return DefinitelyNo; // too short

	char sig[4];
	content.seekg(0, stream::start);
	content.read(sig, 4);

	// TESTED BY: fmt_rff_blood_isinstance_c00
	if (strncmp(sig, "RFF\x1A", 4) == 0) return DefinitelyYes;

	// TESTED BY: fmt_rff_blood_isinstance_c01
	return DefinitelyNo;
}

std::unique_ptr<Archive> ArchiveType_RFF_Blood::create(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	content->seekp(0, stream::start);
	*content
		<< "RFF\x1A"
		<< u32le(0x0200)          // Default version
		<< u32le(RFF_HEADER_LEN)  // FAT offset
		<< u32le(0)               // Number of files
		<< u32le(0)               // Unknown
		<< u32le(0)               // Unknown
		<< u32le(0)               // Unknown
		<< u32le(0);              // Unknown
	return std::make_unique<Archive_RFF_Blood>(std::move(content));
}

std::unique_ptr<Archive> ArchiveType_RFF_Blood::open(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	return std::make_unique<Archive_RFF_Blood>(std::move(content));
}

SuppFilenames ArchiveType_RFF_Blood::getRequiredSupps(stream::input& content,
	const std::string& filenameArchive) const
{
	// No supplemental types/empty list
	return SuppFilenames();
}


Archive_RFF_Blood::Archive_RFF_Blood(std::unique_ptr<stream::inout> content)
	:	FATArchive(std::move(content), RFF_FIRST_FILE_OFFSET, ARCH_STD_DOS_FILENAMES),
		modifiedFAT(false)
{
	stream::pos lenArchive = this->content->size();

	if (lenArchive < 16) throw stream::error("File too short");

	this->content->seekg(4, stream::start); // skip "RFF\x1A" sig

	uint32_t offFAT, numFiles;
	uint16_t unknown1;

	*this->content
		>> u16le(this->version)
		>> u16le(unknown1)
		>> u32le(offFAT)
		>> u32le(numFiles);

	if (numFiles >= RFF_SAFETY_MAX_FILECOUNT) {
		// TESTED BY: test_rff_blood::invalidcontent_i01
		throw stream::error("too many files or corrupted archive");
	}

	// Create a substream to decrypt the FAT
	auto fatSubStream = std::make_shared<stream::sub>(
		this->content,
		offFAT,
		numFiles * RFF_FAT_ENTRY_LEN,
		stream::fn_truncate_sub()
	);

	// Decrypt the FAT if needed
	std::shared_ptr<stream::input> fatPlaintext;
	if (this->version >= 0x301) {
		// The FAT is encrypted in this version
		fatPlaintext = std::make_shared<stream::input_filtered>(
			fatSubStream,
			std::make_shared<filter_rff_crypt>(0, offFAT & 0xFF)
		);
	} else {
		fatPlaintext = fatSubStream;
	}

	// Copy the decrypted FAT into memory
	auto tempStorage = std::make_shared<stream::memory>();
	this->fatStream = std::make_shared<stream::seg>(tempStorage);
	this->fatStream->seekp(0, stream::start);
	this->fatStream->insert(numFiles * RFF_FAT_ENTRY_LEN);
	stream::copy(*this->fatStream, *fatPlaintext);

	this->fatStream->seekg(0, stream::start);

	for (unsigned int i = 0; i < numFiles; i++) {
		auto f = this->createNewFATEntry();

		f->iIndex = i;
		f->lenHeader = 0;
		f->type = FILETYPE_GENERIC;
		f->fAttr = 0;
		f->bValid = true;

		uint32_t lastModified, unknown2, unknown3;
		uint8_t flags;
		std::string dummy;
		std::string filename;
		//fatFilter->seekg(16, stream::cur);
		*this->fatStream
			>> fixedLength(dummy, 16)
			>> u32le(f->iOffset)
			>> u32le(f->storedSize)
			>> u32le(unknown2)
			>> u32le(lastModified)
			>> u8(flags)
			>> fixedLength(filename, 11)
			>> u32le(unknown3);

		if (flags & RFF_FILE_ENCRYPTED) {
			f->fAttr |= EA_ENCRYPTED;
			f->filter = "xor-blood"; // decryption algorithm
		}

		int lenExt = 0, lenBase = 0;
		while (lenExt  < 3) { if (!filename[    lenExt ]) break; lenExt++; }
		while (lenBase < 8) { if (!filename[3 + lenBase]) break; lenBase++; }
		f->strName = filename.substr(3, lenBase) + "." + filename.substr(0, lenExt);

		f->realSize = f->storedSize;
		this->vcFAT.push_back(std::move(f));
	}
}

Archive_RFF_Blood::~Archive_RFF_Blood()
{
}

Archive_RFF_Blood::MetadataTypes Archive_RFF_Blood::getMetadataList() const
{
	// TESTED BY: fmt_rff_blood_get_metadata_version
	MetadataTypes m;
	m.push_back(Version);
	return m;
}

std::string Archive_RFF_Blood::getMetadata(MetadataType item) const
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

void Archive_RFF_Blood::setMetadata(MetadataType item, const std::string& value)
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
				for (auto& i : this->vcFAT) {
					auto pFAT = dynamic_cast<const FATEntry *>(&*i);
					if (pFAT->fAttr & EA_ENCRYPTED) {
						throw stream::error("cannot change to this version while the "
							"archive contains encrypted files (the target version does not "
							"support encryption)");
					}
				}
			}
			this->version = newVersion;

			// Write new version number into file header
			this->content->seekg(4, stream::start);
			*this->content << u16le(this->version);
			*this->content << u16le(0); // TODO: write 1 here for 0x200?
			break;
		}
		default:
			assert(false);
			throw stream::error("unsupported metadata item");
	}
	return;
}

void Archive_RFF_Blood::flush()
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
			offFAT = pLast->iOffset + pLast->lenHeader + pLast->storedSize;
		}
		this->content->seekp(RFF_FATOFFSET_OFFSET, stream::start);
		*this->content << u32le(offFAT);

		// Work out how much to add to or remove from the end of the archive so that
		// it ends immediately following the FAT.
		stream::pos lenArchive = this->content->size();
		stream::pos offEndFAT = offFAT + this->vcFAT.size() * RFF_FAT_ENTRY_LEN;
		stream::delta lenDelta = offEndFAT - lenArchive;

		// If we need to make room for a larger FAT, do that now so there's room to
		// commit it.  If we're removing data we'll do that later.
		if (lenDelta > 0) {
			//this->content->seekp(offEndFAT, stream::start);
			this->content->seekp(offFAT, stream::start);
			this->content->insert(lenDelta);

		} else if (lenDelta < 0) {
			// If there's extra data in the archive following the FAT, remove that now.
			// This will remove data from the FAT but that's ok because we have it all
			// in memory and we're about to write it out.
			//this->content->seekp(offEndFAT, stream::start);
			this->content->seekp(offFAT, stream::start);
			this->content->remove(-lenDelta);
		}

		// Write the FAT back out

		// Create a substream to decrypt the FAT
		auto fatSubStream = std::make_shared<stream::sub>(
			this->content,
			offFAT,
			this->vcFAT.size() * RFF_FAT_ENTRY_LEN,
			stream::fn_truncate_sub()
		);

		std::shared_ptr<stream::output> fatPlaintext;
		if (this->version >= 0x301) {
			// The FAT is encrypted in this version
			fatPlaintext = std::make_shared<stream::output_filtered>(
				fatSubStream,
				std::make_shared<filter_rff_crypt>(0, offFAT & 0xFF),
				stream::fn_truncate_filter()
			);
		} else {
			fatPlaintext = fatSubStream;
		}

		this->fatStream->seekg(0, stream::start);
		stream::copy(*fatPlaintext, *this->fatStream);
		// Need to flush here because we're going to access the underlying stream
		fatPlaintext->flush();

		// Write the new FAT offset into the file header
		this->content->seekp(RFF_FATOFFSET_OFFSET, stream::start);
		*this->content << u32le(offFAT);

		this->modifiedFAT = false;
	}

	// Commit this->content
	this->FATArchive::flush();
	return;
}

void Archive_RFF_Blood::updateFileName(const FATEntry *pid, const std::string& strNewName)
{
	// TESTED BY: fmt_rff_blood_rename

	// See if the filename is valid
	std::string base, ext;
	this->splitFilename(strNewName, &base, &ext);

	// If we reach here the filename was OK

	this->fatStream->seekp(RFF_FILENAME_OFFSET(pid), stream::start);
	*this->fatStream
		<< nullPadded(ext, 3)
		<< nullPadded(base, 8);

	this->modifiedFAT = true;
	return;
}

void Archive_RFF_Blood::updateFileOffset(const FATEntry *pid, stream::delta offDelta)
{
	// TESTED BY: fmt_rff_blood_insert*
	// TESTED BY: fmt_rff_blood_resize*
	this->fatStream->seekp(RFF_FILEOFFSET_OFFSET(pid), stream::start);
	*this->fatStream << u32le(pid->iOffset);
	this->modifiedFAT = true;
	return;
}

void Archive_RFF_Blood::updateFileSize(const FATEntry *pid, stream::delta sizeDelta)
{
	// TESTED BY: fmt_rff_blood_insert*
	// TESTED BY: fmt_rff_blood_resize*
	this->fatStream->seekp(RFF_FILESIZE_OFFSET(pid), stream::start);
	*this->fatStream << u32le(pid->storedSize);
	this->modifiedFAT = true;
	return;
}

void Archive_RFF_Blood::preInsertFile(const FATEntry *idBeforeThis,
	FATEntry *pNewEntry)
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

	*this->fatStream
		<< nullPadded("", 16) // unknown
		<< u32le(pNewEntry->iOffset)
		<< u32le(pNewEntry->storedSize)
		<< u32le(0) // unknown
		<< u32le(0) // last modified time
		<< u8(flags)
		<< nullPadded(ext, 3)
		<< nullPadded(base, 8)
		<< u32le(0); // unknown

	this->modifiedFAT = true;
	return;
}

void Archive_RFF_Blood::postInsertFile(FATEntry *pNewEntry)
{
	this->updateFileCount(this->vcFAT.size());
	return;
}

void Archive_RFF_Blood::preRemoveFile(const FATEntry *pid)
{
	this->fatStream->seekp(RFF_FATENTRY_OFFSET(pid), stream::start);
	this->fatStream->remove(RFF_FAT_ENTRY_LEN);
	this->modifiedFAT = true;
	return;
}

void Archive_RFF_Blood::postRemoveFile(const FATEntry *pid)
{
	this->updateFileCount(this->vcFAT.size());
	return;
}

void Archive_RFF_Blood::updateFileCount(uint32_t newCount)
{
	this->content->seekp(RFF_FILECOUNT_OFFSET, stream::start);
	*this->content << u32le(newCount);
	return;
}

stream::pos Archive_RFF_Blood::getDescOffset() const
{
	stream::pos offDesc;
	if (this->vcFAT.size()) {
		auto lastFile = this->vcFAT.back();
		assert(lastFile);
		auto lastFATEntry = dynamic_cast<const FATEntry *>(lastFile.get());
		offDesc = lastFATEntry->iOffset + lastFATEntry->storedSize;
	} else {
		offDesc = RFF_FIRST_FILE_OFFSET;
	}
	return offDesc;
}

void Archive_RFF_Blood::splitFilename(const std::string& full, std::string *base, std::string *ext)
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
