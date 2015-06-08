/**
 * @file  fmt-rff-blood.hpp
 * @brief Blood .RFF format.
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

#ifndef _CAMOTO_FMT_RFF_BLOOD_HPP_
#define _CAMOTO_FMT_RFF_BLOOD_HPP_

#include <camoto/gamearchive/archivetype.hpp>
#include <camoto/stream_seg.hpp>
#include <camoto/stream_filtered.hpp>
#include <camoto/gamearchive/archive-fat.hpp>
#include "filter-xor-blood.hpp"

namespace camoto {
namespace gamearchive {

/// Blood .RFF format handler.
class ArchiveType_RFF_Blood: virtual public ArchiveType
{
	public:
		ArchiveType_RFF_Blood();
		virtual ~ArchiveType_RFF_Blood();

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
			const std::string& filename) const;
};

/// Blood .RFF archive instance.
class Archive_RFF_Blood: virtual public Archive_FAT
{
	public:
		Archive_RFF_Blood(std::unique_ptr<stream::inout> content);
		virtual ~Archive_RFF_Blood();

		virtual MetadataTypes getMetadataList() const;
		virtual std::string getMetadata(MetadataType item) const;
		virtual void setMetadata(MetadataType item, const std::string& value);

		/// Write out the FAT with the updated encryption key.
		virtual void flush();

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
		/// In-memory stream storing the cleartext FAT
		std::unique_ptr<stream::seg> fatStream;
		uint32_t version;            ///< File format version
		bool modifiedFAT;            ///< Has the FAT been changed?

		void updateFileCount(uint32_t newCount);

		stream::pos getDescOffset() const;

		void splitFilename(const std::string& full, std::string *base,
			std::string *ext);
};

} // namespace gamearchive
} // namespace camoto

#endif // _CAMOTO_FMT_RFF_BLOOD_HPP_
