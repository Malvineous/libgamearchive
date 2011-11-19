/**
 * @file   fmt-dlt-stargunner.hpp
 * @brief  Implementation of Star Gunner .DLT file reader/writer.
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

#ifndef _CAMOTO_FMT_DLT_STARGUNNER_HPP_
#define _CAMOTO_FMT_DLT_STARGUNNER_HPP_

#include <camoto/gamearchive.hpp>

#include "fatarchive.hpp"

namespace camoto {
namespace gamearchive {

class DLTType: virtual public ArchiveType {

	public:

		DLTType()
			throw ();

		virtual ~DLTType()
			throw ();

		virtual std::string getArchiveCode() const
			throw ();

		virtual std::string getFriendlyName() const
			throw ();

		virtual std::vector<std::string> getFileExtensions() const
			throw ();

		virtual std::vector<std::string> getGameList() const
			throw ();

		virtual ArchiveType::Certainty isInstance(stream::input_sptr fsArchive) const
			throw (stream::error);

		virtual ArchivePtr newArchive(stream::inout_sptr psArchive, SuppData& suppData) const
			throw (stream::error);

		virtual ArchivePtr open(stream::inout_sptr fsArchive, SuppData& suppData) const
			throw (stream::error);

		virtual SuppFilenames getRequiredSupps(stream::input_sptr data,
			const std::string& filenameArchive) const
			throw ();

};

class DLTArchive: virtual public FATArchive {
	public:
		DLTArchive(stream::inout_sptr psArchive)
			throw (stream::error);

		virtual ~DLTArchive()
			throw ();

		// As per FATArchive (see there for docs)

		virtual void updateFileName(const FATEntry *pid, const std::string& strNewName)
			throw (stream::error);

		virtual void updateFileOffset(const FATEntry *pid, stream::delta offDelta)
			throw (stream::error);

		virtual void updateFileSize(const FATEntry *pid, stream::delta sizeDelta)
			throw (stream::error);

		virtual FATEntry *preInsertFile(const FATEntry *idBeforeThis, FATEntry *pNewEntry)
			throw (stream::error);

		virtual void preRemoveFile(const FATEntry *pid)
			throw (stream::error);

	protected:
		// Update the header with the number of files in the archive
		void updateFileCount(uint16_t iNewCount)
			throw (stream::error);

};

} // namespace gamearchive
} // namespace camoto

#endif // _CAMOTO_FMT_DLT_STARGUNNER_HPP_