/**
 * @file   archive.cpp
 * @brief  Generic functions common to all Archive types.
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

ArchivePtr ArchiveType::newArchive(stream::inout_sptr psArchive, SuppData& suppData) const
{
	return this->open(psArchive, suppData);
}

Archive::Archive()
{
}

Archive::~Archive()
{
}

ArchivePtr Archive::openFolder(const Archive::EntryPtr id)
{
	// This function should only be called for folders (not files)
	assert(id->fAttr & EA_FOLDER);

	// But if we got this far, the archive format has folders but didn't
	// override this function so they could be opened!
	assert(false);

	// Throw an exception if assertions have been disabled.
	throw stream::error("BUG: Archive format doesn't implement openFolder()");
}

void Archive::move(const EntryPtr idBeforeThis, EntryPtr id)
{
	// Open the file we want to move
	stream::inout_sptr src(this->open(id));
	assert(src);

	// Insert a new file at the destination index
	EntryPtr n = this->insert(idBeforeThis, id->strName, id->storedSize,
		id->type, id->fAttr);
	assert(n->bValid);

	if (n->filter.compare(id->filter) != 0) {
		this->remove(n);
		throw stream::error("Cannot move file to this position (filter change)"
			" - try removing and then adding it instead");
	}

	stream::inout_sptr dst(this->open(n));
	assert(dst);

	// Copy the data into the new file's position
	stream::copy(dst, src);
	dst->flush();

	// If there's a filter set then bring the unfiltered size across too.
	if (!n->filter.empty()) {
		this->resize(n, n->storedSize, id->realSize);
	}

	this->remove(id);

	return;
}

int Archive::getSupportedAttributes() const
{
	return 0;
}

void preventResize(stream::len len)
{
	throw stream::write_error("This file is a fixed size, it cannot be made smaller or larger.");
}

} // namespace gamearchive
} // namespace camoto
