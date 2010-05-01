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

#define BYTEORDER_USE_IOSTREAMS
#include "byteorder.h"

namespace camoto {
namespace gamearchive {

namespace io = boost::iostreams;

void streamMove(std::iostream& ps, io::stream_offset offFrom,
	io::stream_offset offTo, io::stream_offset szLength);

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

// zeroPad will pad a string with nulls when writing it to a stream, e.g.
//   file << zeroPad("hello", 10);  // write 10 bytes, "hello" and five nulls
// It is an error for the string ("hello") to be longer than the pad (10),
// this will cause an assertion failure at runtime.
struct zeroPad {
	const std::string& data;
	int len;
	zeroPad(const std::string& data, int len) :
		data(data),
		len(len)
	{
	}
};

std::ostream& operator << (std::ostream& s, const zeroPad& n);

struct fixedLength {
	std::string& data;
	int len;
	fixedLength(std::string& data, int len) :
		data(data),
		len(len)
	{
	}
};

std::istream& operator >> (std::istream& s, const fixedLength& n);

template <class T, typename I>
inline boost::shared_ptr<T>& operator << (boost::shared_ptr<T>& s, const I& n) {
	(*(s.get())) << n;
	return s;
}

template <class T, typename I>
inline boost::shared_ptr<T>& operator >> (boost::shared_ptr<T>& s, const I& n) {
	(*(s.get())) >> n;
	return s;
}

} // namespace gamearchive
} // namespace camoto

#endif // _CAMOTO_IOSTREAM_HELPERS_HPP_
