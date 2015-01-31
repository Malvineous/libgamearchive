/**
 * @file  filter-xor-sagent.cpp
 * @brief Filter that encrypts and decrypts Secret Agent data files.
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
#include "filter-xor-sagent.hpp"
#include "filter-bitswap.hpp"

namespace camoto {
namespace gamearchive {

#define SAM_KEY     "Copyright 1991 Peder Jungck"
#define SAM_KEYLEN  (strlen(SAM_KEY)+1)  // include terminating null

const char sam_key[] = SAM_KEY;

filter_sam_crypt::filter_sam_crypt(int resetInterval)
	:	filter_xor_crypt(0, 0),
		resetInterval(resetInterval)
{
}

uint8_t filter_sam_crypt::getKey()
{
	if ((this->resetInterval == 42) && ((this->offset % 42) == 41)) {
		// Special case for last char in each row of map file
		return 0;
	}
	return (uint8_t)(sam_key[(this->offset % this->resetInterval) % SAM_KEYLEN]);
}


FilterType_SAM_Base::FilterType_SAM_Base(int resetInterval)
	:	resetInterval(resetInterval)
{
}

FilterType_SAM_Base::~FilterType_SAM_Base()
{
}

std::string FilterType_SAM_Base::getFilterCode() const
{
	return "xor-sagent";
}

std::string FilterType_SAM_Base::getFriendlyName() const
{
	return "Secret Agent XOR encryption";
}

std::vector<std::string> FilterType_SAM_Base::getGameList() const
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Secret Agent");
	return vcGames;
}

stream::inout_sptr FilterType_SAM_Base::apply(stream::inout_sptr target,
	stream::fn_truncate resize) const
{
	stream::filtered_sptr sxor(new stream::filtered());
	// We need two separate filters, otherwise reading from one will
	// affect the XOR key next used when writing to the other.
	filter_sptr de_fxor(new filter_sam_crypt(this->resetInterval));
	filter_sptr en_fxor(new filter_sam_crypt(this->resetInterval));

	stream::filtered_sptr sswap(new stream::filtered());
	// Since the bitswap doesn't care how many bytes have been read or
	// written, we can use the same filter for both reading and writing.
	filter_sptr fswap(new filter_bitswap());

	sswap->open(target, fswap, fswap, resize);
	sxor->open(sswap, de_fxor, en_fxor, NULL);

	return sxor;
}

stream::input_sptr FilterType_SAM_Base::apply(stream::input_sptr target) const
{
	stream::input_filtered_sptr sxor(new stream::input_filtered());
	filter_sptr fxor(new filter_sam_crypt(this->resetInterval));

	stream::input_filtered_sptr sswap(new stream::input_filtered());
	filter_sptr fswap(new filter_bitswap());

	sswap->open(target, fswap);
	sxor->open(sswap, fxor);

	return sxor;
}

stream::output_sptr FilterType_SAM_Base::apply(stream::output_sptr target,
	stream::fn_truncate resize) const
{
	stream::output_filtered_sptr sxor(new stream::output_filtered());
	filter_sptr fxor(new filter_sam_crypt(this->resetInterval));

	stream::output_filtered_sptr sswap(new stream::output_filtered());
	filter_sptr fswap(new filter_bitswap());

	sswap->open(target, fswap, NULL);
	sxor->open(sswap, fxor, resize);

	return sxor;
}


FilterType_SAM_Map::FilterType_SAM_Map()
	:	FilterType_SAM_Base(42)
{
}

FilterType_SAM_Map::~FilterType_SAM_Map()
{
}

std::string FilterType_SAM_Map::getFilterCode() const
{
	return "xor-sagent-map";
}

std::string FilterType_SAM_Map::getFriendlyName() const
{
	return "Secret Agent XOR encryption (map file)";
}


FilterType_SAM_8Sprite::FilterType_SAM_8Sprite()
	:	FilterType_SAM_Base(2048)
{
}

FilterType_SAM_8Sprite::~FilterType_SAM_8Sprite()
{
}

std::string FilterType_SAM_8Sprite::getFilterCode() const
{
	return "xor-sagent-8sprite";
}

std::string FilterType_SAM_8Sprite::getFriendlyName() const
{
	return "Secret Agent XOR encryption (8x8 sprite file)";
}


FilterType_SAM_16Sprite::FilterType_SAM_16Sprite()
	:	FilterType_SAM_Base(8064)
{
}

FilterType_SAM_16Sprite::~FilterType_SAM_16Sprite()
{
}

std::string FilterType_SAM_16Sprite::getFilterCode() const
{
	return "xor-sagent-16sprite";
}

std::string FilterType_SAM_16Sprite::getFriendlyName() const
{
	return "Secret Agent XOR encryption (16x16 sprite file)";
}

} // namespace gamearchive
} // namespace camoto
