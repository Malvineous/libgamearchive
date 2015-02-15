/**
 * @file  filter-xor.hpp
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

#ifndef _CAMOTO_FILTER_XOR_HPP_
#define _CAMOTO_FILTER_XOR_HPP_

#include <camoto/filter.hpp>
#include <camoto/gamearchive/filtertype.hpp>

namespace camoto {
namespace gamearchive {

/// Encrypt data using XOR encryption.
/**
 * This starts by encrypting the first byte with the given seed value, then
 * the seed is incremented by one for the following byte.
 */
class filter_xor_crypt: virtual public filter
{
	protected:
		/// Number of bytes to crypt, after this data is left as plaintext.
		/// 0 means crypt everything.
		int lenCrypt;

		/// Initial XOR value
		int seed;

		/// Current offset (number of bytes processed)
		int offset;

	public:
		/// Create a new encryption filter with the given options.
		/**
		 * @param lenCrypt
		 *   Number of bytes to crypt, after this data is left as plaintext.
		 *   0 means crypt everything.
		 *
		 * @param seed
		 *   Initial XOR value.
		 */
		filter_xor_crypt(int lenCrypt, int seed);
		virtual ~filter_xor_crypt();

		virtual void reset(stream::len lenInput);
		virtual void transform(uint8_t *out, stream::len *lenOut,
			const uint8_t *in, stream::len *lenIn);

		/// Change the next XOR value
		void setSeed(int val);

		/// Get the next byte's seed value.
		/**
		 * This can be overridden by descendent classes to provide
		 * custom algorithms here.
		 */
		virtual uint8_t getKey();
};

/// Encrypt a stream using XOR encryption.
class FilterType_XOR: virtual public FilterType
{
	public:
		FilterType_XOR();
		~FilterType_XOR();

		virtual std::string code() const;
		virtual std::string friendlyName() const;
		virtual std::vector<std::string> games() const;
		virtual std::unique_ptr<stream::inout> apply(std::shared_ptr<stream::inout> target,
			stream::fn_truncate_filter resize) const;
		virtual std::unique_ptr<stream::input> apply(std::shared_ptr<stream::input> target) const;
		virtual std::unique_ptr<stream::output> apply(std::shared_ptr<stream::output> target,
			stream::fn_truncate_filter resize) const;
};

} // namespace gamearchive
} // namespace camoto

#endif // _CAMOTO_FILTER_XOR_HPP_
