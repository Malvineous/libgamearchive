/**
 * @file   fixedarchive.cpp
 * @brief  Generic archive providing access to "files" at specific offsets and
 *         lengths in a host file (e.g. game levels stored in an .exe file.)
 *
 * Copyright (C) 2010-2012 Adam Nielsen <malvineous@shikadi.net>
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
#include <camoto/gamearchive/fixedarchive.hpp>
#include <camoto/util.hpp>

namespace camoto {
namespace gamearchive {

FixedArchive::FixedArchive(stream::inout_sptr psArchive, std::vector<FixedArchiveFile> files)
	:	psArchive(psArchive),
		files(files)
{
	int j = 0;
	for (std::vector<FixedArchiveFile>::iterator i = files.begin(); i != files.end(); i++) {
		FixedEntry *fe = new FixedEntry();
		EntryPtr ep(fe);
		fe->bValid = true;
		fe->storedSize = i->size;
		fe->realSize = i->size;
		fe->strName = i->name;
		fe->type = FILETYPE_GENERIC;
		fe->filter = i->filter;
		fe->fAttr = 0;

		fe->index = j++;

		this->vcFixedEntries.push_back(ep);
	}
}

FixedArchive::~FixedArchive()
{
}

const FixedArchive::VC_ENTRYPTR& FixedArchive::getFileList() const
{
	return this->vcFixedEntries;
}

FixedArchive::EntryPtr FixedArchive::find(const std::string& strFilename) const
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

bool FixedArchive::isValid(const EntryPtr id) const
{
	const FixedEntry *id2 = dynamic_cast<const FixedEntry *>(id.get());
	return ((id2) && (id2->index < this->files.size()));
}

stream::inout_sptr FixedArchive::open(const EntryPtr id)
{
	// TESTED BY: TODO
	const FixedEntry *entry = dynamic_cast<const FixedEntry *>(id.get());
	const FixedArchiveFile *file = &this->files[entry->index];
	stream::fn_truncate fnTrunc = preventResize;
	stream::sub_sptr psSub(new stream::sub());
	psSub->open(
		this->psArchive,
		file->offset,
		file->size,
		fnTrunc
	);
	this->vcSubStream.push_back(psSub);
	return psSub;
}

FixedArchive::EntryPtr FixedArchive::insert(const EntryPtr idBeforeThis,
	const std::string& strFilename, stream::pos storedSize, std::string type,
	int attr
)
{
	throw stream::error("This is a fixed archive, files cannot be inserted.");
}

void FixedArchive::remove(EntryPtr id)
{
	throw stream::error("This is a fixed archive, files cannot be removed.");
}

void FixedArchive::rename(EntryPtr id, const std::string& strNewName)
{
	throw stream::error("This is a fixed archive, files cannot be renamed.");
}

void FixedArchive::move(const EntryPtr idBeforeThis, EntryPtr id)
{
	throw stream::error("This is a fixed archive, files cannot be moved.");
}

void FixedArchive::resize(EntryPtr id, stream::pos newStoredSize,
	stream::pos newRealSize)
{
	if (id->storedSize != newStoredSize) {
		throw stream::error(createString("This is a fixed archive, files "
			"cannot be resized (tried to resize to " << newStoredSize <<
			", must remain as " << id->storedSize << ")"));
	}
	// else no change, so do nothing
	return;
}

void FixedArchive::flush()
{
	// no-op (nothing to flush)
	return;
}

} // namespace gamearchive
} // namespace camoto
