/**
 * @file   filter-bash-rle.cpp
 * @brief  Filter implementation for Monster Bash RLE compression.
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

#include "filter-bash-rle.hpp"

namespace camoto {
namespace gamearchive {

filter_bash_unrle::filter_bash_unrle()
	:	count(0)
{
}

void filter_bash_unrle::transform(uint8_t *out, stream::len *lenOut,
	const uint8_t *in, stream::len *lenIn)
{
	stream::len r = 0, w = 0;

	// While there's more space to write, and either more data to read or
	// more data to write
	while (              // while there is...
		(w < *lenOut)      // more space to write into, and
		&& (
			(r < *lenIn)     // more data to read, or
			|| (this->count) // more data to write
		)
	) {
		// If there is an RLE decode in progress
		if (this->count) {
			*out++ = this->prev;
			this->count--;
			w++;
		} else {
			// Otherwise no RLE decode in progress, keep reading
			if (*in == 0x90) { // RLE trigger byte
				if (r + 2 > *lenIn) {
					// Not enough to read this byte this time
					if (r == 0) {
						// Haven't read anything yet, this is it
						throw filter_error("Data ended on RLE code byte before giving a count!");
					}
					break;
				}
				in++;
				r++;
				this->count = *in++;
				r++;
				if (this->count == 0) {
					// Count of zero means a single 0x90 char
					this->prev = 0x90;
					// We could set count to 1 here and let the loop
					// take care of it, but this is quicker and we
					// wouldn't be here unless there was at least one
					// more byte of space in the output buffer.
					*out++ = 0x90;
					w++;
				} else this->count--; // byte we already wrote before the 0x90 is included in count
			} else { // normal byte
				this->prev = *in;
				*out++ = *in++;
				r++;
				w++;
			}
		}
	}

	*lenIn = r;
	*lenOut = w;
	return;
}


filter_bash_rle::filter_bash_rle()
	:	prev(-1),
		count(0),
		state(S0_NORMAL)
{
}

void filter_bash_rle::transform(uint8_t *out, stream::len *lenOut,
	const uint8_t *in, stream::len *lenIn)
{
	stream::len r = 0, w = 0;

	// While there's more space to write, and either more data to read or
	// more data to write
	while (              // while there is...
		(w < *lenOut)      // more space to write into, and
		&& (
			(r < *lenIn)     // more data to read, or
			|| (this->count) // more data to write
			|| (this->state != S0_NORMAL)
		)
	) {
		switch (this->state) {
			case S0_NORMAL:
				if ((*lenIn == 0) || (r >= *lenIn)) { // No more source data
					if (this->count) {
						// But still some RLE stuff to output
						this->state = S1_MUST_WRITE_RLE_EVENT;
					}
					break;
				}
				if (*in == this->prev) {
					in++;
					r++;
					this->count++;
				} else {
					// byte changed
					if (this->count) {
						// Some queued up RLE data to write
						this->state = S1_MUST_WRITE_RLE_EVENT;
						break;
					} else {
						// No RLE data queued, write out the new byte
						this->prev = *in;
						*out++ = *in++;
						r++;
						w++;
						if (this->prev == 0x90) {
							// Have to escape this byte
							this->prevState = this->state;
							this->state = S3_ESCAPE_0x90;
							break;
						}
					}
				}
				break;
			case S1_MUST_WRITE_RLE_EVENT:
				if (this->count > 2) {
					*out++ = 0x90;
					w++;
					this->state = S2_WROTE_0x90;
				} else {
					// Didn't get enough bytes to make an RLE even worthwhile
					this->state = S4_REPEAT_PREV;
				}
				break;
			case S2_WROTE_0x90:
				if (this->count > 254) {
					*out++ = 255;
					this->count -= 254; // take one more because one of the output chars will count as the input in the next iteration
					this->state = S1_MUST_WRITE_RLE_EVENT; // more data to write
				} else {
					*out++ = (char)this->count + 1; // count includes byte already written
					this->count = 0;
					this->state = S0_NORMAL;
				}
				w++;
				break;
			case S3_ESCAPE_0x90:
				*out++ = 0x00;  // zero RLE repeats escapes the control char
				w++;
				this->state = this->prevState;
				break;
			case S4_REPEAT_PREV:
				*out++ = this->prev;
				w++;
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

	*lenIn = r;
	*lenOut = w;
	return;
}


} // namespace gamearchive
} // namespace camoto
