/**
 * @file   filter-xor-sagent.cpp
 * @brief  Filter that encrypts and decrypts Secret Agent data files.
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

#ifndef _CAMOTO_FILTER_XOR_SAGENT_HPP_
#define _CAMOTO_FILTER_XOR_SAGENT_HPP_

#include <stdint.h>
#include <camoto/gamearchive/filtertype.hpp>
#include "filter-xor.hpp"

namespace camoto {
namespace gamearchive {

/// Encrypt a stream using XOR encryption, with a fixed key.
class sam_crypt_filter: public filter_xor_crypt
{
	public:
		sam_crypt_filter(int resetInterval);
		virtual uint8_t getKey();

	protected:
		/// How many bytes to decode before jumping back to the start of the key
		int resetInterval;
};

class SAMBaseFilterType: virtual public FilterType
{
	public:
		SAMBaseFilterType(int resetInterval);
		virtual ~SAMBaseFilterType();

		virtual std::string getFilterCode() const;
		virtual std::string getFriendlyName() const;
		virtual std::vector<std::string> getGameList() const;
		virtual stream::inout_sptr apply(stream::inout_sptr target,
			stream::fn_truncate resize) const;
		virtual stream::input_sptr apply(stream::input_sptr target) const;
		virtual stream::output_sptr apply(stream::output_sptr target,
			stream::fn_truncate resize) const;

	protected:
		int resetInterval;
};

class SAMMapFilterType: virtual public SAMBaseFilterType
{
	public:
		SAMMapFilterType();
		virtual ~SAMMapFilterType();

		virtual std::string getFilterCode() const;
		virtual std::string getFriendlyName() const;
};

class SAM8SpriteFilterType: virtual public SAMBaseFilterType
{
	public:
		SAM8SpriteFilterType();
		virtual ~SAM8SpriteFilterType();

		virtual std::string getFilterCode() const;
		virtual std::string getFriendlyName() const;
};

class SAM16SpriteFilterType: virtual public SAMBaseFilterType
{
	public:
		SAM16SpriteFilterType();
		virtual ~SAM16SpriteFilterType();

		virtual std::string getFilterCode() const;
		virtual std::string getFriendlyName() const;
};

} // namespace gamearchive
} // namespace camoto

#endif // _CAMOTO_FILTER_XOR_SAGENT_HPP_
