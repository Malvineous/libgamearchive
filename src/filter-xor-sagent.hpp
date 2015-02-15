/**
 * @file  filter-xor-sagent.cpp
 * @brief Filter that encrypts and decrypts Secret Agent data files.
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
class filter_sam_crypt: public filter_xor_crypt
{
	public:
		filter_sam_crypt(int resetInterval);
		virtual uint8_t getKey();

	protected:
		/// How many bytes to decode before jumping back to the start of the key
		int resetInterval;
};

class FilterType_SAM_Base: virtual public FilterType
{
	public:
		FilterType_SAM_Base(int resetInterval);
		virtual ~FilterType_SAM_Base();

		virtual std::string code() const;
		virtual std::string friendlyName() const;
		virtual std::vector<std::string> games() const;
		virtual std::unique_ptr<stream::inout> apply(std::shared_ptr<stream::inout> target,
			stream::fn_truncate_filter resize) const;
		virtual std::unique_ptr<stream::input> apply(std::shared_ptr<stream::input> target) const;
		virtual std::unique_ptr<stream::output> apply(std::shared_ptr<stream::output> target,
			stream::fn_truncate_filter resize) const;

	protected:
		int resetInterval;
};

class FilterType_SAM_Map: virtual public FilterType_SAM_Base
{
	public:
		FilterType_SAM_Map();
		virtual ~FilterType_SAM_Map();

		virtual std::string code() const;
		virtual std::string friendlyName() const;
};

class FilterType_SAM_8Sprite: virtual public FilterType_SAM_Base
{
	public:
		FilterType_SAM_8Sprite();
		virtual ~FilterType_SAM_8Sprite();

		virtual std::string code() const;
		virtual std::string friendlyName() const;
};

class FilterType_SAM_16Sprite: virtual public FilterType_SAM_Base
{
	public:
		FilterType_SAM_16Sprite();
		virtual ~FilterType_SAM_16Sprite();

		virtual std::string code() const;
		virtual std::string friendlyName() const;
};

} // namespace gamearchive
} // namespace camoto

#endif // _CAMOTO_FILTER_XOR_SAGENT_HPP_
