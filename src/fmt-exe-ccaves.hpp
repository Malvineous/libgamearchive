/**
 * @file   fmt-exe-ccaves.hpp
 * @brief  FixedArchive implementation for Crystal Caves .exe file.
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

#ifndef _CAMOTO_FMT_EXE_CCAVES_HPP_
#define _CAMOTO_FMT_EXE_CCAVES_HPP_

#include <camoto/gamearchive/archivetype.hpp>
#include "fixedarchive.hpp"

namespace camoto {
namespace gamearchive {

class EXE_CCavesType: virtual public ArchiveType {

	public:

		EXE_CCavesType()
			throw ();

		virtual ~EXE_CCavesType()
			throw ();

		virtual std::string getArchiveCode() const
			throw ();

		virtual std::string getFriendlyName() const
			throw ();

		virtual std::vector<std::string> getFileExtensions() const
			throw ();

		virtual std::vector<std::string> getGameList() const
			throw ();

		virtual ArchiveType::Certainty isInstance(stream::inout_sptr fsArchive) const
			throw (stream::error);

		virtual ArchivePtr newArchive(stream::inout_sptr psArchive, SuppData& suppData) const
			throw (stream::error);

		virtual ArchivePtr open(stream::inout_sptr fsArchive, SuppData& suppData) const
			throw (stream::error);

		virtual SuppFilenames getRequiredSupps(const std::string& filenameArchive) const
			throw ();

};

class EXE_CCavesArchive: virtual public FixedArchive {

	public:

		EXE_CCavesArchive(stream::inout_sptr psArchive)
			throw (stream::error);

		virtual ~EXE_CCavesArchive()
			throw ();

};

} // namespace gamearchive
} // namespace camoto

#endif // _CAMOTO_FMT_EXE_CCAVES_HPP_
