/**
 * @file  fixedarchive.cpp
 * @brief Generic archive providing access to "files" at specific offsets and
 *        lengths in a host file (e.g. game levels stored in an .exe file.)
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

#include <boost/algorithm/string.hpp>
#include <functional>
#include <camoto/util.hpp>
#include <camoto/gamearchive/fixedarchive.hpp>

namespace camoto {
namespace gamearchive {

FixedArchive::FixedArchive(std::unique_ptr<stream::inout> content,
	std::vector<FixedArchiveFile> vcFiles)
	:	content(std::move(content)),
		vcFiles(vcFiles)
{
	int j = 0;
	for (auto& i : this->vcFiles) {
		auto f = std::make_unique<FixedEntry>();
		f->bValid = true;
		f->storedSize = f->realSize = i.size;
		f->strName = i.name;
		f->type = FILETYPE_GENERIC;
		f->filter = i.filter;
		f->fAttr = File::Attribute::Default;
		if (!i.filter.empty()) f->fAttr |= File::Attribute::Compressed;

		f->fixed = &i;
		f->index = j++;

		if (i.fnResize) {
			i.fnResize(*this->content, f.get(), (stream::len)-1, (stream::len)-1);
		}

		this->vcFixedEntries.push_back(std::move(f));
	}
}

FixedArchive::~FixedArchive()
{
}

const Archive::FileVector& FixedArchive::files() const
{
	return this->vcFixedEntries;
}

Archive::FileHandle FixedArchive::find(const std::string& strFilename) const
{
	// TESTED BY: TODO
	for (auto& i : this->vcFixedEntries) {
		const FixedEntry *entry = dynamic_cast<const FixedEntry *>(i.get());
		const FixedArchiveFile *file = &this->vcFiles[entry->index];
		if (boost::iequals(file->name, strFilename)) {
			return i;  // i is the original shared_ptr
		}
	}
	return nullptr;
}

bool FixedArchive::isValid(const FileHandle& id) const
{
	const FixedEntry *id2 = dynamic_cast<const FixedEntry *>(id.get());
	return ((id2) && (id2->index < this->vcFiles.size()));
}

std::unique_ptr<stream::inout> FixedArchive::open(const FileHandle& id,
	bool useFilter)
{
	if (!id->filter.empty() && useFilter) {
		throw stream::error("Filtering on fixed archives is not yet implemented!");
	}
	// TESTED BY: TODO
	const FixedEntry *entry = dynamic_cast<const FixedEntry *>(id.get());
	const FixedArchiveFile *file = &this->vcFiles[entry->index];
	return std::make_unique<stream::sub>(
		this->content,
		file->offset,
		file->size,
		[id, this](stream::output_sub* sub, stream::len newSize) {
			// An open substream belonging to file entry 'id' wants to be resized.

			stream::len newRealSize;
			if (id->fAttr & File::Attribute::Compressed) {
				// We're compressed, so the real and stored sizes are both valid
				newRealSize = id->realSize;
			} else {
				// We're not compressed, so the real size won't be updated by a filter,
				// so we need to update it here.
				newRealSize = newSize;
			}

			// Resize the file in the archive.  This function will also tell the
			// substream it can now write to a larger area.
			// We are updating both the stored (in-archive) and the real (extracted)
			// sizes, to handle the case where no filters are used and the sizes are
			// the same.  When filters are in use, the flush() function that writes
			// the filtered data out should call us first, then call the archive's
			// resize() function with the correct real/extracted size.
			FileHandle& id_noconst = const_cast<FileHandle&>(id);
			this->resize(id_noconst, newSize, newRealSize);
		}
	);
}

std::shared_ptr<Archive> FixedArchive::openFolder(const Archive::FileHandle& id)
{
	// This function should only be called for folders (not files)
	assert(id->fAttr & File::Attribute::Folder);

	// Throw an exception if assertions have been disabled.
	throw stream::error("BUG: openFolder() called for archive format that "
		"doesn't have any folders.");
}

Archive::FileHandle FixedArchive::insert(const FileHandle& idBeforeThis,
	const std::string& strFilename, stream::pos storedSize, std::string type,
	File::Attribute attr)
{
	throw stream::error("This is a fixed archive, files cannot be inserted.");
}

void FixedArchive::remove(FileHandle& id)
{
	throw stream::error("This is a fixed archive, files cannot be removed.");
}

void FixedArchive::rename(FileHandle& id, const std::string& strNewName)
{
	throw stream::error("This is a fixed archive, files cannot be renamed.");
}

void FixedArchive::move(const FileHandle& idBeforeThis, FileHandle& id)
{
	throw stream::error("This is a fixed archive, files cannot be moved.");
}

void FixedArchive::resize(FileHandle& id, stream::pos newStoredSize,
	stream::pos newRealSize)
{
	auto entry = FixedEntry::cast(id);
	const FixedArchiveFile *file = &this->vcFiles[entry->index];
	if (file->fnResize) {
		file->fnResize(*this->content, entry, newStoredSize, newRealSize);
	} else if (id->storedSize != newStoredSize) {
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
