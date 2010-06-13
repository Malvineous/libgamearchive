/*
 * fmt-rff-blood.cpp - Implementation of reader/writer for Blood's .RFF format
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
#include <boost/bind.hpp>
#include <iostream>
#include <exception>
#include <string.h>

#include <camoto/iostream_helpers.hpp>
#include "fmt-rff-blood.hpp"
#include "cipher-rff-blood.hpp"
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

refcount_declclass(RFFType);

RFFType::RFFType()
	throw ()
{
	refcount_qenterclass(RFFType);
}

RFFType::~RFFType()
	throw ()
{
	refcount_qexitclass(RFFType);
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


refcount_declclass(RFFArchive);

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
std::cerr << "fat substream @ " << this->fatSubStream << " is "
<< offFAT << " to " << numFiles * RFF_FAT_ENTRY_LEN << std::endl;
this->fatSubStream->seekp(0, std::ios::end);
io::stream_offset plenStream = this->fatSubStream->tellp();
std::cerr << "fat substream is " << plenStream << " @ " << this->fatSubStream << std::endl;


	iostream_sptr baseFATStream;
	if (this->version >= 0x301) {
		// The FAT is encrypted in this version
		this->cipheredStream.reset(new RFF_FAT_Cipher(this->fatSubStream, offFAT & 0xFF));
		this->cipheredStream->exceptions(std::ios::badbit | std::ios::failbit);
		baseFATStream = this->cipheredStream;
	} else {
		// FAT is not encrypted
		baseFATStream = this->fatSubStream;
	}
	this->fatStream = segstream_sptr(new segmented_stream(baseFATStream));
	this->fatStream->exceptions(std::ios::badbit | std::ios::failbit);
this->fatSubStream->seekp(0, std::ios::end);
plenStream = this->fatSubStream->tellp();
std::cerr << "before read fat: fat substream is " << plenStream << " @ " << this->fatSubStream << std::endl;
	this->fatStream->seekg(0, std::ios::beg);

	for (int i = 0; i < numFiles; i++) {
		RFFEntry *fatEntry = new RFFEntry();
		EntryPtr ep(fatEntry);

		fatEntry->iIndex = i;
		fatEntry->lenHeader = 0;
		fatEntry->eType = EFT_USEFILENAME;
		fatEntry->fAttr = 0;
		fatEntry->bValid = true;

		uint32_t lastModified, unknown2, unknown3;
		uint8_t flags;
		std::string filename;
		this->fatStream->seekg(16, std::ios::cur);
		this->fatStream
			>> u32le(fatEntry->iOffset)
			>> u32le(fatEntry->iSize)
			>> u32le(unknown2)
			>> u32le(lastModified)
			>> u8(fatEntry->rffFlags)
			>> fixedLength(filename, 11) // TODO convert to 8.3
			>> u32le(unknown3);

		if (fatEntry->rffFlags & RFF_FILE_ENCRYPTED) fatEntry->fAttr |= EA_ENCRYPTED;

		int lenExt = 0, lenBase = 0;
		while (lenExt  < 3) { if (!filename[    lenExt ]) break; lenExt++; }
		while (lenBase < 8) { if (!filename[3 + lenBase]) break; lenBase++; }
		fatEntry->strName = filename.substr(3, lenBase) + "." + filename.substr(0, lenExt);
		// TODO: test this with 1) <8 char filename, 2) 8 char filename, 3) <3 char file ext
		/*std::cout
			<< "File: " << fatEntry->strName << "/" << fatEntry->strName.length()
			<< ", off: " << fatEntry->iOffset
			<< ", size: " << fatEntry->iSize
			<< ", unknown2: " << unknown2
			<< ", lastModified: " << lastModified
			<< ", flags: " << flags
			<< ", unknown3: " << unknown3
			<< std::endl;*/
		this->vcFAT.push_back(ep);
		// TODO: What happens when reading files past EOF?
	}
	refcount_qenterclass(RFFArchive);
this->fatSubStream->seekp(0, std::ios::end);
plenStream = this->fatSubStream->tellp();
std::cerr << "after read fat: fat substream is " << plenStream << " @ " << this->fatSubStream << std::endl;
}

RFFArchive::~RFFArchive()
	throw ()
{
	refcount_qexitclass(RFFArchive);
}

boost::shared_ptr<std::iostream> RFFArchive::open(const EntryPtr& id)
	throw ()
{
	const RFFEntry *fatEntry = dynamic_cast<const RFFEntry *>(id.get());
	assert(fatEntry);

	iostream_sptr file = this->FATArchive::open(id);

	if (fatEntry->rffFlags & RFF_FILE_ENCRYPTED) {
		iostream_sptr enc(new RFF_File_Cipher(file));
		return enc;
	} else {
		return file;
	}
}

// Does not invalidate existing EntryPtrs
void RFFArchive::rename(EntryPtr& id, const std::string& strNewName)
	throw (std::ios_base::failure)
{
this->fatSubStream->seekp(0, std::ios::end);
io::stream_offset plenStream = this->fatSubStream->tellp();
std::cerr << "before rename: fat substream is " << plenStream << " @ " << this->fatSubStream << std::endl;
	// TESTED BY: fmt_rff_blood_rename
	assert(this->isValid(id));
	FATEntry *pEntry = dynamic_cast<FATEntry *>(id.get());

	std::string base, ext;
	this->splitFilename(strNewName, &base, &ext);

	this->fatStream->seekp(RFF_FILENAME_OFFSET(pEntry));
	this->fatStream
		<< nullPadded(ext, 3)
		<< nullPadded(base, 8);

	pEntry->strName = strNewName;

this->fatSubStream->seekp(0, std::ios::end);
plenStream = this->fatSubStream->tellp();
std::cerr << "after rename: fat substream is " << plenStream << " @ " << this->fatSubStream << std::endl;
	return;
}

void RFFArchive::flush()
	throw (std::ios::failure)
{
	this->psArchive->seekp(0, std::ios::end);
	io::stream_offset plenStream = this->psArchive->tellp();
	assert(this->psArchive->good());

std::cerr << "main stream is " << plenStream << " @ " << this->psArchive << std::endl;

this->fatSubStream->seekp(0, std::ios::end);
plenStream = this->fatSubStream->tellp();
std::cerr << "fat stream is " << plenStream << " @ " << this->fatSubStream << std::endl;

this->cipheredStream->seekp(0, std::ios::end);
plenStream = this->cipheredStream->tellp();
std::cerr << "ciphered stream is " << plenStream << ", about to commit" << std::endl;

	this->fatStream->commit(boost::bind(&RFFArchive::truncateFAT, this, _1));
	this->FATArchive::flush();
	return;
}

FATArchive::EntryPtr RFFArchive::entryPtrFromStream(const iostream_sptr openFile)
	throw ()
{
	assert(false); // TODO: test!
	const RFF_File_Cipher *cipher = static_cast<RFF_File_Cipher *>(openFile.get());
	return this->FATArchive::entryPtrFromStream(cipher->getParentStream());
}

void RFFArchive::updateFileOffset(const FATEntry *pid, std::streamsize offDelta)
	throw (std::ios::failure)
{
	this->fatStream->seekp(RFF_FILEOFFSET_OFFSET(pid));
	this->fatStream << u32le(pid->iOffset);
	return;
}

void RFFArchive::updateFileSize(const FATEntry *pid, std::streamsize sizeDelta)
	throw (std::ios::failure)
{
	// TESTED BY: fmt_rff_blood_insert*
	// TESTED BY: fmt_rff_blood_resize*

	this->updateFATOffset(sizeDelta);

	this->fatStream->seekp(RFF_FILESIZE_OFFSET(pid));
	this->fatStream << u32le(pid->iSize);

	return;
}

FATArchive::FATEntry *RFFArchive::preInsertFile(const FATEntry *idBeforeThis, FATEntry *pNewEntry)
	throw (std::ios::failure)
{
	// TESTED BY: fmt_rff_blood_insert*

	boost::to_upper(pNewEntry->strName);
	std::string base, ext;
	this->splitFilename(pNewEntry->strName, &base, &ext);
	pNewEntry->lenHeader = 0;

	RFFEntry *rff = new RFFEntry();
	*static_cast<FATEntry *>(rff) = *pNewEntry;
	assert(rff->iOffset == pNewEntry->iOffset);
	// TODO: Shared pointer stuff, plus custom RFF attributes
	// Set the format-specific variables

	// Add the new entry into the on-disk FAT.  This has to happen here (rather
	// than in postInsertFile()) because on return FATArchive will update the
	// offsets of all FAT entries following this one.  If we don't insert a new
	// entry now, all the offset changes will be applied to the wrong files.
	this->fatStream->seekp(RFF_FATENTRY_OFFSET(pNewEntry));
	this->fatStream->insert(RFF_FAT_ENTRY_LEN);

	uint8_t flags = 0;
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

	return rff;
}

void RFFArchive::postInsertFile(FATEntry *pNewEntry)
	throw (std::ios::failure)
{
	this->updateFileCount(this->vcFAT.size());
	this->updateFATOffset(pNewEntry->iSize);
	return;
}

void RFFArchive::preRemoveFile(const FATEntry *pid)
	throw (std::ios::failure)
{
	this->fatStream->seekp(RFF_FATENTRY_OFFSET(pid));
	this->fatStream->remove(RFF_FAT_ENTRY_LEN);
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

	if (this->cipheredStream) {
		// The FAT is encrypted, but because it has moved now it needs to be
		// re-encrypted (because the offset is the initial encryption key.)
		this->cipheredStream->changeSeed(this->fatSubStream->getOffset() & 0xFF);
	}

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

void RFFArchive::truncateFAT(std::streamsize len)
	throw (std::ios::failure)
{
	this->psArchive->seekp(0, std::ios::end);
	io::stream_offset lenArchive = this->psArchive->tellp();
	io::stream_offset offEndFAT = this->fatSubStream->getOffset() + len;
	std::streamsize lenDelta = offEndFAT - lenArchive;
	if (lenDelta < 0) {
		this->psArchive->seekp(offEndFAT, std::ios::beg);
		this->psArchive->remove(-lenDelta);
	} else if (lenDelta > 0) {
		this->psArchive->seekp(offEndFAT, std::ios::beg);
		this->psArchive->insert(lenDelta);
	} // else lenDelete == 0, no change required
	this->fatSubStream->setSize(len);
std::cerr << "set fat len to " << len << std::endl;
	return;
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
