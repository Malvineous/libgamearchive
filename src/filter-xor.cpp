/**
 * @file   filter-xor.cpp
 * @brief  Filter implementation for encrypting and decrypting XOR coded files.
 *
 * Copyright (C) 2010-2013 Adam Nielsen <malvineous@shikadi.net>
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
#include <camoto/stream_filtered.hpp>
#include <camoto/bitstream.hpp>

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

void filter_xor_crypt::reset()
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


XORFilterType::XORFilterType()
{
}

XORFilterType::~XORFilterType()
{
}

std::string XORFilterType::getFilterCode() const
{
	return "xor-inc";
}

std::string XORFilterType::getFriendlyName() const
{
	return "Incremental XOR encryption";
}

std::vector<std::string> XORFilterType::getGameList() const
{
	return std::vector<std::string>();
}

stream::inout_sptr XORFilterType::apply(stream::inout_sptr target,
	stream::fn_truncate resize) const
{
	stream::filtered_sptr st(new stream::filtered());
	// We need two separate filters, otherwise reading from one will
	// affect the XOR key next used when writing to the other.
	filter_sptr de(new filter_xor_crypt(0, 0));
	filter_sptr en(new filter_xor_crypt(0, 0));
	st->open(target, de, en, resize);
	return st;
}

stream::input_sptr XORFilterType::apply(stream::input_sptr target) const
{
	stream::input_filtered_sptr st(new stream::input_filtered());
	filter_sptr de(new filter_xor_crypt(0, 0));
	st->open(target, de);
	return st;
}

stream::output_sptr XORFilterType::apply(stream::output_sptr target,
	stream::fn_truncate resize) const
{
	stream::output_filtered_sptr st(new stream::output_filtered());
	filter_sptr en(new filter_xor_crypt(0, 0));
	st->open(target, en, resize);
	return st;
}

} // namespace gamearchive
} // namespace camoto
