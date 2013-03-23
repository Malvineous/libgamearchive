/**
 * @file   fixedarchive.cpp
 * @brief  Generic archive providing access to "files" at specific offsets and
 *         lengths in a host file (e.g. game levels stored in an .exe file.)
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
#include <camoto/util.hpp>
#include <camoto/gamearchive/fixedarchive.hpp>

namespace camoto {
namespace gamearchive {

void preventResize(stream::len len)
{
	throw stream::write_error("This file is a fixed size, it cannot be made smaller or larger.");
}


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

ArchivePtr createFixedArchive(stream::inout_sptr psArchive,
	std::vector<FixedArchiveFile> files)
{
	return ArchivePtr(new FixedArchive(psArchive, files));
}


FixedArchive::FixedArchive(stream::inout_sptr psArchive, std::vector<FixedArchiveFile> files)
	:	psArchive(psArchive),
		files(files)
{
	int j = 0;
	for (std::vector<FixedArchiveFile>::iterator i = files.begin(); i != files.end(); i++) {
		FixedEntry *fe = new FixedEntry();
		EntryPtr ep(fe);
		fe->bValid = true;
		fe->storedSize = i->size;
		fe->realSize = i->size;
		fe->strName = i->name;
		fe->type = FILETYPE_GENERIC;
		fe->filter = i->filter;
		fe->fAttr = 0;

		fe->index = j++;

		this->vcFixedEntries.push_back(ep);
	}
}

FixedArchive::~FixedArchive()
{
}

const FixedArchive::VC_ENTRYPTR& FixedArchive::getFileList() const
{
	return this->vcFixedEntries;
}

FixedArchive::EntryPtr FixedArchive::find(const std::string& strFilename) const
{
	// TESTED BY: TODO
	for (VC_ENTRYPTR::const_iterator i = this->vcFixedEntries.begin();
		i != this->vcFixedEntries.end();
		i++
	) {
		const FixedEntry *entry = dynamic_cast<const FixedEntry *>(i->get());
		const FixedArchiveFile *file = &this->files[entry->index];
		if (boost::iequals(file->name, strFilename)) {
			return *i;  // *i is the original shared_ptr
		}
	}
	return EntryPtr();
}

bool FixedArchive::isValid(const EntryPtr id) const
{
	const FixedEntry *id2 = dynamic_cast<const FixedEntry *>(id.get());
	return ((id2) && (id2->index < this->files.size()));
}

stream::inout_sptr FixedArchive::open(const EntryPtr id)
{
	// TESTED BY: TODO
	const FixedEntry *entry = dynamic_cast<const FixedEntry *>(id.get());
	const FixedArchiveFile *file = &this->files[entry->index];
	stream::fn_truncate fnTrunc = preventResize;
	stream::sub_sptr psSub(new stream::sub());
	psSub->open(
		this->psArchive,
		file->offset,
		file->size,
		fnTrunc
	);
	this->vcSubStream.push_back(psSub);
	return psSub;
}

ArchivePtr FixedArchive::openFolder(const Archive::EntryPtr id)
{
	// This function should only be called for folders (not files)
	assert(id->fAttr & EA_FOLDER);

	// Throw an exception if assertions have been disabled.
	throw stream::error("BUG: openFolder() called for archive format that "
		"doesn't have any folders.");
}

FixedArchive::EntryPtr FixedArchive::insert(const EntryPtr idBeforeThis,
	const std::string& strFilename, stream::pos storedSize, std::string type,
	int attr
)
{
	throw stream::error("This is a fixed archive, files cannot be inserted.");
}

void FixedArchive::remove(EntryPtr id)
{
	throw stream::error("This is a fixed archive, files cannot be removed.");
}

void FixedArchive::rename(EntryPtr id, const std::string& strNewName)
{
	throw stream::error("This is a fixed archive, files cannot be renamed.");
}

void FixedArchive::move(const EntryPtr idBeforeThis, EntryPtr id)
{
	throw stream::error("This is a fixed archive, files cannot be moved.");
}

void FixedArchive::resize(EntryPtr id, stream::pos newStoredSize,
	stream::pos newRealSize)
{
	if (id->storedSize != newStoredSize) {
		throw stream::error(createString("This is a fixed archive, files "
			"cannot be resized (tried to resize to " << newStoredSize <<
			", must remain as " << id->storedSize << ")"));
	}
	// else no change, so do nothing
	return;
}

void FixedArchive::flush()
{
	// no-op (nothing to flush)
	return;
}

int FixedArchive::getSupportedAttributes() const
{
	return 0;
}

} // namespace gamearchive
} // namespace camoto
