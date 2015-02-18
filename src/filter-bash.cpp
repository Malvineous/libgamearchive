/**
 * @file  filter-bash.cpp
 * @brief Filter implementation for decompressing Monster Bash files.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/DAT_Format_(Monster_Bash)
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

#include <camoto/iostream_helpers.hpp>
#include <camoto/stream_filtered.hpp>
#include <camoto/util.hpp> // std::make_unique
#include <camoto/lzw.hpp>

#include "filter-bash-rle.hpp"
#include "filter-bash.hpp"

namespace camoto {
namespace gamearchive {

FilterType_Bash::FilterType_Bash()
{
}

FilterType_Bash::~FilterType_Bash()
{
}

std::string FilterType_Bash::code() const
{
	return "lzw-bash";
}

std::string FilterType_Bash::friendlyName() const
{
	return "Monster Bash compression";
}

std::vector<std::string> FilterType_Bash::games() const
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Monster Bash");
	return vcGames;
}

std::unique_ptr<stream::inout> FilterType_Bash::apply(
	std::unique_ptr<stream::inout> target, stream::fn_notify_prefiltered_size resize)
	const
{
	auto st1 = std::make_unique<stream::filtered>(
		std::move(target),
		std::make_shared<filter_lzw_decompress>(
			9,   // initial codeword length (in bits)
			12,  // maximum codeword length (in bits)
			257, // first valid codeword
			256, // EOF codeword is first codeword
			256, // reset codeword is unused
			LZW_LITTLE_ENDIAN    | // bits are split into bytes in little-endian order
			LZW_RESET_PARAM_VALID  // Has codeword reserved for dictionary reset/EOF
		),
		std::make_shared<filter_lzw_compress>(
			9,   // initial codeword length (in bits)
			12,  // maximum codeword length (in bits)
			257, // first valid codeword
			256, // EOF codeword is first codeword
			256, // reset codeword is shared with EOF
			LZW_LITTLE_ENDIAN    | // bits are split into bytes in little-endian order
			LZW_EOF_PARAM_VALID  | // Has codeword reserved for EOF
			LZW_RESET_PARAM_VALID  // Has codeword reserved for dictionary reset
		),
		stream::fn_notify_prefiltered_size()
	);

	return std::make_unique<stream::filtered>(
		std::move(st1),
		std::make_shared<filter_bash_unrle>(),
		std::make_shared<filter_bash_rle>(),
		resize
	);
}

std::unique_ptr<stream::input> FilterType_Bash::apply(
	std::unique_ptr<stream::input> target) const
{
	auto st1 = std::make_unique<stream::input_filtered>(
		std::move(target),
		std::make_shared<filter_lzw_decompress>(
			9,   // initial codeword length (in bits)
			12,  // maximum codeword length (in bits)
			257, // first valid codeword
			256, // EOF codeword is first codeword
			256, // reset codeword is unused
			LZW_LITTLE_ENDIAN    | // bits are split into bytes in little-endian order
			LZW_RESET_PARAM_VALID  // Has codeword reserved for dictionary reset/EOF
		)
	);

	return std::make_unique<stream::input_filtered>(
		std::move(st1),
		std::make_shared<filter_bash_unrle>()
	);
}

std::unique_ptr<stream::output> FilterType_Bash::apply(
	std::unique_ptr<stream::output> target, stream::fn_notify_prefiltered_size resize)
	const
{
	auto st1 = std::make_unique<stream::output_filtered>(
		std::move(target),
		std::make_shared<filter_lzw_compress>(
			9,   // initial codeword length (in bits)
			12,  // maximum codeword length (in bits)
			257, // first valid codeword
			256, // EOF codeword is first codeword
			256, // reset codeword is shared with EOF
			LZW_LITTLE_ENDIAN    | // bits are split into bytes in little-endian order
			LZW_EOF_PARAM_VALID  | // Has codeword reserved for EOF
			LZW_RESET_PARAM_VALID  // Has codeword reserved for dictionary reset
		),
		stream::fn_notify_prefiltered_size()
	);

	return std::make_unique<stream::output_filtered>(
		std::move(st1),
		std::make_shared<filter_bash_rle>(),
		resize
	);
}

} // namespace gamearchive
} // namespace camoto
