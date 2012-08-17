/**
 * @file   filter-xor-blood.cpp
 * @brief  Filter that encrypts and decrypts files in Blood RFF archives.
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
#include <camoto/stream_filtered.hpp>
#include "filter-xor-blood.hpp"

namespace camoto {
namespace gamearchive {

#define RFF_FILE_CRYPT_LEN 256  // number of bytes encrypted from start of file

filter_rff_crypt::filter_rff_crypt(int lenCrypt, int seed)
	:	filter_xor_crypt(lenCrypt, seed)
{
}

uint8_t filter_rff_crypt::getKey()
{
	return (uint8_t)(this->seed + (this->offset >> 1));
}


RFFFilterType::RFFFilterType()
{
}

RFFFilterType::~RFFFilterType()
{
}

std::string RFFFilterType::getFilterCode() const
{
	return "xor-blood";
}

std::string RFFFilterType::getFriendlyName() const
{
	return "Blood RFF encryption";
}

std::vector<std::string> RFFFilterType::getGameList() const
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Blood");
	return vcGames;
}

stream::inout_sptr RFFFilterType::apply(stream::inout_sptr target,
	stream::fn_truncate resize)
{
	stream::filtered_sptr st(new stream::filtered());
	// We need two separate filters, otherwise reading from one will
	// affect the XOR key next used when writing to the other.
	filter_sptr de(new filter_rff_crypt(RFF_FILE_CRYPT_LEN, 0));
	filter_sptr en(new filter_rff_crypt(RFF_FILE_CRYPT_LEN, 0));
	st->open(target, de, en, resize);
	return st;
}

stream::input_sptr RFFFilterType::apply(stream::input_sptr target)
{
	stream::input_filtered_sptr st(new stream::input_filtered());
	filter_sptr de(new filter_rff_crypt(RFF_FILE_CRYPT_LEN, 0));
	st->open(target, de);
	return st;
}

stream::output_sptr RFFFilterType::apply(stream::output_sptr target,
	stream::fn_truncate resize)
{
	stream::output_filtered_sptr st(new stream::output_filtered());
	filter_sptr en(new filter_rff_crypt(RFF_FILE_CRYPT_LEN, 0));
	st->open(target, en, resize);
	return st;
}

} // namespace gamearchive
} // namespace camoto
