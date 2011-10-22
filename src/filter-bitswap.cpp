/**
 * @file   filter-bitswap.cpp
 * @brief  Filter implementation for swapping the bits in each byte.
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

#include "filter-bitswap.hpp"

namespace camoto {
namespace gamearchive {

void filter_bitswap::transform(uint8_t *out, stream::len *lenOut,
	const uint8_t *in, stream::len *lenIn)
	throw (filter_error)
{
	stream::len w; /// number of bytes copied

	for (w = 0; (w < *lenOut) && (w < *lenIn); w++) {
		*out++ =
			((*in >> 7) & 1) |
			((*in >> 5) & 2) |
			((*in >> 3) & 4) |
			((*in >> 1) & 8) |
			((*in << 1) & 16) |
			((*in << 3) & 32) |
			((*in << 5) & 64) |
			((*in << 7) & 128)
		;
		in++;
	}

	*lenOut = w;
	*lenIn = w;

	return;
}

} // namespace gamearchive
} // namespace camoto
