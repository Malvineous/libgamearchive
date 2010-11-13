/**
 * @file   filter-zone66.cpp
 * @brief  Filter implementation for decompressing Zone 66 files.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/Zone_66_Compression
 *
 * Copyright (C) 2010 Adam Nielsen <malvineous@shikadi.net>
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
#include <camoto/iostream_helpers.hpp>
#include <camoto/filteredstream.hpp>
#include <camoto/bitstream.hpp>

#include "filter-zone66.hpp"

namespace camoto {
namespace gamearchive {

/// Read the next character from the Source, and return a success/error code.
template <typename Source>
int nextSourceChar(Source& src, uint8_t *out) {
	int c = boost::iostreams::get(src);
	if (c < 0) return c;
	*out = c;
	return 1;
}

class z66_decompress_filter: public io::multichar_input_filter {

	protected:
		bitstream data;
		int state;

		int code, curCode;

		std::stack<int> stack;
		int codeLength, curDicIndex, maxDicIndex;

		struct {
			int code, nextCode;
		} nodes[8192];

	public:

		z66_decompress_filter()
			: data(bitstream::bigEndian),
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

		template<typename Source>
		std::streamsize read(Source& src, char* s, std::streamsize n)
		{
			fn_getnextchar cbNext = boost::bind(
				nextSourceChar<Source>, boost::ref(src), _1
			);
			int r = 0;  // number of bytes "read" (written into *s)

			while (r < n) {
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
							s[r++] = this->curCode;
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
								throw ECorruptedData("Corrupted Zone 66 data - token stack > 64k");
							}
						}
						break;
					case 3: {
						int value;
						if (this->data.read(cbNext, 8, &value) != 8) goto done;
						s[r++] = value;

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
			if (r == 0) return EOF;
			return r;
		}

};

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

iostream_sptr Zone66FilterType::apply(iostream_sptr target, FN_TRUNCATE fnTruncate)
	throw (ECorruptedData)
{
	filtering_istream_sptr pinf(new io::filtering_istream());

	// Decompress the data using the Zone 66 algorithm
	pinf->push(z66_decompress_filter());

	filtering_ostream_sptr poutf(new io::filtering_ostream());
	//poutf->push(z66_compress_filter());

	iostream_sptr dec(new filteredstream(target, pinf, poutf));
	return dec;
}

istream_sptr Zone66FilterType::apply(istream_sptr target)
	throw (ECorruptedData)
{
	filtering_istream_sptr pinf(new io::filtering_istream());

	// Decompress the data using the Zone 66 algorithm
	pinf->push(z66_decompress_filter());

	pinf->push(*target);
	return pinf;
}

ostream_sptr Zone66FilterType::apply(ostream_sptr target, FN_TRUNCATE fnTruncate)
	throw (ECorruptedData)
{
	filtering_ostream_sptr poutf(new io::filtering_ostream());
	//poutf->push(z66_compress_filter());

	poutf->push(*target);
	return poutf;
}

} // namespace gamearchive
} // namespace camoto
