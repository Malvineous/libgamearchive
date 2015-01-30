/**
 * @file  archive.cpp
 * @brief Generic functions common to all Archive types.
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

#include <boost/iostreams/copy.hpp>
#include <camoto/gamearchive/archivetype.hpp>
#include <camoto/gamearchive/archive.hpp>

namespace camoto {
namespace gamearchive {

Archive::FileEntry::FileEntry()
{
}

Archive::FileEntry::~FileEntry()
{
}

std::string Archive::FileEntry::getContent() const
{
	std::ostringstream ss;
	ss << "name=" << this->strName
		<< ";size=" << this->storedSize
		<< ";realSize=" << this->realSize
		<< ";type=" << this->type
		<< ";filter=" << this->filter
		<< ";attr=" << this->fAttr;
	return ss.str();
}

} // namespace gamearchive
} // namespace camoto
