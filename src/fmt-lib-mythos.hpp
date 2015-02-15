/**
 * @file  fmt-lib-mythos.hpp
 * @brief Mythos .LIB format.
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

#ifndef _CAMOTO_FMT_LIB_MYTHOS_HPP_
#define _CAMOTO_FMT_LIB_MYTHOS_HPP_

#include <camoto/gamearchive/archivetype.hpp>
#include "fatarchive.hpp"

namespace camoto {
namespace gamearchive {

/// Mythos .LIB format handler.
class ArchiveType_LIB_Mythos: virtual public ArchiveType
{
	public:
		ArchiveType_LIB_Mythos();
		virtual ~ArchiveType_LIB_Mythos();

		virtual std::string code() const;
		virtual std::string friendlyName() const;
		virtual std::vector<std::string> fileExtensions() const;
		virtual std::vector<std::string> games() const;
		virtual ArchiveType::Certainty isInstance(stream::input& content) const;
		virtual std::unique_ptr<Archive> create(
			std::unique_ptr<stream::inout> content, SuppData& suppData) const;
		virtual std::unique_ptr<Archive> open(
			std::unique_ptr<stream::inout> content, SuppData& suppData) const;
		virtual SuppFilenames getRequiredSupps(stream::input& content,
			const std::string& filenameArchive) const;
};

/// Mythos .LIB archive instance.
class Archive_LIB_Mythos: virtual public FATArchive {
	public:
		Archive_LIB_Mythos(std::unique_ptr<stream::inout> content);
		virtual ~Archive_LIB_Mythos();

		virtual void updateFileName(const FATEntry *pid,
			const std::string& strNewName);
		virtual void updateFileOffset(const FATEntry *pid, stream::delta offDelta);
		virtual void updateFileSize(const FATEntry *pid, stream::delta sizeDelta);
		virtual void preInsertFile(const FATEntry *idBeforeThis,
			FATEntry *pNewEntry);
		virtual void preRemoveFile(const FATEntry *pid);

	protected:
		stream::pos lenArchive;

		// Update the last FAT entry to point to EOF.
		void updateLastEntry(stream::delta lenDelta);

		// Update the header with the number of files in the archive
		void updateFileCount(uint16_t iNewCount);
};

} // namespace gamearchive
} // namespace camoto

#endif // _CAMOTO_FMT_LIB_MYTHOS_HPP_
