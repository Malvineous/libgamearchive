/*
 * manager.hpp - Manager class, used for accessing the various archive format
 *               readers.
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

#ifndef _CAMOTO_GAMEARCHIVE_MANAGER_HPP_
#define _CAMOTO_GAMEARCHIVE_MANAGER_HPP_

#include <boost/shared_ptr.hpp>
#include <vector>

#include <camoto/types.hpp>
#include <camoto/gamearchive/archivetype.hpp>

namespace camoto {
namespace gamearchive {

class Manager {
	public:
		typedef boost::shared_ptr<const ArchiveType> arch_sptr;
		typedef std::vector<arch_sptr> VC_TYPES;

	private:
		VC_TYPES vcTypes;

	public:
		Manager()
			throw ();

		~Manager()
			throw ();

		// Get an ArchiveType instance for a supported file format.  Returns NULL
		// once iIndex goes out of range.  First format is at iIndex == 0.
		arch_sptr getArchiveType(int iIndex)
			throw ();

		// Save as above but uses codes like "grp-duke3d" for the lookup (useful
		// for passing in from the command line.)  Returns NULL on an invalid code.
		arch_sptr getArchiveTypeByCode(const std::string& strCode)
			throw ();
};

// Library entry point.  All further functionality is provided by calling
// functions in the Manager class.  Remember to delete the returned pointer
// when you have finished using it.  Stick it in a boost::shared_ptr if you
// don't want to worry about that.
Manager *getManager(void)
	throw ();

} // namespace gamearchive
} // namespace camoto

#endif // _CAMOTO_GAMEARCHIVE_MANAGER_HPP_
