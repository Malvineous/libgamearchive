/*
 * archivetype.hpp - ArchiveType class, used to identify and open an instance
 *                   of a particular archive format.
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

#ifndef _CAMOTO_GAMEARCHIVE_ARCHIVETYPE_HPP_
#define _CAMOTO_GAMEARCHIVE_ARCHIVETYPE_HPP_

#include <vector>

#include <camoto/types.hpp>
#include <camoto/gamearchive/archive.hpp>

namespace camoto {
namespace gamearchive {

enum E_CERTAINTY {
	EC_DEFINITELY_NO,
	EC_UNSURE,
	EC_POSSIBLY_YES,
	EC_DEFINITELY_YES
};

class ArchiveType {

	public:

		// Get a short code to identify this file format (e.g. "grp-duke3d", useful
		// for command-line arguments.
		virtual std::string getArchiveCode() const
			throw () = 0;

		// Get the archive name, e.g. "Duke Nukem 3D GRP file"
		virtual std::string getFriendlyName() const
			throw () = 0;

		// Get a list of the known file extensions for this format.
		// Example: "vol", "stn", "cmp", "ms1", "ms2"
		virtual std::vector<std::string> getFileExtensions() const
			throw () = 0;

		// List of games using this format
		// Example: "Major Stryker", "Cosmo's Cosmic Adventures", "Duke Nukem II"
		virtual std::vector<std::string> getGameList() const
			throw () = 0;

		virtual E_CERTAINTY isInstance(iostream_sptr psArchive) const
			throw (std::ios::failure) = 0;

		// Preconditions: isInstance() has returned > EC_DEFINITELY_NO
		virtual Archive *open(iostream_sptr psArchive) const
			throw (std::ios::failure) = 0;

};

} // namespace gamearchive
} // namespace camoto

#endif // _CAMOTO_GAMEARCHIVE_ARCHIVETYPE_HPP_
