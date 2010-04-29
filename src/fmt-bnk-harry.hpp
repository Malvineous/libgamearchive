/*
 * fmt-bnk-harry.hpp - Halloween Harry .BNK file reader/writer.
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

#ifndef _CAMOTO_FMT_BNK_HARRY_HPP_
#define _CAMOTO_FMT_BNK_HARRY_HPP_

#include <camoto/gamearchive.hpp>

#include "fatarchive.hpp"

namespace camoto {
namespace gamearchive {

class BNKType: virtual public ArchiveType {

	public:

		BNKType()
			throw ();

		virtual ~BNKType()
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

		virtual ArchivePtr open(iostream_sptr psArchive, MP_SUPPDATA& suppData) const
			throw (std::ios::failure);

		virtual MP_SUPPLIST getRequiredSupps(const std::string& filenameArchive) const
			throw ();

};

class BNKArchive: virtual public FATArchive {
	protected:
		segstream_sptr psFAT;
		FN_TRUNCATE fnTruncFAT;
		bool isAC;  // true == Alien Carnage, false == Halloween Harry

	public:
		BNKArchive(iostream_sptr psArchive, iostream_sptr psFAT, FN_TRUNCATE fnTruncFAT)
			throw (std::ios::failure);

		virtual ~BNKArchive()
			throw ();

		// As per Archive (see there for docs)

		virtual void rename(EntryPtr& id, const std::string& strNewName)
			throw (std::ios_base::failure);

		virtual void flush()
			throw (std::ios::failure);

		// As per FATArchive (see there for docs)

		virtual void updateFileOffset(const FATEntry *pid)
			throw (std::ios::failure);

		virtual void updateFileSize(const FATEntry *pid)
			throw (std::ios_base::failure);

		void insertFATEntry(const FATEntry *idBeforeThis, FATEntry *pNewEntry)
			throw (std::ios_base::failure);

		void removeFATEntry(const FATEntry *pid)
			throw (std::ios_base::failure);

	protected:
		// Update the header with the number of files in the archive
		void updateFileCount(uint32_t iNewCount)
			throw (std::ios_base::failure);

};

} // namespace gamearchive
} // namespace camoto

#endif // _CAMOTO_FMT_BNK_HARRY_HPP_
