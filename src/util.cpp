/**
 * @file  util.cpp
 * @brief Utility functions.
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

#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>

#include <camoto/gamearchive/util.hpp>

namespace fs = boost::filesystem;

namespace camoto {
namespace gamearchive {

Archive::EntryPtr findFile(ArchivePtr& archive,
	const std::string& filename)
{
	// Save the original archive pointer in case we get half way through a
	// subfolder tree and get a file-not-found.
	ArchivePtr orig = archive;

	// See if it's an index.  This check is performed first so that regardless
	// of what the filename is, it will always be possible to extract files
	// by index number.
	if ((filename[0] == '@') && (filename.length() > 1)) {
// TODO: split at dots into subfolders
		char *endptr;
		// strtoul allows arbitrary whitespace at the start, so if ever there is
		// a file called "@5" which gets extracted instead of the fifth file,
		// giving -x "@ 5" should do the trick.
		unsigned long index = strtoul(&(filename.c_str()[1]), &endptr, 10);
		if (*endptr == '\0') {
			// The number was entirely valid (no junk at end)
			Archive::VC_ENTRYPTR files = archive->getFileList();
			if (index < files.size()) return files[index];
			throw stream::error("index too large");
		}
	}

	// Filename isn't an index, see if it matches a name
	Archive::EntryPtr id = archive->find(filename);
	if (archive->isValid(id)) return id;

	// The file doesn't exist, it's not an index, see if it can be split up
	// into subfolders.
	fs::path p(filename);
	for (fs::path::iterator i = p.begin(); i != p.end(); i++) {

		if (archive->isValid(id)) {
			// The ID is valid, which means it was set to a file in the
			// previous loop iteration, but if we're here then it means
			// there is another element in the path.  Since this means
			// a file has been specified like a folder, we have to abort.
			id = Archive::EntryPtr();
			break;
		}

		Archive::EntryPtr j = archive->find(i->string());
		if (!archive->isValid(j)) break;

		if (j->fAttr & EA_FOLDER) {
			// Open the folder and continue with the next path element
			archive = archive->openFolder(j);
			if (!archive) {
				archive = orig; // make sure archive is valid for below
				break;
			}
		} else {
			// This is a file, it had better be the last one!
			id = j;
		}

	}
	if (archive->isValid(id)) return id;

	// Restore the archive pointer to what it was originally
	archive = orig;

	return Archive::EntryPtr(); // file not found
}

} // namespace gamearchive
} // namespace camoto
