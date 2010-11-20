/**
 * @file   archive.cpp
 * @brief  Generic functions common to all Archive types.
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

ArchivePtr ArchiveType::newArchive(iostream_sptr psArchive, MP_SUPPDATA& suppData) const
	throw (std::ios::failure)
{
	return this->open(psArchive, suppData);
}

Archive::Archive()
	throw () :
		fnTruncate(NULL)
{
}

Archive::~Archive()
	throw ()
{
}

ArchivePtr Archive::openFolder(const Archive::EntryPtr& id)
	throw (std::ios::failure)
{
	// This function should only be called for folders (not files)
	assert(id->fAttr & EA_FOLDER);

	// But if we got this far, the archive format has folders but didn't
	// override this function so they could be opened!
	assert(false);
	throw std::ios::failure("BUG: Archive format doesn't implement openFolder()");
}

void Archive::move(const EntryPtr& idBeforeThis, EntryPtr& id)
	throw (std::ios::failure)
{
	// Open the file we want to move
	iostream_sptr src(this->open(id));
	assert(src);

	// Insert a new file at the destination index
	EntryPtr n = this->insert(idBeforeThis, id->strName, id->iSize,
		id->type, id->fAttr);
	assert(n->bValid);

	iostream_sptr dst(this->open(n));
	assert(dst);

	// Copy the data into the new file's position
	boost::iostreams::copy(*src, *dst);
	dst->flush();

	// Remove the original file
	// TODO: Figure out which is best and whether entryPtrFromStream should be removed
	/*
	EntryPtr orig = this->entryPtrFromStream(src);
	assert(orig);
	assert(orig->bValid);
	this->remove(orig);
	*/
	this->remove(id);

	return;
}

int Archive::getSupportedAttributes() const
	throw ()
{
	return 0;
}

VC_METADATA_ITEMS Archive::getMetadataList() const
	throw ()
{
	return VC_METADATA_ITEMS();
}

std::string Archive::getMetadata(E_METADATA item) const
	throw (std::ios::failure)
{
	// This should never be called because getMetadataList() returned an empty
	// list.
	assert(false);
	throw std::ios::failure("unsupported metadata item");
}

// Change the value of a metadata element.
void Archive::setMetadata(E_METADATA item, const std::string& value)
	throw (std::ios::failure)
{
	// This should never be called because getMetadataList() returned an empty
	// list.
	assert(false);
	throw std::ios::failure("unsupported metadata item");
}

} // namespace gamearchive
} // namespace camoto
