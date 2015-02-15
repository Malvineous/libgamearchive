/**
 * @file  fmt-dat-mystic.hpp
 * @brief Mystic Towers .DAT format.
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

#ifndef _CAMOTO_FMT_DAT_MYSTIC_HPP_
#define _CAMOTO_FMT_DAT_MYSTIC_HPP_

#include <camoto/gamearchive/archivetype.hpp>
#include <camoto/stream_seg.hpp>
#include <camoto/stream_filtered.hpp>
#include "fatarchive.hpp"

namespace camoto {
namespace gamearchive {

/// Mystic Towers .DAT format handler.
class ArchiveType_DAT_Mystic: virtual public ArchiveType
{
	public:
		ArchiveType_DAT_Mystic();
		virtual ~ArchiveType_DAT_Mystic();

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

/// Mystic Towers .DAT archive instance.
class Archive_DAT_Mystic: virtual public FATArchive
{
	public:
		Archive_DAT_Mystic(std::unique_ptr<stream::inout> content);
		virtual ~Archive_DAT_Mystic();

		virtual void updateFileName(const FATEntry *pid,
			const std::string& strNewName);
		virtual void updateFileOffset(const FATEntry *pid, stream::delta offDelta);
		virtual void updateFileSize(const FATEntry *pid, stream::delta sizeDelta);
		virtual void preInsertFile(const FATEntry *idBeforeThis,
			FATEntry *pNewEntry);
		virtual void postInsertFile(FATEntry *pNewEntry);
		virtual void preRemoveFile(const FATEntry *pid);
		virtual void postRemoveFile(const FATEntry *pid);

	protected:
		void updateFileCount(uint32_t newCount);

		/// Number of FAT entries on disk but not yet in vcFAT
		int uncommittedFiles;
};

} // namespace gamearchive
} // namespace camoto

#endif // _CAMOTO_FMT_DAT_MYSTIC_HPP_
