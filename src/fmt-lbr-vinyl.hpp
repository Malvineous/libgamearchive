/**
 * @file   fmt-lbr-vinyl.hpp
 * @brief  Implementation of Vinyl Goddess From Mars .LBR file reader/writer.
 *
 * Copyright (C) 2010 Adam Nielsen <malvineous@shikadi.net>
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

#ifndef _CAMOTO_FMT_LBR_VINYL_HPP_
#define _CAMOTO_FMT_LBR_VINYL_HPP_

#include <camoto/gamearchive.hpp>

#include "fatarchive.hpp"

namespace camoto {
namespace gamearchive {

class LBRType: virtual public ArchiveType {

	public:

		LBRType()
			throw ();

		virtual ~LBRType()
			throw ();

		virtual std::string getArchiveCode() const
			throw ();

		virtual std::string getFriendlyName() const
			throw ();

		virtual std::vector<std::string> getFileExtensions() const
			throw ();

		virtual std::vector<std::string> getGameList() const
			throw ();

		virtual E_CERTAINTY isInstance(iostream_sptr fsArchive) const
			throw (std::ios::failure);

		virtual ArchivePtr newArchive(iostream_sptr psArchive, MP_SUPPDATA& suppData) const
			throw (std::ios::failure);

		virtual ArchivePtr open(iostream_sptr fsArchive, MP_SUPPDATA& suppData) const
			throw (std::ios::failure);

		virtual MP_SUPPLIST getRequiredSupps(const std::string& filenameArchive) const
			throw ();

};

class LBRArchive: virtual public FATArchive {
	public:
		LBRArchive(iostream_sptr psArchive)
			throw (std::ios::failure);

		virtual ~LBRArchive()
			throw ();

		// As per Archive (see there for docs)

		virtual void rename(EntryPtr& id, const std::string& strNewName)
			throw (std::ios_base::failure);

		// As per FATArchive (see there for docs)

		virtual void updateFileOffset(const FATEntry *pid, std::streamsize offDelta)
			throw (std::ios::failure);

		virtual void updateFileSize(const FATEntry *pid, std::streamsize sizeDelta)
			throw (std::ios_base::failure);

		virtual FATEntry *preInsertFile(const FATEntry *idBeforeThis, FATEntry *pNewEntry)
			throw (std::ios_base::failure);

		void preRemoveFile(const FATEntry *pid)
			throw (std::ios_base::failure);

	protected:
		// Update the header with the number of files in the archive
		void updateFileCount(uint32_t iNewCount)
			throw (std::ios_base::failure);

};

} // namespace gamearchive
} // namespace camoto

#endif // _CAMOTO_FMT_LBR_VINYL_HPP_