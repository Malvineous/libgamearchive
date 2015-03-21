/**
 * @file  fmt-res-stellar7.hpp
 * @brief Stellar 7 .RES format.
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

#ifndef _CAMOTO_FMT_RES_STELLAR7_HPP_
#define _CAMOTO_FMT_RES_STELLAR7_HPP_

#include <camoto/gamearchive/archivetype.hpp>
#include <camoto/gamearchive/archive-fat.hpp>

namespace camoto {
namespace gamearchive {

/// Stellar 7 .RES format handler.
class ArchiveType_RES_Stellar7: virtual public ArchiveType
{
	public:
		ArchiveType_RES_Stellar7();
		virtual ~ArchiveType_RES_Stellar7();

		virtual std::string code() const;
		virtual std::string friendlyName() const;
		virtual std::vector<std::string> fileExtensions() const;
		virtual std::vector<std::string> games() const;
		virtual ArchiveType::Certainty isInstance(stream::input& content) const;
		virtual std::shared_ptr<Archive> create(
			std::unique_ptr<stream::inout> content, SuppData& suppData) const;
		virtual std::shared_ptr<Archive> open(
			std::unique_ptr<stream::inout> content, SuppData& suppData) const;
		virtual SuppFilenames getRequiredSupps(stream::input& content,
			const std::string& filenameArchive) const;
};

/// Stellar 7 .RES archive instance.
class Archive_RES_Stellar7_Folder: virtual public Archive_FAT
{
	public:
		Archive_RES_Stellar7_Folder(std::unique_ptr<stream::inout> content);
		virtual ~Archive_RES_Stellar7_Folder();

		virtual std::shared_ptr<Archive> openFolder(const FileHandle& id);

		virtual void updateFileName(const FATEntry *pid,
			const std::string& strNewName);
		virtual void updateFileSize(const FATEntry *pid, stream::delta sizeDelta);
		virtual void preInsertFile(const FATEntry *idBeforeThis,
			FATEntry *pNewEntry);
};

} // namespace gamearchive
} // namespace camoto

#endif // _CAMOTO_FMT_RES_STELLAR7_HPP_
