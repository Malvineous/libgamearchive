/**
 * @file   fixedarchive.hpp
 * @brief  Generic archive providing access to "files" at specific offsets and
 *         lengths in a host file (e.g. game levels stored in an .exe file.)
 *
 * Copyright (C) 2010-2012 Adam Nielsen <malvineous@shikadi.net>
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

namespace camoto {
namespace gamearchive {

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
};

class FixedArchive: virtual public Archive
{
	protected:
		// The archive stream must be mutable, because we need to change it by
		// seeking and reading data in our get() functions, which don't logically
		// change the archive's state.
		mutable stream::inout_sptr psArchive;

		struct FixedEntry: virtual public FileEntry {
			unsigned int index;  ///< Index into FixedArchiveFile array
		};

		std::vector<FixedArchiveFile> files;  ///< Array of files passed in via the constructor

		// This is a vector of file entries.  Although we have a specific FAT type
		// for each entry we can't use a vector of them here because getFileList()
		// must return a vector of the base type.  So instead each FAT entry type
		// inherits from the base type so that the specific FAT entry types can
		// still be added to this vector.
		//
		// The entries in this vector can be in any order (not necessarily the
		// order on-disk.  Use the iIndex member for that.)
		VC_ENTRYPTR vcFixedEntries;

		typedef std::vector<stream::sub_sptr> substream_vc;
		substream_vc vcSubStream; // List of substreams currently open

	public:
		FixedArchive(stream::inout_sptr psArchive, std::vector<FixedArchiveFile> files);
		virtual ~FixedArchive();

		virtual EntryPtr find(const std::string& strFilename) const;
		virtual const VC_ENTRYPTR& getFileList(void) const;
		virtual bool isValid(const EntryPtr id) const;
		virtual stream::inout_sptr open(const EntryPtr id);

		/**
		 * @note Will always throw an exception as there are never any subfolders.
		 */
		virtual ArchivePtr openFolder(const EntryPtr id);

		/**
		 * @note Will always throw an exception as the files are fixed and
		 *       thus can't be added to.
		 */
		virtual EntryPtr insert(const EntryPtr idBeforeThis,
			const std::string& strFilename, stream::pos storedSize, std::string type,
			int attr
		);

		/**
		 * @note Will always throw an exception as the files are fixed and
		 *       thus can't be removed.
		 */
		virtual void remove(EntryPtr id);

		/**
		 * @note Will always throw an exception as it makes no sense to rename
		 *       the made up filenames in this archive format.
		 */
		virtual void rename(EntryPtr id, const std::string& strNewName);

		/**
		 * @note Will always throw an exception as fixed files can't be moved.
		 */
		virtual void move(const EntryPtr idBeforeThis, EntryPtr id);

		/**
		 * @note Will always throw an exception as fixed files can't be resized.
		 */
		virtual void resize(EntryPtr id, stream::pos newStoredSize,
			stream::pos newRealSize);

		virtual void flush();
		virtual int getSupportedAttributes() const;
};

} // namespace gamearchive
} // namespace camoto

#endif // _CAMOTO_FIXEDARCHIVE_HPP_
