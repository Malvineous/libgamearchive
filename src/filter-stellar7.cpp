/**
 * @file  filter-stellar7.cpp
 * @brief FilterType for Stellar 7 compression algorithm.
 *
 * This algorithm is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/RES_Format_(Stellar_7)
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
#include <camoto/filter-lzw.hpp>

#include "filter-stellar7.hpp"

namespace camoto {
namespace gamearchive {

FilterType_Stellar7::FilterType_Stellar7()
{
}

FilterType_Stellar7::~FilterType_Stellar7()
{
}

std::string FilterType_Stellar7::code() const
{
	return "lzw-stellar7";
}

std::string FilterType_Stellar7::friendlyName() const
{
	return "Stellar 7 compression";
}

std::vector<std::string> FilterType_Stellar7::games() const
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Stellar 7");
	return vcGames;
}

std::unique_ptr<stream::inout> FilterType_Stellar7::apply(
	std::unique_ptr<stream::inout> target, stream::fn_notify_prefiltered_size resize)
	const
{
	return std::make_unique<stream::filtered>(
		std::move(target),
		std::make_shared<filter_lzw_decompress>(
			9,   // initial codeword length (in bits)
			12,  // maximum codeword length (in bits)
			257, // first valid codeword
			0,   // EOF codeword is unused
			256, // reset codeword is first codeword
			LZW_LITTLE_ENDIAN     | // bits are split into bytes in little-endian order
			LZW_RESET_PARAM_VALID | // has codeword reserved for dictionary reset
			LZW_FLUSH_ON_RESET      // Jump to next word boundary on dict reset
		),
		std::make_shared<filter_lzw_compress>(
			9,   // initial codeword length (in bits)
			12,  // maximum codeword length (in bits)
			257, // first valid codeword
			0,   // EOF codeword is unused
			256, // reset codeword is first codeword
			LZW_LITTLE_ENDIAN     | // bits are split into bytes in little-endian order
			LZW_RESET_PARAM_VALID | // has codeword reserved for dictionary reset
			LZW_FLUSH_ON_RESET      // Jump to next word boundary on dict reset
		),
		resize
	);
}

std::unique_ptr<stream::input> FilterType_Stellar7::apply(
	std::unique_ptr<stream::input> target) const
{
	return std::make_unique<stream::input_filtered>(
		std::move(target),
		std::make_shared<filter_lzw_decompress>(
			9,   // initial codeword length (in bits)
			12,  // maximum codeword length (in bits)
			257, // first valid codeword
			0,   // EOF codeword is unused
			256, // reset codeword is first codeword
			LZW_LITTLE_ENDIAN     | // bits are split into bytes in little-endian order
			LZW_RESET_PARAM_VALID | // has codeword reserved for dictionary reset
			LZW_FLUSH_ON_RESET      // Jump to next word boundary on dict reset
		)
	);
}

std::unique_ptr<stream::output> FilterType_Stellar7::apply(
	std::unique_ptr<stream::output> target, stream::fn_notify_prefiltered_size resize)
	const
{
	return std::make_unique<stream::output_filtered>(
		std::move(target),
		std::make_shared<filter_lzw_compress>(
			9,   // initial codeword length (in bits)
			12,  // maximum codeword length (in bits)
			257, // first valid codeword
			0,   // EOF codeword is unused
			256, // reset codeword is first codeword
			LZW_LITTLE_ENDIAN     | // bits are split into bytes in little-endian order
			LZW_RESET_PARAM_VALID | // has codeword reserved for dictionary reset
			LZW_FLUSH_ON_RESET      // Jump to next word boundary on dict reset
		),
		resize
	);
}

} // namespace gamearchive
} // namespace camoto
