/**
 * @file  filter-zone66.cpp
 * @brief Filter implementation for decompressing Zone 66 files.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/Zone_66_Compression
 *
 * Copyright (C) 2010-2015 Adam Nielsen <malvineous@shikadi.net>
 * Decompression algorithm by john_doe
 *   <http://forum.xentax.com/memberlist.php?mode=viewprofile&u=1896>
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

#include <boost/bind.hpp>
#include <camoto/stream_filtered.hpp>
#include <camoto/util.hpp> // std::make_unique

#include "filter-zone66.hpp"

namespace camoto {
namespace gamearchive {

filter_z66_decompress::filter_z66_decompress()
	:	data(bitstream::bigEndian)
{
}

filter_z66_decompress::~filter_z66_decompress()
{
}

int filter_z66_decompress::nextChar(const uint8_t **in, stream::len *lenIn, stream::len *r, uint8_t *out)
{
	if (*r < *lenIn) {
		*out = **in; // "read" byte
		(*in)++;     // increment read buffer
		(*r)++;      // increment read count
		return 1;    // return number of bytes read
	}
	return 0; // EOF
}

void filter_z66_decompress::reset(stream::len lenInput)
{
	this->outputLimit = 4; // need to allow enough to read the length field
	this->totalWritten = 0;
	this->state = 0;
	this->codeLength = 9;
	this->curDicIndex = 0;
	this->maxDicIndex = 255;

	for (int i = 0; i < 8192; i++) {
		nodes[i].code = 0;
		nodes[i].nextCode = 0;
	}

	this->data.flushByte(); // drop any pending byte
}

void filter_z66_decompress::transform(uint8_t *out, stream::len *lenOut,
	const uint8_t *in, stream::len *lenIn)
{
	stream::len r = 0, w = 0;

	fn_getnextchar cbNext = boost::bind(&filter_z66_decompress::nextChar, this, &in, lenIn, &r, _1);

	while (
		(w < *lenOut)  // while there is more space to write into
		&& (
			(r + 2 < *lenIn) // and there's at least two more bytes to read
			|| (
				(*lenIn < 10)   // or there's less than 10 bytes to read in this buffer (i.e. near EOF)
				&& (r < *lenIn) // and there's at least one more byte to read
			)
			|| (this->state > 1) // or we're still processing what we read previously
		)
		&& (this->totalWritten < this->outputLimit) // and we haven't reached the target file size yet
	) {
		switch (this->state) {
			case 0: {
				// Read the first four bytes (decompressed size) so we can limit the
				// output size appropriately.
				this->data.changeEndian(bitstream::littleEndian);
				this->data.read(cbNext, 32, &this->outputLimit);
				this->data.changeEndian(bitstream::bigEndian);
				this->state++;
				break;
			}
			case 1: {
				if (this->data.read(cbNext, this->codeLength, &this->code) != this->codeLength) {
					goto done;
				}
				this->curCode = this->code;
				this->state++;
				break;
			}
			case 2:
				if (this->curCode < 256) {
					*out++ = this->curCode;
					w++;
					this->totalWritten++;
					if (!this->stack.empty()) {
						this->curCode = this->stack.top();
						this->stack.pop();
					} else {
						this->state++;
						break;
					}
				} else {
					this->curCode -= 256;
					this->stack.push(this->nodes[this->curCode].nextCode);
					this->curCode = this->nodes[this->curCode].code;
					if (this->stack.size() > 65534) {
						throw filter_error("Corrupted Zone 66 data - token stack > 64k");
					}
				}
				break;
			case 3: {
				unsigned int value;
				if (this->data.read(cbNext, 8, &value) != 8) goto done;
				*out++ = value;
				w++;
				this->totalWritten++;

				if (this->code >= 0x100u + this->curDicIndex) {
					// This code hasn't been put in the dictionary yet (tpal.z66)
					this->code = 0x100;
				}
				nodes[this->curDicIndex].code = this->code;
				nodes[this->curDicIndex].nextCode = value;
				this->curDicIndex++;

				if (this->curDicIndex >= this->maxDicIndex) {
					this->codeLength++;
					if (this->codeLength == 13) {
						this->codeLength = 9;
						this->curDicIndex = 64;
						this->maxDicIndex = 255;
					} else {
						this->maxDicIndex = (1 << this->codeLength) - 257;
					}
				}
				this->state = 1;
				break;
			} // case 4
		} // switch(state)
	} // while (more data to be read)

done:
	*lenIn = r;
	*lenOut = w;
	return;
}


filter_z66_compress::filter_z66_compress()
	:	data(bitstream::bigEndian)
{
}

filter_z66_compress::~filter_z66_compress()
{
}

int filter_z66_compress::putChar(uint8_t **out, const stream::len *lenOut, stream::len *w, uint8_t in)
{
	if (*w < *lenOut) {
		**out = in; // "write" byte
		(*out)++;     // increment write buffer
		(*w)++;      // increment write count
		return 1;    // return number of bytes written
	}
	return 0; // EOF
}

void filter_z66_compress::reset(stream::len lenInput)
{
	this->outputLimit = lenInput;
	this->state = 0;
	this->codeLength = 9;
	this->curDicIndex = 0;
	this->maxDicIndex = 255;

	this->data.flushByte(); // drop any pending byte
}

void filter_z66_compress::transform(uint8_t *out, stream::len *lenOut,
	const uint8_t *in, stream::len *lenIn)
{
	stream::len r = 0, w = 0;

	fn_putnextchar cbNext = boost::bind(&filter_z66_compress::putChar, this, &out, lenOut, &w, _1);

	if (*lenIn == 0) {
		// No more data to read, so flush
		this->data.flushByte(cbNext);
	}

	while (
		(w + 2 < *lenOut)  // while there is more space to write into
		&& (r < *lenIn) // and there's at least one more byte to read
	) {
		switch (this->state) {
			case 0:
				// Read the first four bytes (decompressed size) so we can limit the
				// output size appropriately.
				this->data.changeEndian(bitstream::littleEndian);
				this->data.write(cbNext, 32, this->outputLimit);
				this->data.changeEndian(bitstream::bigEndian);
				this->state++;
				break;
			case 1:
				this->data.write(cbNext, this->codeLength, *in++);
				r++;
				this->state++;
				break;
			case 2:
				this->data.write(cbNext, 8, *in++);
				r++;

				this->curDicIndex++;
				if (this->curDicIndex >= this->maxDicIndex) {
					this->codeLength++;
					if (this->codeLength == 13) {
						this->codeLength = 9;
						this->curDicIndex = 64;
						this->maxDicIndex = 255;
					} else {
						this->maxDicIndex = (1 << this->codeLength) - 257;
					}
				}
				this->state = 1;
				break;
		} // switch(state)
	} // while (more data to be read)

	*lenIn = r;
	*lenOut = w;
	return;
}

FilterType_Zone66::FilterType_Zone66()
{
}

FilterType_Zone66::~FilterType_Zone66()
{
}

std::string FilterType_Zone66::code() const
{
	return "lzw-zone66";
}

std::string FilterType_Zone66::friendlyName() const
{
	return "Zone 66 compression";
}

std::vector<std::string> FilterType_Zone66::games() const
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Zone 66");
	return vcGames;
}

std::unique_ptr<stream::inout> FilterType_Zone66::apply(
	std::unique_ptr<stream::inout> target, stream::fn_notify_prefiltered_size resize)
	const
{
	return std::make_unique<stream::filtered>(
		std::move(target),
		std::make_shared<filter_z66_decompress>(),
		std::make_shared<filter_z66_compress>(),
		resize
	);
}

std::unique_ptr<stream::input> FilterType_Zone66::apply(
	std::unique_ptr<stream::input> target) const
{
	return std::make_unique<stream::input_filtered>(
		std::move(target),
		std::make_shared<filter_z66_decompress>()
	);
}

std::unique_ptr<stream::output> FilterType_Zone66::apply(
	std::unique_ptr<stream::output> target, stream::fn_notify_prefiltered_size resize)
	const
{
	return std::make_unique<stream::output_filtered>(
		std::move(target),
		std::make_shared<filter_z66_compress>(),
		resize
	);
}

} // namespace gamearchive
} // namespace camoto
