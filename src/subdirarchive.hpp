/*
 * subdirarchive.hpp - Implementation of a generic archive format
 *   supporting subdirectories, where each subdirectory is handled by a
 *   separate class instance.  This class combines them into a flat file
 *   structure.
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

#ifndef _CAMOTO_SUBDIRARCHIVE_HPP_
#define _CAMOTO_SUBDIRARCHIVE_HPP_

#include <iostream>
#include <vector>
#include <boost/iostreams/stream.hpp>

#include <camoto/gamearchive/archive.hpp>

namespace camoto {
namespace gamearchive {

namespace io = boost::iostreams;

// Interface class that archives can inherit if they have subfolders
class ArchiveWithFolders {

	protected:

		struct SubdirEntry: virtual public Archive::FileEntry {
			bool isFolder;
		};
		typedef boost::shared_ptr<const SubdirEntry> SubdirEntryPtr;

	public:

		// Open one of the folders previously returned by getFolderList() or
		// find().  It must be a folder (i.e. instance of SubdirEntry)
		virtual ArchivePtr openFolder(const Archive::EntryPtr& id)
			throw (std::ios::failure) = 0;

	friend class SubdirArchive;

};

class SubdirArchive: virtual public Archive {

	protected:

		ArchivePtr rootFolder;
		VC_ENTRYPTR files;
		VC_ARCHIVE openFolders;

		// This class wraps around another FileEntry, and allows a different
		// name to be specified (i.e. one with a path)
		struct WrapperEntry: virtual public Archive::FileEntry {
			EntryPtr original;
			ArchivePtr containingFolder;
			// We need to store the prefix (even though it has already been prepended
			// onto the filename) so that when inserting files next to this one we
			// know what prefix/folder to use.
			std::string prefix;
		};

	public:

		// We can't throw an 'invalid format' error here, because we wouldn't be
		// in this function if the format's isValid() function returned true!
		SubdirArchive(ArchivePtr& rootFolder)
			throw ();

		virtual ~SubdirArchive()
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
			const std::string& strFilename, offset_t iSize, std::string type,
			int attr
		)
			throw (std::ios::failure);

		virtual void remove(EntryPtr& id)
			throw (std::ios::failure);

		virtual void rename(EntryPtr& id, const std::string& strNewName)
			throw (std::ios::failure);

		virtual void move(const EntryPtr& idBeforeThis, EntryPtr& id)
			throw (std::ios::failure);

		virtual void resize(EntryPtr& id, size_t iNewSize)
			throw (std::ios::failure);

		virtual void flush()
			throw (std::ios::failure);

		virtual EntryPtr entryPtrFromStream(const iostream_sptr openFile)
			throw ();

	protected:

		SubdirArchive::VC_ENTRYPTR listFiles(ArchivePtr& parent,
			const std::string& prefix
		)
			throw (std::ios::failure);

		void loadFiles()
			throw (std::ios::failure);

		WrapperEntry *wrapFileEntry(Archive::EntryPtr& ep, ArchivePtr& parent,
			const std::string& prefix
		)
			throw ();

};

} // namespace gamearchive
} // namespace camoto

#endif // _CAMOTO_SUBDIRARCHIVE_HPP_
