/**
 * @file  filter-got-lzss.hpp
 * @brief Filter implementation for God of Thunder LZSS compression.
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

#ifndef _CAMOTO_FILTER_GOT_LZSS_HPP_
#define _CAMOTO_FILTER_GOT_LZSS_HPP_

#include <boost/shared_array.hpp>
#include <camoto/filter.hpp>
#include <camoto/gamearchive/filtertype.hpp>

namespace camoto {
namespace gamearchive {

class filter_got_unlzss: virtual public filter
{
	public:
		virtual void reset(stream::len lenInput);
		virtual void transform(uint8_t *out, stream::len *lenOut,
			const uint8_t *in, stream::len *lenIn);

	protected:
		uint8_t flags; ///< Flags for next eight blocks
		unsigned int blocksLeft; ///< Number of blocks left
		unsigned int lzssDictPos;
		unsigned int lzssLength;
		boost::shared_array<uint8_t> dictionary;
		unsigned int dictPos;
		unsigned int lenDecomp; ///< Target output size
		unsigned int numDecomp; ///< Current output size
		enum {
			S0_READ_LEN,     ///< Read the header
			S1_READ_FLAGS,   ///< Read a flags byte
			S2_LITERAL,      ///< Copy a byte
			S3_GET_OFFSET,   ///< Read the LZSS offset/length data
			S4_COPY_OFFSET,  ///< Copy data from the dictionary
		} state;
};

class filter_got_lzss: virtual public filter
{
	public:
		virtual void reset(stream::len lenInput);
		virtual void transform(uint8_t *out, stream::len *lenOut,
			const uint8_t *in, stream::len *lenIn);

	protected:
		unsigned int lenInput; ///< Decompressed size
		int count; ///< Number of bytes written
		/// Current state
		enum {
			S0_START,  ///< Write header
			S1_CODE,   ///< LZSS code for next eight blocks
			S2_DATA,   ///< Write data
		} state;
};

/// God of Thunder decompression filter.
class FilterType_DAT_GOT: virtual public FilterType
{
	public:
		FilterType_DAT_GOT();
		~FilterType_DAT_GOT();

		virtual std::string code() const;
		virtual std::string friendlyName() const;
		virtual std::vector<std::string> games() const;
		virtual std::unique_ptr<stream::inout> apply(std::shared_ptr<stream::inout> target,
			stream::fn_truncate_filter resize) const;
		virtual std::unique_ptr<stream::input> apply(std::shared_ptr<stream::input> target) const;
		virtual std::unique_ptr<stream::output> apply(std::shared_ptr<stream::output> target,
			stream::fn_truncate_filter resize) const;
};

} // namespace gamearchive
} // namespace camoto

#endif // _CAMOTO_FILTER_GOT_LZSS_HPP_
