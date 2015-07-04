/**
 * @file  fmt-dat-got.cpp
 * @brief Implementation of God of Thunder .DAT file reader/writer.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/DAT_Format_(God_of_Thunder)
 *
 * Copyright (C) 2011 Adam Nielsen <malvineous@shikadi.net>
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
#include <functional>
#include <camoto/iostream_helpers.hpp>
#include <camoto/util.hpp>

#include "fmt-dat-got.hpp"

#define GOT_MAX_FILES         256
#define GOT_MAX_FILENAME_LEN    8
#define GOT_FILENAME_FIELD_LEN  9
#define GOT_FAT_ENTRY_LEN      23
#define GOT_FAT_LENGTH        (GOT_MAX_FILES * GOT_FAT_ENTRY_LEN)
#define GOT_FIRST_FILE_OFFSET GOT_FAT_LENGTH

#define GOT_FATENTRY_OFFSET(e)   (e->iIndex * GOT_FAT_ENTRY_LEN)

#define GOT_FILENAME_OFFSET(e)    GOT_FATENTRY_OFFSET(e)
#define GOT_FILEOFFSET_OFFSET(e) (GOT_FATENTRY_OFFSET(e) + GOT_FILENAME_FIELD_LEN)
#define GOT_FILESIZE_OFFSET(e)   (GOT_FILEOFFSET_OFFSET(e) + 4)

// Comment the next line out and also in the test file to run the tests
// with no encryption to assist in debugging.
#define USE_XOR

namespace camoto {
namespace gamearchive {

ArchiveType_DAT_GoT::ArchiveType_DAT_GoT()
{
}

ArchiveType_DAT_GoT::~ArchiveType_DAT_GoT()
{
}

std::string ArchiveType_DAT_GoT::code() const
{
	return "dat-got";
}

std::string ArchiveType_DAT_GoT::friendlyName() const
{
	return "God of Thunder Resource File";
}

std::vector<std::string> ArchiveType_DAT_GoT::fileExtensions() const
{
	return {
		"dat",
	};
}

std::vector<std::string> ArchiveType_DAT_GoT::games() const
{
	return {
		"God of Thunder",
	};
}

ArchiveType::Certainty ArchiveType_DAT_GoT::isInstance(
	stream::input& content) const
{
	stream::pos lenArchive = content.size();

	// Make sure the archive is large enough to hold a FAT
	// TESTED BY: fmt_dat_got_isinstance_c02
	if (lenArchive < GOT_FAT_LENGTH) return DefinitelyNo;

	// Create a substream to decrypt the FAT
	auto fatSubStream = std::make_unique<stream::input_sub>(
		// Create a fake shared_ptr so we can run the content through a decryption
		// filter.
		std::shared_ptr<stream::input>(std::shared_ptr<stream::input>(), &content),
		0,
		GOT_MAX_FILES * GOT_FAT_ENTRY_LEN
	);

#ifdef USE_XOR
	auto fatStream = std::make_shared<stream::input_filtered>(
		std::move(fatSubStream),
		std::make_shared<filter_xor_crypt>(0, 128)
	);
#else
	auto fatStream = std::move(fatSubStream);
#endif

	fatStream->seekg(0, stream::start);

	try {
		// Check each FAT entry
		char fn[GOT_FILENAME_FIELD_LEN];
		for (int i = 0; i < GOT_MAX_FILES; i++) {
			fatStream->read(fn, GOT_FILENAME_FIELD_LEN);
			// Make sure there aren't any invalid characters in the filename
			for (int j = 0; j < GOT_MAX_FILENAME_LEN; j++) {
				if (!fn[j]) break; // stop on terminating null

				// Fail on control characters in the filename
				if (fn[j] < 32) return DefinitelyNo; // TESTED BY: fmt_dat_got_isinstance_c01
			}

			uint32_t offEntry, lenEntry, lenDecomp;
			uint16_t flags;
			*fatStream
				>> u32le(offEntry)
				>> u32le(lenEntry)
				>> u32le(lenDecomp)
				>> u16le(flags)
			;

			// If a file entry points past the end of the archive then it's an invalid
			// format.
			// TESTED BY: fmt_dat_got_isinstance_c03
			// TESTED BY: fmt_dat_got_isinstance_c04
			if (offEntry + lenEntry > lenArchive) return DefinitelyNo;
		}
	} catch (const stream::incomplete_read&) {
		return DefinitelyNo;
	}

	// If we've made it this far, this is almost certainly a GoT file.

	// TESTED BY: fmt_dat_got_isinstance_c00
	return DefinitelyYes;
}

std::shared_ptr<Archive> ArchiveType_DAT_GoT::create(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	// Create an empty FAT (of 0x00 bytes) and XOR encode it.  We should really
	// use the XOR filter but it's much quicker to do it directly.
	char emptyFAT[GOT_FAT_LENGTH];
	for (int i = 0, j = 128; i < GOT_FAT_LENGTH; i++, j++) {
		emptyFAT[i] = (uint8_t)j;
	}
	content->seekp(0, stream::start);
	content->write(emptyFAT, GOT_FAT_LENGTH);
	return std::make_shared<Archive_DAT_GoT>(std::move(content));
}

std::shared_ptr<Archive> ArchiveType_DAT_GoT::open(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	return std::make_shared<Archive_DAT_GoT>(std::move(content));
}

SuppFilenames ArchiveType_DAT_GoT::getRequiredSupps(stream::input& content,
	const std::string& filename) const
{
	return {};
}


Archive_DAT_GoT::Archive_DAT_GoT(std::unique_ptr<stream::inout> content)
	:	Archive_FAT(std::move(content), GOT_FIRST_FILE_OFFSET, GOT_MAX_FILENAME_LEN)
{
	// Create a substream to decrypt the FAT
	auto fatSubStream = std::make_unique<stream::sub>(
		this->content,
		0,
		GOT_MAX_FILES * GOT_FAT_ENTRY_LEN,
		std::bind<void>(&Archive_DAT_GoT::truncateFAT, this, std::placeholders::_2)
	);

#ifdef USE_XOR
	auto fatFilter = std::make_unique<stream::filtered>(
		std::move(fatSubStream),
		std::make_shared<filter_xor_crypt>(0, 128),
		std::make_shared<filter_xor_crypt>(0, 128),
		stream::fn_notify_prefiltered_size()
	);
#else
	auto fatFilter = std::move(fatSubStream);
#endif

	this->fatStream = std::make_unique<stream::seg>(std::move(fatFilter));
	this->fatStream->seekg(0, stream::start);

	this->vcFAT.reserve(256);
	for (int i = 0; i < GOT_MAX_FILES; i++) {
		auto f = this->createNewFATEntry();

		uint16_t flags;
		*this->fatStream
			>> nullPadded(f->strName, GOT_FILENAME_FIELD_LEN)
			>> u32le(f->iOffset)
			>> u32le(f->storedSize)
			>> u32le(f->realSize)
			>> u16le(flags)
		;

		f->iIndex = i;
		f->lenHeader = 0;
		f->type = FILETYPE_GENERIC;
		if (flags & 1) {
			f->fAttr = File::Attribute::Compressed;
			f->filter = "lzss-got";
		} else {
			f->fAttr = File::Attribute::Default;
		}
		f->bValid = true;
		// Blank FAT entries have an offset of zero
		if (f->iOffset > 0) {
			this->vcFAT.push_back(std::move(f));
		}
	}
}

Archive_DAT_GoT::~Archive_DAT_GoT()
{
}

void Archive_DAT_GoT::flush()
{
	this->fatStream->flush();

	// Commit this->content
	this->Archive_FAT::flush();
	return;
}

Archive::File::Attribute Archive_DAT_GoT::getSupportedAttributes() const
{
	return File::Attribute::Compressed;
}

void Archive_DAT_GoT::updateFileName(const FATEntry *pid, const std::string& strNewName)
{
	// TESTED BY: fmt_got_dat_rename
	assert(strNewName.length() <= GOT_MAX_FILENAME_LEN);
	this->fatStream->seekp(GOT_FILENAME_OFFSET(pid), stream::start);
	*this->fatStream << nullPadded(strNewName, GOT_FILENAME_FIELD_LEN);
	return;
}

void Archive_DAT_GoT::updateFileOffset(const FATEntry *pid, stream::delta offDelta)
{
	// TESTED BY: fmt_got_dat_insert*
	// TESTED BY: fmt_got_dat_resize*
	this->fatStream->seekp(GOT_FILEOFFSET_OFFSET(pid), stream::start);
	*this->fatStream << u32le(pid->iOffset);
	return;
}

void Archive_DAT_GoT::updateFileSize(const FATEntry *pid, stream::delta sizeDelta)
{
	// TESTED BY: fmt_got_dat_insert*
	// TESTED BY: fmt_got_dat_resize*
	this->fatStream->seekp(GOT_FILESIZE_OFFSET(pid), stream::start);
	*this->fatStream
		<< u32le(pid->storedSize)
		<< u32le(pid->realSize)
	;
	return;
}

void Archive_DAT_GoT::preInsertFile(const FATEntry *idBeforeThis, FATEntry *pNewEntry)
{
	// TESTED BY: fmt_got_dat_insert*
	assert(pNewEntry->strName.length() <= GOT_MAX_FILENAME_LEN);

	// Set the format-specific variables
	pNewEntry->lenHeader = 0;

	// Make sure there's space for one more entry
	if (this->vcFAT.size() >= GOT_MAX_FILES) {
		throw stream::error("too many files, maximum is "
			TOSTRING(GOT_MAX_FILES));
	}

	if (pNewEntry->fAttr & File::Attribute::Compressed) {
		pNewEntry->filter = "lzss-got";
	}

	// Allocate the space in the FAT now, so that the correct offsets can be
	// updated on return.
	this->fatStream->seekp(GOT_FATENTRY_OFFSET(pNewEntry), stream::start);
	this->fatStream->insert(GOT_FAT_ENTRY_LEN);
	boost::to_upper(pNewEntry->strName);

	// Because the FAT is a fixed size we have to remove a blank entry to
	// compensate for the entry we just added.
	if (this->vcFAT.size() > 0) {
		unsigned int indexLast = GOT_MAX_FILES - 1;
		for (FileVector::reverse_iterator i = this->vcFAT.rbegin(); i != this->vcFAT.rend(); i++) {
			const FATEntry *pFAT = dynamic_cast<const FATEntry *>(i->get());
			if (pFAT->iIndex != indexLast) {
				// The previous slot is free, so delete it
				this->fatStream->seekp(indexLast * GOT_FAT_ENTRY_LEN, stream::start);
				this->fatStream->remove(GOT_FAT_ENTRY_LEN);
				break;
			} else {
				indexLast = pFAT->iIndex - 1;
			}
		}

		// Make sure an entry was removed.  This should never fail as failure would
		// indicate there were more than GOT_MAX_FILES files, which means an
		// exception should've been thrown at the start of this function.
		//assert(indexLast >= 0);
		assert(indexLast < GOT_MAX_FILES); // unsigned version of previous line
	} else {
		// No files so just remove the following entry
		this->fatStream->seekp(1 * GOT_FAT_ENTRY_LEN, stream::start);
		this->fatStream->remove(GOT_FAT_ENTRY_LEN);
	}

	return;
}

void Archive_DAT_GoT::postInsertFile(FATEntry *pNewEntry)
{
	// Write out the entry into the space we allocated in preInsertFile(),
	// now that the sizes are set.
	this->fatStream->seekp(GOT_FATENTRY_OFFSET(pNewEntry), stream::start);
	uint16_t flags = (pNewEntry->fAttr & File::Attribute::Compressed) ? 1 : 0; // 0 == not compressed
	*this->fatStream
		<< nullPadded(pNewEntry->strName, GOT_FILENAME_FIELD_LEN)
		<< u32le(pNewEntry->iOffset)
		<< u32le(pNewEntry->storedSize)
		<< u32le(pNewEntry->realSize)
		<< u16le(flags)
	;
	return;
}

void Archive_DAT_GoT::preRemoveFile(const FATEntry *pid)
{
	// TESTED BY: fmt_got_dat_remove*

	// Remove the FAT entry
	this->fatStream->seekp(GOT_FATENTRY_OFFSET(pid), stream::start);
	this->fatStream->remove(GOT_FAT_ENTRY_LEN);

	// Add an empty FAT entry onto the end to keep the FAT the same size
	const FATEntry *pFAT = dynamic_cast<const FATEntry *>(this->vcFAT.back().get());
	this->fatStream->seekp((pFAT->iIndex + 1) * GOT_FAT_ENTRY_LEN, stream::start);
	this->fatStream->insert(GOT_FAT_ENTRY_LEN);

	return;
}

void Archive_DAT_GoT::truncateFAT(stream::pos newSize)
{
	// Sanity check to make sure the FAT is not actually changing size.
	assert(newSize == GOT_FAT_LENGTH);
	return;
}

} // namespace gamearchive
} // namespace camoto
