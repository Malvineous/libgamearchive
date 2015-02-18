/**
 * @file  stream_archfile.cpp
 * @brief Provide a stream that accesses a file within an Archive instance.
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

#include <cassert>
#include <camoto/util.hpp>
#include "stream_archfile.hpp"

using namespace camoto;
using namespace camoto::gamearchive;

archfile_core::archfile_core(const Archive::FileHandle& id)
	:	sub_core(0, 0),
		id(const_cast<Archive::FileHandle&>(id)),
		fat(dynamic_cast<const FATArchive::FATEntry *>(&*id))
{
}

void archfile_core::relocate(stream::delta off)
{
	throw stream::error("archfile_core::relocate() should never be called");
}

void archfile_core::resize(stream::len len)
{
	throw stream::error("archfile_core::resize() should never be called");
}

stream::pos archfile_core::sub_start() const
{
	if (!this->fat->bValid) {
		throw stream::error("Attempt to access closed or deleted file.");
	}
	return this->fat->iOffset + this->fat->lenHeader;
}

stream::len archfile_core::sub_size() const
{
	if (!this->fat->bValid) {
		throw stream::error("Attempt to access closed or deleted file.");
	}
	return this->fat->storedSize;
}


input_archfile::input_archfile(const Archive::FileHandle& id,
	std::shared_ptr<stream::input> content)
	:	sub_core(0, 0), // length values are unused as we will be overriding them
		input_sub(content, 0, 0),
		archfile_core(id)
{
}


output_archfile::output_archfile(std::shared_ptr<Archive> archive,
	Archive::FileHandle id, std::shared_ptr<stream::output> content)
	:	sub_core(0, 0), // length values are unused as we will be overriding them
		output_sub(content, 0, 0, stream::fn_truncate_sub()),
		archfile_core(id),
		archive(archive)
{
}

void output_archfile::truncate(stream::len size)
{
	if (this->sub_size() == size) return; // nothing to do
	assert(this->id);

	stream::len newRealSize;
	if (this->id->fAttr & EA_COMPRESSED) {
		// We're compressed, so the real and stored sizes are both valid
		newRealSize = this->id->realSize;
	} else {
		// We're not compressed, so the real size won't be updated by a filter,
		// so we need to update it here.
		newRealSize = size;
	}

	// Resize the file in the archive.  This function will also tell the
	// substream it can now write to a larger area.
	// We are updating both the stored (in-archive) and the real (extracted)
	// sizes, to handle the case where no filters are used and the sizes are
	// the same.  When filters are in use, the flush() function that writes
	// the filtered data out should call us first, then call the archive's
	// resize() function with the correct real/extracted size.
	this->archive->resize(this->id, size, newRealSize);

	// After a truncate the file pointer is always left at the new EOF
	try {
		this->seekp(size, stream::start);
	} catch (const stream::seek_error& e) {
		throw stream::write_error("Archive resize function silently failed!  "
			"Unable to seek to EOF after truncate: " + e.get_message());
	}
	return;
}

void output_archfile::setRealSize(stream::len newRealSize)
{
	this->archive->resize(this->id, this->id->storedSize, newRealSize);
	return;
}

void output_archfile::flush()
{
	this->out_parent->flush();
	if (this->archive.unique()) {
		// We are the only user of the shared archive, so the caller has no other
		// means to flush it.  So we will have to flush it for them.
		this->archive->flush();
	}
	return;
}


archfile::archfile(std::shared_ptr<Archive> archive, Archive::FileHandle id,
	std::shared_ptr<stream::inout> content)
	:	sub_core(0, 0),
		input_sub(content, 0, 0),
		output_sub(content, 0, 0, stream::fn_truncate_sub()),
		sub(content, 0, 0, stream::fn_truncate_sub()),
		archfile_core(id),
		input_archfile(id, content),
		output_archfile(archive, id, content)
{
}
