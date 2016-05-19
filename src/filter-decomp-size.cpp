/**
 * @file  filter-decomp-size.cpp
 * @brief Filter for handing first few bytes as decompressed file size.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/Dangerous_Dave_Graphics_Format
 *
 * Copyright (C) 2010-2016 Adam Nielsen <malvineous@shikadi.net>
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

#include <algorithm>
#include <cassert>
#include <camoto/filter.hpp>
#include <camoto/util.hpp> // std::make_unique
#include "filter-decomp-size.hpp"

namespace camoto {
namespace gamearchive {

filter_decomp_size_remove::filter_decomp_size_remove(
	std::unique_ptr<filter> childFilter)
	:	childFilter(std::move(childFilter))
{
}

void filter_decomp_size_remove::reset(stream::len lenInput)
{
	this->lenTarget = -1;
	return;
}

void filter_decomp_size_remove::transform(uint8_t *out, stream::len *lenOut,
	const uint8_t *in, stream::len *lenIn)
{
	stream::len r = 0, w = 0;

	if ((lenTarget < 0) && (*lenIn >= 4)) {
		this->lenTarget =
			in[0]
			| (in[1] << 8)
			| (in[2] << 16)
			| (in[3] << 24)
		;
		r += 4;
		in += 4;
		*lenIn -= 4;
	}

	if (this->lenTarget > 0) {
		auto origLenOut = *lenOut;
		*lenOut = std::min(*lenOut, (stream::len)this->lenTarget);
		this->childFilter->transform(out, lenOut, in, lenIn);
		r += *lenIn;
		w += *lenOut;
		this->lenTarget -= *lenOut;

		// Would need to adjust this, except we'll only use it when the adjustment
		// is zero, so don't bother adjusting.
		//origLenOut -= *lenOut;
		//out += *lenOut;
		if ((*lenIn == 0) && (*lenOut == 0) && (this->lenTarget > 0)) {
			// Child filter is done, no more to read or write, but the output size
			// hasn't been reached yet, so zero-pad the data.
			auto amount = std::min(origLenOut, (stream::len)this->lenTarget);
			memset(out, 0, amount);
			this->lenTarget -= amount;
			w += amount;
		}
	}

	*lenIn = r;
	*lenOut = w;
	return;
}


filter_decomp_size_insert::filter_decomp_size_insert(
	std::unique_ptr<filter> childFilter)
	:	childFilter(std::move(childFilter))
{
}

void filter_decomp_size_insert::reset(stream::len lenInput)
{
	this->lenInput = lenInput;
	return;
}

void filter_decomp_size_insert::transform(uint8_t *out, stream::len *lenOut,
	const uint8_t *in, stream::len *lenIn)
{
	if (this->lenInput >= 0) {
		*lenIn = 0;
		if (*lenOut >= 4) {
			out[0] =  this->lenInput        & 0xFF;
			out[1] = (this->lenInput >>  8) & 0xFF;
			out[2] = (this->lenInput >> 16) & 0xFF;
			out[3] = (this->lenInput >> 24) & 0xFF;
			*lenOut = 4;
			this->lenInput = -1; // we've written the length field now
		} else {
			*lenOut = 0;
		}
		return;
	}

	this->childFilter->transform(out, lenOut, in, lenIn);
	return;
}

} // namespace gamearchive
} // namespace camoto
