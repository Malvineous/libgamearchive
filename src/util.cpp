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

void findFile(std::shared_ptr<Archive> *pArchive,
	Archive::FileHandle *pFile, const std::string& filename)
{
	Archive& archive = *pArchive->get();

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
			const Archive::FileVector& files = archive.files();
			if (index < files.size()) {
				*pFile = files[index];
				return;
			}
			throw stream::error("index too large");
		}
	}

	// Filename isn't an index, see if it matches a name
	auto id = archive.find(filename);
	if (archive.isValid(id)) {
		*pFile = id;
		return;
	}

	// The file doesn't exist, it's not an index, see if it can be split up
	// into subfolders.
	std::shared_ptr<Archive> destArchive = *pArchive;
	for (const auto& i : fs::path(filename)) {

		if (destArchive->isValid(id)) {
			// The ID is valid, which means it was set to a file in the
			// previous loop iteration, but if we're here then it means
			// there is another element in the path.  Since this means
			// a file has been specified like a folder, we have to abort.
			id.reset();
			break;
		}

		auto j = destArchive->find(i.string());
		if ((!j) || (!destArchive->isValid(j))) break;

		if (j->fAttr & Archive::File::Attribute::Folder) {
			// Open the folder and continue with the next path element
			destArchive = destArchive->openFolder(j);
			if (!destArchive) break;
		} else {
			// This is a file, it had better be the last one!
			id = j;
		}

	}

	if (destArchive->isValid(id)) {
		*pArchive = destArchive;
		*pFile = id;
	} else {
		// File not found
		*pFile = nullptr;
	}

	return;
}

void preventResize(stream::output_sub* sub, stream::len len)
{
	throw stream::write_error("This file is a fixed size, it cannot be made "
		"smaller or larger.");
}

} // namespace gamearchive
} // namespace camoto
