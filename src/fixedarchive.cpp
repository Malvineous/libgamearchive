/**
 * @file   fixedarchive.cpp
 * @brief  Generic archive providing access to "files" at specific offsets and
 *         lengths in a host file (e.g. game levels stored in an .exe file.)
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

#include <boost/algorithm/string.hpp>
#include <camoto/util.hpp>
#include "fixedarchive.hpp"

namespace camoto {
namespace gamearchive {

FixedArchive::FixedArchive(iostream_sptr psArchive, FixedArchiveFile *files,
	int numFiles
)
	throw (std::ios::failure) :
		psArchive(psArchive),
		files(files),
		numFiles(numFiles)
{
	for (int i = 0; i < numFiles; i++) {
		FixedEntry *fe = new FixedEntry();
		EntryPtr ep(fe);
		fe->bValid = true;
		fe->iSize = files[i].size;
		fe->strName = files[i].name;
		fe->type = FILETYPE_GENERIC;
		fe->filter = files[i].filter;
		fe->fAttr = 0;

		fe->index = i;

		this->vcFixedEntries.push_back(ep);
	}
}

FixedArchive::~FixedArchive()
	throw ()
{
}

const FixedArchive::VC_ENTRYPTR& FixedArchive::getFileList() const
	throw ()
{
	return this->vcFixedEntries;
}

FixedArchive::EntryPtr FixedArchive::find(const std::string& strFilename) const
	throw ()
{
	// TESTED BY: TODO
	for (VC_ENTRYPTR::const_iterator i = this->vcFixedEntries.begin();
		i != this->vcFixedEntries.end();
		i++
	) {
		const FixedEntry *entry = dynamic_cast<const FixedEntry *>(i->get());
		const FixedArchiveFile *file = &this->files[entry->index];
		if (boost::iequals(file->name, strFilename)) {
			return *i;  // *i is the original shared_ptr
		}
	}
	return EntryPtr();
}

bool FixedArchive::isValid(const EntryPtr& id) const
	throw ()
{
	const FixedEntry *id2 = dynamic_cast<const FixedEntry *>(id.get());
	return ((id2) && (id2->index < this->numFiles));
}

iostream_sptr FixedArchive::open(const EntryPtr& id)
	throw ()
{
	// TESTED BY: TODO
	const FixedEntry *entry = dynamic_cast<const FixedEntry *>(id.get());
	const FixedArchiveFile *file = &this->files[entry->index];
	substream_sptr psSub(
		new substream(
			this->psArchive,
			file->offset,
			file->size
		)
	);
	this->vcSubStream.push_back(psSub);
	return psSub;
}

FixedArchive::EntryPtr FixedArchive::insert(const EntryPtr& idBeforeThis,
	const std::string& strFilename, offset_t iSize, std::string type,
	int attr
)
	throw (std::ios::failure)
{
	throw std::ios::failure("This is a fixed archive, files cannot be inserted.");
}

void FixedArchive::remove(EntryPtr& id)
	throw (std::ios::failure)
{
	throw std::ios::failure("This is a fixed archive, files cannot be removed.");
}

void FixedArchive::rename(EntryPtr& id, const std::string& strNewName)
	throw (std::ios::failure)
{
	throw std::ios::failure("This is a fixed archive, files cannot be renamed.");
}

void FixedArchive::move(const EntryPtr& idBeforeThis, EntryPtr& id)
	throw (std::ios::failure)
{
	throw std::ios::failure("This is a fixed archive, files cannot be moved.");
}

void FixedArchive::resize(EntryPtr& id, offset_t iNewSize,
	offset_t iNewPrefilteredSize)
	throw (std::ios::failure)
{
	if (id->iPrefilteredSize != iNewPrefilteredSize) {
		throw std::ios::failure(createString("This is a fixed archive, files "
			"cannot be resized (tried to resize to " << iNewPrefilteredSize <<
			", must remain as " << id->iPrefilteredSize << ")"));
	}
	// else no change, so do nothing
	return;
}

void FixedArchive::flush()
	throw (std::ios::failure)
{
	// no-op (nothing to flush)
	return;
}

} // namespace gamearchive
} // namespace camoto
