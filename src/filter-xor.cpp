/**
 * @file   filter-xor.cpp
 * @brief  Filter implementation for encrypting and decrypting XOR coded files.
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

#include <stack>
#include <boost/iostreams/concepts.hpp>     // multichar_input_filter
#include <boost/bind.hpp>
#include <camoto/iostream_helpers.hpp>
#include <camoto/filteredstream.hpp>
#include <camoto/bitstream.hpp>

#include "filter-xor.hpp"

namespace camoto {
namespace gamearchive {

xor_crypt_filter::xor_crypt_filter(int lenCrypt, int seed)
	: lenCrypt(lenCrypt),
	  seed(seed),
	  offset(0)
{
}

void xor_crypt_filter::setSeed(int val)
{
	this->seed = val;
	return;
}

uint8_t xor_crypt_filter::getKey()
{
	return (uint8_t)(this->seed + this->offset);
}


XORFilterType::XORFilterType()
	throw ()
{
}

XORFilterType::~XORFilterType()
	throw ()
{
}

std::string XORFilterType::getFilterCode() const
	throw ()
{
	return "xor-inc";
}

std::string XORFilterType::getFriendlyName() const
	throw ()
{
	return "Incremental XOR encryption";
}

std::vector<std::string> XORFilterType::getGameList() const
	throw ()
{
	return std::vector<std::string>();
}

iostream_sptr XORFilterType::apply(iostream_sptr target, FN_TRUNCATE fnTruncate)
	throw (ECorruptedData)
{
	filtered_iostream_sptr pf(new filtered_iostream());
	pf->push(xor_crypt_filter(0, 0));
	pf->pushShared(target);
	return pf;
}

istream_sptr XORFilterType::apply(istream_sptr target)
	throw (ECorruptedData)
{
	filtered_istream_sptr pinf(new filtered_istream());
	pinf->push(xor_crypt_filter(0, 0));

	pinf->push(*target);
	return pinf;
}

ostream_sptr XORFilterType::apply(ostream_sptr target, FN_TRUNCATE fnTruncate)
	throw (ECorruptedData)
{
	filtered_ostream_sptr poutf(new filtered_ostream());
	poutf->push(xor_crypt_filter(0, 0));

	poutf->push(*target);
	return poutf;
}

} // namespace gamearchive
} // namespace camoto
