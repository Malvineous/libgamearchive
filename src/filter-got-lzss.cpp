/**
 * @file  filter-got-lzss.cpp
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

#include <camoto/stream_filtered.hpp>
#include <camoto/util.hpp> // std::make_unique
#include "filter-got-lzss.hpp"

namespace camoto {
namespace gamearchive {

#define GOT_DICT_SIZE 4096

#define ADD_DICT(c) \
	this->dictionary[this->dictPos] = c; \
	this->dictPos = (this->dictPos + 1) % GOT_DICT_SIZE;

void filter_got_unlzss::reset(stream::len lenInput)
{
	this->flags = 0;
	this->blocksLeft = 0;
	this->state = S0_READ_LEN;
	this->lzssLength = 0;
	this->dictionary.reset(new uint8_t[GOT_DICT_SIZE]);
	this->dictPos = 0;
	this->lenDecomp = 0;
	this->numDecomp = 0;
	return;
}

void filter_got_unlzss::transform(uint8_t *out, stream::len *lenOut,
	const uint8_t *in, stream::len *lenIn)
{
	stream::len r = 0, w = 0;

	// While there's more space to write, and either more data to read or
	// more data to write
	while (              // while there is...
		(w < *lenOut)      // more space to write into, and
		&& (
			(r < *lenIn)     // more data to read, or
			|| (this->lzssLength) // more data to write, and
		) && (
			(this->lenDecomp == 0) || // we haven't read the header yet, or
			(this->numDecomp < this->lenDecomp) // we have read it, and there's more data to decompress
		)
	) {
		bool needMoreData = false;

		switch (this->state) {
			case S0_READ_LEN:
				if (*lenIn - r < 4) {
					needMoreData = true;
					break;
				}
				this->lenDecomp = *in++;
				this->lenDecomp |= *in++ << 8;
				r += 2;
				this->state = S1_READ_FLAGS;
				// Skip the other two bytes
				in += 2;
				r += 2;
				break;

			case S1_READ_FLAGS:
				if (this->blocksLeft == 0) {
					// Read the next lot of flags
					this->flags = *in++;
					r++;
					this->blocksLeft = 8;
				}
				if (this->flags & 1) {
					this->state = S2_LITERAL;
				} else {
					this->state = S3_GET_OFFSET;
				}
				this->flags >>= 1;
				this->blocksLeft--;
				break;

			case S2_LITERAL:
				ADD_DICT(*in);
				*out++ = *in++;
				r++;
				w++;
				this->numDecomp++;
				this->state = S1_READ_FLAGS;
				break;

			case S3_GET_OFFSET: {
				if (*lenIn - r < 2) {
					needMoreData = true;
					break;
				}
				// Now we have at least two bytes to read
				unsigned int code = *in++;
				code |= *in++ << 8;
				r += 2;
				this->lzssLength = (code >> 12) + 2;
				this->lzssDictPos = (GOT_DICT_SIZE + this->dictPos - (code & 0x0FFF)) % GOT_DICT_SIZE;
				this->state = S4_COPY_OFFSET;
				break;
			}

			case S4_COPY_OFFSET:
				// Put this first in case we ever get an offset of zero
				if (this->lzssLength == 0) {
					this->state = S1_READ_FLAGS;
					break;
				}

				*out = this->dictionary[this->lzssDictPos++];
				ADD_DICT(*out);
				out++;
				w++;
				this->numDecomp++;
				this->lzssDictPos %= GOT_DICT_SIZE;
				this->lzssLength--;
				break;

		}
		if (needMoreData) break;
	}

	*lenIn = r;
	*lenOut = w;
	return;
}


void filter_got_lzss::reset(stream::len lenInput)
{
	if (lenInput > 65535) throw stream::error(
		"God of Thunder compression only supports files less than 64kB in size.");
	this->lenInput = lenInput;
	this->count = 0;
	this->state = S0_START;
	return;
}

void filter_got_lzss::transform(uint8_t *out, stream::len *lenOut,
	const uint8_t *in, stream::len *lenIn)
{
	stream::len r = 0, w = 0;

	while (              // while there is...
		(w < *lenOut)      // more space to write into, and
		&& (r < *lenIn)    // more data to read, or
	) {
		switch (this->state) {

			case S0_START:
				assert(*lenOut > 4);
				*out++ = this->lenInput & 0xFF;
				*out++ = (this->lenInput >> 8) & 0xFF;
				*out++ = 0x01;
				*out++ = 0x00;
				w += 4;
				this->state = S1_CODE;
				break;

			case S1_CODE:
				*out++ = 0xFF;
				w++;
				this->state = S2_DATA;
				this->count = 8;
				break;

			case S2_DATA:
				*out++ = *in++;
				r++;
				w++;
				this->count--;
				if (this->count == 0) this->state = S1_CODE;
				break;
		}
	}

	*lenIn = r;
	*lenOut = w;
	return;
}


FilterType_DAT_GOT::FilterType_DAT_GOT()
{
}

FilterType_DAT_GOT::~FilterType_DAT_GOT()
{
}

std::string FilterType_DAT_GOT::code() const
{
	return "lzss-got";
}

std::string FilterType_DAT_GOT::friendlyName() const
{
	return "God of Thunder compression";
}

std::vector<std::string> FilterType_DAT_GOT::games() const
{
	std::vector<std::string> vcGames;
	vcGames.push_back("God of Thunder");
	return vcGames;
}

std::unique_ptr<stream::inout> FilterType_DAT_GOT::apply(
	std::unique_ptr<stream::inout> target, stream::fn_notify_prefiltered_size resize)
	const
{
	return std::make_unique<stream::filtered>(
		std::move(target),
		std::make_shared<filter_got_unlzss>(),
		std::make_shared<filter_got_lzss>(),
		resize
	);
}

std::unique_ptr<stream::input> FilterType_DAT_GOT::apply(
	std::unique_ptr<stream::input> target) const
{
	return std::make_unique<stream::input_filtered>(
		std::move(target),
		std::make_shared<filter_got_unlzss>()
	);
}

std::unique_ptr<stream::output> FilterType_DAT_GOT::apply(
	std::unique_ptr<stream::output> target, stream::fn_notify_prefiltered_size resize)
	const
{
	return std::make_unique<stream::output_filtered>(
		std::move(target),
		std::make_shared<filter_got_lzss>(),
		resize
	);
}


} // namespace gamearchive
} // namespace camoto
