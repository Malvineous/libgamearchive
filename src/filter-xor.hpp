/**
 * @file   filter-xor.hpp
 * @brief  Filter implementation for encrypting and decrypting XOR coded files.
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

#ifndef _CAMOTO_FILTER_XOR_HPP_
#define _CAMOTO_FILTER_XOR_HPP_

#include <boost/iostreams/concepts.hpp>     // multichar_dual_use_filter
#include <boost/iostreams/operations.hpp>   // read
#include <camoto/types.hpp>
#include <camoto/gamearchive/filtertype.hpp>

namespace camoto {
namespace gamearchive {

namespace io = boost::iostreams;

/// Encrypt a stream using XOR encryption.
/**
 * This starts by encrypting the first byte with the given seed value, then
 * the seed is incremented by one for the following byte.
 */
class xor_crypt_filter: public io::multichar_filter<io::seekable> {

	protected:
		/// Number of bytes to crypt, after this data is left as plaintext.
		/// 0 means crypt everything.
		int lenCrypt;

		/// Initial XOR value
		int seed;

		/// Current offset (number of bytes processed)
		int offset;

	public:

		struct category: io::multichar_seekable_filter_tag, io::flushable_tag { };

		xor_crypt_filter(int lenCrypt, int seed);

		template<typename Source>
		std::streamsize read(Source& src, char *s, std::streamsize n)
		{
			std::streamsize r = io::read(src, s, n);
			if (r == 0) return EOF;
			if (r < 0) return r;
			for (int i = 0; (i < r) && (
				(this->lenCrypt == 0) || (this->offset < this->lenCrypt)
			); i++) {
				*s++ ^= this->getKey();
				// We have to alter the offset here as its value is used by getKey()
				this->offset++;
			}
			return r;
		}

		template<typename Sink>
		std::streamsize write(Sink& dst, const char *s, std::streamsize n)
		{
			int r = 0;
			while (n--) {
				if (!io::put(dst, *s++ ^ this->getKey())) break;
				// We have to alter the offset here as its value is used by getKey()
				this->offset++;
				r++;
			}
			return r;
		}

		template<typename Source>
		io::stream_offset seek(Source& src, io::stream_offset off, std::ios::seekdir way)
		{
			io::stream_offset target = io::seek(src, off, way);
			if (target < 0) return target;
			this->offset = target;
			return this->offset;
		}

		template<typename Sink>
		bool flush(Sink& snk)
		{
			return true; // nothing to flush
		}

		/// Change the next XOR value
		void setSeed(int val);

		/// Get the next byte's seed value.
		/**
		 * This can be overridden by descendent classes to provide
		 * custom algorithms here.
		 *
		 * @param delta
		 *   Offset.  Normally +1 for each byte read, but can be more, or negative,
		 *   when seeking.
		 */
		virtual uint8_t getKey();

};

class XORFilterType: virtual public FilterType {

	public:
		XORFilterType()
			throw ();

		~XORFilterType()
			throw ();

		virtual std::string getFilterCode() const
			throw ();

		virtual std::string getFriendlyName() const
			throw ();

		virtual std::vector<std::string> getGameList() const
			throw ();

		virtual iostream_sptr apply(iostream_sptr target, FN_TRUNCATE fnTruncate)
			throw (ECorruptedData);

		virtual istream_sptr apply(istream_sptr target)
			throw (ECorruptedData);

		virtual ostream_sptr apply(ostream_sptr target, FN_TRUNCATE fnTruncate)
			throw (ECorruptedData);

};

} // namespace gamearchive
} // namespace camoto

#endif // _CAMOTO_FILTER_XOR_HPP_
