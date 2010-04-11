/*
 * fatarchive.cpp - Implementation of a FAT-style archive format.
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

#include "fatarchive.hpp"
#include "debug.hpp"

namespace camoto {
namespace gamearchive {

refcount_declclass(FATEntry);

FATArchive::FATEntry::FATEntry()
{
	refcount_qenterclass(FATEntry);
}
FATArchive::FATEntry::~FATEntry()
{
	refcount_qexitclass(FATEntry);
}
std::string FATArchive::FATEntry::getContent() const
{
	std::ostringstream ss;
	ss << this->FileEntry::getContent() << ";offset=" << iOffset;
	return ss.str();
}


refcount_declclass(FATArchive);

FATArchive::FATArchive(iostream_sptr psArchive)
	throw (std::ios::failure) :
		psArchive(new segmented_stream(psArchive))
{
	refcount_enterclass(FATArchive);
}

FATArchive::~FATArchive()
	throw ()
{
	// Can't flush here as it could throw std::ios::failure and we have no way
	// of handling it.
	//this->flush(); // make sure it saves on close just in case
	refcount_exitclass(FATArchive);
}

const FATArchive::VC_ENTRYPTR& FATArchive::getFileList()
	throw ()
{
	return this->vcFAT;
}

FATArchive::EntryPtr FATArchive::find(std::string strFilename)
	throw ()
{
	// TESTED BY: fmt_grp_duke3d_*
	for (VC_ENTRYPTR::iterator i = this->vcFAT.begin(); i != this->vcFAT.end(); i++) {
		const FATEntry *pFAT = const_FATEntryPtr_from_EntryPtr(*i);
		if (boost::iequals(pFAT->strName, strFilename)) {
			return *i;  // *i is the original shared_ptr
		}
	}
	return EntryPtr();
}

bool FATArchive::isValid(const EntryPtr& id)
	throw ()
{
	const FATEntry *id2 = static_cast<const FATEntry *>(id.get());
	return ((id2) && (id2->bValid));
}

boost::shared_ptr<std::iostream> FATArchive::open(const EntryPtr& id)
	throw ()
{
	// TESTED BY: fmt_grp_duke3d_open
	const FATEntry *pFAT = const_FATEntryPtr_from_EntryPtr(id);
	substream_sptr psSub(
		new substream(
			this->psArchive,
			pFAT->iOffset,
			pFAT->iSize
		)
	);
	this->vcSubStream.push_back(psSub);
	return psSub;
}

FATArchive::EntryPtr FATArchive::insert(const EntryPtr& idBeforeThis, std::string strFilename, offset_t iSize)
	throw (std::ios::failure)
{
	// TESTED BY: fmt_grp_duke3d_insert2
	// TESTED BY: fmt_grp_duke3d_remove_insert
	// TESTED BY: fmt_grp_duke3d_insert_remove

	FATEntry *pNewFile = new FATEntry();
	EntryPtr ep(pNewFile);

	pNewFile->strName = strFilename;
	pNewFile->iSize = iSize;
	pNewFile->eType = EFT_USEFILENAME;
	pNewFile->fAttr = 0;
	pNewFile->bValid = true;

	FATEntry *pFATBeforeThis = NULL;
	if (this->isValid(idBeforeThis)) {
		// Insert just after idBefore
		// TESTED BY: fmt_grp_duke3d_insert_mid
		pFATBeforeThis = FATEntryPtr_from_EntryPtr(idBeforeThis);
		assert(pFATBeforeThis);
		pNewFile->iOffset = pFATBeforeThis->iOffset;
		pNewFile->iIndex = pFATBeforeThis->iIndex;

		// Update the offsets of any files located after this one (since they will
		// all have been shifted forward to make room for the insert.)
		this->shiftFiles(pNewFile->iOffset, pNewFile->iSize, 1);

	} else {
		// Append to end of archive
		// TESTED BY: fmt_grp_duke3d_insert_end
		FATEntry *pFATAfterThis = FATEntryPtr_from_EntryPtr(this->vcFAT.back());
		assert(pFATAfterThis);
		pNewFile->iOffset = pFATAfterThis->iOffset + pFATAfterThis->iSize;
		pNewFile->iIndex = pFATAfterThis->iIndex + 1;
	}

	// Update the vector
	if (this->isValid(idBeforeThis)) {
		// TESTED BY: fmt_grp_duke3d_insert_mid
		VC_ENTRYPTR::iterator itBeforeThis = std::find(this->vcFAT.begin(), this->vcFAT.end(), idBeforeThis);
		assert(itBeforeThis != this->vcFAT.end());
		this->vcFAT.insert(itBeforeThis, ep);
	} else {
		// TESTED BY: fmt_grp_duke3d_insert_end
		this->vcFAT.push_back(ep);
	}

	// Insert space for the file's data into the archive
	this->psArchive->seekp(pNewFile->iOffset);
	this->psArchive->insert(pNewFile->iSize);

	// Add the file's entry from the FAT
	this->insertFATEntry(pFATBeforeThis, pNewFile);

	return ep;
}

void FATArchive::remove(EntryPtr& id)
	throw (std::ios::failure)
{
	// TESTED BY: fmt_grp_duke3d_remove
	// TESTED BY: fmt_grp_duke3d_remove2
	// TESTED BY: fmt_grp_duke3d_remove_insert
	// TESTED BY: fmt_grp_duke3d_insert_remove

	// Make sure the caller doesn't try to remove something that doesn't exist!
	assert(this->isValid(id));

	FATEntry *pFATDel = FATEntryPtr_from_EntryPtr(id);
	assert(pFATDel);

	// Remove the file's data from the archive
	this->psArchive->seekp(pFATDel->iOffset);
	this->psArchive->remove(pFATDel->iSize);

	// Remove the file's entry from the FAT
	this->removeFATEntry(pFATDel);

	// Update the offsets of any files located after this one (since they will
	// all have been shifted back to fill the gap made by the removal.)
	this->shiftFiles(pFATDel->iOffset, -pFATDel->iSize, -1);

	// Mark it as invalid in case some other code is still holding on to it.
	pFATDel->bValid = false;

	// Update the vector
	VC_ENTRYPTR::iterator itErase = std::find(this->vcFAT.begin(), this->vcFAT.end(), id);
	assert(itErase != this->vcFAT.end());
	this->vcFAT.erase(itErase);

	return;
}

// Enlarge or shrink an existing file entry.
// Postconditions: Existing EntryPtrs and open files remain valid.
void FATArchive::resize(EntryPtr& id, size_t iNewSize)
	throw (std::ios::failure)
{
	assert(this->isValid(id));
	std::streamsize iDelta = iNewSize - id->iSize;
	FATEntry *pFAT = FATEntryPtr_from_EntryPtr(id);

	// Add or remove the data in the underlying stream
	io::stream_offset iStart;
	if (iDelta > 0) { // inserting data
		// TESTED BY: fmt_grp_duke3d_resize_larger
		iStart = pFAT->iOffset + pFAT->iSize;
		this->psArchive->seekp(iStart);
		this->psArchive->insert(iDelta);
	} else if (iDelta < 0) { // removing data
		// TESTED BY: fmt_grp_duke3d_resize_smaller
		iStart = pFAT->iOffset + iNewSize;
		this->psArchive->seekp(iStart);
		this->psArchive->remove(-iDelta);
	} else {
		return; // no change
	}

	pFAT->iSize += iDelta;

	// Update the FAT with the file's new size
	this->updateFileSize(pFAT);

	// Adjust the in-memory offsets etc. of the rest of the files in the archive,
	// including any open streams.
	this->shiftFiles(iStart, iDelta, 0);

	pFAT->iSize = iNewSize;
	return;
}

void FATArchive::flush()
	throw (std::ios::failure)
{
	// Write out to the underlying stream
	this->psArchive->commit();

	return;
}

FATArchive::EntryPtr FATArchive::entryPtrFromStream(const iostream_sptr openFile)
	throw ()
{
	substream *d = static_cast<substream *>(openFile.get());
	io::stream_offset offStart = d->getOffset();

	// Find an EntryPtr with the same offset
	for (VC_ENTRYPTR::iterator i = this->vcFAT.begin(); i != this->vcFAT.end(); i++) {
		FATEntry *pFAT = FATEntryPtr_from_EntryPtr(*i);
		if (pFAT->iOffset >= offStart) {
			return *i;
		}
	}
	return EntryPtr();
}

void FATArchive::shiftFiles(io::stream_offset offStart, std::streamsize deltaOffset,
	int deltaIndex)
	throw (std::ios::failure)
{
	for (VC_ENTRYPTR::iterator i = this->vcFAT.begin(); i != this->vcFAT.end(); i++) {
		FATEntry *pFAT = FATEntryPtr_from_EntryPtr(*i);
		if (pFAT->iOffset >= offStart) {
			// This file is located after the one we're deleting, so tweak its offset
			pFAT->iOffset += deltaOffset;
			this->updateFileOffset(pFAT);
			// We can't update the index until last or the other changes will end
			// up in the wrong spot!
			pFAT->iIndex += deltaIndex;
		}
	}

	// Relocate any open substreams
	for (substream_vc::iterator i = this->vcSubStream.begin(); i != this->vcSubStream.end(); i++) {
		if ((*i)->getOffset() >= offStart) {
			(*i)->relocate(deltaOffset);
		}
	}
	return;
}

} // namespace gamearchive
} // namespace camoto
