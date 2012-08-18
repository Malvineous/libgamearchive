/**
 * @file   filter-xor-sagent.cpp
 * @brief  Filter that encrypts and decrypts Secret Agent data files.
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

#include <camoto/stream_filtered.hpp>
#include "filter-xor-sagent.hpp"
#include "filter-bitswap.hpp"

namespace camoto {
namespace gamearchive {

#define SAM_KEY     "Copyright 1991 Peder Jungck"
#define SAM_KEYLEN  (strlen(SAM_KEY)+1)  // include terminating null

const char sam_key[] = SAM_KEY;

sam_crypt_filter::sam_crypt_filter(int resetInterval)
	:	filter_xor_crypt(0, 0),
		resetInterval(resetInterval)
{
}

uint8_t sam_crypt_filter::getKey()
{
	return (uint8_t)(sam_key[(this->offset % this->resetInterval) % SAM_KEYLEN]);
}


SAMBaseFilterType::SAMBaseFilterType(int resetInterval)
	:	resetInterval(resetInterval)
{
}

SAMBaseFilterType::~SAMBaseFilterType()
{
}

std::string SAMBaseFilterType::getFilterCode() const
{
	return "xor-sagent";
}

std::string SAMBaseFilterType::getFriendlyName() const
{
	return "Secret Agent XOR encryption";
}

std::vector<std::string> SAMBaseFilterType::getGameList() const
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Secret Agent");
	return vcGames;
}

stream::inout_sptr SAMBaseFilterType::apply(stream::inout_sptr target,
	stream::fn_truncate resize) const
{
	stream::filtered_sptr st1(new stream::filtered());
	// We need two separate filters, otherwise reading from one will
	// affect the XOR key next used when writing to the other.
	filter_sptr de_cr(new sam_crypt_filter(this->resetInterval));
	filter_sptr en_cr(new sam_crypt_filter(this->resetInterval));
	st1->open(target, de_cr, en_cr, NULL);

	stream::filtered_sptr st2(new stream::filtered());
	// Since the bitswap doesn't care how many bytes have been read or
	// written, we can use the same filter for both reading and writing.
	filter_sptr bs(new filter_bitswap());
	st2->open(st1, bs, bs, resize);

	return st2;
}

stream::input_sptr SAMBaseFilterType::apply(stream::input_sptr target) const
{
	stream::input_filtered_sptr st1(new stream::input_filtered());
	filter_sptr cr(new sam_crypt_filter(this->resetInterval));
	st1->open(target, cr);

	stream::input_filtered_sptr st2(new stream::input_filtered());
	filter_sptr bs(new filter_bitswap());
	st2->open(st1, bs);

	return st2;
}

stream::output_sptr SAMBaseFilterType::apply(stream::output_sptr target,
	stream::fn_truncate resize) const
{
	stream::output_filtered_sptr st1(new stream::output_filtered());
	filter_sptr cr(new sam_crypt_filter(this->resetInterval));
	st1->open(target, cr, NULL);

	stream::output_filtered_sptr st2(new stream::output_filtered());
	filter_sptr bs(new filter_bitswap());
	st2->open(st1, bs, resize);

	return st2;
}


SAMMapFilterType::SAMMapFilterType()
	:	SAMBaseFilterType(42)
{
}

SAMMapFilterType::~SAMMapFilterType()
{
}

std::string SAMMapFilterType::getFilterCode() const
{
	return "xor-sagent-map";
}

std::string SAMMapFilterType::getFriendlyName() const
{
	return "Secret Agent XOR encryption (map file)";
}


SAM8SpriteFilterType::SAM8SpriteFilterType()
	:	SAMBaseFilterType(8064)
{
}

SAM8SpriteFilterType::~SAM8SpriteFilterType()
{
}

std::string SAM8SpriteFilterType::getFilterCode() const
{
	return "xor-sagent-8sprite";
}

std::string SAM8SpriteFilterType::getFriendlyName() const
{
	return "Secret Agent XOR encryption (8x8 sprite file)";
}


SAM16SpriteFilterType::SAM16SpriteFilterType()
	:	SAMBaseFilterType(2048)
{
}

SAM16SpriteFilterType::~SAM16SpriteFilterType()
{
}

std::string SAM16SpriteFilterType::getFilterCode() const
{
	return "xor-sagent-16sprite";
}

std::string SAM16SpriteFilterType::getFriendlyName() const
{
	return "Secret Agent XOR encryption (16x16 sprite file)";
}

} // namespace gamearchive
} // namespace camoto
