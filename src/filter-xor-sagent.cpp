/**
 * @file   filter-xor-sagent.cpp
 * @brief  Filter that encrypts and decrypts files in Sagent RFF archives.
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

#include <boost/iostreams/invert.hpp>
#include <camoto/filteredstream.hpp>
#include "filter-xor-sagent.hpp"
#include "filter-bitswap.hpp"

namespace camoto {
namespace gamearchive {

#define SAM_KEY     "Copyright 1991 Peder Jungck"
#define SAM_KEYLEN  (strlen(SAM_KEY)+1)  // include terminating null

const char sam_key[] = SAM_KEY;

sam_crypt_filter::sam_crypt_filter()
	: xor_crypt_filter(0, 0)
{
}

uint8_t sam_crypt_filter::getKey()
{
	return (uint8_t)(sam_key[this->offset % SAM_KEYLEN]);
}


SAMFilterType::SAMFilterType()
	throw ()
{
}

SAMFilterType::~SAMFilterType()
	throw ()
{
}

std::string SAMFilterType::getFilterCode() const
	throw ()
{
	return "xor-sagent";
}

std::string SAMFilterType::getFriendlyName() const
	throw ()
{
	return "Secret Agent XOR encryption";
}

std::vector<std::string> SAMFilterType::getGameList() const
	throw ()
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Secret Agent");
	return vcGames;
}

iostream_sptr SAMFilterType::apply(iostream_sptr target, FN_TRUNCATE fnTruncate)
	throw (ECorruptedData)
{
	filtered_iostream_sptr pf(new filtered_iostream());
	pf->push(sam_crypt_filter());
	pf->push(bitswap_filter());
	pf->pushShared(target);
	return pf;
}

istream_sptr SAMFilterType::apply(istream_sptr target)
	throw (ECorruptedData)
{
	filtered_istream_sptr pinf(new filtered_istream());
	pinf->push(sam_crypt_filter());
	pinf->push(bitswap_filter());

	pinf->pushShared(target);
	return pinf;
}

ostream_sptr SAMFilterType::apply(ostream_sptr target, FN_TRUNCATE fnTruncate)
	throw (ECorruptedData)
{
	filtered_ostream_sptr poutf(new filtered_ostream());
	poutf->push(io::invert(sam_crypt_filter()));
	poutf->push(bitswap_filter());

	poutf->pushShared(target);
	return poutf;
}

} // namespace gamearchive
} // namespace camoto
