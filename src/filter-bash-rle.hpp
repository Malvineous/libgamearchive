/**
 * @file   filter-bash-rle.hpp
 * @brief  Filter for packing and unpacking data using the RLE
 *         method employed by Monster Bash.
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

#ifndef _CAMOTO_FILTER_BASH_RLE_HPP_
#define _CAMOTO_FILTER_BASH_RLE_HPP_

#include <camoto/filter.hpp>

namespace camoto {
namespace gamearchive {

class filter_bash_unrle: virtual public filter
{
	public:
		virtual void reset(stream::len lenInput);
		virtual void transform(uint8_t *out, stream::len *lenOut,
			const uint8_t *in, stream::len *lenIn);

	protected:
		uint8_t prev; ///< Previous byte read
		int count; ///< How many times to repeat prev
};

class filter_bash_rle: virtual public filter
{
	public:
		virtual void reset(stream::len lenInput);
		virtual void transform(uint8_t *out, stream::len *lenOut,
			const uint8_t *in, stream::len *lenIn);

	protected:
		int prev;  ///< Previous byte read
		int count; ///< How many times to repeat prev
		/// Current state
		enum {
			S0_NORMAL,               ///< Normal input processing
			S1_MUST_WRITE_RLE_EVENT, ///< Have to write 0x90
			S2_WROTE_0x90,           ///< Wrote 0x90, have to write count
			S3_ESCAPE_0x90,          ///< Wrote 0x90 as data byte, have to escape
			S4_REPEAT_PREV,          ///< Repeat %prev %count times, it's not enough to make an RLE event more efficient
		} state, prevState;
};

} // namespace gamearchive
} // namespace camoto

#endif // _CAMOTO_FILTER_BASH_RLE_HPP_
