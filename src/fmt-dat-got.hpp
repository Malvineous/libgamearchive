/**
 * @file   fmt-dat-got.hpp
 * @brief  Implementation of God of Thunder .DAT file reader/writer.
 *
 * Copyright (C) 2010-2012 Adam Nielsen <malvineous@shikadi.net>
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

#include <camoto/stream_filtered.hpp>
#include <camoto/gamearchive/archivetype.hpp>
#include "fatarchive.hpp"
#include "filter-xor.hpp"

namespace camoto {
namespace gamearchive {

class DAT_GoTType: virtual public ArchiveType
{
	public:
		DAT_GoTType();
		virtual ~DAT_GoTType();

		virtual std::string getArchiveCode() const;
		virtual std::string getFriendlyName() const;
		virtual std::vector<std::string> getFileExtensions() const;
		virtual std::vector<std::string> getGameList() const;
		virtual ArchiveType::Certainty isInstance(stream::input_sptr fsArchive)
			const;
		virtual ArchivePtr newArchive(stream::inout_sptr psArchive,
			SuppData& suppData) const;
		virtual ArchivePtr open(stream::inout_sptr fsArchive, SuppData& suppData)
			const;
		virtual SuppFilenames getRequiredSupps(stream::input_sptr data,
			const std::string& filenameArchive) const;
};

class DAT_GoTArchive: virtual public FATArchive
{
	public:
		DAT_GoTArchive(stream::inout_sptr psArchive);
		virtual ~DAT_GoTArchive();

		virtual void flush();
		virtual int getSupportedAttributes() const;

		virtual void updateFileName(const FATEntry *pid,
			const std::string& strNewName);
		virtual void updateFileOffset(const FATEntry *pid, stream::delta offDelta);
		virtual void updateFileSize(const FATEntry *pid, stream::delta sizeDelta);
		virtual FATEntry *preInsertFile(const FATEntry *idBeforeThis,
			FATEntry *pNewEntry);
		virtual void postInsertFile(FATEntry *pNewEntry);
		virtual void preRemoveFile(const FATEntry *pid);

	protected:
		stream::sub_sptr fatSubStream;   ///< On-disk stream storing the possibly encrypted FAT
		stream::seg_sptr fatStream;      ///< Segstream wrapper around decrypted fatSubStream

		/// Dummy function - does nothing
		/**
		 * Since this format's FAT is a constant size it will never be changed,
		 * but fatStream requires a truncate callback when commit() is called.
		 */
		void truncateFAT(stream::pos newSize);
};

} // namespace gamearchive
} // namespace camoto

#endif // _CAMOTO_FMT_DAT_GOT_HPP_
