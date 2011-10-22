/**
 * @file   util.hpp
 * @brief  Utility functions.
 *
 * Copyright (C) 2010-2011 Adam Nielsen <malvineous@shikadi.net>
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

#include <boost/shared_ptr.hpp>
#include <vector>

#include <camoto/stream.hpp>
#include <stdint.h>
#include <camoto/gamearchive/archivetype.hpp>
#include <camoto/gamearchive/filtertype.hpp>

namespace camoto {
namespace gamearchive {

/// Find the given file, or if it starts with an '@', the file at that index.
/**
 * @param archive
 *   On input, the archive file.  On output, the archive holding the file.
 *   Normally this will be the same, unless the archive has subfolders, in
 *   which case the value on output will be the Archive instance of the
 *   subfolder itself.
 *
 * @param filename
 *   Filename to look for.
 *
 * @return EntryPtr to the file (or folder!) specified, or an empty EntryPtr if
 *   the file couldn't be found.  Note that the EntryPtr is only valid for the
 *   output archive parameter, which may be a different archive to what was
 *   passed in.
 */
Archive::EntryPtr findFile(ArchivePtr& archive,
	const std::string& filename)
	throw (stream::error);

} // namespace gamearchive
} // namespace camoto

#endif // _CAMOTO_GAMEARCHIVE_UTIL_HPP_
