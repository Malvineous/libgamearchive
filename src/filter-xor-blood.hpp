/**
 * @file  filter-xor-blood.hpp
 * @brief Filter that encrypts and decrypts data in Blood RFF archives.
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

#ifndef _CAMOTO_FILTER_XOR_BLOOD_HPP_
#define _CAMOTO_FILTER_XOR_BLOOD_HPP_

#include <camoto/stream.hpp>
#include <stdint.h>
#include <camoto/gamearchive/filtertype.hpp>

#include "filter-xor.hpp"

namespace camoto {
namespace gamearchive {

/// Encrypt a stream using XOR encryption, incrementing the key only every
/// second byte.
class filter_rff_crypt: public filter_xor_crypt
{
	public:
		filter_rff_crypt(int lenCrypt, int seed);

		virtual uint8_t getKey();
};

class RFFFilterType: virtual public FilterType
{
	public:
		RFFFilterType();
		~RFFFilterType();

		virtual std::string getFilterCode() const;
		virtual std::string getFriendlyName() const;
		virtual std::vector<std::string> getGameList() const;
		virtual stream::inout_sptr apply(stream::inout_sptr target,
			stream::fn_truncate resize) const;
		virtual stream::input_sptr apply(stream::input_sptr target) const;
		virtual stream::output_sptr apply(stream::output_sptr target,
			stream::fn_truncate resize) const;
};

} // namespace gamearchive
} // namespace camoto

#endif // _CAMOTO_FILTER_XOR_BLOOD_HPP_
