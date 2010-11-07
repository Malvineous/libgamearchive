/**
 * @file   filter-epfs.cpp
 * @brief  FilterType for EPFS compression algorithm.
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

iostream_sptr EPFSFilterType::apply(iostream_sptr target, FN_TRUNCATE fnTruncate)
	throw (ECorruptedData)
{
	// File needs to be decompressed
	filtering_istream_sptr pinf(new io::filtering_istream());
	pinf->push(lzw_decompress_filter(
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
	filtering_ostream_sptr poutf(new io::filtering_ostream());
	//poutf->push(bash_rle_filter());
	//poutf->push(lzw_compress_filter(12, LZW_LITTLE_ENDIAN));
	iostream_sptr dec(new filteredstream(target, pinf, poutf));
	return dec;
}

istream_sptr EPFSFilterType::apply(istream_sptr target)
	throw (ECorruptedData)
{
	filtering_istream_sptr pinf(new io::filtering_istream());
	pinf->push(lzw_decompress_filter(
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
	pinf->push(*target);
	return pinf;
}

ostream_sptr EPFSFilterType::apply(ostream_sptr target, FN_TRUNCATE fnTruncate)
	throw (ECorruptedData)
{
	filtering_ostream_sptr poutf(new io::filtering_ostream());
	poutf->push(*target);
	return poutf;
}

} // namespace gamearchive
} // namespace camoto
