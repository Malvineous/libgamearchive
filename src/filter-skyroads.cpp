/**
 * @file  filter-skyroads.cpp
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

#include <iostream>
#include <functional>
#include <camoto/stream_filtered.hpp>
#include <camoto/util.hpp> // std::make_unique
#include "filter-skyroads.hpp"

namespace camoto {
namespace gamearchive {

#define SKYROADS_DICT_SIZE 4096

#define ADD_DICT(c) \
	this->dictionary[this->dictPos] = c; \
	this->dictPos = (this->dictPos + 1) % SKYROADS_DICT_SIZE;

filter_skyroads_unlzs::filter_skyroads_unlzs()
	:	data(bitstream::bigEndian)
{
}

void filter_skyroads_unlzs::reset(stream::len lenInput)
{
	this->state = S0_READ_LEN;
	this->lzsLength = 0;
	this->dictionary.reset(new uint8_t[SKYROADS_DICT_SIZE]);
	this->dictPos = 0;
	return;
}

void filter_skyroads_unlzs::transform(uint8_t *out, stream::len *lenOut,
	const uint8_t *in, stream::len *lenIn)
{
	stream::len r = 0, w = 0;
	fn_getnextchar cbNext = std::bind(bitstreamFilterNextChar, &in, lenIn, &r,
		std::placeholders::_1);

	// While there's more space to write, and either more data to read or
	// more data to write
	while (              // while there is...
		(w < *lenOut)      // more space to write into, and
		&& (
			(r < *lenIn)     // more data to read, or
			|| (this->lzsLength) // more data to write, and
		)
	) {
		bool needMoreData = false;
		unsigned int bitsRead, code;

		switch (this->state) {
			case S0_READ_LEN:
				if (*lenIn - r < 3) {
					needMoreData = true;
					break;
				}
				this->data.changeEndian(bitstream::littleEndian);
				bitsRead = this->data.read(cbNext, 8, &this->width1);
				bitsRead = this->data.read(cbNext, 8, &this->width2);
				bitsRead = this->data.read(cbNext, 8, &this->width3);
				this->data.changeEndian(bitstream::bigEndian);

				this->state = S1_READ_FLAG1;
				break;

			case S1_READ_FLAG1:
				bitsRead = this->data.read(cbNext, 1, &code);
				if (bitsRead == 0) {
					needMoreData = true;
					break;
				}
				if (code == 0) {
					this->state = S3_DECOMP_SHORT;
				} else {
					this->state = S2_READ_FLAG2;
				}
				break;

			case S2_READ_FLAG2:
				bitsRead = this->data.read(cbNext, 1, &code);
				if (bitsRead == 0) {
					needMoreData = true;
					break;
				}
				if (code == 0) {
					this->state = S4_DECOMP_LONG;
				} else {
					this->state = S5_COPY_BYTE;
				}
				break;

			case S3_DECOMP_SHORT:
				bitsRead = this->data.read(cbNext, this->width2, &code);
				if (bitsRead != this->width2) {
					needMoreData = true;
					break;
				}
				this->dist = 2 + code;
				this->state = S6_GET_COUNT;
				break;

			case S4_DECOMP_LONG:
				bitsRead = this->data.read(cbNext, this->width3, &code);
				if (bitsRead != this->width3) {
					needMoreData = true;
					break;
				}
				this->dist = 2 + (1 << width2) + code;
				this->state = S6_GET_COUNT;
				break;

			case S5_COPY_BYTE:
				bitsRead = this->data.read(cbNext, 8, &code);
				if (bitsRead != 8) {
					needMoreData = true;
					break;
				}
				ADD_DICT(code);
				*out++ = code;
				w++;
				this->state = S1_READ_FLAG1;
				break;

			case S6_GET_COUNT:
				bitsRead = this->data.read(cbNext, this->width1, &code);
				if (bitsRead != this->width1) {
					needMoreData = true;
					break;
				}
				this->lzsLength = 2 + code;

				if (this->lzsLength > SKYROADS_DICT_SIZE) {
					std::cerr << "lzs-skyroads: Length is > dict size, data is probably "
						"corrupt, aborting." << std::endl;
					throw stream::error("SkyRoads compressed data has backreference "
						"larger than dictionary length.  Data is probably corrupt or not "
						"in this compression format.");
				}

				this->state = S7_COPY_OFFSET;
				this->lzsDictPos = (SKYROADS_DICT_SIZE + this->dictPos - this->dist) % SKYROADS_DICT_SIZE;
				break;

			case S7_COPY_OFFSET:
				// Put this first in case we ever get an offset of zero
				if (this->lzsLength == 0) {
					this->state = S1_READ_FLAG1;
					break;
				}

				*out = this->dictionary[this->lzsDictPos++];
				ADD_DICT(*out);
				out++;
				w++;
				this->lzsDictPos %= SKYROADS_DICT_SIZE;
				this->lzsLength--;
				break;

		}
		if (needMoreData) break;
	}

	*lenIn = r;
	*lenOut = w;
	return;
}


filter_skyroads_lzs::filter_skyroads_lzs()
	:	data(bitstream::bigEndian)
{
}

void filter_skyroads_lzs::reset(stream::len lenInput)
{
	return;
}

void filter_skyroads_lzs::transform(uint8_t *out, stream::len *lenOut,
	const uint8_t *in, stream::len *lenIn)
{
	stream::len r = 0, w = 0;
	fn_putnextchar cbNext = std::bind(bitstreamFilterPutChar, &out, lenOut, &w,
		std::placeholders::_1);

	while (              // while there is...
		(w + 2 < *lenOut) // leave some leftover bytes to guarantee the codeword will be written
		&& (r < *lenIn)    // more data to read, or
	) {
		this->data.write(cbNext, 2, 0x03);
		this->data.write(cbNext, 8, *in++);
		r++;
	}

	*lenIn = r;
	*lenOut = w;
	return;
}


FilterType_SkyRoads::FilterType_SkyRoads()
{
}

FilterType_SkyRoads::~FilterType_SkyRoads()
{
}

std::string FilterType_SkyRoads::code() const
{
	return "lzs-skyroads";
}

std::string FilterType_SkyRoads::friendlyName() const
{
	return "SkyRoads compression";
}

std::vector<std::string> FilterType_SkyRoads::games() const
{
	return {
		"SkyRoads",
	};
}

std::unique_ptr<stream::inout> FilterType_SkyRoads::apply(
	std::unique_ptr<stream::inout> target, stream::fn_notify_prefiltered_size resize)
	const
{
	return std::make_unique<stream::filtered>(
		std::move(target),
		std::make_shared<filter_skyroads_unlzs>(),
		std::make_shared<filter_skyroads_lzs>(),
		resize
	);
}

std::unique_ptr<stream::input> FilterType_SkyRoads::apply(
	std::unique_ptr<stream::input> target) const
{
	return std::make_unique<stream::input_filtered>(
		std::move(target),
		std::make_shared<filter_skyroads_unlzs>()
	);
}

std::unique_ptr<stream::output> FilterType_SkyRoads::apply(
	std::unique_ptr<stream::output> target, stream::fn_notify_prefiltered_size resize)
	const
{
	return std::make_unique<stream::output_filtered>(
		std::move(target),
		std::make_shared<filter_skyroads_lzs>(),
		resize
	);
}


} // namespace gamearchive
} // namespace camoto
