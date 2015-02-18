/**
 * @file  filter-zone66.hpp
 * @brief Filter implementation for decompressing Zone 66 files.
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

#ifndef _CAMOTO_FILTER_ZONE66_HPP_
#define _CAMOTO_FILTER_ZONE66_HPP_

#include <stack>
#include <camoto/stream.hpp>
#include <camoto/bitstream.hpp>
#include <camoto/gamearchive/filtertype.hpp>

namespace camoto {
namespace gamearchive {

/// Zone 66 decompression filter
class filter_z66_decompress: virtual public filter
{
	public:
		filter_z66_decompress();
		virtual ~filter_z66_decompress();

		int nextChar(const uint8_t **in, stream::len *lenIn, stream::len *r,
			uint8_t *out);

		virtual void reset(stream::len lenInput);
		virtual void transform(uint8_t *out, stream::len *lenOut,
			const uint8_t *in, stream::len *lenIn);

	protected:
		bitstream data;
		int state;

		unsigned int code, curCode;

		std::stack<int> stack;
		int codeLength, curDicIndex, maxDicIndex;

		struct {
			unsigned int code, nextCode;
		} nodes[8192];

		unsigned int totalWritten; ///< Number of bytes written out so far overall
		unsigned int outputLimit;  ///< Maximum number of bytes to write out overall
};

/// Zone 66 compression filter
/**
 * This is a "fake" compression filter, in that it does not actually compress
 * the data, it just writes it out in such a way that when the game tries to
 * decompress it, it will recover the original data.  This also means the
 * "compressed" data will always be larger than the original data.
 */
class filter_z66_compress: virtual public filter
{
	public:
		filter_z66_compress();
		virtual ~filter_z66_compress();

		int putChar(uint8_t **out, const stream::len *lenOut, stream::len *w,
			uint8_t in);

		virtual void reset(stream::len lenInput);
		virtual void transform(uint8_t *out, stream::len *lenOut,
			const uint8_t *in, stream::len *lenIn);

	protected:
		bitstream data;
		int state;
		int codeLength, curDicIndex, maxDicIndex;
		unsigned int outputLimit;  ///< Maximum number of bytes to write out overall
};

/// Zone 66 compression handler
class FilterType_Zone66: virtual public FilterType
{
	public:
		FilterType_Zone66();
		virtual ~FilterType_Zone66();

		virtual std::string code() const;
		virtual std::string friendlyName() const;
		virtual std::vector<std::string> games() const;
		virtual std::unique_ptr<stream::inout> apply(
			std::unique_ptr<stream::inout> target, stream::fn_notify_prefiltered_size resize)
			const;
		virtual std::unique_ptr<stream::input> apply(
			std::unique_ptr<stream::input> target) const;
		virtual std::unique_ptr<stream::output> apply(
			std::unique_ptr<stream::output> target, stream::fn_notify_prefiltered_size resize)
			const;
};

} // namespace gamearchive
} // namespace camoto

#endif // _CAMOTO_FILTER_ZONE66_HPP_
