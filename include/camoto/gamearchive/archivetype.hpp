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
#include <map>

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

enum E_SUPPTYPE {
	EST_FAT,  // FAT is stored externally
	EST_DICT  // Compression dictionary is external
};

typedef std::map<E_SUPPTYPE, std::string> MP_SUPPLIST;
typedef std::map<E_SUPPTYPE, iostream_sptr> MP_SUPPDATA;

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

		// Write out the necessary headers to create a blank archive in this format
		virtual ArchivePtr newArchive(iostream_sptr psArchive, MP_SUPPDATA& suppData) const
			throw (std::ios::failure) = 0;

		// Preconditions: isInstance() has returned > EC_DEFINITELY_NO, any
		// supplemental files returned by getRequiredSupps() have been set by
		// setSupplementalFile().
		virtual ArchivePtr open(iostream_sptr psArchive, MP_SUPPDATA& suppData) const
			throw (std::ios::failure) = 0;

		// Get a list of the required supplemental files.  The archive filename is
		// given (for the case where a supplemental file has the same name but a
		// different extension) so the return value will be a map between the
		// required types and the filename for those types.  For each returned
		// value the file should be opened and passed to setSupplementalFile()
		virtual MP_SUPPLIST getRequiredSupps(const std::string& filenameArchive) const
			throw () = 0;

};

} // namespace gamearchive
} // namespace camoto

#endif // _CAMOTO_GAMEARCHIVE_ARCHIVETYPE_HPP_
