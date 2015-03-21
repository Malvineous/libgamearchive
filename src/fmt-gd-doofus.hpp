/**
 * @file  fmt-gd-doofus.hpp
 * @brief Doofus .G-D format.
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

#ifndef _CAMOTO_FMT_GD_DOOFUS_HPP_
#define _CAMOTO_FMT_GD_DOOFUS_HPP_

#include <camoto/gamearchive/archivetype.hpp>
#include <camoto/gamearchive/archive-fat.hpp>

namespace camoto {
namespace gamearchive {

/// Doofus .G-D format handler.
class ArchiveType_GD_Doofus: virtual public ArchiveType
{
	public:
		ArchiveType_GD_Doofus();
		virtual ~ArchiveType_GD_Doofus();

		virtual std::string code() const;
		virtual std::string friendlyName() const;
		virtual std::vector<std::string> fileExtensions() const;
		virtual std::vector<std::string> games() const;
		virtual ArchiveType::Certainty isInstance(stream::input& content) const;
		virtual std::shared_ptr<Archive> open(
			std::unique_ptr<stream::inout> content, SuppData& suppData) const;
		virtual std::shared_ptr<Archive> create(
			std::unique_ptr<stream::inout> content, SuppData& suppData) const;
		virtual SuppFilenames getRequiredSupps(stream::input& content,
			const std::string& filenameArchive) const;
};

/// Doofus .G-D archive instance.
class Archive_GD_Doofus: virtual public Archive_FAT
{
	protected:
		std::unique_ptr<stream::seg> psFAT; ///< FAT stream (doofus.exe)
		uint32_t maxFiles;    ///< Maximum number of files in FAT
		uint32_t numFiles;    ///< Current number of files in FAT

	public:
		Archive_GD_Doofus(std::unique_ptr<stream::inout> content,
			std::unique_ptr<stream::inout> psFAT);
		virtual ~Archive_GD_Doofus();

		virtual void flush();

		virtual void updateFileSize(const FATEntry *pid, stream::delta sizeDelta);
		virtual void preInsertFile(const FATEntry *idBeforeThis,
			FATEntry *pNewEntry);
		virtual void preRemoveFile(const FATEntry *pid);

	protected:
		// Update the header with the number of files in the archive
		void updateFileCount(uint32_t iNewCount);
};

} // namespace gamearchive
} // namespace camoto

#endif // _CAMOTO_FMT_GD_DOOFUS_HPP_
