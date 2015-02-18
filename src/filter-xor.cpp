/**
 * @file  filter-xor.cpp
 * @brief Filter implementation for encrypting and decrypting XOR coded files.
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

#include "filter-xor.hpp"

namespace camoto {
namespace gamearchive {

filter_xor_crypt::filter_xor_crypt(int lenCrypt, int seed)
	:	lenCrypt(lenCrypt),
		seed(seed)
{
}

filter_xor_crypt::~filter_xor_crypt()
{
}

void filter_xor_crypt::reset(stream::len lenInput)
{
	this->offset = 0;
	return;
}

void filter_xor_crypt::transform(uint8_t *out, stream::len *lenOut,
	const uint8_t *in, stream::len *lenIn)
{
	stream::len w = 0;

	// Copy the crypted portion
	while (
		(w < *lenOut)
		&& (w < *lenIn)
		&& (
			(this->lenCrypt == 0)
			|| (this->offset < this->lenCrypt)
		)
	) {
		*out++ = *in++ ^ this->getKey();
		// We have to alter the offset here as its value is used by getKey()
		this->offset++;
		w++;
	}

	// Copy any plaintext portion
	stream::len rem = *lenIn - w;
	stream::len remOut = *lenOut - w;
	if (remOut < rem) rem = remOut;

	memcpy(out, in, rem);
	w += rem;
	*lenOut = w;
	*lenIn = w;

	return;
}

void filter_xor_crypt::setSeed(int val)
{
	this->seed = val;
	return;
}

uint8_t filter_xor_crypt::getKey()
{
	return (uint8_t)(this->seed + this->offset);
}


FilterType_XOR::FilterType_XOR()
{
}

FilterType_XOR::~FilterType_XOR()
{
}

std::string FilterType_XOR::code() const
{
	return "xor-inc";
}

std::string FilterType_XOR::friendlyName() const
{
	return "Incremental XOR encryption";
}

std::vector<std::string> FilterType_XOR::games() const
{
	return std::vector<std::string>();
}

std::unique_ptr<stream::inout> FilterType_XOR::apply(
	std::unique_ptr<stream::inout> target, stream::fn_notify_prefiltered_size resize)
	const
{
	return std::make_unique<stream::filtered>(
		std::move(target),
		// We need two separate filters, otherwise reading from one will
		// affect the XOR key next used when writing to the other.
		std::make_shared<filter_xor_crypt>(0, 0),
		std::make_shared<filter_xor_crypt>(0, 0),
		resize
	);
}

std::unique_ptr<stream::input> FilterType_XOR::apply(
	std::unique_ptr<stream::input> target) const
{
	return std::make_unique<stream::input_filtered>(
		std::move(target),
		std::make_shared<filter_xor_crypt>(0, 0)
	);
}

std::unique_ptr<stream::output> FilterType_XOR::apply(
	std::unique_ptr<stream::output> target, stream::fn_notify_prefiltered_size resize)
	const
{
	return std::make_unique<stream::output_filtered>(
		std::move(target),
		std::make_shared<filter_xor_crypt>(0, 0),
		resize
	);
}

} // namespace gamearchive
} // namespace camoto
