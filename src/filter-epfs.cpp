/**
 * @file  filter-epfs.cpp
 * @brief FilterType for EPFS compression algorithm.
 *
 * This algorithm is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/EPF_Format
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
#include <camoto/lzw.hpp>

#include "filter-epfs.hpp"

namespace camoto {
namespace gamearchive {

FilterType_EPFS::FilterType_EPFS()
{
}

FilterType_EPFS::~FilterType_EPFS()
{
}

std::string FilterType_EPFS::code() const
{
	return "lzw-epfs";
}

std::string FilterType_EPFS::friendlyName() const
{
	return "East Point Software EPFS compression";
}

std::vector<std::string> FilterType_EPFS::games() const
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Alien Breed Tower Assault");
	vcGames.push_back("Arcade Pool");
	vcGames.push_back("Jungle Book, The");
	vcGames.push_back("Lion King, The");
	vcGames.push_back("Overdrive");
	vcGames.push_back("Project X");
	vcGames.push_back("Sensible Golf");
	vcGames.push_back("Smurfs, The");
	vcGames.push_back("Spirou");
	vcGames.push_back("Tin Tin in Tibet");
	vcGames.push_back("Universe");
	return vcGames;
}

std::unique_ptr<stream::inout> FilterType_EPFS::apply(
	std::shared_ptr<stream::inout> target, stream::fn_truncate_filter resize)
	const
{
	return std::make_unique<stream::filtered>(
		target,
		std::make_shared<filter_lzw_decompress>(
			9,   // initial codeword length (in bits)
			14,  // maximum codeword length (in bits)
			256, // first valid codeword
			0,   // EOF codeword is max codeword
			-1,  // reset codeword is max-1
			LZW_BIG_ENDIAN        | // bits are split into bytes in big-endian order
			LZW_NO_BITSIZE_RESET  | // bitsize doesn't go back to 9 after dict reset
			LZW_EOF_PARAM_VALID   | // Has codeword reserved for EOF
			LZW_RESET_PARAM_VALID   // Has codeword reserved for dict reset
		),
		std::make_shared<filter_lzw_compress>(
			9,   // initial codeword length (in bits)
			14,  // maximum codeword length (in bits)
			256, // first valid codeword
			0,   // EOF codeword is max codeword
			-1,  // reset codeword is max-1
			LZW_BIG_ENDIAN        | // bits are split into bytes in big-endian order
			LZW_NO_BITSIZE_RESET  | // bitsize doesn't go back to 9 after dict reset
			LZW_EOF_PARAM_VALID   | // Has codeword reserved for EOF
			LZW_RESET_PARAM_VALID   // Has codeword reserved for dict reset
		),
		resize
	);
}

std::unique_ptr<stream::input> FilterType_EPFS::apply(
	std::shared_ptr<stream::input> target) const
{
	return std::make_unique<stream::input_filtered>(
		target,
		std::make_shared<filter_lzw_decompress>(
			9,   // initial codeword length (in bits)
			14,  // maximum codeword length (in bits)
			256, // first valid codeword
			0,   // EOF codeword is max codeword
			-1,  // reset codeword is max-1
			LZW_BIG_ENDIAN        | // bits are split into bytes in big-endian order
			LZW_NO_BITSIZE_RESET  | // bitsize doesn't go back to 9 after dict reset
			LZW_EOF_PARAM_VALID   | // Has codeword reserved for EOF
			LZW_RESET_PARAM_VALID   // Has codeword reserved for dict reset
		)
	);
}

std::unique_ptr<stream::output> FilterType_EPFS::apply(
	std::shared_ptr<stream::output> target, stream::fn_truncate_filter resize)
	const
{
	return std::make_unique<stream::output_filtered>(
		target,
		std::make_shared<filter_lzw_compress>(
			9,   // initial codeword length (in bits)
			14,  // maximum codeword length (in bits)
			256, // first valid codeword
			0,   // EOF codeword is max codeword
			-1,  // reset codeword is max-1
			LZW_BIG_ENDIAN        | // bits are split into bytes in big-endian order
			LZW_NO_BITSIZE_RESET  | // bitsize doesn't go back to 9 after dict reset
			LZW_EOF_PARAM_VALID   | // Has codeword reserved for EOF
			LZW_RESET_PARAM_VALID   // Has codeword reserved for dict reset
		),
		resize
	);
}

} // namespace gamearchive
} // namespace camoto
