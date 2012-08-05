/**
 * @file   filter-bash.cpp
 * @brief  Filter implementation for decompressing Monster Bash files.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/DAT_Format_(Monster_Bash)
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

#include <camoto/iostream_helpers.hpp>
#include <camoto/stream_filtered.hpp>
#include <camoto/lzw.hpp>

#include "filter-bash-rle.hpp"
#include "filter-bash.hpp"

namespace camoto {
namespace gamearchive {

BashFilterType::BashFilterType()
	throw ()
{
}

BashFilterType::~BashFilterType()
	throw ()
{
}

std::string BashFilterType::getFilterCode() const
	throw ()
{
	return "lzw-bash";
}

std::string BashFilterType::getFriendlyName() const
	throw ()
{
	return "Monster Bash compression";
}

std::vector<std::string> BashFilterType::getGameList() const
	throw ()
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Monster Bash");
	return vcGames;
}

stream::inout_sptr BashFilterType::apply(stream::inout_sptr target,
	stream::fn_truncate resize)
	throw (filter_error, stream::read_error)
{
	stream::filtered_sptr st1(new stream::filtered());
	filter_sptr f_delzw(new filter_lzw_decompress(
		9,   // initial codeword length (in bits)
		12,  // maximum codeword length (in bits)
		257, // first valid codeword
		256, // EOF codeword is first codeword
		256, // reset codeword is unused
		LZW_LITTLE_ENDIAN    | // bits are split into bytes in little-endian order
		LZW_RESET_PARAM_VALID  // Has codeword reserved for dictionary reset/EOF
	));
	filter_sptr f_lzw(new filter_lzw_compress(
		9,   // initial codeword length (in bits)
		12,  // maximum codeword length (in bits)
		257, // first valid codeword
		256, // EOF codeword is first codeword
		256, // reset codeword is shared with EOF
		LZW_LITTLE_ENDIAN    | // bits are split into bytes in little-endian order
		LZW_EOF_PARAM_VALID  | // Has codeword reserved for EOF
		LZW_RESET_PARAM_VALID  // Has codeword reserved for dictionary reset
	));
	st1->open(target, f_delzw, f_lzw, NULL);

	stream::filtered_sptr st2(new stream::filtered());
	filter_sptr f_unrle(new filter_bash_unrle());
	filter_sptr f_rle(new filter_bash_rle());
	st2->open(st1, f_unrle, f_rle, resize);

	return st2;
}

stream::input_sptr BashFilterType::apply(stream::input_sptr target)
	throw (filter_error, stream::read_error)
{
	stream::input_filtered_sptr st1(new stream::input_filtered());
	filter_sptr f_delzw(new filter_lzw_decompress(
		9,   // initial codeword length (in bits)
		12,  // maximum codeword length (in bits)
		257, // first valid codeword
		256, // EOF codeword is first codeword
		256, // reset codeword is shared with EOF
		LZW_LITTLE_ENDIAN    | // bits are split into bytes in little-endian order
		LZW_EOF_PARAM_VALID    // Has codeword reserved for EOF - TODO: confirm
	));
	st1->open(target, f_delzw);

	stream::input_filtered_sptr st2(new stream::input_filtered());
	filter_sptr f_unrle(new filter_bash_unrle());
	st2->open(st1, f_unrle);

	return st2;
}

stream::output_sptr BashFilterType::apply(stream::output_sptr target,
	stream::fn_truncate resize)
	throw (filter_error)
{
	stream::output_filtered_sptr st1(new stream::output_filtered());
	filter_sptr f_lzw(new filter_lzw_compress(
		9,   // initial codeword length (in bits)
		12,  // maximum codeword length (in bits)
		257, // first valid codeword
		256, // EOF codeword is first codeword
		0,   // reset codeword is unused
		LZW_LITTLE_ENDIAN    | // bits are split into bytes in little-endian order
		LZW_EOF_PARAM_VALID    // Has codeword reserved for EOF - TODO: confirm
	));
	st1->open(target, f_lzw, NULL);

	stream::output_filtered_sptr st2(new stream::output_filtered());
	filter_sptr f_rle(new filter_bash_rle());
	st2->open(st1, f_rle, resize);

	return st2;
}

} // namespace gamearchive
} // namespace camoto
