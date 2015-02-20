/**
 * @file  fmt-epf-lionking.cpp
 * @brief Implementation of reader/writer for East Point Software's .EPF file
 *         format, used in The Lion King among other games.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/EPF_Format
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

#include "fmt-epf-lionking.hpp"

#define EPF_HEADER_LEN               11
#define EPF_FAT_OFFSET_POS           4
#define EPF_FILECOUNT_POS            9
#define EPF_FIRST_FILE_OFFSET        EPF_HEADER_LEN

#define EPF_FAT_FILENAME_OFFSET      0
#define EPF_MAX_FILENAME_LEN         12
#define EPF_FILENAME_FIELD_LEN       13
#define EPF_FAT_ISCOMPRESSED_OFFSET  13
#define EPF_FAT_FILESIZE_OFFSET      14
#define EPF_FAT_DECOMP_SIZE_OFFSET   18
#define EPF_FAT_ENTRY_LEN            22

#define EPF_FAT_FLAG_COMPRESSED      1

namespace camoto {
namespace gamearchive {

ArchiveType_EPF_LionKing::ArchiveType_EPF_LionKing()
{
}

ArchiveType_EPF_LionKing::~ArchiveType_EPF_LionKing()
{
}

std::string ArchiveType_EPF_LionKing::code() const
{
	return "epf-lionking";
}

std::string ArchiveType_EPF_LionKing::friendlyName() const
{
	return "East Point Software EPFS File";
}

std::vector<std::string> ArchiveType_EPF_LionKing::fileExtensions() const
{
	std::vector<std::string> vcExtensions;
	vcExtensions.push_back("epf");
	return vcExtensions;
}

std::vector<std::string> ArchiveType_EPF_LionKing::games() const
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Alien Breed Tower Assault");
	vcGames.push_back("Arcade Pool");
	vcGames.push_back("Asterix & Obelix");
	vcGames.push_back("Jungle Book, The");
	vcGames.push_back("Lion King, The");
	vcGames.push_back("Overdrive");
	vcGames.push_back("Project X");
	vcGames.push_back("Sensible Golf");
	vcGames.push_back("Smurfs, The");
	vcGames.push_back("Spirou");
	vcGames.push_back("Tin Tin in Tibet");
	vcGames.push_back("Universe");
	return vcGames;
}

ArchiveType::Certainty ArchiveType_EPF_LionKing::isInstance(
	stream::input& content) const
{
	stream::pos lenArchive = content.size();

	// TESTED BY: fmt_epf_lionking_isinstance_c02
	if (lenArchive < EPF_HEADER_LEN) return DefinitelyNo; // too short

	char sig[4];
	content.seekg(0, stream::start);
	content.read(sig, 4);

	// TESTED BY: fmt_epf_lionking_isinstance_c00
	if (strncmp(sig, "EPFS", 4) == 0) return DefinitelyYes;

	// TESTED BY: fmt_epf_lionking_isinstance_c01
	return DefinitelyNo;
}

std::shared_ptr<Archive> ArchiveType_EPF_LionKing::create(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	content->seekp(0, stream::start);
	*content
		<< nullPadded("EPFS", 4)
		<< u32le(11) // FAT offset
		<< u8(0)     // Unknown/flags?
		<< u16le(0); // File count
	return std::make_shared<Archive_EPF_LionKing>(std::move(content));
}

std::shared_ptr<Archive> ArchiveType_EPF_LionKing::open(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	return std::make_shared<Archive_EPF_LionKing>(std::move(content));
}

SuppFilenames ArchiveType_EPF_LionKing::getRequiredSupps(stream::input& content,
	const std::string& filenameArchive) const
{
	// No supplemental types/empty list
	return SuppFilenames();
}


Archive_EPF_LionKing::Archive_EPF_LionKing(std::unique_ptr<stream::inout> content)
	:	FATArchive(std::move(content), EPF_FIRST_FILE_OFFSET, EPF_MAX_FILENAME_LEN)
{
	stream::pos lenArchive = this->content->size();

	// We still have to perform sanity checks in case the user forced an archive
	// to open even though it failed the signature check.
	if (lenArchive < EPF_HEADER_LEN) throw stream::error("file too short");

	this->content->seekg(4, stream::start); // skip "EPFS" sig

	uint8_t unknown;
	uint16_t numFiles;
	*this->content
		>> u32le(this->offFAT)
		>> u8(unknown)
		>> u16le(numFiles);

	if (
		// These two comparisons are sort of redundant, but we need the first
		// one in case the values are so large they wrap and the second one
		// returns an incorrect result.

		// TESTED BY: test_epf_lionking::invalidcontent_i01
		(this->offFAT > lenArchive) ||

		// TESTED BY: test_epf_lionking::invalidcontent_i02, when stream::pos <= 32bit
		((this->offFAT + numFiles * EPF_FAT_ENTRY_LEN) > lenArchive)
	) {
		throw stream::error("header corrupted or file truncated");
	}
	this->content->seekg(this->offFAT, stream::start);

	stream::pos offNext = EPF_FIRST_FILE_OFFSET;
	uint8_t flags;
	for (int i = 0; i < numFiles; i++) {
		auto f = this->createNewFATEntry();

		f->iIndex = i;
		f->iOffset = offNext;
		f->lenHeader = 0;
		f->type = FILETYPE_GENERIC;
		f->fAttr = EA_NONE;
		f->bValid = true;

		// Read the data in from the FAT entry in the file
		*this->content
			>> nullPadded(f->strName, EPF_FILENAME_FIELD_LEN)
			>> u8(flags)
			>> u32le(f->storedSize)
			>> u32le(f->realSize);

		if (flags & EPF_FAT_FLAG_COMPRESSED) {
			f->fAttr |= EA_COMPRESSED;
			f->filter = "lzw-epfs";
		}

		offNext += f->storedSize;
		this->vcFAT.push_back(std::move(f));
	}
	// TODO: hidden data after FAT until EOF?
}

Archive_EPF_LionKing::~Archive_EPF_LionKing()
{
}

int Archive_EPF_LionKing::getSupportedAttributes() const
{
	return EA_COMPRESSED;
}

Archive_EPF_LionKing::MetadataTypes Archive_EPF_LionKing::getMetadataList() const
{
	// TESTED BY: fmt_epf_lionking_get_metadata_description
	MetadataTypes m;
	m.push_back(Description);
	return m;
}

std::string Archive_EPF_LionKing::getMetadata(MetadataType item) const
{
	// TESTED BY: fmt_epf_lionking_get_metadata_description
	switch (item) {
		case Description: {
			stream::pos offDesc = this->getDescOffset();
			stream::len sizeDesc = this->offFAT - offDesc;
			std::string strDesc;
			if (sizeDesc) {
				this->content->seekg(offDesc, stream::start);
				*this->content >> fixedLength(strDesc, sizeDesc);
				//content->read(strDesc.c_str(), sizeDesc);
			}
			return strDesc;
		}
		default:
			assert(false);
			throw stream::error("unsupported metadata item");
	}
}

void Archive_EPF_LionKing::setMetadata(MetadataType item, const std::string& value)
{
	// TESTED BY: fmt_epf_lionking_set_metadata_description
	// TESTED BY: fmt_epf_lionking_new_to_initialstate
	switch (item) {
		case Description: {
			stream::pos offDesc = this->getDescOffset();
			stream::len sizeDesc = this->offFAT - offDesc;
			stream::delta sizeDelta = value.length() - sizeDesc;
			this->content->seekp(offDesc, stream::start);
			if (sizeDelta < 0) {
				// TESTED BY: ?
				this->content->remove(-sizeDelta);
			} else {
				// TESTED BY: ?
				this->content->insert(sizeDelta);
			}
			*this->content << value; // TODO: confirm no terminating null
			this->offFAT += sizeDelta;
			this->updateFATOffset();
			break;
		}
		default:
			assert(false);
			throw stream::error("unsupported metadata item");
	}
	return;
}

void Archive_EPF_LionKing::updateFileName(const FATEntry *pid, const std::string& strNewName)
{
	// TESTED BY: fmt_epf_lionking_rename
	assert(strNewName.length() <= EPF_MAX_FILENAME_LEN);
	this->content->seekp(this->offFAT + pid->iIndex * EPF_FAT_ENTRY_LEN, stream::start);
	*this->content << nullPadded(strNewName, EPF_FILENAME_FIELD_LEN);
	return;
}

void Archive_EPF_LionKing::updateFileOffset(const FATEntry *pid, stream::delta offDelta)
{
	return;
}

void Archive_EPF_LionKing::updateFileSize(const FATEntry *pid, stream::delta sizeDelta)
{
	// TESTED BY: fmt_epf_lionking_insert*
	// TESTED BY: fmt_epf_lionking_resize*

	this->content->seekp(this->offFAT + pid->iIndex * EPF_FAT_ENTRY_LEN + EPF_FAT_FILESIZE_OFFSET, stream::start);
	*this->content
		<< u32le(pid->storedSize)
		<< u32le(pid->realSize)
	;

	this->offFAT += sizeDelta;
	this->updateFATOffset();

	return;
}

void Archive_EPF_LionKing::preInsertFile(const FATEntry *idBeforeThis, FATEntry *pNewEntry)
{
	// TESTED BY: fmt_epf_lionking_insert*
	assert(pNewEntry->strName.length() <= EPF_MAX_FILENAME_LEN);

	// Set the format-specific variables
	pNewEntry->lenHeader = 0;

	// Set the filter to use if the file should be compressed
	if (pNewEntry->fAttr & EA_COMPRESSED) {
		pNewEntry->filter = "lzw-epfs";
	}

	return;
}

void Archive_EPF_LionKing::postInsertFile(FATEntry *pNewEntry)
{
	this->offFAT += pNewEntry->storedSize;

	this->content->seekp(this->offFAT + pNewEntry->iIndex * EPF_FAT_ENTRY_LEN, stream::start);
	this->content->insert(EPF_FAT_ENTRY_LEN);
	boost::to_upper(pNewEntry->strName);
	uint8_t flags = 0;
	if (pNewEntry->fAttr & EA_COMPRESSED) flags = 1;
	*this->content
		<< nullPadded(pNewEntry->strName, EPF_FILENAME_FIELD_LEN)
		<< u8(flags)  // 0 == uncompressed, 1 == compressed
		<< u32le(pNewEntry->storedSize)  // compressed
		<< u32le(pNewEntry->realSize); // decompressed

	this->updateFATOffset();
	this->updateFileCount(this->vcFAT.size());

	return;
}

void Archive_EPF_LionKing::preRemoveFile(const FATEntry *pid)
{
	// TESTED BY: fmt_epf_lionking_remove*

	this->content->seekp(this->offFAT + pid->iIndex * EPF_FAT_ENTRY_LEN, stream::start);
	this->content->remove(EPF_FAT_ENTRY_LEN);

	this->offFAT -= pid->storedSize;
	this->updateFATOffset();
	this->updateFileCount(this->vcFAT.size() - 1);

	return;
}

void Archive_EPF_LionKing::updateFileCount(uint16_t iNewCount)
{
	// TESTED BY: fmt_epf_lionking_insert*
	// TESTED BY: fmt_epf_lionking_remove*
	this->content->seekp(EPF_FILECOUNT_POS, stream::start);
	*this->content << u16le(iNewCount);
	return;
}

void Archive_EPF_LionKing::updateFATOffset()
{
	// TESTED BY: fmt_epf_lionking_insert*
	// TESTED BY: fmt_epf_lionking_remove*

	this->content->seekp(EPF_FAT_OFFSET_POS, stream::start);
	*this->content << u32le(this->offFAT);

	return;
}

stream::pos Archive_EPF_LionKing::getDescOffset() const
{
	stream::pos offDesc;
	if (this->vcFAT.size()) {
		auto lastFile = this->vcFAT.back();
		assert(lastFile);
		auto lastFATEntry = dynamic_cast<const FATEntry *>(lastFile.get());
		offDesc = lastFATEntry->iOffset + lastFATEntry->storedSize;
	} else {
		offDesc = EPF_FIRST_FILE_OFFSET;
	}
	return offDesc;
}

} // namespace gamearchive
} // namespace camoto
