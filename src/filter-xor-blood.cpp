/**
 * @file  filter-xor-blood.cpp
 * @brief Filter that encrypts and decrypts files in Blood RFF archives.
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


FilterType_RFF::FilterType_RFF()
{
}

FilterType_RFF::~FilterType_RFF()
{
}

std::string FilterType_RFF::code() const
{
	return "xor-blood";
}

std::string FilterType_RFF::friendlyName() const
{
	return "Blood RFF encryption";
}

std::vector<std::string> FilterType_RFF::games() const
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Blood");
	return vcGames;
}

std::unique_ptr<stream::inout> FilterType_RFF::apply(
	std::shared_ptr<stream::inout> target, stream::fn_truncate_filter resize)
	const
{
	return std::make_unique<stream::filtered>(
		target,
		// We need two separate filters, otherwise reading from one will
		// affect the XOR key next used when writing to the other.
		std::make_shared<filter_rff_crypt>(RFF_FILE_CRYPT_LEN, 0),
		std::make_shared<filter_rff_crypt>(RFF_FILE_CRYPT_LEN, 0),
		resize
	);
}

std::unique_ptr<stream::input> FilterType_RFF::apply(
	std::shared_ptr<stream::input> target) const
{
	return std::make_unique<stream::input_filtered>(
		target,
		std::make_shared<filter_rff_crypt>(RFF_FILE_CRYPT_LEN, 0)
	);
}

std::unique_ptr<stream::output> FilterType_RFF::apply(
	std::shared_ptr<stream::output> target, stream::fn_truncate_filter resize)
	const
{
	return std::make_unique<stream::output_filtered>(
		target,
		std::make_shared<filter_rff_crypt>(RFF_FILE_CRYPT_LEN, 0),
		resize
	);
}

} // namespace gamearchive
} // namespace camoto
