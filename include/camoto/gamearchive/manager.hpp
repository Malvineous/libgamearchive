/**
 * @file  camoto/gamearchive/manager.hpp
 * @brief Manager class, used for accessing the various archive format readers.
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

#ifndef _CAMOTO_GAMEARCHIVE_MANAGER_HPP_
#define _CAMOTO_GAMEARCHIVE_MANAGER_HPP_

#include <camoto/formatenum.hpp>
#include <camoto/gamearchive/archivetype.hpp>
#include <camoto/gamearchive/filtertype.hpp>

#ifndef DLL_EXPORT
#define DLL_EXPORT
#endif

namespace camoto {

template class FormatEnumerator<gamearchive::ArchiveType>;
template class FormatEnumerator<gamearchive::FilterType>;

namespace gamearchive {

typedef FormatEnumerator<ArchiveType> ArchiveManager;
typedef FormatEnumerator<FilterType> FilterManager;

} // namespace gamearchive
} // namespace camoto

#endif // _CAMOTO_GAMEARCHIVE_MANAGER_HPP_
