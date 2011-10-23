/**
 * @file   filter-xor-blood.hpp
 * @brief  Filter that encrypts and decrypts Secret Agent data files.
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

#ifndef _CAMOTO_FILTER_XOR_SAGENT_HPP_
#define _CAMOTO_FILTER_XOR_SAGENT_HPP_

#include <camoto/stream.hpp>
#include <stdint.h>
#include <camoto/gamearchive/filtertype.hpp>

#include "filter-xor.hpp"

namespace camoto {
namespace gamearchive {

/// Encrypt a stream using XOR encryption, with a fixed key.
class sam_crypt_filter: public filter_xor_crypt {

	public:
		sam_crypt_filter(int resetInterval);
		virtual uint8_t getKey();

	protected:
		int resetInterval;

};

class SAMBaseFilterType: virtual public FilterType {

	public:
		SAMBaseFilterType(int resetInterval)
			throw ();

		virtual ~SAMBaseFilterType()
			throw ();

		virtual std::string getFilterCode() const
			throw ();

		virtual std::string getFriendlyName() const
			throw ();

		virtual std::vector<std::string> getGameList() const
			throw ();

		virtual stream::inout_sptr apply(stream::inout_sptr target,
			stream::fn_truncate resize)
			throw (filter_error, stream::read_error);

		virtual stream::input_sptr apply(stream::input_sptr target)
			throw (filter_error, stream::read_error);

		virtual stream::output_sptr apply(stream::output_sptr target,
			stream::fn_truncate resize)
			throw (filter_error);

	protected:
		int resetInterval;

};

class SAMMapFilterType: virtual public SAMBaseFilterType {

	public:
		SAMMapFilterType()
			throw ();

		virtual ~SAMMapFilterType()
			throw ();

		virtual std::string getFilterCode() const
			throw ();

		virtual std::string getFriendlyName() const
			throw ();

};

class SAM8SpriteFilterType: virtual public SAMBaseFilterType {

	public:
		SAM8SpriteFilterType()
			throw ();

		virtual ~SAM8SpriteFilterType()
			throw ();

		virtual std::string getFilterCode() const
			throw ();

		virtual std::string getFriendlyName() const
			throw ();

};

class SAM16SpriteFilterType: virtual public SAMBaseFilterType {

	public:
		SAM16SpriteFilterType()
			throw ();

		virtual ~SAM16SpriteFilterType()
			throw ();

		virtual std::string getFilterCode() const
			throw ();

		virtual std::string getFriendlyName() const
			throw ();

};

} // namespace gamearchive
} // namespace camoto

#endif // _CAMOTO_FILTER_XOR_SAGENT_HPP_
