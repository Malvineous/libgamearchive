/**
 * @file  filter-skyroads.hpp
 * @brief Filter implementation for SkyRoads LZS compression.
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

#ifndef _CAMOTO_FILTER_SKYROADS_LZS_HPP_
#define _CAMOTO_FILTER_SKYROADS_LZS_HPP_

#include <camoto/bitstream.hpp>
#include <boost/shared_array.hpp>
#include <camoto/filter.hpp>
#include <camoto/gamearchive/filtertype.hpp>

namespace camoto {
namespace gamearchive {

class filter_skyroads_unlzs: virtual public filter
{
	public:
		filter_skyroads_unlzs();

		virtual void reset(stream::len lenInput);
		virtual void transform(uint8_t *out, stream::len *lenOut,
			const uint8_t *in, stream::len *lenIn);

	protected:
		bitstream data;

		unsigned int width1, width2, width3;
		unsigned int dist;
		unsigned int lzsDictPos;
		unsigned int lzsLength;
		boost::shared_array<uint8_t> dictionary;
		unsigned int dictPos;
		enum {
			S0_READ_LEN,     ///< Read the header
			S1_READ_FLAG1,   ///< Read the first code/flag
			S2_READ_FLAG2,   ///< Read the second code/flag
			S3_DECOMP_SHORT, ///< Short-length decompression
			S4_DECOMP_LONG,  ///< Long-length decompression
			S5_COPY_BYTE,    ///< Copy a literal byte
			S6_GET_COUNT,    ///< Read the LZS length data
			S7_COPY_OFFSET,  ///< Copy data from the dictionary
		} state;
};

class filter_skyroads_lzs: virtual public filter
{
	public:
		filter_skyroads_lzs();

		virtual void reset(stream::len lenInput);
		virtual void transform(uint8_t *out, stream::len *lenOut,
			const uint8_t *in, stream::len *lenIn);

	protected:
		bitstream data;
};

/// SkyRoads decompression filter.
class FilterType_SkyRoads: virtual public FilterType
{
	public:
		FilterType_SkyRoads();
		~FilterType_SkyRoads();

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

#endif // _CAMOTO_FILTER_SKYROADS_LZS_HPP_
