/**
 * @file  filter-glb-raptor.cpp
 * @brief Filter that encrypts and decrypts files in Raptor GLB archives.
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
#include "filter-glb-raptor.hpp"

namespace camoto {
namespace gamearchive {

/// Key to use for .GLB files
#define GLB_KEY "32768GLB"

/// Length of each cipher block in the .GLB FAT
#define GLB_BLOCKLEN 28

filter_glb_decrypt::filter_glb_decrypt(const std::string& key, int lenBlock)
	:	lenBlock(lenBlock),
		key(key),
		lenKey(key.length()),
		// posKey in reset()
		offset(0)
		// lastByte in reset()
{
	this->reset(0);
}

filter_glb_decrypt::~filter_glb_decrypt()
{
}

void filter_glb_decrypt::reset(stream::len lenInput)
{
	this->posKey = 25 % this->lenKey;
	this->lastByte = this->key[this->posKey];
	return;
}

void filter_glb_decrypt::transform(uint8_t *out, stream::len *lenOut,
	const uint8_t *in, stream::len *lenIn)
{
	stream::len lenRemaining = std::min(*lenIn, *lenOut);
	*lenIn = 0;
	*lenOut = 0;
	while (lenRemaining--) {
		// Reset the cipher if the block length has been reached
		if (this->lenBlock != 0) {
			if ((this->offset % this->lenBlock) == 0) {
				this->reset(0);
			}
		}

		*out = (*in - this->key[this->posKey] - this->lastByte) & 0xFF;
		this->posKey++;
		this->posKey %= this->lenKey;
		this->lastByte = *in;
		out++;
		in++;
		(*lenIn)++;
		(*lenOut)++;
		this->offset++;
	}
	return;
}

filter_glb_encrypt::filter_glb_encrypt(const std::string& key, int lenBlock)
	:	lenBlock(lenBlock),
		key(key),
		lenKey(key.length()),
		// posKey in reset()
		offset(0)
		// lastByte in reset()
{
	this->reset(0);
}

filter_glb_encrypt::~filter_glb_encrypt()
{
}

void filter_glb_encrypt::reset(stream::len lenInput)
{
	this->posKey = 25 % this->lenKey;
	this->lastByte = this->key[this->posKey];
	return;
}

void filter_glb_encrypt::transform(uint8_t *out, stream::len *lenOut,
	const uint8_t *in, stream::len *lenIn)
{
	stream::len lenRemaining = std::min(*lenIn, *lenOut);
	*lenIn = 0;
	*lenOut = 0;
	while (lenRemaining--) {
		// Reset the cipher if the block length has been reached
		if (this->lenBlock != 0) {
			if ((this->offset % this->lenBlock) == 0) {
				this->reset(0);
			}
		}

		*out = (*in + this->lastByte + this->key[this->posKey]) & 0xFF;
		this->posKey++;
		this->posKey %= this->lenKey;
		this->lastByte = *out;//in;
		out++;
		in++;
		(*lenIn)++;
		(*lenOut)++;
		this->offset++;
	}
	return;
}


FilterType_GLB_Raptor_FAT::FilterType_GLB_Raptor_FAT()
{
}

FilterType_GLB_Raptor_FAT::~FilterType_GLB_Raptor_FAT()
{
}

std::string FilterType_GLB_Raptor_FAT::code() const
{
	return "glb-raptor-fat";
}

std::string FilterType_GLB_Raptor_FAT::friendlyName() const
{
	return "Raptor GLB FAT encryption";
}

std::vector<std::string> FilterType_GLB_Raptor_FAT::games() const
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Raptor");
	return vcGames;
}

std::unique_ptr<stream::inout> FilterType_GLB_Raptor_FAT::apply(
	std::shared_ptr<stream::inout> target, stream::fn_truncate_filter resize)
	const
{
	return std::make_unique<stream::filtered>(
		target,
		std::make_shared<filter_glb_decrypt>(GLB_KEY, GLB_BLOCKLEN),
		std::make_shared<filter_glb_encrypt>(GLB_KEY, GLB_BLOCKLEN),
		resize
	);
}

std::unique_ptr<stream::input> FilterType_GLB_Raptor_FAT::apply(
	std::shared_ptr<stream::input> target) const
{
	return std::make_unique<stream::input_filtered>(
		target,
		std::make_shared<filter_glb_decrypt>(GLB_KEY, GLB_BLOCKLEN)
	);
}

std::unique_ptr<stream::output> FilterType_GLB_Raptor_FAT::apply(
	std::shared_ptr<stream::output> target, stream::fn_truncate_filter resize)
	const
{
	return std::make_unique<stream::output_filtered>(
		target,
		std::make_shared<filter_glb_encrypt>(GLB_KEY, GLB_BLOCKLEN),
		resize
	);
}


FilterType_GLB_Raptor_File::FilterType_GLB_Raptor_File()
{
}

FilterType_GLB_Raptor_File::~FilterType_GLB_Raptor_File()
{
}

std::string FilterType_GLB_Raptor_File::code() const
{
	return "glb-raptor";
}

std::string FilterType_GLB_Raptor_File::friendlyName() const
{
	return "Raptor GLB file encryption";
}

std::vector<std::string> FilterType_GLB_Raptor_File::games() const
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Raptor");
	return vcGames;
}

std::unique_ptr<stream::inout> FilterType_GLB_Raptor_File::apply(
	std::shared_ptr<stream::inout> target, stream::fn_truncate_filter resize)
	const
{
	return std::make_unique<stream::filtered>(
		target,
		std::make_shared<filter_glb_decrypt>(GLB_KEY, 0),
		std::make_shared<filter_glb_encrypt>(GLB_KEY, 0),
		resize
	);
}

std::unique_ptr<stream::input> FilterType_GLB_Raptor_File::apply(
	std::shared_ptr<stream::input> target) const
{
	return std::make_unique<stream::input_filtered>(
		target,
		std::make_shared<filter_glb_decrypt>(GLB_KEY, 0)
	);
}

std::unique_ptr<stream::output> FilterType_GLB_Raptor_File::apply(
	std::shared_ptr<stream::output> target, stream::fn_truncate_filter resize)
	const
{
	return std::make_unique<stream::output_filtered>(
		target,
		std::make_shared<filter_glb_encrypt>(GLB_KEY, 0),
		resize
	);
}

} // namespace gamearchive
} // namespace camoto
