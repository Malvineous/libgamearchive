/*
 * archive.hpp - Declaration of top-level Archive class, for accessing file
 *               archives storing game data.
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

#ifndef _CAMOTO_GAMEARCHIVE_ARCHIVE_HPP_
#define _CAMOTO_GAMEARCHIVE_ARCHIVE_HPP_

#include <boost/shared_ptr.hpp>
#include <exception>
#include <iostream>
#include <sstream>
#include <vector>

#include <camoto/types.hpp>

namespace camoto {
namespace gamearchive {

enum E_FILETYPE {
	EFT_USEFILENAME, EFT_RAWEGA_PLANAR
};

enum E_ATTRIBUTE {
	EA_EMPTY      = 0x01,  // There's currently no file at this location
	EA_HIDDEN     = 0x02,  // File is hidden between two FAT entries
	EA_COMPRESSED = 0x04
};

enum E_METADATA {
	EM_DESCRIPTION  // Archive description
};

typedef std::vector<E_METADATA> VC_METADATA_ITEMS;

class ENotFound: public std::exception {
};
class EInvalidFormat: public std::exception {
};

// Multithreading: Only call one function in this class at a time.  Many of the
// functions seek around the underlying stream and thus will break if two or
// more functions are executing at the same time.
class Archive {

	public:
		// Base class to represent entries in the file.  Will be extended by
		// descendent classes to hold format-specific data.  The entries here will
		// be valid for all archive types.
		typedef unsigned long offset_t;
		struct FileEntry {
			bool bValid;          // Are the other fields valid?
			offset_t iSize;       // Size of the file
			std::string strName;  // Filename (may be empty for some archives)
			E_FILETYPE eType;     // A single E_FILETYPE value
			int fAttr;            // One or more E_ATTRIBUTE flags
			inline virtual std::string getContent() const {
				std::ostringstream ss;
				ss << "name=" << this->strName << ";size=" << this->iSize << ";type="
					<< this->eType << ";attr=" << fAttr;
				return ss.str();
			}
		};

		typedef boost::shared_ptr<FileEntry> EntryPtr;
		typedef std::vector<EntryPtr> VC_ENTRYPTR;

		// This function is called with a single io::stream_offset parameter when
		// the underlying archive file needs to be shrunk to the given size.
		FN_TRUNCATE fnTruncate;

		Archive()
			throw ();

		virtual ~Archive()
			throw ();

		// Find the given file.  Returns empty EntryPtr() if the file can't be
		// found (EntryPtr::bValid will be false.)
		virtual EntryPtr find(const std::string& strFilename)
			throw () = 0;

		virtual const VC_ENTRYPTR& getFileList(void)
			throw () = 0;

		// Checks the EntryPtr is not NULL (as it would be at the end of the file
		// list) and also that it still references a file (as it would no longer do
		// if that file has been deleted.)
		virtual bool isValid(const EntryPtr& id)
			throw () = 0;

		virtual iostream_sptr open(const EntryPtr& id)
			throw () = 0;

		// Insert a new file into the archive.  It will be inserted before
		// idBeforeThis, or at the end of the archive if idBeforeThis is not valid.
		// Does not check if this filename already exists - check first yourself
		// and don't add duplicates!
		//
		// Returns an EntryPtr to the newly added file, which can be immediately
		//   passed to open() if needed.
		//
		// Preconditions: strFilename is not used by a file already in the archive.
		// Postconditions: Existing EntryPtrs become invalid.  Any open files
		//   remain valid.
		virtual EntryPtr insert(const EntryPtr& idBeforeThis,
			const std::string& strFilename, offset_t iSize
		)
			throw (std::ios::failure) = 0;

		// Delete the given entry from the archive.
		//
		// Postconditions: Existing EntryPtrs become invalid.  Any open files
		//   remain valid.
		virtual void remove(EntryPtr& id)
			throw (std::ios::failure) = 0;

		// Does not invalidate existing EntryPtrs.  Will throw exceptions on
		// invalid names (e.g. name too long)
		virtual void rename(EntryPtr& id, const std::string& strNewName)
			throw (std::ios::failure) = 0;

		// Move an entry to a different position within the archive.  Take id and
		// place it after idBefore.  If idBefore is not valid, id will be moved to
		// become the first file in the archive.
		//
		// Postconditions: Existing EntryPtrs become invalid.  Any open files
		//   remain valid.
		virtual void move(const EntryPtr& idBeforeThis, EntryPtr& id)
			throw (std::ios::failure);

		// Enlarge or shrink an existing file entry.
		// Postconditions: Existing EntryPtrs and open files remain valid.
		virtual void resize(EntryPtr& id, size_t iNewSize)
			throw (std::ios::failure) = 0;

		// Write out any changes to the underlying stream.  Some functions write
		// immediately, others cache their changes.  All changes are written out
		// when the class is destroyed by an implicit call to this function,
		// however this function can be called at any time to write all pending
		// changes without destroying the class.  Note that some changes can
		// involve shuffling around many hundreds of megabytes of data, so don't
		// call this function unless you have good reason to!
		virtual void flush()
			throw (std::ios::failure) = 0;

		// Convert an iostream pointer of an already-open file back into an
		// EntryPtr.  This is useful after a call to insert/remove, as these
		// functions invalidate any existing EntryPtrs but the EntryPtr can be
		// recovered from the open stream with this function.
		virtual EntryPtr entryPtrFromStream(const iostream_sptr openFile)
			throw () = 0;

		// The metadata functions all have no-op defaults, they only need to be
		// overridden for archive formats that have metadata.

		// Get a list of supported metadata elements that can be set.
		virtual VC_METADATA_ITEMS getMetadataList() const
			throw ();
		// Get the value of a metadata element.
		virtual std::string getMetadata(E_METADATA item) const
			throw (std::ios::failure);
		// Change the value of a metadata element.
		virtual void setMetadata(E_METADATA item, const std::string& value) const
			throw (std::ios::failure);

};

typedef boost::shared_ptr<Archive> ArchivePtr;
typedef std::vector<ArchivePtr> VC_ARCHIVE;

} // namespace gamearchive
} // namespace camoto

#endif // _CAMOTO_GAMEARCHIVE_ARCHIVE_HPP_
