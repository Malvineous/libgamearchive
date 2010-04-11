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

#include <camoto/gamearchive.hpp>

#include <iostream>
//#include <fstream>
//#include <sstream>
#include <vector>
#include <boost/iostreams/stream.hpp>

#include "substream.hpp"
#include "segmented_stream.hpp"

namespace camoto {
namespace gamearchive {

namespace io = boost::iostreams;

// Convert generic entry pointers into the type extended by this class
#define FATEntryPtr_from_EntryPtr(eid) \
	static_cast<FATEntry *>( \
		const_cast<FileEntry *>( \
			(eid).get() \
		) \
	);
#define const_FATEntryPtr_from_EntryPtr(eid) \
	static_cast<const FATEntry *>((eid).get());


class FATArchive: virtual public Archive {

	protected:
		typedef unsigned long size_t;

		//typedef boost::shared_ptr<segmented_stream> segstream_sptr;
		//std::iostream *psArchive;
		segstream_sptr psArchive;
		//iostream_sptr psArchive;

		struct FATEntry: public FileEntry {
			int iIndex; // can't use vector order as entries are passed around outside the vector
			offset_t iOffset;
			//bool bValid;

			FATEntry();
			virtual ~FATEntry();
			virtual std::string getContent() const;
		};

		// Don't think we need this as we won't be passing around FATEntry pointers
		// outside our descendent classes.  (They'll all be converted to base
		// FileEntry pointers first.
		//typedef boost::shared_ptr<FATEntry> FATEntry_sptr;

		// This is a vector of file entries.  Although we have a specific FAT type
		// for each entry we can't use a vector of them here because getFileList()
		// must return a vector of the base type.  So instead each FAT entry type
		// inherits from the base type so that the specific FAT entry types can
		// still be added to this vector.
		//
		// The entries in this vector can be in any order (not necessarily the
		// order on-disk.  Use the iIndex member for that.)
		VC_ENTRYPTR vcFAT;

		//typedef boost::iostreams::stream<substream_device> substream_str;
		//typedef boost::shared_ptr<substream_str> substream_sptr;
		typedef std::vector<substream_sptr> substream_vc;
		substream_vc vcSubStream; // List of substreams currently open

	public:

		// We can't throw an 'invalid format' error here, because we wouldn't be
		// in this function if the format's isValid() function returned true!
		FATArchive(iostream_sptr psArchive)
			throw (std::ios::failure);

		virtual ~FATArchive()
			throw ();

		virtual EntryPtr find(std::string strFilename)
			throw ();

		virtual const VC_ENTRYPTR& getFileList(void)
			throw ();

		virtual bool isValid(const EntryPtr& id)
			throw ();

		virtual boost::shared_ptr<std::iostream> open(const EntryPtr& id)
			throw ();

		// Does not check if this filename already exists - check first yourself
		// and don't add duplicates!
		virtual EntryPtr insert(const EntryPtr& idBeforeThis, std::string strFilename, offset_t iSize)
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
		virtual void updateFileOffset(const FATEntry *pid)
			throw (std::ios::failure) = 0;

		// Adjust the size of the given file in the on-disk FAT.
		virtual void updateFileSize(const FATEntry *pid)
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
		// new entry.
		// Invalidates existing EntryPtrs.
		virtual void insertFATEntry(const FATEntry *idBeforeThis, FATEntry *pNewEntry)
			throw (std::ios::failure) = 0;

		// Remove the entry from the FAT.  The file data has already been removed
		// from the archive, but the offsets have not yet been updated.
		// On return, pid will be removed from the FAT vector and the on-disk
		// offsets of files following this one will be updated (via calls to
		// updateFileOffset()) - so they don't need changing here.  However the
		// offsets will not take into account any changes resulting from the FAT
		// changing size, which must be handled by this function.
		// Invalidates existing EntryPtrs.
		virtual void removeFATEntry(const FATEntry *pid)
			throw (std::ios::failure) = 0;

};

} // namespace gamearchive
} // namespace camoto

#endif // _CAMOTO_FATARCHIVE_HPP_
