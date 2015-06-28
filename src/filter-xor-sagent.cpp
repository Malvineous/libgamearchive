/**
 * @file  filter-xor-sagent.cpp
 * @brief Filter that encrypts and decrypts Secret Agent data files.
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
#include "filter-xor-sagent.hpp"
#include "filter-bitswap.hpp"

namespace camoto {
namespace gamearchive {

#define SAM_KEY     "Copyright 1991 Peder Jungck"
#define SAM_KEYLEN  (strlen(SAM_KEY)+1)  // include terminating null

const char sam_key[] = SAM_KEY;

filter_sam_crypt::filter_sam_crypt(int resetInterval)
	:	filter_xor_crypt(0, 0),
		resetInterval(resetInterval)
{
}

uint8_t filter_sam_crypt::getKey()
{
	if ((this->resetInterval == 42) && ((this->offset % 42) == 41)) {
		// Special case for last char in each row of map file
		return 0;
	}
	return (uint8_t)(sam_key[(this->offset % this->resetInterval) % SAM_KEYLEN]);
}


FilterType_SAM_Base::FilterType_SAM_Base(int resetInterval)
	:	resetInterval(resetInterval)
{
}

FilterType_SAM_Base::~FilterType_SAM_Base()
{
}

std::string FilterType_SAM_Base::code() const
{
	return "xor-sagent";
}

std::string FilterType_SAM_Base::friendlyName() const
{
	return "Secret Agent XOR encryption";
}

std::vector<std::string> FilterType_SAM_Base::games() const
{
	return {
		"Secret Agent",
	};
}

std::unique_ptr<stream::inout> FilterType_SAM_Base::apply(
	std::unique_ptr<stream::inout> target, stream::fn_notify_prefiltered_size resize)
	const
{
	auto fswap = std::make_shared<filter_bitswap>();

	return std::make_unique<stream::filtered>(
		std::make_unique<stream::filtered>(
			std::move(target),
			// Since the bitswap doesn't care how many bytes have been read or
			// written, we can use the same filter for both reading and writing.
			fswap,
			fswap,
			resize
		),
		// We need two separate filters, otherwise reading from one will
		// affect the XOR key next used when writing to the other.
		std::make_shared<filter_sam_crypt>(this->resetInterval),
		std::make_shared<filter_sam_crypt>(this->resetInterval),
		stream::fn_notify_prefiltered_size()
	);
}

std::unique_ptr<stream::input> FilterType_SAM_Base::apply(
	std::unique_ptr<stream::input> target) const
{
	auto fswap = std::make_shared<filter_bitswap>();

	return std::make_unique<stream::input_filtered>(
		std::make_unique<stream::input_filtered>(
			std::move(target),
			fswap
		),
		std::make_shared<filter_sam_crypt>(this->resetInterval)
	);
}

std::unique_ptr<stream::output> FilterType_SAM_Base::apply(
	std::unique_ptr<stream::output> target, stream::fn_notify_prefiltered_size resize)
	const
{
	auto fswap = std::make_shared<filter_bitswap>();

	return std::make_unique<stream::output_filtered>(
		std::make_unique<stream::output_filtered>(
			std::move(target),
			fswap,
			resize
		),
		std::make_shared<filter_sam_crypt>(this->resetInterval),
		stream::fn_notify_prefiltered_size()
	);
}


FilterType_SAM_Map::FilterType_SAM_Map()
	:	FilterType_SAM_Base(42)
{
}

FilterType_SAM_Map::~FilterType_SAM_Map()
{
}

std::string FilterType_SAM_Map::code() const
{
	return "xor-sagent-map";
}

std::string FilterType_SAM_Map::friendlyName() const
{
	return "Secret Agent XOR encryption (map file)";
}


FilterType_SAM_8Sprite::FilterType_SAM_8Sprite()
	:	FilterType_SAM_Base(2048)
{
}

FilterType_SAM_8Sprite::~FilterType_SAM_8Sprite()
{
}

std::string FilterType_SAM_8Sprite::code() const
{
	return "xor-sagent-8sprite";
}

std::string FilterType_SAM_8Sprite::friendlyName() const
{
	return "Secret Agent XOR encryption (8x8 sprite file)";
}


FilterType_SAM_16Sprite::FilterType_SAM_16Sprite()
	:	FilterType_SAM_Base(8064)
{
}

FilterType_SAM_16Sprite::~FilterType_SAM_16Sprite()
{
}

std::string FilterType_SAM_16Sprite::code() const
{
	return "xor-sagent-16sprite";
}

std::string FilterType_SAM_16Sprite::friendlyName() const
{
	return "Secret Agent XOR encryption (16x16 sprite file)";
}

} // namespace gamearchive
} // namespace camoto
