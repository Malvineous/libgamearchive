/**
 * @file   fmt-glb-raptor.cpp
 * @brief  Implementation of Raptor .GLB file reader/writer.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/GLB_Format
 *
 * Copyright (C) 2010-2013 Adam Nielsen <malvineous@shikadi.net>
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
#include <boost/weak_ptr.hpp>
#include <camoto/iostream_helpers.hpp>
#include <camoto/stream_memory.hpp>
#include <camoto/util.hpp>
#include "filter-glb-raptor.hpp"
#include "fmt-glb-raptor.hpp"

#define GLB_FILECOUNT_OFFSET    4
#define GLB_HEADER_LEN          28  // first FAT entry
#define GLB_FAT_OFFSET          GLB_HEADER_LEN
#define GLB_FILENAME_FIELD_LEN  16
#define GLB_MAX_FILENAME_LEN    (GLB_FILENAME_FIELD_LEN-1)
#define GLB_FAT_ENTRY_LEN       28
#define GLB_FIRST_FILE_OFFSET   GLB_FAT_OFFSET  // empty archive only

#define GLB_SAFETY_MAX_FILECOUNT  8192 // Maximum value we will load

#define GLB_FATENTRY_OFFSET(e) (GLB_HEADER_LEN + e->iIndex * GLB_FAT_ENTRY_LEN)

#define GLB_FILENAME_OFFSET(e)   (GLB_FATENTRY_OFFSET(e) + 12)
#define GLB_FILESIZE_OFFSET(e)   (GLB_FATENTRY_OFFSET(e) + 8)
#define GLB_FILEOFFSET_OFFSET(e) (GLB_FATENTRY_OFFSET(e) + 4)

// Uncomment to temporarily disable FAT encryption (for debugging)
//#define GLB_CLEARTEXT

namespace camoto {
namespace gamearchive {

GLBType::GLBType()
{
}

GLBType::~GLBType()
{
}

std::string GLBType::getArchiveCode() const
{
	return "glb-raptor";
}

std::string GLBType::getFriendlyName() const
{
	return "Raptor GLB File";
}

std::vector<std::string> GLBType::getFileExtensions() const
{
	std::vector<std::string> vcExtensions;
	vcExtensions.push_back("glb");
	return vcExtensions;
}

std::vector<std::string> GLBType::getGameList() const
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Raptor");
	return vcGames;
}

ArchiveType::Certainty GLBType::isInstance(stream::input_sptr psArchive) const
{
	uint8_t sig[4];
	psArchive->seekg(0, stream::start);
	psArchive->read(sig, 4);
	uint8_t sig_match[] = {0x64, 0x9B, 0xD1, 0x09};
	for (unsigned int i = 0; i < sizeof(sig_match); i++) {
		if (sig[i] != sig_match[i]) {
			// TESTED BY: fmt_glb_raptor_isinstance_c01
			return DefinitelyNo;
		}
	}

	// Don't really need to bother checking offset validity and other things,
	// the signature is good enough.

	// TESTED BY: fmt_glb_raptor_isinstance_c00
	return DefinitelyYes;
}

ArchivePtr GLBType::newArchive(stream::inout_sptr psArchive, SuppData& suppData) const
{
	psArchive->seekp(0, stream::start);
	psArchive << nullPadded(
#ifdef GLB_CLEARTEXT
		"\x00\x00\x00\x00\x00\x00\x00\x00"
		"\x00\x00\x00\x00\x00\x00\x00\x00"
		"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
#else
		"\x64\x9B\xD1\x09\x50\x9C\xDE\x11"
		"\x43\x7A\xB0\xE8\x2F\x7B\xBD\xF0"
		"\x22\x59\x8F\xC7\x0E\x5A\x9C\xCF\x01\x38\x6E\xA6"
#endif
		, GLB_HEADER_LEN);
	return ArchivePtr(new GLBArchive(psArchive));
}

ArchivePtr GLBType::open(stream::inout_sptr psArchive, SuppData& suppData) const
{
	return ArchivePtr(new GLBArchive(psArchive));
}

SuppFilenames GLBType::getRequiredSupps(stream::input_sptr data,
	const std::string& filenameArchive) const
{
	// No supplemental types/empty list
	return SuppFilenames();
}


/// Resize the substream without regard to the underlying data
/**
 * This is possible because every time we expand or shrink the FAT, we also make
 * extra room, so the underlying data is already of the correct size.  We just
 * have to tell the substream it can now use this extra data.
 */
void fakeResizeSubstream(boost::weak_ptr<stream::output_sub> w_s, stream::len newSize)
{
	stream::output_sub_sptr s = w_s.lock();
	if (!s) return;

	s->resize(newSize);
	return;
}

void dummyResize(stream::pos newSize)
{
	return;
}


GLBArchive::GLBArchive(stream::inout_sptr psArchive)
	:	FATArchive(psArchive, GLB_FIRST_FILE_OFFSET, GLB_MAX_FILENAME_LEN),
		fat(new stream::seg())
{
	GLBFATFilterType glbFilterType;
	uint32_t numFiles;
	{
		// Decode just enough of the FAT to get the file count, so we know the size
		// of the FAT
		stream::input_sub_sptr substrFAT(new stream::input_sub());
		substrFAT->open(psArchive, 0, GLB_HEADER_LEN);
#ifdef GLB_CLEARTEXT
		stream::input_sptr preFAT = substrFAT;
#else
		stream::input_sptr preFAT = glbFilterType.apply(substrFAT);
#endif
		preFAT->seekg(4, stream::start);
		preFAT >> u32le(numFiles);
	}

	// Copy the FAT into memory and decode it
	stream::input_sub_sptr substrFAT(new stream::input_sub());
	substrFAT->open(psArchive, 0, GLB_HEADER_LEN + numFiles * GLB_FAT_ENTRY_LEN);
#ifdef GLB_CLEARTEXT
		stream::input_sptr preFAT = substrFAT;
#else
		stream::input_sptr preFAT = glbFilterType.apply(substrFAT);
#endif
	stream::memory_sptr mem(new stream::memory());
	stream::copy(mem, preFAT);
	this->fat->open(mem);

	if (numFiles >= GLB_SAFETY_MAX_FILECOUNT) {
		throw stream::error("too many files or corrupted archive");
	}

	this->fat->seekg(GLB_FAT_OFFSET, stream::start);
	for (unsigned int i = 0; i < numFiles; i++) {
		FATEntry *fatEntry = new FATEntry();
		EntryPtr ep(fatEntry);

		fatEntry->iIndex = i;
		fatEntry->lenHeader = 0;
		fatEntry->type = FILETYPE_GENERIC;
		fatEntry->fAttr = EA_NONE;
		fatEntry->bValid = true;

		uint32_t glbFlags;

		// Read the data in from the FAT entry in the file
		this->fat
			>> u32le(glbFlags)
			>> u32le(fatEntry->iOffset)
			>> u32le(fatEntry->storedSize)
			>> nullPadded(fatEntry->strName, GLB_FILENAME_FIELD_LEN)
		;
		if (glbFlags == 0x01) {
			fatEntry->fAttr = EA_ENCRYPTED;
			fatEntry->filter = "glb-raptor";
		}
		fatEntry->realSize = fatEntry->storedSize;
		this->vcFAT.push_back(ep);
	}
}

GLBArchive::~GLBArchive()
{
}

void GLBArchive::flush()
{
	GLBFATFilterType glbFilterType;
	stream::output_sub_sptr substrFAT(new stream::output_sub);
	stream::fn_truncate fnTruncateSub = boost::bind<void>(&fakeResizeSubstream,
		boost::weak_ptr<stream::output_sub>(substrFAT), _1);
	substrFAT->open(this->psArchive, 0,
		GLB_HEADER_LEN + this->vcFAT.size() * GLB_FAT_ENTRY_LEN, fnTruncateSub);
#ifdef GLB_CLEARTEXT
	stream::output_sptr bareCrypt = substrFAT;
#else
	stream::fn_truncate fnTruncate = boost::bind<void>(&dummyResize, _1);
	stream::output_sptr bareCrypt = glbFilterType.apply((stream::output_sptr)substrFAT, fnTruncate);
#endif
	this->fat->seekg(0, stream::start);
	bareCrypt->seekp(0, stream::start);
	stream::copy(bareCrypt, this->fat);
	bareCrypt->flush();

	this->FATArchive::flush();
	return;
}

void GLBArchive::updateFileName(const FATEntry *pid, const std::string& strNewName)
{
	// TESTED BY: fmt_glb_raptor_rename
	assert(strNewName.length() <= GLB_MAX_FILENAME_LEN);
	this->fat->seekp(GLB_FILENAME_OFFSET(pid), stream::start);
	this->fat << nullPadded(strNewName, GLB_FILENAME_FIELD_LEN);
	return;
}

void GLBArchive::updateFileOffset(const FATEntry *pid, stream::delta offDelta)
{
	// TESTED BY: fmt_glb_raptor_insert*
	// TESTED BY: fmt_glb_raptor_resize*
	this->fat->seekp(GLB_FILEOFFSET_OFFSET(pid), stream::start);
	this->fat << u32le(pid->iOffset);
	return;
}

void GLBArchive::updateFileSize(const FATEntry *pid, stream::delta sizeDelta)
{
	// TESTED BY: fmt_glb_raptor_insert*
	// TESTED BY: fmt_glb_raptor_resize*
	this->fat->seekp(GLB_FILESIZE_OFFSET(pid), stream::start);
	this->fat << u32le(pid->storedSize);
	return;
}

FATArchive::FATEntry *GLBArchive::preInsertFile(const FATEntry *idBeforeThis, FATEntry *pNewEntry)
{
	// TESTED BY: fmt_glb_raptor_insert*
	assert(pNewEntry->strName.length() <= GLB_MAX_FILENAME_LEN);

	// Set the format-specific variables
	pNewEntry->lenHeader = 0;

	// Because the new entry isn't in the vector yet we need to shift it manually
	pNewEntry->iOffset += GLB_FAT_ENTRY_LEN;

	this->fat->seekp(GLB_FATENTRY_OFFSET(pNewEntry), stream::start);
	this->fat->insert(GLB_FAT_ENTRY_LEN);
	this->psArchive->seekp(GLB_FATENTRY_OFFSET(pNewEntry), stream::start);
	this->psArchive->insert(GLB_FAT_ENTRY_LEN);

	boost::to_upper(pNewEntry->strName);

	uint32_t flags = 0;
	if (pNewEntry->fAttr & EA_COMPRESSED) flags = 1;

	this->fat
		<< u32le(flags)
		<< u32le(pNewEntry->iOffset)
		<< u32le(pNewEntry->storedSize)
		<< nullPadded(pNewEntry->strName, GLB_FILENAME_FIELD_LEN)
	;

	// Update the offsets now there's a new FAT entry taking up space.
	this->shiftFiles(
		NULL,
		GLB_FAT_OFFSET + this->vcFAT.size() * GLB_FAT_ENTRY_LEN,
		GLB_FAT_ENTRY_LEN,
		0
	);

	this->updateFileCount(this->vcFAT.size() + 1);
	return pNewEntry;
}

void GLBArchive::preRemoveFile(const FATEntry *pid)
{
	// TESTED BY: fmt_glb_raptor_remove*

	// Update the offsets now there's one less FAT entry taking up space.  This
	// must be called before the FAT is altered, because it will write a new
	// offset into the FAT entry we're about to erase (and if we erase it first
	// it'll overwrite something else.)
	this->shiftFiles(
		NULL,
		GLB_FAT_OFFSET + this->vcFAT.size() * GLB_FAT_ENTRY_LEN,
		-GLB_FAT_ENTRY_LEN,
		0
	);

	this->fat->seekp(GLB_FATENTRY_OFFSET(pid), stream::start);
	this->fat->remove(GLB_FAT_ENTRY_LEN);

	this->psArchive->seekp(GLB_FATENTRY_OFFSET(pid), stream::start);
	this->psArchive->remove(GLB_FAT_ENTRY_LEN);

	this->updateFileCount(this->vcFAT.size() - 1);
	return;
}

void GLBArchive::updateFileCount(uint32_t iNewCount)
{
	// TESTED BY: fmt_glb_raptor_insert*
	// TESTED BY: fmt_glb_raptor_remove*
	this->fat->seekp(GLB_FILECOUNT_OFFSET, stream::start);
	this->fat << u32le(iNewCount);
	return;
}

} // namespace gamearchive
} // namespace camoto
