/**
 * @file   fmt-bnk-harry.hpp
 * @brief  Halloween Harry .BNK file reader/writer.
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

#ifndef _CAMOTO_FMT_BNK_HARRY_HPP_
#define _CAMOTO_FMT_BNK_HARRY_HPP_

#include <camoto/gamearchive.hpp>

#include "fatarchive.hpp"

namespace camoto {
namespace gamearchive {

class BNKType: virtual public ArchiveType {

	public:

		BNKType();

		virtual ~BNKType();

		virtual std::string getArchiveCode() const;

		virtual std::string getFriendlyName() const;

		virtual std::vector<std::string> getFileExtensions() const;

		virtual std::vector<std::string> getGameList() const;

		virtual ArchiveType::Certainty isInstance(stream::input_sptr fsArchive) const;

		virtual ArchivePtr open(stream::inout_sptr psArchive, SuppData& suppData) const;

		virtual SuppFilenames getRequiredSupps(stream::input_sptr data,
			const std::string& filenameArchive) const;

};

class BNKArchive: virtual public FATArchive {
	protected:
		stream::seg_sptr psFAT;
		bool isAC;  // true == Alien Carnage, false == Halloween Harry

	public:
		BNKArchive(stream::inout_sptr psArchive, stream::inout_sptr psFAT);

		virtual ~BNKArchive();

		// As per Archive (see there for docs)

		virtual void flush();

		// As per FATArchive (see there for docs)

		virtual void updateFileName(const FATEntry *pid, const std::string& strNewName);

		virtual void updateFileOffset(const FATEntry *pid, stream::delta offDelta);

		virtual void updateFileSize(const FATEntry *pid, stream::delta sizeDelta);

		virtual FATEntry *preInsertFile(const FATEntry *idBeforeThis, FATEntry *pNewEntry);

		virtual void preRemoveFile(const FATEntry *pid);

	protected:
		// Update the header with the number of files in the archive
		void updateFileCount(uint32_t iNewCount);

};

} // namespace gamearchive
} // namespace camoto

#endif // _CAMOTO_FMT_BNK_HARRY_HPP_
