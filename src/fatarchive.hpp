/*
 * fatarchive.hpp - Implementation of a FAT-style archive format.
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

#ifndef _CAMOTO_FATARCHIVE_HPP_
#define _CAMOTO_FATARCHIVE_HPP_

#include <camoto/gamearchive/archive.hpp>

#include <iostream>
#include <vector>
#include <boost/iostreams/stream.hpp>

#include "substream.hpp"
#include "segmented_stream.hpp"

namespace camoto {
namespace gamearchive {

namespace io = boost::iostreams;

class FATArchive: virtual public Archive {

	protected:
		// The archive stream must be mutable, because we need to change it by
		// seeking and reading data in our get() functions, which don't logically
		// change the archive's state.
		mutable segstream_sptr psArchive;

		io::stream_offset offFirstFile; // offset of first file in empty archive

		struct FATEntry: virtual public FileEntry {
			int iIndex; // can't use vector order as entries are passed around outside the vector
			offset_t iOffset;
			offset_t lenHeader;  // length of embedded FAT entry at start of file data

			FATEntry();
			virtual ~FATEntry();
			virtual std::string getContent() const;
		};

		// This is a vector of file entries.  Although we have a specific FAT type
		// for each entry we can't use a vector of them here because getFileList()
		// must return a vector of the base type.  So instead each FAT entry type
		// inherits from the base type so that the specific FAT entry types can
		// still be added to this vector.
		//
		// The entries in this vector can be in any order (not necessarily the
		// order on-disk.  Use the iIndex member for that.)
		VC_ENTRYPTR vcFAT;

		typedef std::vector<substream_sptr> substream_vc;
		substream_vc vcSubStream; // List of substreams currently open

	public:

		// We can't throw an 'invalid format' error here, because we wouldn't be
		// in this function if the format's isValid() function returned true!
		FATArchive(iostream_sptr psArchive, io::stream_offset offFirstFile)
			throw (std::ios::failure);

		virtual ~FATArchive()
			throw ();

		virtual EntryPtr find(const std::string& strFilename)
			throw ();

		virtual const VC_ENTRYPTR& getFileList(void)
			throw ();

		virtual bool isValid(const EntryPtr& id)
			throw ();

		virtual boost::shared_ptr<std::iostream> open(const EntryPtr& id)
			throw ();

		virtual EntryPtr insert(const EntryPtr& idBeforeThis,
			const std::string& strFilename, offset_t iSize
		)
			throw (std::ios::failure);

		virtual void remove(EntryPtr& id)
			throw (std::ios::failure);

		// rename() [inherited here from Archive] must be implemented by descendent
		// classes

		// move() uses implementation from Archive

		virtual void resize(EntryPtr& id, size_t iNewSize)
			throw (std::ios::failure);

		virtual void flush()
			throw (std::ios::failure);

		virtual EntryPtr entryPtrFromStream(const iostream_sptr openFile)
			throw ();

		// Shift any files *starting* at or after offStart by delta bytes, updating
		// the internal offsets and index numbers.  The FAT is updated by calling
		// updateFileOffset().  If offStart is in the middle of a file (which
		// should never happen) that file won't be affected, only those following
		// it.  This function must notify any open files that their offset has
		// moved.
		virtual void shiftFiles(io::stream_offset offStart, std::streamsize deltaOffset,
			int deltaIndex)
			throw (std::ios::failure);

		// Methods to be filled out by descendent classes

		// Adjust the offset of the given file in the on-disk FAT.
		virtual void updateFileOffset(const FATEntry *pid, std::streamsize offDelta)
			throw (std::ios::failure) = 0;

		// Adjust the size of the given file in the on-disk FAT.
		virtual void updateFileSize(const FATEntry *pid, std::streamsize sizeDelta)
			throw (std::ios::failure) = 0;

		// Insert a new entry in the on-disk FAT.  It should be inserted before
		// idBeforeThis, or at the end of the archive if idBeforeThis is not valid.
		// All the FAT entries will be updated with new offsets after this function
		// returns, however the offsets will not take into account any changes
		// resulting from the FAT changing size, which must be handled by this
		// function.  The FAT vector does not contain the new entry, so
		// pNewEntry->iIndex may be the same as an existing file (but the existing
		// file will have its index moved after this function returns.)
		// All this function has to do is make room in the FAT and write out the
		// new entry.  It also needs to set the lenHeader field in pNewEntry.
		// Invalidates existing EntryPtrs. TODO - does it?
		virtual void preInsertFile(const FATEntry *idBeforeThis, FATEntry *pNewEntry)
			throw (std::ios::failure) = 0;

		// Called after the file data has been inserted.  Only needs to be
		// overridden if there are tasks to perform after the file has been set.
		// pNewEntry can be changed if need be, but this is not required.
		virtual void postInsertFile(FATEntry *pNewEntry)
			throw (std::ios::failure);

		// Remove the entry from the FAT.  The file data has already been removed
		// from the archive, but the offsets have not yet been updated.
		// On return, pid will be removed from the FAT vector and the on-disk
		// offsets of files following this one will be updated (via calls to
		// updateFileOffset()) - so they don't need changing here.  However the
		// offsets will not take into account any changes resulting from the FAT
		// changing size, which must be handled by this function.
		// Invalidates existing EntryPtrs.
		virtual void preRemoveFile(const FATEntry *pid)
			throw (std::ios::failure) = 0;

		// Called after the file data has been removed.  Only override if needed.
		virtual void postRemoveFile(const FATEntry *pid)
			throw (std::ios::failure);

};

} // namespace gamearchive
} // namespace camoto

#endif // _CAMOTO_FATARCHIVE_HPP_
