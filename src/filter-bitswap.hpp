/**
 * @file   filter-bitswap.hpp
 * @brief  Filter implementation for swapping the bits in each byte.
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

#ifndef _CAMOTO_FILTER_BITSWAP_HPP_
#define _CAMOTO_FILTER_BITSWAP_HPP_

#include <camoto/filter.hpp>

namespace camoto {
namespace gamearchive {

/// Encrypt a stream by swapping all the bits in each byte.
class filter_bitswap: virtual public filter
{
	public:
		virtual void reset(stream::len lenInput);
		virtual void transform(uint8_t *out, stream::len *lenOut,
			const uint8_t *in, stream::len *lenIn);
};

} // namespace gamearchive
} // namespace camoto

#endif // _CAMOTO_FILTER_BITSWAP_HPP_
