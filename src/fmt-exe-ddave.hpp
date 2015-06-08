/**
 * @file  fmt-exe-ddave.hpp
 * @brief Dangerous Dave .EXE file.
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

#ifndef _CAMOTO_FMT_EXE_DDAVE_HPP_
#define _CAMOTO_FMT_EXE_DDAVE_HPP_

#include <camoto/gamearchive/archivetype.hpp>

namespace camoto {
namespace gamearchive {

/// Dangerous Dave .EXE file handler.
class ArchiveType_EXE_DDave: virtual public ArchiveType
{
	public:
		ArchiveType_EXE_DDave();
		virtual ~ArchiveType_EXE_DDave();

		virtual std::string code() const;
		virtual std::string friendlyName() const;
		virtual std::vector<std::string> fileExtensions() const;
		virtual std::vector<std::string> games() const;
		virtual ArchiveType::Certainty isInstance(stream::input& content) const;
		virtual std::shared_ptr<Archive> create(
			std::unique_ptr<stream::inout> content, SuppData& suppData) const;
		virtual std::shared_ptr<Archive> open(
			std::unique_ptr<stream::inout> content, SuppData& suppData) const;
		virtual SuppFilenames getRequiredSupps(stream::input& content,
			const std::string& filename) const;
};

} // namespace gamearchive
} // namespace camoto

#endif // _CAMOTO_FMT_EXE_DDAVE_HPP_
