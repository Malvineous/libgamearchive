/**
 * @file   filter-bash-rle.hpp
 * @brief  Boost iostream filter for packing and unpacking data using the RLE
 *         method employed by Monster Bash.
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

#ifndef _CAMOTO_FILTER_BASH_RLE_HPP_
#define _CAMOTO_FILTER_BASH_RLE_HPP_

#include <vector>
#include <deque>
#include <iosfwd>                           // streamsize
#include <boost/iostreams/concepts.hpp>     // multichar_input_filter
#include <boost/iostreams/char_traits.hpp>  // EOF, WOULD_BLOCK
#include <boost/iostreams/operations.hpp>   // get
#include <boost/bind.hpp>

#include <camoto/bitstream.hpp>
#include <camoto/types.hpp>

namespace camoto {
namespace gamearchive {

namespace io = boost::iostreams;

class bash_unrle_filter: public io::multichar_input_filter {

	protected:
		char prev; ///< Previous byte read
		int count; ///< How many times to repeat prev

	public:

		bash_unrle_filter() :
			count(0)
		{
		};

		template<typename Source>
		std::streamsize read(Source& src, char* s, std::streamsize n)
		{
			int r = 0;
			while (r < n) {
				// If there is an RLE decode in progress
				if (this->count) {
					*s++ = this->prev;
					this->count--;
					r++;
				} else {
					// Otherwise no RLE decode in progress, keep reading
					int c = boost::iostreams::get(src);
					if (c < 0) {
						if (r == 0) return c; // no bytes read, return error
						break; // bytes read, return those
					} else if (c == 0x90) { // RLE trigger byte
						// Read the next char, c now becomes the count
						// TODO: Handle WOULD_BLOCK being returned here
						this->count = boost::iostreams::get(src);
						if (this->count < 0) {
							this->count = 0; // prevent massive loop if we are called again
							if (r == 0) return this->count; // no bytes read, return error
							break; // bytes read, return those
						} else if (this->count == 0) {
							// Count of zero means a single 0x90 char
							this->prev = 0x90;
							// We could set count to 1 here and let the loop
							// take care of it, but this is quicker and we
							// wouldn't be here unless there was at least one
							// more byte of space in the output buffer.
							*s++ = 0x90;
							r++;
						} else this->count--; // byte we already wrote is included in count
					} else { // normal byte
						*s++ = c;
						r++;
						this->prev = c;
					}
				}
			}
			return r;
		}

};

class bash_rle_filter: public io::multichar_input_filter {

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

	public:

		bash_rle_filter() :
			prev(-1),
			count(0),
			state(S0_NORMAL)
		{
		};

		template<typename Source>
		std::streamsize read(Source& src, char* s, std::streamsize n)
		{
			if (n < 1) return 0; // just in case
			int r = 0;
			while (r < n) {
				switch (this->state) {
					case S0_NORMAL:
						while (r < n) {
							int c = boost::iostreams::get(src);
							if (c < 0) { // No more source data
								if (this->count) {
									// But still some RLE stuff to output
									this->state = S1_MUST_WRITE_RLE_EVENT;
									break;
								} else if (r == 0) return c; // no data read and none to write
								else return r; // some data read
							}
							if (c == this->prev) {//&& (this->count < 255)) {
								this->count++;
							} else {
								if (this->count) {
									// Some queued up RLE data to write
									this->state = S1_MUST_WRITE_RLE_EVENT;
									boost::iostreams::putback(src, c);
									break;
								} else {
									// No RLE data queued, write out the new byte
									*s++ = c;
									r++;
									this->prev = c;
									if (c == 0x90) {
										// Have to escape this byte
										this->prevState = this->state;
										this->state = S3_ESCAPE_0x90;
										break;
									}
								}
							}
						}
						break;
					case S1_MUST_WRITE_RLE_EVENT:
						if (this->count > 2) {
							*s++ = 0x90;
							r++;
							this->state = S2_WROTE_0x90;
						} else {
							// Didn't get enough bytes to make an RLE even worthwhile
							this->state = S4_REPEAT_PREV;
						}
						break;
					case S2_WROTE_0x90:
						if (this->count > 254) {
							*s++ = 255;
							this->count -= 254+1; // take one more because one of the output chars will count as the input in the next iteration
							this->state = S1_MUST_WRITE_RLE_EVENT; // more data to write
						} else {
							*s++ = (char)this->count + 1; // count includes byte already written
							this->count = 0;
							this->state = S0_NORMAL;
						}
						r++;
						break;
					case S3_ESCAPE_0x90:
						*s++ = 0x00;  // zero RLE repeats escapes the control char
						r++;
						this->state = this->prevState;
						break;
					case S4_REPEAT_PREV:
						*s++ = this->prev;
						r++;
						--this->count;
						if (!this->count) this->state = S0_NORMAL; // no more to write
						if (this->prev == 0x90) {
							// Have to escape this byte
							this->prevState = this->state;
							this->state = S3_ESCAPE_0x90;
						}
						break;
				}
			}
			return r;
		}

};

} // namespace gamearchive
} // namespace camoto

#endif // _CAMOTO_FILTER_BASH_RLE_HPP_
