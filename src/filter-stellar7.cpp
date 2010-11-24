/**
 * @file   filter-stellar7.cpp
 * @brief  FilterType for Stellar 7 compression algorithm.
 *
 * This algorithm is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/RES_Format_(Stellar_7)
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

//#include <camoto/iostream_helpers.hpp>
#include <camoto/filteredstream.hpp>
#include <camoto/lzw.hpp>
//#include <camoto/debug.hpp>

#include "filter-stellar7.hpp"

namespace camoto {
namespace gamearchive {

Stellar7FilterType::Stellar7FilterType()
	throw ()
{
}

Stellar7FilterType::~Stellar7FilterType()
	throw ()
{
}

std::string Stellar7FilterType::getFilterCode() const
	throw ()
{
	return "lzw-bash";
}

std::string Stellar7FilterType::getFriendlyName() const
	throw ()
{
	return "Monster Stellar7 compression";
}

std::vector<std::string> Stellar7FilterType::getGameList() const
	throw ()
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Monster Stellar7");
	return vcGames;
}

iostream_sptr Stellar7FilterType::apply(iostream_sptr target, FN_TRUNCATE fnTruncate)
	throw (ECorruptedData)
{
	// File needs to be decompressed
	filtering_istream_sptr pinf(new io::filtering_istream());
	pinf->push(lzw_decompress_filter(
		9,   // initial codeword length (in bits)
		12,  // maximum codeword length (in bits)
		257, // first valid codeword
		0,   // EOF codeword is unused
		256, // reset codeword is first codeword
		LZW_LITTLE_ENDIAN     | // bits are split into bytes in little-endian order
		LZW_RESET_PARAM_VALID | // has codeword reserved for dictionary reset
		LZW_FLUSH_ON_RESET      // Jump to next word boundary on dict reset
	));
	filtering_ostream_sptr poutf(new io::filtering_ostream());
	//poutf->push(lzw_compress_filter(12, LZW_LITTLE_ENDIAN));
	iostream_sptr dec(new filteredstream(target, pinf, poutf));
	return dec;
}

istream_sptr Stellar7FilterType::apply(istream_sptr target)
	throw (ECorruptedData)
{
	filtering_istream_sptr pinf(new io::filtering_istream());
	pinf->push(lzw_decompress_filter(
		9,   // initial codeword length (in bits)
		12,  // maximum codeword length (in bits)
		257, // first valid codeword
		0,   // EOF codeword is unused
		256, // reset codeword is first codeword
		LZW_LITTLE_ENDIAN     | // bits are split into bytes in little-endian order
		LZW_RESET_PARAM_VALID | // has codeword reserved for dictionary reset
		LZW_FLUSH_ON_RESET      // Jump to next word boundary on dict reset
	));
	pinf->push(*target);
	return pinf;
}

ostream_sptr Stellar7FilterType::apply(ostream_sptr target, FN_TRUNCATE fnTruncate)
	throw (ECorruptedData)
{
	filtering_ostream_sptr poutf(new io::filtering_ostream());
	poutf->push(*target);
	return poutf;
}

} // namespace gamearchive
} // namespace camoto
