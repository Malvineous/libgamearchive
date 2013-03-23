/**
 * @file   fixedarchive.hpp
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

/// Create an archive by splitting up the given stream into files.
ArchivePtr DLL_EXPORT createFixedArchive(stream::inout_sptr psArchive,
	std::vector<FixedArchiveFile> files);

} // namespace gamearchive
} // namespace camoto

#endif // _CAMOTO_FIXEDARCHIVE_HPP_
