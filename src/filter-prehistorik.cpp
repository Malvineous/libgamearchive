/**
 * @file  filter-prehistorik.cpp
 * @brief Filter implementation for decompressing Prehistorik files.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/Prehistorik
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

#include <camoto/iostream_helpers.hpp>
#include <camoto/stream_filtered.hpp>
#include <camoto/util.hpp> // std::make_unique
#include <camoto/lzss.hpp>
#include <camoto/filter-crop.hpp>
#include <camoto/filter-pad.hpp>
#include <camoto/stream_sub.hpp>

#include "filter-prehistorik.hpp"

namespace camoto {
namespace gamearchive {

/// Length of the field storing the decompressed file size
#define PH_DECOMP_LEN 4

FilterType_Prehistorik::FilterType_Prehistorik()
{
}

FilterType_Prehistorik::~FilterType_Prehistorik()
{
}

std::string FilterType_Prehistorik::code() const
{
	return "lzss-prehistorik";
}

std::string FilterType_Prehistorik::friendlyName() const
{
	return "Prehistorik compression";
}

std::vector<std::string> FilterType_Prehistorik::games() const
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Prehistorik");
	return vcGames;
}

std::unique_ptr<stream::inout> FilterType_Prehistorik::apply(
	std::unique_ptr<stream::inout> target, stream::fn_notify_prefiltered_size resize)
	const
{
	auto filtPad = std::make_shared<filter_pad>();
	std::shared_ptr<stream::inout> target_sh(std::move(target));
	auto st1 = std::make_unique<stream::filtered>(
		target_sh,
		std::make_shared<filter_crop>(PH_DECOMP_LEN),
		filtPad,
		stream::fn_notify_prefiltered_size()
	);

	return std::make_unique<stream::filtered>(
		std::move(st1),
		std::make_shared<filter_lzss_decompress>(bitstream::bigEndian, 2, 8),
		std::make_shared<filter_lzss_compress>(bitstream::bigEndian, 2, 8),
		[resize, filtPad](stream::output_filtered* s, stream::len newSize) {
			// Write the prefiltered size to the start of the original stream
			filtPad->pad.seekp(0, stream::start);
			filtPad->pad << u32be(newSize);
			// Call the original notification function
			if (resize) resize(s, newSize);
		}
	);
}

std::unique_ptr<stream::input> FilterType_Prehistorik::apply(
	std::unique_ptr<stream::input> target) const
{
	auto st1 = std::make_unique<stream::input_filtered>(
		std::move(target),
		std::make_shared<filter_crop>(PH_DECOMP_LEN)
	);

	return std::make_unique<stream::input_filtered>(
		std::move(st1),
		std::make_shared<filter_lzss_decompress>(bitstream::bigEndian, 2, 8)
	);
}

std::unique_ptr<stream::output> FilterType_Prehistorik::apply(
	std::unique_ptr<stream::output> target, stream::fn_notify_prefiltered_size resize)
	const
{
	auto filtPad = std::make_shared<filter_pad>();
	auto st1 = std::make_unique<stream::output_filtered>(
		std::move(target),
		filtPad,
		stream::fn_notify_prefiltered_size()
	);

	return std::make_unique<stream::output_filtered>(
		std::move(st1),
		std::make_shared<filter_lzss_compress>(bitstream::bigEndian, 2, 8),
		[resize, filtPad](stream::output_filtered* s, stream::len newSize) {
			// Write the prefiltered size to the start of the original stream
			filtPad->pad.seekp(0, stream::start);
			filtPad->pad << u32be(newSize);
			// Call the original notification function
			if (resize) resize(s, newSize);
		}
	);
}

} // namespace gamearchive
} // namespace camoto
