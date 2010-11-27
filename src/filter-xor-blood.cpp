/**
 * @file   filter-xor-blood.cpp
 * @brief  Filter that encrypts and decrypts files in Blood RFF archives.
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

#include <boost/iostreams/invert.hpp>
#include <camoto/filteredstream.hpp>
#include "filter-xor-blood.hpp"

namespace camoto {
namespace gamearchive {

#define RFF_FILE_CRYPT_LEN 256  // number of bytes encrypted from start of file

RFFFilterType::RFFFilterType()
	throw ()
{
}

RFFFilterType::~RFFFilterType()
	throw ()
{
}

std::string RFFFilterType::getFilterCode() const
	throw ()
{
	return "xor-blood";
}

std::string RFFFilterType::getFriendlyName() const
	throw ()
{
	return "Blood RFF encryption";
}

std::vector<std::string> RFFFilterType::getGameList() const
	throw ()
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Blood");
	return vcGames;
}

iostream_sptr RFFFilterType::apply(iostream_sptr target, FN_TRUNCATE fnTruncate)
	throw (ECorruptedData)
{
	filtering_istream_sptr pinf(new io::filtering_istream());

	// Decrypt the data using the Blood XOR algorithm
	pinf->push(rff_crypt_filter(RFF_FILE_CRYPT_LEN, 0));

	filtering_ostream_sptr poutf(new io::filtering_ostream());
	poutf->push(io::invert(rff_crypt_filter(RFF_FILE_CRYPT_LEN, 0)));

	iostream_sptr dec(new filteredstream(target, pinf, poutf));
	return dec;
}

istream_sptr RFFFilterType::apply(istream_sptr target)
	throw (ECorruptedData)
{
	filtering_istream_sptr pinf(new io::filtering_istream());

	// Decrypt the data using the Blood XOR algorithm
	pinf->push(rff_crypt_filter(RFF_FILE_CRYPT_LEN, 0));

	pinf->push(*target);
	return pinf;
}

ostream_sptr RFFFilterType::apply(ostream_sptr target, FN_TRUNCATE fnTruncate)
	throw (ECorruptedData)
{
	filtering_ostream_sptr poutf(new io::filtering_ostream());
	poutf->push(io::invert(rff_crypt_filter(RFF_FILE_CRYPT_LEN, 0)));

	poutf->push(*target);
	return poutf;
}

} // namespace gamearchive
} // namespace camoto
