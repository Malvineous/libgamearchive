/**
 * @file   filter-epfs.cpp
 * @brief  FilterType for EPFS compression algorithm.
 *
 * This algorithm is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/EPF_Format
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

#include "filter-epfs.hpp"

namespace camoto {
namespace gamearchive {

EPFSFilterType::EPFSFilterType()
	throw ()
{
}

EPFSFilterType::~EPFSFilterType()
	throw ()
{
}

std::string EPFSFilterType::getFilterCode() const
	throw ()
{
	return "lzw-epfs";
}

std::string EPFSFilterType::getFriendlyName() const
	throw ()
{
	return "East Point Software EPFS compression";
}

std::vector<std::string> EPFSFilterType::getGameList() const
	throw ()
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Alien Breed Tower Assault");
	vcGames.push_back("Arcade Pool");
	vcGames.push_back("Jungle Book, The");
	vcGames.push_back("Lion King, The");
	vcGames.push_back("Project X");
	vcGames.push_back("Overdrive");
	vcGames.push_back("Sensible Golf");
	vcGames.push_back("Smurfs, The");
	vcGames.push_back("Spirou");
	vcGames.push_back("Tin Tin in Tibet");
	vcGames.push_back("Universe");
	return vcGames;
}

stream::inout_sptr EPFSFilterType::apply(stream::inout_sptr target,
	stream::fn_truncate resize)
	throw (filter_error, stream::read_error)
{
	stream::filtered_sptr st(new stream::filtered());
	filter_sptr de(new filter_lzw_decompress(
		9,   // initial codeword length (in bits)
		14,  // maximum codeword length (in bits)
		256, // first valid codeword
		0,   // EOF codeword is max codeword
		-1,  // reset codeword is max-1
		LZW_BIG_ENDIAN        | // bits are split into bytes in big-endian order
		LZW_NO_BITSIZE_RESET  | // bitsize doesn't go back to 9 after dict reset
		LZW_EOF_PARAM_VALID   | // Has codeword reserved for EOF
		LZW_RESET_PARAM_VALID   // Has codeword reserved for dict reset
	));
	filter_sptr en = de; /// @todo Fix when LZW compression has been implemented
	st->open(target, de, en, resize);
	return st;
}

stream::input_sptr EPFSFilterType::apply(stream::input_sptr target)
	throw (filter_error, stream::read_error)
{
	stream::input_filtered_sptr st(new stream::input_filtered());
	filter_sptr de(new filter_lzw_decompress(
		9,   // initial codeword length (in bits)
		14,  // maximum codeword length (in bits)
		256, // first valid codeword
		0,   // EOF codeword is max codeword
		-1,  // reset codeword is max-1
		LZW_BIG_ENDIAN        | // bits are split into bytes in big-endian order
		LZW_NO_BITSIZE_RESET  | // bitsize doesn't go back to 9 after dict reset
		LZW_EOF_PARAM_VALID   | // Has codeword reserved for EOF
		LZW_RESET_PARAM_VALID   // Has codeword reserved for dict reset
	));
	st->open(target, de);
	return st;
}

stream::output_sptr EPFSFilterType::apply(stream::output_sptr target,
	stream::fn_truncate resize)
	throw (filter_error)
{
	return target; /// @todo Fix when LZW compression has been implemented
	stream::output_filtered_sptr st(new stream::output_filtered());
	filter_sptr de(new filter_lzw_decompress(
		9,   // initial codeword length (in bits)
		14,  // maximum codeword length (in bits)
		256, // first valid codeword
		0,   // EOF codeword is max codeword
		-1,  // reset codeword is max-1
		LZW_BIG_ENDIAN        | // bits are split into bytes in big-endian order
		LZW_NO_BITSIZE_RESET  | // bitsize doesn't go back to 9 after dict reset
		LZW_EOF_PARAM_VALID   | // Has codeword reserved for EOF
		LZW_RESET_PARAM_VALID   // Has codeword reserved for dict reset
	));
	//filter_sptr en = ...
	//st->open(target, en, resize);
	return st;
}

} // namespace gamearchive
} // namespace camoto
