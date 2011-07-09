/**
 * @file   filter-bitswap.hpp
 * @brief  Filter implementation for swapping the bits in each byte.
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

#ifndef _CAMOTO_FILTER_BITSWAP_HPP_
#define _CAMOTO_FILTER_BITSWAP_HPP_

#include <boost/iostreams/concepts.hpp>     // multichar_dual_use_filter
#include <boost/iostreams/operations.hpp>   // read
#include <camoto/types.hpp>
#include <camoto/gamearchive/filtertype.hpp>

namespace camoto {
namespace gamearchive {

namespace io = boost::iostreams;

/// Encrypt a stream by swapping all the bits in each byte.
class bitswap_filter: public io::multichar_filter<io::seekable> {

	public:

		struct category: io::multichar_seekable_filter_tag, io::flushable_tag { };

		template<typename Source>
		std::streamsize read(Source& src, char *s, std::streamsize n)
		{
			std::streamsize r = io::read(src, s, n);
			if (r == 0) return EOF;
			if (r < 0) return r;
			for (int i = 0; i < r; i++) {
				*s =
					((*s >> 7) & 1) |
					((*s >> 5) & 2) |
					((*s >> 3) & 4) |
					((*s >> 1) & 8) |
					((*s << 1) & 16) |
					((*s << 3) & 32) |
					((*s << 5) & 64) |
					((*s << 7) & 128)
				;
				s++;
			}
			return r;
		}

		template<typename Sink>
		std::streamsize write(Sink& dst, const char *s, std::streamsize n)
		{
			int r = 0;
			while (n--) {
				uint8_t c =
					((*s >> 7) & 1) |
					((*s >> 5) & 2) |
					((*s >> 3) & 4) |
					((*s >> 1) & 8) |
					((*s << 1) & 16) |
					((*s << 3) & 32) |
					((*s << 5) & 64) |
					((*s << 7) & 128)
				;
				if (!io::put(dst, *s++)) break;
				r++;
			}
			return r;
		}

		template<typename Source>
		io::stream_offset seek(Source& src, io::stream_offset off, std::ios::seekdir way)
		{
			return io::seek(src, off, way);
		}

		template<typename Sink>
		bool flush(Sink& snk)
		{
			return true; // nothing to flush
		}

};

} // namespace gamearchive
} // namespace camoto

#endif // _CAMOTO_FILTER_BITSWAP_HPP_
