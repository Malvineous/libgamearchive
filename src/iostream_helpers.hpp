/*
 * iostream_helpers.hpp - Functions to get variables in and out of a stream.
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

#ifndef _CAMOTO_IOSTREAM_HELPERS_HPP_
#define _CAMOTO_IOSTREAM_HELPERS_HPP_

#include <iostream>
#include <exception>
#include <string.h>

#include <camoto/types.hpp>
#include "byteorder.h"

namespace camoto {
namespace gamearchive {

namespace io = boost::iostreams;

void streamMove(std::iostream& ps, io::stream_offset offFrom,
	io::stream_offset offTo, io::stream_offset szLength);

void writeZeroPaddedString(iostream_sptr out, const std::string& data, int len);
inline uint32_t read_u16le(iostream_sptr ps) { uint16_t x = 0; ps->read((char *)&x, 2); return le16toh(x); }
inline uint32_t read_u32le(iostream_sptr ps) { uint32_t x = 0; ps->read((char *)&x, 4); return le32toh(x); }
inline uint32_t read_u64le(iostream_sptr ps) { uint64_t x = 0; ps->read((char *)&x, 8); return le64toh(x); }
inline void write_u16le(iostream_sptr ps, uint16_t x) { x = htole16(x); ps->write((char *)&x, 2); return; }
inline void write_u32le(iostream_sptr ps, uint32_t x) { x = htole32(x); ps->write((char *)&x, 4); return; }
inline void write_u64le(iostream_sptr ps, uint64_t x) { x = htole64(x); ps->write((char *)&x, 8); return; }

inline uint32_t read_u16be(iostream_sptr ps) { uint16_t x = 0; ps->read((char *)&x, 2); return be16toh(x); }
inline uint32_t read_u32be(iostream_sptr ps) { uint32_t x = 0; ps->read((char *)&x, 4); return be32toh(x); }
inline uint32_t read_u64be(iostream_sptr ps) { uint64_t x = 0; ps->read((char *)&x, 8); return be64toh(x); }
inline void write_u16be(iostream_sptr ps, uint16_t x) { x = htobe16(x); ps->write((char *)&x, 2); return; }
inline void write_u32be(iostream_sptr ps, uint32_t x) { x = htobe32(x); ps->write((char *)&x, 4); return; }
inline void write_u64be(iostream_sptr ps, uint64_t x) { x = htobe64(x); ps->write((char *)&x, 8); return; }

inline uint32_t u32le_from_buf(const uint8_t *pbuf)
{
	return le32toh(*((uint32_t *)pbuf));
}

inline std::string string_from_buf(const uint8_t *pbuf, int maxlen)
{
	// Find any terminating NULLs which will shorten the string
	for (int i = 0; i < maxlen; i++) {
		if (!pbuf[i]) {
			maxlen = i;
			break;
		}
	}
	// Create and return the string
	return std::string((char *)pbuf, maxlen);
}

} // namespace gamearchive
} // namespace camoto

#endif // _CAMOTO_IOSTREAM_HELPERS_HPP_
