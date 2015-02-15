/**
 * @file  camoto/gamearchive/archive.hpp
 * @brief Declaration of top-level Archive class, for accessing file
 *        archives storing game data.
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

#ifndef _CAMOTO_GAMEARCHIVE_ARCHIVE_HPP_
#define _CAMOTO_GAMEARCHIVE_ARCHIVE_HPP_

#include <memory>
#include <exception>
#include <vector>

#include <camoto/stream.hpp>
#include <stdint.h>
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

/// Primary interface to an archive file.
/**
 * This class represents an archive file.  Its functions are used to manipulate
 * the contents of the archive.
 *
 * @note Multithreading: Only call one function in this class at a time.  Many
 *       of the functions seek around the underlying stream and thus will break
 *       if two or more functions are executing at the same time.
 */
class Archive: virtual public Metadata
{
	public:
		/// Base class to represent entries in the file.
		/**
		 * Will be extended by descendent classes to hold format-specific data.
		 * The entries here will be valid for all archive types.
		 */
		struct File {
			/// Are the other fields valid?
			/**
			 * This only confirms whether the rest of the values are valid, as
			 * opposed to Archive::isValid() which checks that the file still exists
			 * in the archive.
			 */
			bool bValid;

			/// Size of the file in the archive.
			stream::len storedSize;

			/// Size before filtering/compression (uncompressed size)
			/**
			 * If fAttr has EA_COMPRESSED set then this indicates the file size after
			 * decompression.  If the file is not compressed or filtered it will be
			 * ignored but by convention should be set to the same value as storedSize.
			 */
			stream::len realSize;

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
			File();

			/// Empty destructor
			virtual ~File();

			/// Helper function (for debugging) to return all the data as a string
			virtual std::string getContent() const;

			private:
				/// Can't copy File instances, must use references/pointers.
				File(const File&) = delete;
				File& operator=(const File&) = delete;
				File(File&&) = delete;
				File& operator=(File&&) = delete;
		};

		//typedef typename std::shared_ptr<File> FileHandle;
		typedef typename std::shared_ptr<const File> FileHandle;
		typedef typename std::vector<FileHandle> FileVector;

		/// Get a list of all files in the archive.
		/**
		 * @return A vector of FileHandle with one element for each file in the
		 *   archive.
		 */
		virtual const FileVector& files() const = 0;

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
		 * FileHandle instances only, as returned by files().
		 *
		 * @param strFilename
		 *   Name of the file to search for.
		 *
		 * @return Shared pointer to the requested file, or an empty pointer if the
		 *   file can't be found.
		 */
		virtual FileHandle find(const std::string& strFilename) const = 0;

		/// Checks that the File instance points to a file that still exists.
		/**
		 * This is different to the bValid member in File as it confirms
		 * that the File instance is still valid for this particular archive file.
		 *
		 * @param id
		 *   Entry to check.
		 *
		 * @return true if the parameter refers to a valid file that can be opened,
		 *   hasn't been deleted, etc.
		 */
		virtual bool isValid(const FileHandle& id) const = 0;

		/// Open a file in the archive.
		/**
		 * @param id
		 *   A valid iterator, obtained from find(), getFileList(), etc.
		 *
		 * @return A C++ iostream containing the file data.  Writes to this stream
		 *   will immediately update the data in the archive.  Writing beyond EOF
		 *   is not permitted - use resize() if the file needs to change size (grow
		 *   or shrink.)
		 *
		 * @note It would be nice to return a unique_ptr, but often the archive
		 *   instance will need to monitor all open files so that some can be moved
		 *   around, as files that come before them in the archive get resized.
		 */
		virtual std::shared_ptr<stream::inout> open(const FileHandle& id) = 0;

		/// Open a folder in the archive.
		/**
		 * There is a default implementation of this which triggers an
		 * assertion failure, which means the function only needs to be
		 * overridden for archives actually supporting subfolders.
		 *
		 * @note This function only needs to be implemented for archive formats
		 *   where each subfolder has an independent FAT.  For those formats which
		 *   simply have paths in the filenames, this function does not need to
		 *   be implemented.
		 *
		 * @pre The entry must have the EA_FOLDER attribute set.
		 *
		 * @param id
		 *   A valid iterator, obtained from find(), getFileList(), etc.
		 *
		 * @return Another Archive instance representing the files in the folder.
		 */
		virtual std::unique_ptr<Archive> openFolder(const FileHandle& id) = 0;

		/// Insert a new file into the archive.
		/**
		 * It will be inserted before idBeforeThis, or at the end of the archive if
		 * idBeforeThis is not valid.  Does not check if this filename already
		 * exists - check first yourself or you will add duplicates!
		 *
		 * @note For performance reasons, this operation is cached so it does not
		 *   immediately affect the archive file.  When the time comes to
		 *   flush() the changes, all the insert/delete/resize operations are
		 *   done in a single pass.  However providing this class is the sole
		 *   method of accessing the archive file, this is of no concern.
		 *
		 * @post Existing iterators become invalid.  Any open files remain valid.
		 *
		 * @param idBeforeThis
		 *   The new file will be inserted before this one.  If it is not valid,
		 *   the new file will be last in the archive.
		 *
		 * @param strFilename
		 *   Filename of the new file.
		 *
		 * @param storedSize
		 *   Initial size of the new file.  If the file is compressed (attr
		 *   includes EA_COMPRESSED) then this is the compressed size of the file -
		 *   the amount of space to allocate inside the archive.
		 *
		 * @param type
		 *   MIME-like file type, or empty string for generic file.  See
		 *   File::type.
		 *
		 * @param attr
		 *   File attributes (one or more E_ATTRIBUTEs)
		 *
		 * @return A pointer to the newly added file, which can be immediately
		 *   passed to open() if needed.
		 *
		 * @note The returned pointer may have a filter set, in the case of a
		 *   file with the EA_COMPRESSED attribute.  In this case the caller must
		 *   pass the data through the appropriate filter before writing it to the
		 *   file.  The file may also need to be resized if the filtered data ends
		 *   up being a different size to the unfiltered data.
		 *
		 * @post All existing iterators are invalidated.  However adding one to the
		 *   return value will produce a valid iterator to the same file that
		 *   idBeforeThis pointed to before the call.
		 */
		virtual FileHandle insert(const FileHandle& idBeforeThis,
			const std::string& strFilename, stream::len storedSize, std::string type,
			int attr) = 0;

		/// Delete the given entry from the archive.
		/**
		 * @note For performance reasons, this operation is cached so it does not
		 *   immediately affect the archive file.  When the time comes to
		 *   flush() the changes, all the insert/delete/resize operations are
		 *   done in a single pass.  However providing this class is the sole
		 *   method of accessing the archive file, this is of no concern.
		 *
		 * @param id
		 *   The file to delete.
		 *
		 * @post id->valid becomes false.  All existing iterators are invalidated.
		 *   Any open files remain valid.
		 */
		virtual void remove(FileHandle& id) = 0;

		/// Rename a file.
		/**
		 * @note Will throw exceptions on invalid names (e.g. name too long)
		 *
		 * @param id
		 *   The file to rename.
		 *
		 * @param strNewName
		 *   The new filename.
		 */
		virtual void rename(FileHandle& id, const std::string& strNewName)
			= 0;

		/// Move an entry to a different position within the archive.
		/**
		 * Take id and place it before idBeforeThis, or last if idBeforeThis is not
		 * valid.
		 *
		 * @param idBeforeThis
		 *   File will be inserted before this one.  If it is not valid, the file
		 *   will become last in the archive.
		 *
		 * @param id
		 *   File to move.
		 *
		 * @post All existing iterators are invalidated.  Any open files remain
		 *   valid.
		 */
		virtual void move(const FileHandle& idBeforeThis, FileHandle& id) = 0;

		/// Enlarge or shrink an existing file.
		/**
		 * @note For performance reasons, this operation is cached so it does not
		 *   immediately affect the archive file.  When the time comes to flush()
		 *   the changes, all the insert/delete/resize operations are done in a
		 *   single pass.  However providing this class is the sole method of
		 *   accessing the archive file stream, this is of no concern.
		 *
		 * @param id
		 *   File to resize.
		 *
		 * @param newStoredSize
		 *   File's new size.  This is the actual amount of space to allocate
		 *   within the archive file.  If this is smaller than the current size the
		 *   excess data is lost, if it is larger than the current size the new data
		 *   is undefined/random (whatever was there from before.)
		 *
		 * @param newRealSize
		 *   File's new size before filtering (if any.)  Should be set to the same
		 *   value as newStoredSize unless the file is compressed, in which case this
		 *   value will usually be larger (the decompressed size.)
		 *
		 * @post Existing iterators remain valid.
		 *
		 * @note Resizing files to zero will cause problems if files are already
		 *   opened.  This is because already open files are identified by offset
		 *   and having zero-length files means multiple files will share the same
		 *   offset.  If these are open during a resize and one of the zero-length
		 *   files is resized, all the streams sharing the same offset will be
		 *   resized (but the actual files in the archive won't.)  This problem
		 *   does not exist if the resize is done while none of the archive's files
		 *   are open.
		 */
		virtual void resize(FileHandle& id, stream::len newStoredSize,
			stream::len newRealSize) = 0;

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
		 */
		virtual void flush() = 0;

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
		virtual int getSupportedAttributes() const = 0;
};

} // namespace gamearchive
} // namespace camoto

#endif // _CAMOTO_GAMEARCHIVE_ARCHIVE_HPP_
