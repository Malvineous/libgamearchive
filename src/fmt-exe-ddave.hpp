/**
 * @file   fmt-exe-ddave.hpp
 * @brief  FixedArchive implementation for Dangerous Dave .exe file.
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

#ifndef _CAMOTO_FMT_EXE_DDAVE_HPP_
#define _CAMOTO_FMT_EXE_DDAVE_HPP_

#include <camoto/gamearchive/archivetype.hpp>
#include "fixedarchive.hpp"

namespace camoto {
namespace gamearchive {

class EXE_DDaveType: virtual public ArchiveType {

	public:

		EXE_DDaveType()
			throw ();

		virtual ~EXE_DDaveType()
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

		virtual SuppFilenames getRequiredSupps(stream::input_sptr data,
			const std::string& filenameArchive) const
			throw ();

};

class EXE_DDaveArchive: virtual public FixedArchive {

	public:

		EXE_DDaveArchive(stream::inout_sptr psArchive)
			throw (stream::error);

		virtual ~EXE_DDaveArchive()
			throw ();

};

} // namespace gamearchive
} // namespace camoto

#endif // _CAMOTO_FMT_EXE_DDAVE_HPP_
