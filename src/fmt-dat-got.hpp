/**
 * @file   fmt-dat-got.hpp
 * @brief  Implementation of God of Thunder .DAT file reader/writer.
 *
 * Copyright (C) 2011 Adam Nielsen <malvineous@shikadi.net>
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

#ifndef _CAMOTO_FMT_DAT_GOT_HPP_
#define _CAMOTO_FMT_DAT_GOT_HPP_

#include <camoto/filteredstream.hpp>
#include <camoto/gamearchive.hpp>

#include "fatarchive.hpp"
#include "filter-xor.hpp"

namespace camoto {
namespace gamearchive {

class DAT_GoTType: virtual public ArchiveType {

	public:

		DAT_GoTType()
			throw ();

		virtual ~DAT_GoTType()
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

class DAT_GoTArchive: virtual public FATArchive {
	public:
		DAT_GoTArchive(iostream_sptr psArchive)
			throw (std::ios::failure);

		virtual ~DAT_GoTArchive()
			throw ();

		// As per Archive (see there for docs)

		virtual void rename(EntryPtr& id, const std::string& strNewName)
			throw (std::ios_base::failure);

		virtual void flush()
			throw (std::ios::failure);

		virtual int getSupportedAttributes() const
			throw ();

		// As per FATArchive (see there for docs)

		virtual void updateFileOffset(const FATEntry *pid, std::streamsize offDelta)
			throw (std::ios::failure);

		virtual void updateFileSize(const FATEntry *pid, std::streamsize sizeDelta)
			throw (std::ios_base::failure);

		virtual FATEntry *preInsertFile(const FATEntry *idBeforeThis, FATEntry *pNewEntry)
			throw (std::ios_base::failure);

		virtual void postInsertFile(FATEntry *pNewEntry)
			throw (std::ios::failure);

		void preRemoveFile(const FATEntry *pid)
			throw (std::ios_base::failure);

	protected:
		substream_sptr fatSubStream;   ///< On-disk stream storing the possibly encrypted FAT
		segstream_sptr fatStream;      ///< Segstream wrapper around decrypted fatSubStream
		xor_crypt_filter fatCrypt;     ///< Encryption/decryption to use on the FAT
		filtered_iostream_sptr fatFilter; ///< Decrypted FAT stream

		/// Dummy function - does nothing
		/**
		 * Since this format's FAT is a constant size it will never be changed,
		 * but fatStream requires a truncate callback when commit() is called.
		 */
		void truncateFAT(io::stream_offset newSize)
			throw (std::ios::failure);

};

} // namespace gamearchive
} // namespace camoto

#endif // _CAMOTO_FMT_DAT_GOT_HPP_
