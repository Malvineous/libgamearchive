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
#include <boost/iostreams/stream.hpp>
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

// nullPadded will pad a string with nulls when writing it to a stream, e.g.
//   file << nullPadded("hello", 10);  // write 10 bytes, "hello" and five nulls
// It is an error for the string ("hello") to be longer than the pad (10),
// this will cause an assertion failure at runtime.
// It can also be used when reading from a stream, e.g.
//   file >> nullPadded(str, 10);  // read 10 bytes, store until first null
// In this case 10 bytes will always be read, but they will only be stored in
// str until the first null is encountered.  If there are no null bytes then
// the final string will be the full 10 chars (std::string will provide the
// null termination in this case.)


struct null_padded_read {
	null_padded_read(std::string& r, std::streamsize len, bool chop);
	void read(std::istream& s) const;

	private:
		std::string& r;
		std::streamsize len;
		bool chop;
};

struct null_padded_write {
	null_padded_write(const std::string& r, std::streamsize len);
	void write(std::ostream& s) const;

	private:
		const std::string& r;
		std::streamsize len;
};

struct null_padded_const: public null_padded_write {
	null_padded_const(const std::string& r, std::streamsize len);
};

struct null_padded: public null_padded_read, public null_padded_write {
	null_padded(std::string& r, std::streamsize len, bool chop);
};

// If you get an error related to the next line (e.g. no match for operator >>)
// it's because you're trying to read a value into a const variable.
inline std::istream& operator >> (std::istream& s, const null_padded_read& n) {
	n.read(s);
	return s;
}

inline std::ostream& operator << (std::ostream& s, const null_padded_write& n) {
	n.write(s);
	return s;
}

inline null_padded nullPadded(std::string& r, int len)
{
	return null_padded(r, len, true);
}
inline null_padded_const nullPadded(const std::string& r, int len)
{
	return null_padded_const(r, len);
}
inline null_padded fixedLength(std::string& r, int len)
{
	return null_padded(r, len, false);
}

// Convenience operators to allow iostream_sptr variables to be used with
// stream operators the same way normal std::iostream variables are.
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

// uint8_t / byte iostream operators

struct number_format_u8: public number_format_read, public number_format_write {
	number_format_u8(uint8_t& r);
	void read(std::istream& s) const;
	void write(std::ostream& s) const;

	private:
		uint8_t& r;
};

struct number_format_const_u8: public number_format_write {
	number_format_const_u8(const uint8_t& r);
	void write(std::ostream& s) const;

	private:
		const uint8_t& r;
};

inline number_format_u8 u8(uint8_t& r)
{
	return number_format_u8(r);
}
inline number_format_const_u8 u8(const uint8_t& r)
{
	return number_format_const_u8(r);
}

} // namespace gamearchive
} // namespace camoto

#endif // _CAMOTO_IOSTREAM_HELPERS_HPP_
