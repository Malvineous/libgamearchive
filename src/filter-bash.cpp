/**
 * @file   filter-bash.cpp
 * @brief  Filter implementation for decompressing Monster Bash files.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/DAT_Format_(Monster_Bash)
 *
 * Copyright (C) 2010 Adam Nielsen <malvineous@shikadi.net>
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
#include <camoto/filteredstream.hpp>
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

iostream_sptr BashFilterType::apply(iostream_sptr target, FN_TRUNCATE fnTruncate)
	throw (ECorruptedData)
{
	// File needs to be decompressed
	filtering_istream_sptr pinf(new io::filtering_istream());
	pinf->push(bash_unrle_filter());
	pinf->push(lzw_decompress_filter(
		9,   // initial codeword length (in bits)
		12,  // maximum codeword length (in bits)
		257, // first valid codeword
		256, // EOF codeword is first codeword
		0,   // reset codeword is unused
		LZW_LITTLE_ENDIAN    | // bits are split into bytes in little-endian order
		LZW_EOF_PARAM_VALID    // Has codeword reserved for EOF - TODO: confirm
	));
	filtering_ostream_sptr poutf(new io::filtering_ostream());
	//poutf->push(bash_rle_filter());
	//poutf->push(lzw_compress_filter(12, LZW_LITTLE_ENDIAN));
	iostream_sptr dec(new filteredstream(target, pinf, poutf));
	return dec;
}

istream_sptr BashFilterType::apply(istream_sptr target)
	throw (ECorruptedData)
{
	filtering_istream_sptr pinf(new io::filtering_istream());
	pinf->push(bash_unrle_filter());
	pinf->push(lzw_decompress_filter(
		9,   // initial codeword length (in bits)
		12,  // maximum codeword length (in bits)
		257, // first valid codeword
		256, // EOF codeword is first codeword
		0,   // reset codeword is unused
		LZW_LITTLE_ENDIAN    | // bits are split into bytes in little-endian order
		LZW_EOF_PARAM_VALID    // Has codeword reserved for EOF - TODO: confirm
	));
	pinf->push(*target);
	return pinf;
}

ostream_sptr BashFilterType::apply(ostream_sptr target, FN_TRUNCATE fnTruncate)
	throw (ECorruptedData)
{
	filtering_ostream_sptr poutf(new io::filtering_ostream());
	poutf->push(*target);
	return poutf;
}

} // namespace gamearchive
} // namespace camoto
