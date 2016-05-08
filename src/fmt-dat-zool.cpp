/**
 * @file  fmt-dat-zool.cpp
 * @brief Implementation of Zool .DAT file reader/writer.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/DAT_Format_%28Zool%29
 *
 * Copyright (C) 2010-2016 Adam Nielsen <malvineous@shikadi.net>
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

#include "fmt-dat-zool.hpp"

#define DAT_CHUNK_SIZE          512
#define DAT_HEADER_LEN          2  // Offset of EOF
#define DAT_FAT_OFFSET          DAT_HEADER_LEN
#define DAT_FILENAME_FIELD_LEN  8
#define DAT_MAX_FILENAME_LEN    DAT_FILENAME_FIELD_LEN
#define DAT_FAT_ENTRY_LEN       10  // filename + u16le size
#define DAT_FIRST_FILE_OFFSET   DAT_CHUNK_SIZE  // empty archive only

#define DAT_SAFETY_MAX_FILECOUNT  8192 // Maximum value we will load

#define DAT_FATENTRY_OFFSET(e) (DAT_HEADER_LEN + e->iIndex * DAT_FAT_ENTRY_LEN)

#define DAT_FILENAME_OFFSET(e) DAT_FATENTRY_OFFSET(e)
#define DAT_FILEOFFSET_OFFSET(e) (DAT_FATENTRY_OFFSET(e) + DAT_FILENAME_FIELD_LEN)

namespace camoto {
namespace gamearchive {

ArchiveType_DAT_Zool::ArchiveType_DAT_Zool()
{
}

ArchiveType_DAT_Zool::~ArchiveType_DAT_Zool()
{
}

std::string ArchiveType_DAT_Zool::code() const
{
	return "dat-zool";
}

std::string ArchiveType_DAT_Zool::friendlyName() const
{
	return "Zool DAT File";
}

std::vector<std::string> ArchiveType_DAT_Zool::fileExtensions() const
{
	return {
		"dat",
	};
}

std::vector<std::string> ArchiveType_DAT_Zool::games() const
{
	return {
		"Zool",
	};
}

ArchiveType::Certainty ArchiveType_DAT_Zool::isInstance(
	stream::input& content) const
{
	stream::pos lenArchive = content.size();

	// File too short
	// TESTED BY: fmt_dat_zool_isinstance_c01
	if (lenArchive < DAT_CHUNK_SIZE) return DefinitelyNo;

	uint16_t eofChunk;
	content.seekg(0, stream::start);
	content >> u16le(eofChunk);

	// Incorrect archive size
	// TESTED BY: fmt_dat_zool_isinstance_c02
	if (eofChunk * DAT_CHUNK_SIZE != lenArchive) return DefinitelyNo;

	// TESTED BY: fmt_dat_zool_isinstance_c00
	return DefinitelyYes;
}

std::shared_ptr<Archive> ArchiveType_DAT_Zool::create(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	content->seekp(0, stream::start);
	*content << nullPadded("\x01\x00", DAT_CHUNK_SIZE);
	return std::make_shared<Archive_DAT_Zool>(std::move(content));
}

std::shared_ptr<Archive> ArchiveType_DAT_Zool::open(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	return std::make_shared<Archive_DAT_Zool>(std::move(content));
}

SuppFilenames ArchiveType_DAT_Zool::getRequiredSupps(stream::input& data,
	const std::string& filename) const
{
	return {};
}


Archive_DAT_Zool::Archive_DAT_Zool(std::unique_ptr<stream::inout> content)
	:	Archive_FAT(std::move(content), DAT_FIRST_FILE_OFFSET, DAT_MAX_FILENAME_LEN)
{
	this->content->seekg(DAT_FAT_OFFSET, stream::start);

	for (unsigned int i = 0; i < DAT_SAFETY_MAX_FILECOUNT; i++) {
		auto f = std::make_unique<FATEntry>();

		f->iIndex = i;
		f->lenHeader = 0;
		f->type = FILETYPE_GENERIC;
		f->fAttr = File::Attribute::Default;
		f->bValid = true;

		// Read the data in from the FAT entry in the file
		char filename[DAT_FILENAME_FIELD_LEN];
		this->content->read(filename, DAT_FILENAME_FIELD_LEN);

		// Reached last file?
		if (filename[0] == 0) break;

		// Remove filename padding
		for (int i = DAT_FILENAME_FIELD_LEN - 1; i >= 0; i--) {
			if (filename[i] == ' ') filename[i] = 0;
		}
		f->strName = filename;

		*this->content >> u16le(f->iOffset);
		f->iOffset *= DAT_CHUNK_SIZE;

		// Update the size of the previous entry
		if (this->vcFAT.size()) {
			auto prev = FATEntry::cast(this->vcFAT.back());
			prev->realSize = prev->storedSize = f->iOffset - prev->iOffset;
		}
		this->vcFAT.push_back(std::move(f));
	}

	// Set size of last file
	if (this->vcFAT.size()) {
		this->content->seekg(0, stream::start);
		uint16_t finalChunk;
		*this->content >> u16le(finalChunk);
		finalChunk *= DAT_CHUNK_SIZE;

		auto prev = FATEntry::cast(this->vcFAT.back());

		// Fall back to the filesize in case the final chunk is invalid
		if (finalChunk < prev->iOffset) finalChunk = this->content->size();

		prev->realSize = prev->storedSize = finalChunk - prev->iOffset;
	}
}

Archive_DAT_Zool::~Archive_DAT_Zool()
{
}

void Archive_DAT_Zool::resize(const FileHandle& id, stream::len newStoredSize,
	stream::len newRealSize)
{
	// Pad the file up to the nearest chunk boundary.
	newStoredSize += DAT_CHUNK_SIZE - (newStoredSize % DAT_CHUNK_SIZE);
	assert(newStoredSize % DAT_CHUNK_SIZE == 0);

	Archive_FAT::resize(id, newStoredSize, newRealSize);
	return;
}

void Archive_DAT_Zool::updateFileName(const FATEntry *pid,
	const std::string& strNewName)
{
	assert(strNewName.length() <= DAT_MAX_FILENAME_LEN);
	this->content->seekp(DAT_FILENAME_OFFSET(pid), stream::start);
	std::string paddedName(DAT_FILENAME_FIELD_LEN, ' ');
	for (int i = 0; i < std::min<int>(strNewName.length(), DAT_FILENAME_FIELD_LEN); i++) {
		paddedName[i] = strNewName[i];
	}
	*this->content << nullPadded(paddedName, DAT_FILENAME_FIELD_LEN);
	return;
}

void Archive_DAT_Zool::updateFileOffset(const FATEntry *pid,
	stream::delta offDelta)
{
	// Files can only start at chunk boundaries.
	assert(pid->iOffset % DAT_CHUNK_SIZE == 0);

	this->content->seekp(DAT_FILEOFFSET_OFFSET(pid), stream::start);
	unsigned int offset = pid->iOffset / DAT_CHUNK_SIZE;
	*this->content << u16le(offset);

	// Also update the size, which will only have an effect for the last file in
	// the archive, as the size of the final file is stored as an offset.
	this->updateHeader();
	return;
}

void Archive_DAT_Zool::updateFileSize(const FATEntry *pid,
	stream::delta sizeDelta)
{
	// Only the size of the last file is stored
	auto lastFile = FATEntry::cast(this->vcFAT.back());
	if (pid->iIndex == lastFile->iIndex) {
		this->updateHeader();
	}
	return;
}

void Archive_DAT_Zool::preInsertFile(const FATEntry *idBeforeThis,
	FATEntry *pNewEntry)
{
	// TESTED BY: fmt_dat_zool_insert*
	assert(pNewEntry->strName.length() <= DAT_MAX_FILENAME_LEN);

	// Files can only start at chunk boundaries.
	assert(pNewEntry->iOffset % DAT_CHUNK_SIZE == 0);

	// Set the format-specific variables
	pNewEntry->lenHeader = 0;

	// Pad the file up to the nearest chunk boundary.
	pNewEntry->storedSize += (DAT_CHUNK_SIZE - (pNewEntry->storedSize % DAT_CHUNK_SIZE)) % DAT_CHUNK_SIZE;
	assert(pNewEntry->storedSize % DAT_CHUNK_SIZE == 0);

	stream::len maxFileSize = 32*1048576-512 - pNewEntry->iOffset;
	if (pNewEntry->storedSize > maxFileSize) {
		throw error(createString("This file is too large for this location in a "
			"Zool DAT file (max filesize is " << maxFileSize << " bytes.)"));
	}

	stream::len lenFAT = (this->vcFAT.size() + 1) * DAT_FAT_ENTRY_LEN;
	int curChunksFAT = (DAT_HEADER_LEN + lenFAT + DAT_CHUNK_SIZE - 1) / DAT_CHUNK_SIZE;
	stream::len lenPostFAT = lenFAT + DAT_FAT_ENTRY_LEN;
	int postChunksFAT = (DAT_HEADER_LEN + lenPostFAT + DAT_CHUNK_SIZE - 1) / DAT_CHUNK_SIZE;

	if (curChunksFAT != postChunksFAT) {
		// In order to add the new file to the FAT, we have to expand the FAT into
		// a new chunk.

		this->content->seekp(curChunksFAT * DAT_CHUNK_SIZE,
			stream::start);
		this->content->insert(DAT_CHUNK_SIZE);

		// Update the offsets now there's a new FAT chunk taking up space.
		this->shiftFiles(
			NULL,
			DAT_FAT_OFFSET + this->vcFAT.size() * DAT_FAT_ENTRY_LEN,
			DAT_CHUNK_SIZE,
			0
		);

		// Because the new entry isn't in the vector yet we need to shift it manually
		pNewEntry->iOffset += DAT_CHUNK_SIZE;
	}

	this->content->seekp(DAT_FATENTRY_OFFSET(pNewEntry), stream::start);
	this->content->insert(DAT_FAT_ENTRY_LEN);
	boost::to_upper(pNewEntry->strName);

	std::string paddedName(DAT_FILENAME_FIELD_LEN, ' ');
	for (int i = 0; i < std::min<int>(pNewEntry->strName.length(), DAT_FILENAME_FIELD_LEN); i++) {
		paddedName[i] = pNewEntry->strName[i];
	}

	unsigned int offset = pNewEntry->iOffset / DAT_CHUNK_SIZE;

	*this->content
		<< nullPadded(paddedName, DAT_FILENAME_FIELD_LEN)
		<< u16le(offset);

	// Trim off an equivalent number of unused bytes from the end of the FAT to
	// compensate for the new entry we inserted above.
	this->content->seekp(DAT_HEADER_LEN
		+ DAT_FAT_ENTRY_LEN * (this->vcFAT.size() + 1),
		stream::start);
	this->content->remove(DAT_FAT_ENTRY_LEN);

	return;
}

void Archive_DAT_Zool::postInsertFile(FATEntry *pNewEntry)
{
	// Update the size of the last file, which will update the header at the start
	// of the archive.
	this->updateHeader();
	return;
}

void Archive_DAT_Zool::preRemoveFile(const FATEntry *pid)
{
	// TESTED BY: fmt_dat_zool_remove*

	// Remove the FAT entry
	this->content->seekp(DAT_FATENTRY_OFFSET(pid), stream::start);
	this->content->remove(DAT_FAT_ENTRY_LEN);

	stream::len lenFAT = this->vcFAT.size() * DAT_FAT_ENTRY_LEN;
	int curChunksFAT = (DAT_HEADER_LEN + lenFAT + DAT_CHUNK_SIZE - 1) / DAT_CHUNK_SIZE;

	// Seek to the new end of the FAT, one FAT entry's worth of space before
	// the end of the last FAT chunk.
	this->content->seekp(curChunksFAT * DAT_CHUNK_SIZE
		- DAT_FAT_ENTRY_LEN, // account for removed FAT entry above
		stream::start);

	// Add more space to the end of the last FAT entry to compensate, bringing
	// it back up to the length of a full chunk.
	this->content->insert(DAT_FAT_ENTRY_LEN);

	return;
}

void Archive_DAT_Zool::postRemoveFile(const FATEntry *pid)
{
	stream::len lenFAT = (this->vcFAT.size() + 1) * DAT_FAT_ENTRY_LEN;
	int curChunksFAT = (DAT_HEADER_LEN + lenFAT + DAT_CHUNK_SIZE - 1) / DAT_CHUNK_SIZE;
	stream::len lenPostFAT = lenFAT - DAT_FAT_ENTRY_LEN;
	int postChunksFAT = (DAT_HEADER_LEN + lenPostFAT + DAT_CHUNK_SIZE - 1) / DAT_CHUNK_SIZE;

	if (curChunksFAT != postChunksFAT) {
		// Update the offsets now there's one less FAT chunk taking up space.  This
		// must be called before the FAT is altered, because it will write a new
		// offset into the FAT entry we're about to erase (and if we erase it first
		// it'll overwrite something else.)
		this->shiftFiles(
			NULL,
			DAT_FAT_OFFSET + this->vcFAT.size() * DAT_FAT_ENTRY_LEN,
			-DAT_CHUNK_SIZE,
			0
		);

		// Seek to the new end of the FAT, and remove a chunk.
		this->content->seekp(postChunksFAT * DAT_CHUNK_SIZE,
			stream::start);

		// After removing the file from the FAT, we have to shrink the FAT by
		// removing one leftover chunk.  (Minus the data removed above.)
		this->content->remove(DAT_CHUNK_SIZE);
	}

	// Update the size of the last file, which will update the header at the start
	// of the archive.
	this->updateHeader();
	return;
}

void Archive_DAT_Zool::updateHeader()
{
	uint16_t numChunks;

	if (this->vcFAT.empty()) {
		// Special case for removing the last file, since we won't be able to
		// calculate this from the last file's offset (since there will be no files
		// at all.)
		numChunks = 1;
	} else {
		auto pid = FATEntry::cast(this->vcFAT.back());
		numChunks = pid->iOffset / DAT_CHUNK_SIZE
			+ (pid->storedSize + DAT_CHUNK_SIZE - 1) / DAT_CHUNK_SIZE;
	}

	this->content->seekp(0, stream::start);
	*this->content << u16le(numChunks);
	return;
}

} // namespace gamearchive
} // namespace camoto
