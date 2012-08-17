/**
 * @file   filter-stellar7.cpp
 * @brief  FilterType for Stellar 7 compression algorithm.
 *
 * This algorithm is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/RES_Format_(Stellar_7)
 *
 * Copyright (C) 2010-2011 Adam Nielsen <malvineous@shikadi.net>
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
#include <camoto/lzw.hpp>

#include "filter-stellar7.hpp"

namespace camoto {
namespace gamearchive {

Stellar7FilterType::Stellar7FilterType()
{
}

Stellar7FilterType::~Stellar7FilterType()
{
}

std::string Stellar7FilterType::getFilterCode() const
{
	return "lzw-stellar7";
}

std::string Stellar7FilterType::getFriendlyName() const
{
	return "Stellar 7 compression";
}

std::vector<std::string> Stellar7FilterType::getGameList() const
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Stellar 7");
	return vcGames;
}

stream::inout_sptr Stellar7FilterType::apply(stream::inout_sptr target,
	stream::fn_truncate resize)
{
	stream::filtered_sptr st(new stream::filtered());
	filter_sptr de(new filter_lzw_decompress(
		9,   // initial codeword length (in bits)
		12,  // maximum codeword length (in bits)
		257, // first valid codeword
		0,   // EOF codeword is unused
		256, // reset codeword is first codeword
		LZW_LITTLE_ENDIAN     | // bits are split into bytes in little-endian order
		LZW_RESET_PARAM_VALID | // has codeword reserved for dictionary reset
		LZW_FLUSH_ON_RESET      // Jump to next word boundary on dict reset
	));
	filter_sptr en = de; /// @todo Fix when LZW compression has been implemented
	st->open(target, de, en, resize);
	return st;
}

stream::input_sptr Stellar7FilterType::apply(stream::input_sptr target)
{
	stream::input_filtered_sptr st(new stream::input_filtered());
	filter_sptr de(new filter_lzw_decompress(
		9,   // initial codeword length (in bits)
		12,  // maximum codeword length (in bits)
		257, // first valid codeword
		0,   // EOF codeword is unused
		256, // reset codeword is first codeword
		LZW_LITTLE_ENDIAN     | // bits are split into bytes in little-endian order
		LZW_RESET_PARAM_VALID | // has codeword reserved for dictionary reset
		LZW_FLUSH_ON_RESET      // Jump to next word boundary on dict reset
	));
	st->open(target, de);
	return st;
}

stream::output_sptr Stellar7FilterType::apply(stream::output_sptr target,
	stream::fn_truncate resize)
{
	stream::output_filtered_sptr st(new stream::output_filtered());
	filter_sptr en; /// @todo Fix when LZW compression has been implemented
	st->open(target, en, resize);
	return st;
}

} // namespace gamearchive
} // namespace camoto
