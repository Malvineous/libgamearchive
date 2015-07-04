/**
 * @file  fmt-bpa-drally.cpp
 * @brief Death Rally .BPA format.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/Death_Rally_BPA_Format
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
#include "fmt-bpa-drally.hpp"

#define BPA_MAX_FILES          255
#define BPA_FAT_ENTRY_LEN      17  // filename + u32le size
#define BPA_FAT_LENGTH         (BPA_MAX_FILES * BPA_FAT_ENTRY_LEN)
#define BPA_FILENAME_FIELD_LEN 13
#define BPA_MAX_FILENAME_LEN   12
#define BPA_FILECOUNT_OFFSET   0
#define BPA_FAT_OFFSET         4
#define BPA_FIRST_FILE_OFFSET  (BPA_FAT_OFFSET + BPA_FAT_LENGTH)

#define BPA_FATENTRY_OFFSET(n)     (BPA_FAT_OFFSET + (n) * BPA_FAT_ENTRY_LEN)
#define BPA_FAT_FILENAME_OFFSET(n) BPA_FATENTRY_OFFSET(n)
#define BPA_FAT_FILESIZE_OFFSET(n) (BPA_FATENTRY_OFFSET(n) + BPA_FILENAME_FIELD_LEN)

namespace camoto {
namespace gamearchive {

/// Decrypt a single character in a BPA filename
/**
 * @param n
 *   Index of character within filename, 0 == first char.
 *
 * @param c
 *   Encrypted character to decrypt.
 */
constexpr uint8_t bpa_decrypt(unsigned int n, uint8_t c)
{
	return c - (117 - 3 * n);
}

/// Encrypt a single character in a BPA filename
/**
 * @param n
 *   Index of character within filename, 0 == first char.
 *
 * @param c
 *   Cleartext character to encrypt.
 */
constexpr uint8_t bpa_encrypt(unsigned int n, uint8_t c)
{
	return c + (117 - 3 * n);
}

ArchiveType_BPA_DRally::ArchiveType_BPA_DRally()
{
}

ArchiveType_BPA_DRally::~ArchiveType_BPA_DRally()
{
}

std::string ArchiveType_BPA_DRally::code() const
{
	return "bpa-drally";
}

std::string ArchiveType_BPA_DRally::friendlyName() const
{
	return "Death Rally Archive";
}

std::vector<std::string> ArchiveType_BPA_DRally::fileExtensions() const
{
	return {
		"bpa",
	};
}

std::vector<std::string> ArchiveType_BPA_DRally::games() const
{
	return {
		"Death Rally",
	};
}

ArchiveType::Certainty ArchiveType_BPA_DRally::isInstance(
	stream::input& content) const
{
	auto lenArchive = content.size();

	// File too short
	// TESTED BY: fmt_bpa_drally_isinstance_c01
	// TESTED BY: fmt_bpa_drally_isinstance_c02
	if (lenArchive < BPA_FIRST_FILE_OFFSET) return DefinitelyNo;

	content.seekg(0, stream::start);
	uint32_t numFiles;
	content >> u32le(numFiles);

	// Can't store more than 255 files in the fixed-length FAT.
	// TESTED BY: fmt_bpa_drally_isinstance_c03
	if (numFiles > 255) return DefinitelyNo;

	stream::pos lenContent = BPA_FIRST_FILE_OFFSET;

	// Check each FAT entry
	for (unsigned int i = 0; i < numFiles; i++) {
		std::string fn;
		content >> nullPadded(fn, BPA_FILENAME_FIELD_LEN);
		// Make sure there aren't any invalid characters in the filename
		for (unsigned int j = 0; j < fn.length(); j++) {
			// Decrypt the char
			auto c = bpa_decrypt(j, fn[j]);

			// Fail on control characters in the filename
			// TESTED BY: fmt_bpa_drally_isinstance_c04
			if (c < 32) return DefinitelyNo;
		}

		uint32_t lenEntry;
		content >> u32le(lenEntry);
		lenContent += lenEntry;

		// If a file entry points past the end of the archive then it's an invalid
		// format.
		// TESTED BY: fmt_bpa_drally_isinstance_c05
		if (lenContent > lenArchive) return DefinitelyNo;
	}

	// If we've made it this far, this is almost certainly a BPA file.
	// TESTED BY: fmt_bpa_drally_isinstance_c00
	return DefinitelyYes;
}

std::shared_ptr<Archive> ArchiveType_BPA_DRally::create(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	content->seekp(0, stream::start);
	content->write(std::string(BPA_FIRST_FILE_OFFSET, '\0'));
	return std::make_shared<Archive_BPA_DRally>(std::move(content));
}

std::shared_ptr<Archive> ArchiveType_BPA_DRally::open(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	return std::make_shared<Archive_BPA_DRally>(std::move(content));
}

SuppFilenames ArchiveType_BPA_DRally::getRequiredSupps(stream::input& content,
	const std::string& filename) const
{
	return {};
}


Archive_BPA_DRally::Archive_BPA_DRally(std::unique_ptr<stream::inout> content)
	:	Archive_FAT(std::move(content), BPA_FIRST_FILE_OFFSET, BPA_MAX_FILENAME_LEN)
{
	this->content->seekg(0, stream::start);
	uint32_t numFiles;
	*this->content >> u32le(numFiles);
	this->vcFAT.reserve(numFiles);

	stream::pos nextOffset = BPA_FIRST_FILE_OFFSET;
	for (unsigned int i = 0; i < numFiles; i++) {
		auto f = this->createNewFATEntry();

		*this->content
			>> nullPadded(f->strName, BPA_FILENAME_FIELD_LEN)
			>> u32le(f->storedSize)
		;

		// Decrypt the filename
		for (unsigned int j = 0; j < f->strName.length(); j++) {
			f->strName[j] = bpa_decrypt(j, f->strName[j]);
		}

		f->iIndex = i;
		f->lenHeader = 0;
		f->type = FILETYPE_GENERIC;
		f->fAttr = File::Attribute::Compressed;
		f->bValid = true;
		f->iOffset = nextOffset;
		f->realSize = f->storedSize;

		nextOffset += f->storedSize;
		this->vcFAT.push_back(std::move(f));
	}
}

Archive_BPA_DRally::~Archive_BPA_DRally()
{
}

void Archive_BPA_DRally::updateFileName(const FATEntry *pid, const std::string& strNewName)
{
	// TESTED BY: fmt_bpa_drally_rename
	assert(strNewName.length() <= BPA_MAX_FILENAME_LEN);
	std::string nameEncrypted;
	for (unsigned int j = 0; j < strNewName.length(); j++) {
		nameEncrypted += bpa_encrypt(j, strNewName[j]);
	}
	this->content->seekp(BPA_FAT_FILENAME_OFFSET(pid->iIndex), stream::start);
	*this->content << nullPadded(nameEncrypted, BPA_FILENAME_FIELD_LEN);
	return;
}

void Archive_BPA_DRally::updateFileSize(const FATEntry *pid, stream::delta sizeDelta)
{
	// TESTED BY: fmt_bpa_drally_insert*
	// TESTED BY: fmt_bpa_drally_resize*
	this->content->seekp(BPA_FAT_FILESIZE_OFFSET(pid->iIndex), stream::start);
	*this->content << u32le(pid->storedSize);
	return;
}

void Archive_BPA_DRally::preInsertFile(const FATEntry *idBeforeThis,
	FATEntry *pNewEntry)
{
	// TESTED BY: fmt_bpa_drally_insert*
	assert(pNewEntry->strName.length() <= BPA_MAX_FILENAME_LEN);

	// Set the format-specific variables
	pNewEntry->lenHeader = 0;

	// TODO: See if this is in fact a hard limitation
	if (this->vcFAT.size() >= BPA_MAX_FILES) {
		throw stream::error("too many files, maximum is " TOSTRING(BPA_MAX_FILES));
	}
	this->content->seekp(BPA_FATENTRY_OFFSET(pNewEntry->iIndex), stream::start);
	this->content->insert(BPA_FAT_ENTRY_LEN);
	boost::to_upper(pNewEntry->strName);

	std::string nameEncrypted;
	for (unsigned int j = 0; j < pNewEntry->strName.length(); j++) {
		nameEncrypted += bpa_encrypt(j, pNewEntry->strName[j]);
	}

	// Write out the entry
	*this->content
		<< nullPadded(nameEncrypted, BPA_FILENAME_FIELD_LEN)
		<< u32le(pNewEntry->storedSize);

	// Because the FAT is a fixed size we have to remove a blank entry to
	// compensate for the entry we just added.
	if (this->vcFAT.size() > 0) {
		unsigned int indexLast = BPA_MAX_FILES - 1;
		for (auto i = this->vcFAT.rbegin(); i != this->vcFAT.rend(); i++) {
			auto pFAT = dynamic_cast<const FATEntry *>(i->get());
			if (pFAT->iIndex != indexLast) {
				// The previous slot is free, so delete it
				this->content->seekp(indexLast * BPA_FAT_ENTRY_LEN, stream::start);
				this->content->remove(BPA_FAT_ENTRY_LEN);
				break;
			} else {
				indexLast = pFAT->iIndex - 1;
			}
		}

		// Make sure an entry was removed.  This should never fail as failure would
		// indicate there were 200+ files, which means an exception should've been
		// thrown at the start of this function.
		//assert(indexLast >= 0);
		assert(indexLast < BPA_MAX_FILES); // unsigned version of previous line
	} else {
		// No files so just remove the following entry
		this->content->seekp(BPA_FATENTRY_OFFSET(1), stream::start);
		this->content->remove(BPA_FAT_ENTRY_LEN);
	}

	this->updateFileCount(this->vcFAT.size() + 1);
	return;
}

void Archive_BPA_DRally::preRemoveFile(const FATEntry *pid)
{
	// TESTED BY: fmt_bpa_drally_remove*

	// Remove the FAT entry
	this->content->seekp(BPA_FATENTRY_OFFSET(pid->iIndex), stream::start);
	this->content->remove(BPA_FAT_ENTRY_LEN);

	// Add an empty FAT entry onto the end to keep the FAT the same size
	const FATEntry *pFAT = dynamic_cast<const FATEntry *>(this->vcFAT.back().get());
	this->content->seekp(BPA_FATENTRY_OFFSET(pFAT->iIndex + 1), stream::start);
	this->content->insert(BPA_FAT_ENTRY_LEN);

	this->updateFileCount(this->vcFAT.size() - 1);
	return;
}

void Archive_BPA_DRally::updateFileCount(uint32_t iNewCount)
{
	// TESTED BY: fmt_bpa_drally_insert*
	// TESTED BY: fmt_bpa_drally_remove*
	this->content->seekp(BPA_FILECOUNT_OFFSET, stream::start);
	*this->content << u32le(iNewCount);
	return;
}

} // namespace gamearchive
} // namespace camoto
