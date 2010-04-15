/*
 * archive.cpp - Generic functions common to all Archive types.
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

#include <boost/iostreams/copy.hpp>
#include <camoto/gamearchive/archive.hpp>

namespace camoto {
namespace gamearchive {

Archive::~Archive()
	throw ()
{
}

void Archive::move(const EntryPtr& idBeforeThis, EntryPtr& id)
	throw (std::ios::failure)
{
	// Open the file we want to move
	iostream_sptr src(this->open(id));
	assert(src);

	// Insert a new file at the destination index
	EntryPtr n = this->insert(idBeforeThis, id->strName, id->iSize);
	assert(n->bValid);

	iostream_sptr dst(this->open(n));
	assert(dst);

	// Copy the data into the new file's position
	boost::iostreams::copy(*src, *dst);
	dst->flush();

	// Remove the original file
	EntryPtr orig = this->entryPtrFromStream(src);
	assert(orig);
	assert(orig->bValid);
	this->remove(orig);

	return;
}

} // namespace gamearchive
} // namespace camoto
