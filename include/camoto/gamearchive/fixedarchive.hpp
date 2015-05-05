/**
 * @file  camoto/gamearchive/fixedarchive.hpp
 * @brief Generic archive providing access to "files" at specific offsets and
 *        lengths in a host file (e.g. game levels stored in an .exe file.)
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

#ifndef _CAMOTO_FIXEDARCHIVE_HPP_
#define _CAMOTO_FIXEDARCHIVE_HPP_

#include <vector>
#include <camoto/gamearchive/archive.hpp>
#include <camoto/stream_sub.hpp>

#ifndef DLL_EXPORT
#define DLL_EXPORT
#endif

namespace camoto {
namespace gamearchive {

struct FixedArchiveFile;

/// Generic archive with fixed offsets and sizes.
/**
 * This class provides access to "files" at specific offsets and lengths in a
 * host file (e.g. game levels stored in an .exe file.)
 */
class FixedArchive: virtual public Archive
{
	public:
		struct FixedEntry: virtual public File {
			const FixedArchiveFile *fixed;
			unsigned int index;  ///< Index into FixedArchiveFile array

			/// Convert a FileHandle into a FixedEntry pointer
			inline static FixedEntry *cast(const Archive::FileHandle& id)
			{
				return dynamic_cast<FixedArchive::FixedEntry *>(
					const_cast<Archive::File*>(&*id)
				);
			}
		};

		FixedArchive(std::unique_ptr<stream::inout> content,
			std::vector<FixedArchiveFile> vcFiles);
		virtual ~FixedArchive();

		virtual FileHandle find(const std::string& strFilename) const;
		virtual const FileVector& files(void) const;
		virtual bool isValid(const FileHandle& id) const;
		virtual std::unique_ptr<stream::inout> open(const FileHandle& id,
			bool useFilter);

		/**
		 * @note Will always throw an exception as there are never any subfolders.
		 */
		virtual std::shared_ptr<Archive> openFolder(const FileHandle& id);

		/**
		 * @note Will always throw an exception as the files are fixed and
		 *       thus can't be added to.
		 */
		virtual FileHandle insert(const FileHandle& idBeforeThis,
			const std::string& strFilename, stream::pos storedSize, std::string type,
			File::Attribute attr
		);

		/**
		 * @note Will always throw an exception as the files are fixed and
		 *       thus can't be removed.
		 */
		virtual void remove(FileHandle& id);

		/**
		 * @note Will always throw an exception as it makes no sense to rename
		 *       the made up filenames in this archive format.
		 */
		virtual void rename(FileHandle& id, const std::string& strNewName);

		/**
		 * @note Will always throw an exception as fixed files can't be moved.
		 */
		virtual void move(const FileHandle& idBeforeThis, FileHandle& id);

		/**
		 * @note Will always throw an exception as fixed files can't be resized.
		 */
		virtual void resize(FileHandle& id, stream::pos newStoredSize,
			stream::pos newRealSize);

		virtual void flush();

	/// Test code only, do not use, see util.hpp.
	friend FileHandle DLL_EXPORT getFileAt(const FileVector& files,
		unsigned int index);

	protected:
		// The archive stream must be mutable, because we need to change it by
		// seeking and reading data in our get() functions, which don't logically
		// change the archive's state.
		mutable std::shared_ptr<stream::inout> content;

		/// Array of files passed in via the constructor.
		std::vector<FixedArchiveFile> vcFiles;

		// This is a vector of file entries.  Although we have a specific FAT type
		// for each entry we can't use a vector of them here because getFileList()
		// must return a vector of the base type.  So instead each FAT entry type
		// inherits from the base type so that the specific FAT entry types can
		// still be added to this vector.
		//
		// The entries in this vector can be in any order (not necessarily the
		// order on-disk.  Use the iIndex member for that.)
		FileVector vcFixedEntries;
};

/// Callback function to "resize" files in a fixed archive.
/**
 * The callback function, if successful, should update fat.storedSize and
 * fat.realSize.
 *
 * When newStored and newReal are both equal to (stream::len)-1 then the file
 * is not being resized, but the real size is being queried.  Update
 * fat.realSize if needed (defaults to fat.storedSize).
 *
 * If the file can't be resized as requested, throw stream::error.
 */
typedef std::function<void(stream::inout& arch,
	FixedArchive::FixedEntry *fat, stream::len newStored, stream::len newReal)>
	FA_ResizeCallback;

/// Value to put in FixedArchiveFile::fnResize when resizing is not possible.
#define RESIZE_NONE FA_ResizeCallback()

/// File declaration structure
/**
 * This structure is used to describe a file contained within the fixed
 * archive.  It is intended to be used to hard-code the list of files
 * in a specific handler's .cpp file.
 */
struct FixedArchiveFile {
	unsigned long offset;  ///< Offset of the subfile in the parent
	unsigned long size;    ///< Length of the subfile in bytes
	std::string name;      ///< Filename of the subfile
	std::string filter;    ///< Filter type
	FA_ResizeCallback fnResize; ///< Callback if a file needs to be resized
};

inline std::shared_ptr<FixedArchive> make_FixedArchive(
	std::unique_ptr<stream::inout> content,
	const std::vector<FixedArchiveFile>& vcFiles)
{
	return std::make_shared<FixedArchive>(std::move(content), vcFiles);
}

} // namespace gamearchive
} // namespace camoto

#endif // _CAMOTO_FIXEDARCHIVE_HPP_
