/**
 * @file   filter-zone66.cpp
 * @brief  Filter implementation for decompressing Zone 66 files.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/Zone_66_Compression
 *
 * Copyright (C) 2010-2011 Adam Nielsen <malvineous@shikadi.net>
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

#include <stack>
#include <boost/bind.hpp>
#include <camoto/filter.hpp>
#include <camoto/stream_filtered.hpp>
#include <camoto/bitstream.hpp>

#include "filter-zone66.hpp"

namespace camoto {
namespace gamearchive {

filter_z66_decompress::filter_z66_decompress()
	throw () :
	data(bitstream::bigEndian),
	state(0),
	codeLength(9),
	curDicIndex(0),
	maxDicIndex(255)
{
	for (int i = 0; i < 8192; i++) {
		nodes[i].code = 0;
		nodes[i].nextCode = 0;
	}
}

int filter_z66_decompress::nextChar(const uint8_t **in, stream::len *lenIn, stream::len *r, uint8_t *out)
	throw ()
{
	if (*lenIn) {
		*out = **in; // "read" byte
		(*in)++;     // increment read buffer
		(*r)++;      // increment read count
		return 1;    // return number of bytes read
	}
	return 0; // EOF
}

void filter_z66_decompress::transform(uint8_t *out, stream::len *lenOut,
	const uint8_t *in, stream::len *lenIn)
	throw (filter_error)
{
	stream::len r = 0, w = 0;

	fn_getnextchar cbNext = boost::bind(&filter_z66_decompress::nextChar, this, &in, lenIn, &r, _1);

	while ((w < *lenOut) && ((r < *lenIn))) {// || (!this->stack.empty()))) {
		switch (this->state) {
			case 0: {
				// Discard the first four bytes (decompressed size)
				this->data.changeEndian(bitstream::littleEndian);
				int dummy;
				this->data.read(cbNext, 32, &dummy);
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
					if (!stack.empty()) {
						this->curCode = stack.top();
						stack.pop();
					} else {
						this->state++;
						break;
					}
				} else {
					this->curCode -= 256;
					stack.push(this->nodes[this->curCode].nextCode);
					this->curCode = this->nodes[this->curCode].code;
					if (stack.size() > 65534) {
						throw filter_error("Corrupted Zone 66 data - token stack > 64k");
					}
				}
				break;
			case 3: {
				int value;
				if (this->data.read(cbNext, 8, &value) != 8) goto done;
				*out++ = value;
				w++;

				if (this->code >= 0x100 + this->curDicIndex) {
					// This code hasn't been put in the dictionary yet (tpal.z66)
					this->code = 0x100;
				}
				nodes[curDicIndex].code = this->code;
				nodes[curDicIndex].nextCode = value;
				curDicIndex++;

				if (curDicIndex >= maxDicIndex) {
					codeLength++;
					if (codeLength == 13) {
						codeLength = 9;
						curDicIndex = 64;
						maxDicIndex = 255;
					} else {
						maxDicIndex = (1 << codeLength) - 257;
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


Zone66FilterType::Zone66FilterType()
	throw ()
{
}

Zone66FilterType::~Zone66FilterType()
	throw ()
{
}

std::string Zone66FilterType::getFilterCode() const
	throw ()
{
	return "lzw-zone66";
}

std::string Zone66FilterType::getFriendlyName() const
	throw ()
{
	return "Zone 66 compression";
}

std::vector<std::string> Zone66FilterType::getGameList() const
	throw ()
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Zone 66");
	return vcGames;
}

stream::inout_sptr Zone66FilterType::apply(stream::inout_sptr target,
	stream::fn_truncate resize)
	throw (filter_error, stream::read_error)
{
	stream::filtered_sptr st(new stream::filtered());
	filter_sptr de(new filter_z66_decompress());
	/// @todo Implement Zone 66 compression
	filter_sptr en;//(new filter_z66_compress());
	st->open(target, de, en, resize);
	return st;
}

stream::input_sptr Zone66FilterType::apply(stream::input_sptr target)
	throw (filter_error, stream::read_error)
{
	stream::input_filtered_sptr st(new stream::input_filtered());
	filter_sptr de(new filter_z66_decompress());
	st->open(target, de);
	return st;
}

stream::output_sptr Zone66FilterType::apply(stream::output_sptr target,
	stream::fn_truncate resize)
	throw (filter_error)
{
	stream::output_filtered_sptr st(new stream::output_filtered());
	/// @todo Implement Zone 66 compression
	filter_sptr en;//(new filter_z66_compress());
	st->open(target, en, resize);
	return st;
}

} // namespace gamearchive
} // namespace camoto
