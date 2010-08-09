/**
 * @file   archivetype.hpp
 * @brief  ArchiveType class, used to identify and open an instance of a
 *         particular archive format.
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

/// Main namespace
namespace camoto {
/// Namespace for this library
namespace gamearchive {

/// Confidence level when guessing an archive format.
enum E_CERTAINTY {
	/// Certain this file is not of the archive format.
	EC_DEFINITELY_NO,
	/// The checks were inconclusive, it could go either way.
	EC_UNSURE,
	/// Everything checked out OK, but this format has no signature or other
	/// test to be completely certain.
	EC_POSSIBLY_YES,
	/// This format has a signature and it matched.
	EC_DEFINITELY_YES,
};

/// Type of supplemental file.
enum E_SUPPTYPE {
	/// FAT is stored externally
	EST_FAT,
	/// Compression dictionary is external
	EST_DICT,
};

/// Supplementary item for an archive.
/**
 * This class contains data about a supplementary item required to open an
 * archive file.
 *
 * @see ArchiveType::getRequiredSupps()
 */
struct SuppItem {
	/// The stream containing the supplemental data.
	iostream_sptr stream;
	/// The truncate callback (required)
	FN_TRUNCATE fnTruncate;
};

/// A list of required supplemental files and their filenames.  The filenames
/// may contain a path (especially if the main archive file also contains a
/// path.)
typedef std::map<E_SUPPTYPE, std::string> MP_SUPPLIST;

/// A list of the supplemental file types mapped to open file streams.
typedef std::map<E_SUPPTYPE, SuppItem> MP_SUPPDATA;

/// Interface to a particular archive format.
class ArchiveType {

	public:

		/// Get a short code to identify this file format, e.g. "grp-duke3d"
		/**
		 * This can be useful for command-line arguments.
		 *
		 * @return The archive short name/ID.
		 */
		virtual std::string getArchiveCode() const
			throw () = 0;

		/// Get the archive name, e.g. "Duke Nukem 3D GRP file"
		/**
		 * @return The archive name.
		 */
		virtual std::string getFriendlyName() const
			throw () = 0;

		/// Get a list of the known file extensions for this format.
		/**
		 * @return A vector of file extensions, e.g. "vol", "stn", "cmp"
		 */
		virtual std::vector<std::string> getFileExtensions() const
			throw () = 0;

		/// Get a list of games using this format.
		/**
		 * @return A vector of game names, such as "Major Stryker", "Cosmo's Cosmic
		 *         Adventures", "Duke Nukem II"
		 */
		virtual std::vector<std::string> getGameList() const
			throw () = 0;

		/// Check a stream to see if it's in this archive format.
		/**
		 * @param  psArchive A C++ iostream of the file to test.
		 * @return A single confidence value from \ref E_CERTAINTY.
		 */
		virtual E_CERTAINTY isInstance(iostream_sptr psArchive) const
			throw (std::ios::failure) = 0;

		/// Create a blank archive in this format.
		/**
		 * This function writes out the necessary signatures and headers to create
		 * a valid blank archive in this format.
		 *
		 * Note to format implementors: This function only needs to be overridden
		 * if there are headers to write, otherwise an empty stream is passed to
		 * open() which is expected to succeed.
		 *
		 * @param  psArchive A blank stream to store the new archive in.
		 * @param  suppData Any supplemental data required by this format (see
		 *         getRequiredSupps()).
		 * @return A pointer to an instance of the Archive class, just as if a
		 *         valid empty file had been opened by open().
		 */
		virtual ArchivePtr newArchive(iostream_sptr psArchive, MP_SUPPDATA& suppData) const
			throw (std::ios::failure);

		/// Open an archive file.
		/**
		 * @pre    Recommended that isInstance() has returned > EC_DEFINITELY_NO.
		 * @param  psArchive The archive file.
		 * @param  suppData Any supplemental data required by this format (see
		 *         getRequiredSupps()).
		 * @return A pointer to an instance of the Archive class.  Will throw an
		 *         exception if the data is invalid (i.e. if isInstance() returned
		 *         EC_DEFINITELY_NO) however it will try its best to read the data
		 *         anyway, to make it possible to "force" a file to be opened by a
		 *         particular format handler.
		 */
		virtual ArchivePtr open(iostream_sptr psArchive, MP_SUPPDATA& suppData) const
			throw (std::ios::failure) = 0;

		/// Get a list of any required supplemental files.
		/**
		 * For some archive formats, data is stored externally to the archive file
		 * itself (for example the filenames may be stored in a different file than
		 * the actual file data.)  This function obtains a list of these
		 * supplementary files, so the caller can open them and pass them along
		 * to the archive manipulation classes.
		 *
		 * @param  filenameArchive The filename of the archive (no path.)  This is
		 *         for supplemental files which share the same base name as the
		 *         archive, but a different filename extension.
		 * @return A (possibly empty) map associating required supplemental file
		 *         types with their filenames.  For each returned value the file
		 *         should be opened and placed in a SuppItem instance.  The
		 *         SuppItem is then added to an \ref MP_SUPPDATA map where it can
		 *         be passed to newArchive() or open().  Note that the filenames
		 *         returned can have relative paths, and may even have an absolute
		 *         path, if one was passed in with filenameArchive.
		 */
		virtual MP_SUPPLIST getRequiredSupps(const std::string& filenameArchive) const
			throw () = 0;

};

/// Shared pointer to an ArchiveType.
typedef boost::shared_ptr<ArchiveType> ArchiveTypePtr;

/// Vector of ArchiveType shared pointers.
typedef std::vector<ArchiveTypePtr> VC_ARCHIVETYPE;

} // namespace gamearchive
} // namespace camoto

#endif // _CAMOTO_GAMEARCHIVE_ARCHIVETYPE_HPP_
