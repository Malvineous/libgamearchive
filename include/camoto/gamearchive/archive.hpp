/**
 * @file   archive.hpp
 * @brief  Declaration of top-level Archive class, for accessing file
 *         archives storing game data.
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
#include <camoto/metadata.hpp>

namespace camoto {
namespace gamearchive {

#define FILETYPE_GENERIC         std::string()
#define FILETYPE_RAWEGA_PLANAR   "image/ega-planar"

#define FILTER_NONE              std::string()

/// File attribute flags.  Can be OR'd together.
enum E_ATTRIBUTE {
	EA_NONE       = 0x00,  ///< No attributes set
	EA_EMPTY      = 0x01,  ///< There's currently no file at this location
	EA_HIDDEN     = 0x02,  ///< File is hidden between two FAT entries
	EA_COMPRESSED = 0x04,  ///< File is compressed
	EA_ENCRYPTED  = 0x08,  ///< File is encrypted
	EA_FOLDER     = 0x80,  ///< This entry is a folder, not a file
};

/// Generic "file not found" exception.
class ENotFound: public std::exception {
};

/// Generic "invalid archive format" exception.
class EInvalidFormat: public std::exception {
};

class Archive;

/// Shared pointer to an Archive.
typedef boost::shared_ptr<Archive> ArchivePtr;


/// Primary interface to an archive file.
/**
 * This class represents an archive file.  Its functions are used to manipulate
 * the contents of the archive.
 *
 * @note Multithreading: Only call one function in this class at a time.  Many
 *       of the functions seek around the underlying stream and thus will break
 *       if two or more functions are executing at the same time.
 */
class Archive: virtual public Metadata {

	public:
		/// Offset type used by FileEntry
		typedef unsigned long offset_t;

		/// Base class to represent entries in the file.
		/**
		 * Will be extended by descendent classes to hold format-specific data.
		 * The entries here will be valid for all archive types.
		 */
		struct FileEntry {
			/// Are the other fields valid?
			/**
			 * This only confirms whether the rest of the values are valid, as
			 * opposed to Archive::isValid() which checks that the file still exists
			 * in the archive.
			 */
			bool bValid;

			/// Size of the file
			offset_t iSize;

			/// Size after decompression
			/**
			 * If fAttr has EA_COMPRESSED set then this indicates the file size after
			 * decompression.  It may be zero if the field is unused in the archive,
			 * but it should always be set when adding a file.
			 */
			offset_t iExpandedSize;

			/// Filename (may be empty for some archives)
			std::string strName;

			/// File type (like MIME type) or empty for unknown/generic.
			/**
			 * This is used for archives which store file types independently of
			 * filenames.  Leave this as an empty string when the archive does not
			 * treat the data specially (e.g. the type can be determined by the
			 * filename or file signature.)
			 *
			 * If however, the archive has a specific field for the file type
			 * (separate from the filename), then that field should be made
			 * accessible here, probably via mapping the archive codes/values to
			 * MIME-style strings.
			 */
			std::string type;

			/// Compression/encryption type (like MIME type) or empty if not
			/// compressed or encrypted.
			/**
			 * This is used for archives which can compress some or all of their
			 * files.  Leave this as an empty string when this particular file
			 * is not compressed or encrypted.
			 *
			 * When opening the archive file the Archive instance will return the
			 * compressed data exactly as it appears in the file.  If this field
			 * is not empty it can be used by the caller to load a compression filter
			 * to decompress (and recompress) the data.
			 */
			std::string filter;

			/// One or more E_ATTRIBUTE flags
			int fAttr;


			/// Empty constructor
			FileEntry()
				throw ();

			/// Empty destructor
			virtual ~FileEntry()
				throw ();

			/// Helper function (for debugging) to return all the data as a string
			virtual std::string getContent() const
				throw ();

			private:
				/// Can't copy FileEntry instances, must use references/pointers.
				FileEntry(const FileEntry&);
		};

		/// Shared pointer to a FileEntry
		typedef boost::shared_ptr<FileEntry> EntryPtr;

		/// Vector of shared FileEntry pointers
		typedef std::vector<EntryPtr> VC_ENTRYPTR;

		/// Truncate callback
		/**
		 * This function is called with a single unsigned long parameter when
		 * the underlying archive file needs to be shrunk or enlarged to the given
		 * size.  It must be set to a valid function before flush() is called.
		 *
		 * The function signature is:
		 * @code
		 * void fnTruncate(unsigned long newLength);
		 * @endcode
		 *
		 * This example uses boost::bind to package up a call to the Linux
		 * truncate() function (which requires both a filename and size) such that
		 * the filename is supplied in advance and not required when flush() makes
		 * the call.
		 *
		 * @code
		 * Archive *pArchive = ...
		 * pArchive->fnTruncate = boost::bind<void>(truncate, "archive.dat", _1);
		 * pArchive->flush();  // calls truncate("archive.dat", 123)
		 * @endcode
		 *
		 * Unfortunately since there is no cross-platform method for changing a
		 * file's size from an open file handle, this is a necessary evil to avoid
		 * passing the archive filename around all over the place.
		 */
		FN_TRUNCATE fnTruncate;

		Archive()
			throw ();

		virtual ~Archive()
			throw ();

		/// Find the given file.
		/**
		 * In the unlikely event that the filename exists multiple times in the
		 * archive, any one of them could be returned (though it will usually be
		 * the first.)  This is not unheard of, the registered version of Halloween
		 * Harry (Alien Carnage) contains two different music tracks, both called
		 * seb3.mod.
		 *
		 * This could cause problems in a GUI environment when a file is dragged
		 * but its duplicate is affected instead.  For this reason it is best to
		 * use this function only with user input, and to otherwise use
		 * EntryPtrs only, as returned by getFileList().
		 *
		 * @param  strFilename Name of the file to search for.
		 * @return EntryPtr to the requested file, or an empty EntryPtr() if the
		 * file can't be found (EntryPtr::bValid will be false.)
		 */
		virtual EntryPtr find(const std::string& strFilename) const
			throw () = 0;

		/// Get a list of all files in the archive.
		/**
		 * @return A vector of FileEntry with one element for each file in the
		 *         archive.
		 */
		virtual const VC_ENTRYPTR& getFileList(void) const
			throw () = 0;

		/// Checks that the EntryPtr points to a file that still exists.
		/**
		 * This is different to the bValid member in FileEntry as it confirms
		 * that the EntryPtr is still valid for this particular archive file.
		 *
		 * @param  id Entry to check
		 * @return true if the EntryPtr is valid.
		 */
		virtual bool isValid(const EntryPtr& id) const
			throw () = 0;

		/// Open a file in the archive.
		/**
		 * @param  id A valid file entry, obtained from find(), getFileList(), etc.
		 * @return A C++ iostream containing the file data.  Writes to this stream
		 *         will immediately update the data in the archive.  Writing beyond
		 *         EOF is not permitted - use resize() if the file needs to change
		 *         size (grow or shrink.)
		 */
		virtual iostream_sptr open(const EntryPtr& id)
			throw () = 0;

		/// Open a folder in the archive.
		/**
		 * There is a default implementation of this which triggers an
		 * assertion failure, which means the function only needs to be
		 * overridden for archives actually supporting subfolders.
		 *
		 * @note  This function only needs to be implemented for archive formats
		 *   where each subfolder has an independent FAT.  For those formats which
		 *   simply have paths in the filenames, this function does not need to
		 *   be implemented.
		 *
		 * @pre  The entry must have the EA_FOLDER attribute set.
		 *
		 * @param id
		 *   A valid file entry, obtained from find(), getFileList(), etc.
		 *
		 * @return  Another Archive instance representing the files in the folder.
		 */
		virtual ArchivePtr openFolder(const EntryPtr& id)
			throw (std::ios::failure);

		/// Insert a new file into the archive.
		/**
		 * It will be inserted before idBeforeThis, or at the end of the archive if
		 * idBeforeThis is not valid.  Does not check if this filename already
		 * exists - check first yourself or you will add duplicates!
		 *
		 * @note  For performance reasons, this operation is cached so it does not
		 *   immediately affect the archive file.  When the time comes to
		 *   flush() the changes, all the insert/delete/resize operations are
		 *   done in a single pass.  However providing this class is the sole
		 *   method of accessing the archive file, this is of no concern.
		 *
		 * @post  Existing EntryPtrs become invalid.  Any open files remain valid.
		 *
		 * @param idBeforeThis
		 *   The new file will be inserted before this one.  If it is not valid,
		 *   the new file will be last in the archive.
		 *
		 * @param strFilename
		 *   Filename of the new file.
		 *
		 * @param iSize
		 *   Initial size of the new file.
		 *
		 * @param type
		 *   MIME-like file type, or empty string for generic file.  See
		 *   FileEntry::type.
		 *
		 * @param attr
		 *   File attributes (one or more E_ATTRIBUTEs)
		 *
		 * @return An EntryPtr to the newly added file, which can be immediately
		 *   passed to open() if needed.
		 */
		virtual EntryPtr insert(const EntryPtr& idBeforeThis,
			const std::string& strFilename, offset_t iSize, std::string type,
			int attr
		)
			throw (std::ios::failure) = 0;

		/// Delete the given entry from the archive.
		/**
		 * @note   For performance reasons, this operation is cached so it does not
		 *         immediately affect the archive file.  When the time comes to
		 *         flush() the changes, all the insert/delete/resize operations are
		 *         done in a single pass.  However providing this class is the sole
		 *         method of accessing the archive file, this is of no concern.
		 * @param  id The file to delete.
		 * @post   Existing EntryPtrs become invalid.  Any open files remain valid.
		 */
		virtual void remove(EntryPtr& id)
			throw (std::ios::failure) = 0;

		/// Rename a file.
		/**
		 * @note   Will throw exceptions on invalid names (e.g. name too long)
		 * @param  id The file to rename.
		 * @param  strNewName The new filename.
		 * @post   Existing EntryPtrs remain valid.
		 */
		virtual void rename(EntryPtr& id, const std::string& strNewName)
			throw (std::ios::failure) = 0;

		/// Move an entry to a different position within the archive.
		/**
		 * Take id and place it before idBeforeThis, or last if idBeforeThis is not
		 * valid.
		 *
		 * @param  id File to move
		 * @param  idBeforeThis File will be inserted before this one.  If it is
		 *         not valid, the file will become last in the archive.
		 * @post   Existing EntryPtrs become invalid.  Any open files remain valid.
		 */
		virtual void move(const EntryPtr& idBeforeThis, EntryPtr& id)
			throw (std::ios::failure);

		/// Enlarge or shrink an existing file.
		/**
		 * @note   For performance reasons, this operation is cached so it does not
		 *         immediately affect the archive file.  When the time comes to
		 *         flush() the changes, all the insert/delete/resize operations are
		 *         done in a single pass.  However providing this class is the sole
		 *         method of accessing the archive file, this is of no concern.
		 * @param  id File to resize.
		 * @param  iNewSize File's new size.  If this is smaller than the current
		 *         size the excess data is lost, if it is larger than the current
		 *         size the new data is random (whatever was there from before.)
		 * @post   Existing EntryPtrs remain valid.
		 * @note   Resizing files to zero will cause problems if files are already
		 *         opened.  This is because already open files are identified by
		 *         offset and having zero-length files means multiple files will
		 *         share the same offset.  If these are open during a resize and
		 *         one of the zero-length files is resized, all the streams
		 *         sharing the same offset will be resized (but the actual files in
		 *         the archive won't.)  This problem does not exist if the resize
		 *         is done while none of the archive's files are open.
		 */
		virtual void resize(EntryPtr& id, size_t iNewSize)
			throw (std::ios::failure) = 0;

		/// Write out any cached changes to the underlying stream.
		/**
		 * Some functions write their changes to the archive file immediately,
		 * while others cache their changes for performance reasons.  Any cached
		 * changes are NOT automatically written out when the class is destroyed
		 * (as there would be no way to handle any write failures), so this
		 * function must be called before the class is destroyed or the archive
		 * file will become corrupted.
		 *
		 * This function can also be called at any time to write all pending
		 * changes without destroying the class.  However some changes can involve
		 * shuffling around many hundreds of megabytes of data, so don't call this
		 * function unless you have good reason to!
		 *
		 * @pre    The \ref fnTruncate member must be valid before this call.
		 */
		virtual void flush()
			throw (std::ios::failure) = 0;

		/// Find out which attributes can be set on files in this archive.
		/**
		 * If an attribute is not returned by this function, that attribute must
		 * not be supplied to insert().
		 *
		 * Note to archive format implementors: There is a default implementation
		 * of this function which returns 0.  Thus this only needs to be overridden
		 * if the archive format does actually support any of the attributes.
		 *
		 * @return Zero or more E_ATTRIBUTE values OR'd together.
		 */
		virtual int getSupportedAttributes() const
			throw ();

};

/// Vector of Archive shared pointers.
typedef std::vector<ArchivePtr> VC_ARCHIVE;

} // namespace gamearchive
} // namespace camoto

#endif // _CAMOTO_GAMEARCHIVE_ARCHIVE_HPP_
