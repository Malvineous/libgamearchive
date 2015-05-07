/**
 * @file  camoto/gamearchive/util.hpp
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

#ifndef _CAMOTO_GAMEARCHIVE_UTIL_HPP_
#define _CAMOTO_GAMEARCHIVE_UTIL_HPP_

#include <camoto/stream_sub.hpp>
#include <camoto/gamearchive/archive.hpp>

#ifndef DLL_EXPORT
#define DLL_EXPORT
#endif

namespace camoto {
namespace gamearchive {

/// Find the given file within any subfolders, or if it starts with an '@', the
/// file at that index.
/**
 * @param pArchive
 *   On input, the archive file.  On output, the archive holding the file.
 *   Normally this will be the same, unless the archive has subfolders, in
 *   which case the value on output will be the Archive instance of the
 *   subfolder itself.
 *
 * @param pFile
 *   On output, the ID of the file itself, or an empty shared_ptr if the file
 *   could not be found.  Note that this value is only valid for the
 *   output pArchive value, which may be a different archive to what was
 *   passed in.
 *
 * @param filename
 *   Filename to look for.
 */
void DLL_EXPORT findFile(std::shared_ptr<Archive> *pArchive,
	Archive::FileHandle *pFile, const std::string& filename);

/// Truncate callback for substreams that are a fixed size.
void preventResize(stream::output_sub* sub, stream::len len);

} // namespace gamearchive
} // namespace camoto

#endif // _CAMOTO_GAMEARCHIVE_UTIL_HPP_
