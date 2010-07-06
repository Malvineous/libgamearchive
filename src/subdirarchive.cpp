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

#include <boost/algorithm/string.hpp>
#include <boost/bind.hpp>

#include "subdirarchive.hpp"
#include <camoto/debug.hpp>

namespace camoto {
namespace gamearchive {

refcount_declclass(SubdirArchive);

SubdirArchive::SubdirArchive(ArchivePtr& rootFolder)
	throw () :
		rootFolder(rootFolder)
{
	refcount_qenterclass(SubdirArchive);
}

SubdirArchive::~SubdirArchive()
	throw ()
{
	refcount_qexitclass(SubdirArchive);
}

const SubdirArchive::VC_ENTRYPTR& SubdirArchive::getFileList()
	throw ()
{
	// Get files from root folder
	if (this->files.empty()) this->loadFiles();
	return this->files;
}

SubdirArchive::EntryPtr SubdirArchive::find(const std::string& strFilename)
	throw ()
{
	// TESTED BY: fmt_res_stellar7_*

	if (this->files.empty()) this->loadFiles();

	for (VC_ENTRYPTR::iterator i = this->files.begin(); i != this->files.end(); i++) {
		if (boost::iequals((*i)->strName, strFilename)) {
			return *i;
		}
	}

	return EntryPtr();
}

bool SubdirArchive::isValid(const EntryPtr& id)
	throw ()
{
	WrapperEntry *wrapper = dynamic_cast<WrapperEntry *>(id.get());
	return wrapper && wrapper->containingFolder->isValid(wrapper->original);
}

boost::shared_ptr<std::iostream> SubdirArchive::open(const EntryPtr& id)
	throw ()
{
	// TESTED BY: fmt_res_stellar7_open
	assert(this->isValid(id));

	WrapperEntry *wrapper = dynamic_cast<WrapperEntry *>(id.get());
	assert(wrapper); // if false, caller passed us someone else's EntryPtr

	// TODO: Figure out how to refcount these or otherwise avoid flushing
	// the same archive multiple times.
	this->openFolders.push_back(wrapper->containingFolder);

	return wrapper->containingFolder->open(wrapper->original);
}

SubdirArchive::EntryPtr SubdirArchive::insert(const EntryPtr& idBeforeThis,
	const std::string& strFilename, offset_t iSize)
	throw (std::ios::failure)
{
	// TESTED BY: fmt_res_stellar7_insert2
	// TESTED BY: fmt_res_stellar7_remove_insert
	// TESTED BY: fmt_res_stellar7_insert_remove

	// Since the insert invalidates all the EntryPtrs we'll clear our cache
	// so they'll be reopened next time.
	this->files.clear();

	// TODO: Figure out how to handle the path that's been specified in the filename
	if (idBeforeThis) {
		WrapperEntry *wrapper = dynamic_cast<WrapperEntry *>(idBeforeThis.get());
		assert(wrapper); // if false, caller passed us someone else's EntryPtr
		EntryPtr newFile = wrapper->containingFolder->insert(
			wrapper->original, strFilename, iSize
		);
		WrapperEntry *newWrapper = this->wrapFileEntry(
			newFile, wrapper->containingFolder, wrapper->prefix
		);
		return EntryPtr(newWrapper);
	} else {
		// Append, so use root folder
		EntryPtr newFile = this->rootFolder->insert(EntryPtr(), strFilename, iSize);
		WrapperEntry *newWrapper = this->wrapFileEntry(newFile, this->rootFolder, "");
		return EntryPtr(newWrapper);
	}
}

void SubdirArchive::remove(EntryPtr& id)
	throw (std::ios::failure)
{
	// TESTED BY: fmt_res_stellar7_remove
	// TESTED BY: fmt_res_stellar7_remove2
	// TESTED BY: fmt_res_stellar7_remove_insert
	// TESTED BY: fmt_res_stellar7_insert_remove

	// Make sure the caller doesn't try to remove something that doesn't exist!
	assert(this->isValid(id));

	// Since the insert invalidates all the EntryPtrs we'll clear our cache
	// so they'll be reopened next time.
	this->files.clear();

	WrapperEntry *wrapper = dynamic_cast<WrapperEntry *>(id.get());
	assert(wrapper); // if false, caller passed us someone else's EntryPtr

	wrapper->containingFolder->remove(wrapper->original);

	return;
}

void SubdirArchive::rename(EntryPtr& id, const std::string& strNewName)
	throw (std::ios_base::failure)
{
	// TESTED BY: fmt_res_stellar7_rename
	assert(this->isValid(id));

	WrapperEntry *wrapper = dynamic_cast<WrapperEntry *>(id.get());
	assert(wrapper); // if false, caller passed us someone else's EntryPtr

	// TODO: Handle path in filename
	// TODO: Handle different path in filename (move from one folder to another)
	//  - Perhaps remove and reinsert file?
	wrapper->containingFolder->rename(wrapper->original, strNewName);

	return;
}

void SubdirArchive::move(const EntryPtr& idBeforeThis, EntryPtr& id)
	throw (std::ios::failure)
{
	assert(this->isValid(idBeforeThis));
	assert(this->isValid(id));

	// Since the insert invalidates all the EntryPtrs we'll clear our cache
	// so they'll be reopened next time.
	this->files.clear();

	WrapperEntry *wrapperBeforeThis = dynamic_cast<WrapperEntry *>(idBeforeThis.get());
	assert(wrapperBeforeThis); // if false, caller passed us someone else's EntryPtr

	WrapperEntry *wrapper = dynamic_cast<WrapperEntry *>(id.get());
	assert(wrapper); // if false, caller passed us someone else's EntryPtr

	if (wrapperBeforeThis->containingFolder != wrapper->containingFolder) {
		// TODO: Eventually we should perform an insert/delete operation to do this
		throw std::ios::failure("cannot move files into different subdirectories (yet)");
	}

	wrapper->containingFolder->move(wrapperBeforeThis->original, wrapper->original);
	return;
}

// Enlarge or shrink an existing file entry.
// Postconditions: Existing EntryPtrs and open files remain valid.
void SubdirArchive::resize(EntryPtr& id, size_t iNewSize)
	throw (std::ios::failure)
{
	assert(this->isValid(id));

	WrapperEntry *wrapper = dynamic_cast<WrapperEntry *>(id.get());
	assert(wrapper); // if false, caller passed us someone else's EntryPtr

	wrapper->containingFolder->resize(wrapper->original, iNewSize);

	return;
}

void SubdirArchive::flush()
	throw (std::ios::failure)
{
	// The truncate function was given to us, but because we're only posing as
	// the archive we need to pass this function along too.  We couldn't do it
	// earlier (e.g. in the constructor) because the caller/user doesn't set
	// the truncate function until the constructor returns.
	this->rootFolder->fnTruncate = this->fnTruncate;

	// Write out to the underlying stream
	for (VC_ARCHIVE::iterator i = this->openFolders.begin(); i != this->openFolders.end(); i++) {
		assert((*i)->fnTruncate); // checked anyway, but just to be sure at this point
		(*i)->flush();
	}

	this->rootFolder->flush();
	// TODO: Figure out if these are no longer being used (access count == 1)
	// and if so free them
	return;
}

Archive::EntryPtr SubdirArchive::entryPtrFromStream(const iostream_sptr openFile)
	throw ()
{
	for (VC_ARCHIVE::iterator i = this->openFolders.begin(); i != this->openFolders.end(); i++) {
		EntryPtr p = (*i)->entryPtrFromStream(openFile);
		if (p) return p;
	}
	return EntryPtr();
}

void SubdirArchive::loadFiles()
	throw (std::ios::failure)
{
	this->files = this->listFiles(this->rootFolder, std::string());
	return;
}

SubdirArchive::VC_ENTRYPTR SubdirArchive::listFiles(ArchivePtr& parent, const std::string& prefix)
	throw (std::ios::failure)
{
	// Get the list of files from the folder/root
	VC_ENTRYPTR files = parent->getFileList();

	// Check to see if any of these files are subfolders
	VC_ENTRYPTR newfiles;
	for (VC_ENTRYPTR::iterator i = files.begin(); i != files.end(); i++) {

		// Create a wrapper for the parent folder (in case it's actually just a
		// normal file.)  If it's a folder then doing it this way allows a folder's
		// files to be listed under each folder in the main file list.
		WrapperEntry *wrapper = this->wrapFileEntry(*i, parent, prefix);
		newfiles.push_back(EntryPtr(wrapper));

		// Now see if this entry is actually a folder
		ArchiveWithFolders::SubdirEntry *s = dynamic_cast<ArchiveWithFolders::SubdirEntry *>(i->get());
		if ((s) && (s->isFolder)) {
			wrapper->strName += "/"; // Add trailing slash to parent folder

			// look up child folders
			ArchiveWithFolders *parentFolder = dynamic_cast<ArchiveWithFolders *>(parent.get());
			if (parentFolder) {
				ArchivePtr folder = parentFolder->openFolder(*i);
				assert(folder);

				// Set the truncate function to call Archive::resize() on the parent
				// folder.
				folder->fnTruncate = boost::bind<void>(&Archive::resize, parent.get(), *i, _1);
				assert(folder->fnTruncate);

				std::string thisPrefix = prefix + (*i)->strName + "/";
				std::cerr << "Look up folder " << (*i)->strName << std::endl;
				VC_ENTRYPTR sub = this->listFiles(folder, thisPrefix);
				// For each of the returned files, wrap them up so we can alter the
				// name (by prepending the path.)
				for (VC_ENTRYPTR::iterator j = sub.begin(); j != sub.end(); j++) {
					WrapperEntry *wrapper = this->wrapFileEntry(*j, folder, thisPrefix);
					newfiles.push_back(EntryPtr(wrapper));
				}
			} // else not an archive with subfolders
		} // else EntryPtr doesn't inherit from SubdirEntry
	}
	return newfiles;
}

SubdirArchive::WrapperEntry *SubdirArchive::wrapFileEntry(Archive::EntryPtr& ep, ArchivePtr& parent, const std::string& prefix)
	throw ()
{
	WrapperEntry *wrapper = new WrapperEntry();
	*static_cast<FileEntry *>(wrapper) = *(ep.get());
	wrapper->strName = prefix + wrapper->strName;
	wrapper->original = ep;
	wrapper->containingFolder = parent;
	wrapper->prefix = prefix;
	return wrapper;
}

} // namespace gamearchive
} // namespace camoto
