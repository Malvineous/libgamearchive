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

#include <vector>
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

/// Top-level class to manage archive types.
/**
 * This class provides access to the different archive file formats supported
 * by the library.
 *
 * In order to open an archive, this class must be used to access an instance
 * of the archive type.  This ArchiveType instance is then used to create an
 * Archive instance around a particular file.  It is this Archive instance that
 * is then used to manipulate the archive file itself.
 *
 * @note Use the free function getManager() to obtain a pointer to an instance
 *   of an object implementing the Manager interface.
 */
/*
class Manager
{
	public:
		/// Get an ArchiveType instance for a supported file format.
		/ **
		 * This can be used to enumerate all available file formats.
		 *
		 * @param iIndex
		 *   Index of the format, starting from 0.
		 *
		 * @return A shared pointer to an ArchiveType for the given index, or
		 *   an empty pointer once iIndex goes out of range.
		 * /
		virtual const ArchiveTypePtr getArchiveType(unsigned int iIndex) const = 0;

		/// Get an ArchiveType instance by its code.
		/ **
		 * @param strCode
		 *   %Archive code (e.g. "grp-duke3d")
		 *
		 * @return A shared pointer to an ArchiveType for the given code, or
		 *         an empty pointer on an invalid code.
		 * /
		virtual const ArchiveTypePtr getArchiveTypeByCode(
			const std::string& strCode) const = 0;

		/// Get a FilterType instance for a supported filtering algorithm.
		/ **
		 * This can be used to enumerate all available filters.
		 *
		 * @param iIndex
		 *   Index of the filter, starting from 0.
		 *
		 * @return A shared pointer to a FilterType for the given index, or
		 *   an empty pointer once iIndex goes out of range.
		 * /
		virtual const FilterTypePtr getFilterType(unsigned int iIndex) const = 0;

		/// Get a FilterType instance by its code.
		/ **
		 * @param strCode
		 *   Filter code (e.g. "lzw-bash")
		 *
		 * @return A shared pointer to a FilterType for the given code, or
		 *   an empty pointer on an invalid code.
		 * /
		virtual const FilterTypePtr getFilterTypeByCode(const std::string& strCode)
			const = 0;
};
*/

/// Library entry point.
/**
 * All further functionality is provided by calling functions in the Manager
 * class.
 *
 * @return A Manager instance.
 */
//const std::unique_ptr<Manager> DLL_EXPORT getManager(void);
/*
template <class T>
std::unique_ptr<FormatEnumerator<T> > DLL_EXPORT createFormatEnumerator()
{
	return std::unique_ptr<FormatEnumerator<T> >();
}

template <>
const std::unique_ptr<FormatEnumerator<ArchiveType> > DLL_EXPORT createFormatEnumerator();

template <>
const std::unique_ptr<FormatEnumerator<FilterType> > DLL_EXPORT createFormatEnumerator();
*/

} // namespace gamearchive
} // namespace camoto

#endif // _CAMOTO_GAMEARCHIVE_MANAGER_HPP_
