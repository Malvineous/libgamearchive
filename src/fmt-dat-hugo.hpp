/**
 * @file   fmt-dat-hugo.cpp
 * @brief  Implementation of Hugo scenery .DAT file reader/writer.
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

#ifndef _CAMOTO_FMT_DAT_HUGO_HPP_
#define _CAMOTO_FMT_DAT_HUGO_HPP_

#include <camoto/gamearchive.hpp>

#include "fatarchive.hpp"

namespace camoto {
namespace gamearchive {

class DAT_HugoType: virtual public ArchiveType {

	public:

		DAT_HugoType()
			throw ();

		virtual ~DAT_HugoType()
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

		virtual ArchivePtr newArchive(iostream_sptr psArchive, SuppData& suppData) const
			throw (std::ios::failure);

		virtual ArchivePtr open(iostream_sptr fsArchive, SuppData& suppData) const
			throw (std::ios::failure);

		virtual SuppFilenames getRequiredSupps(const std::string& filenameArchive) const
			throw ();

};

class DAT_HugoArchive: virtual public FATArchive {
	protected:

		segstream_sptr psFAT;
		FN_TRUNCATE fnTruncFAT;
		struct DAT_HugoEntry: virtual public FATEntry {
			int file;
		};

	public:
		DAT_HugoArchive(iostream_sptr psArchive, iostream_sptr psFAT, FN_TRUNCATE fnTruncFAT)
			throw (std::ios::failure);

		virtual ~DAT_HugoArchive()
			throw ();

		// As per FATArchive (see there for docs)

		virtual void updateFileName(const FATEntry *pid, const std::string& strNewName)
			throw (std::ios::failure);

		virtual void updateFileOffset(const FATEntry *pid, std::streamsize offDelta)
			throw (std::ios::failure);

		virtual void updateFileSize(const FATEntry *pid, std::streamsize sizeDelta)
			throw (std::ios::failure);

		virtual FATEntry *preInsertFile(const FATEntry *idBeforeThis, FATEntry *pNewEntry)
			throw (std::ios::failure);

		virtual void preRemoveFile(const FATEntry *pid)
			throw (std::ios::failure);

		virtual FATEntry *createNewFATEntry()
			throw ();

};

} // namespace gamearchive
} // namespace camoto

#endif // _CAMOTO_FMT_DAT_HUGO_HPP_
