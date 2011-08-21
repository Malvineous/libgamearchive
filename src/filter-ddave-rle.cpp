/**
 * @file   filter-ddave-rle.cpp
 * @brief  Filter implementation for decompressing Dangerous Dave tilesets.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/Dangerous_Dave_Graphics_Format
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

#include <boost/iostreams/concepts.hpp>     // multichar_outputput_filter
#include <boost/iostreams/invert.hpp>
#include <boost/bind.hpp>
#include <camoto/iostream_helpers.hpp>
#include <camoto/filteredstream.hpp>

#include "filter-ddave-rle.hpp"

namespace camoto {
namespace gamearchive {

/// Dangerous Dave RLE expansion filter.
class ddave_unrle_filter: public io::multichar_input_filter {

	protected:
		int count;         ///< How many times to repeat prev
		uint8_t countByte; ///< Byte being repeated count times
		int copying;       ///< Number of bytes left to copy unchanged

	public:
		ddave_unrle_filter() :
			count(0),
			copying(0)
		{
		};

		template<typename Source>
		std::streamsize read(Source& src, char* s, std::streamsize n)
		{
			int r = 0;
			while (r < n) {
				// If there is an RLE decode in progress
				if (this->count) {
					*s++ = this->countByte;
					this->count--;
					r++;
				} else {
					// Otherwise no RLE decode in progress, keep reading
					int c = boost::iostreams::get(src);
					if (c < 0) {
						if (r == 0) return c; // no bytes read, return error
						break; // bytes read, return those
					}

					if (this->copying) {
						*s++ = c;
						r++;
						this->copying--;
					} else if (c & 0x80) { // high bit set
						this->copying = 1 + (c & 0x7F);
					} else { // high bit unset
						this->count = 3 + c;
						int c = boost::iostreams::get(src);
						if (c < 0) {
							this->count = 0;
							if (r == 0) return c; // no bytes read, return error
							break; // bytes read, return those
						}
						this->countByte = c;
					}
				}
			}
			return r;
		}

};

/// Dangerous Dave RLE compression filter.
class ddave_rle_filter: public io::multichar_output_filter {

	protected:
		char buf[129];  ///< Chars to output as-is
		int buflen;     ///< Number of valid chars in buf

		int prev;       ///< Previous byte read
		int count;      ///< How many prev has been seen so far

	public:

		struct category: io::multichar_output_filter_tag, io::flushable_tag { };

		ddave_rle_filter() :
			buflen(0),
			prev(-1),
			count(0)
		{
		};

		template<typename Sink>
		void charChanged(Sink& dst)
		{
			if (this->count >= 3) {
				// Got some repeated bytes to write
				assert(this->count <= 130);
				boost::iostreams::put(dst, (char)(this->count - 3));
				boost::iostreams::put(dst, this->prev);
				this->count = 0;
			}
			// May have some repeated bytes, but they're too short for an RLE code
			while (this->count) {
				assert(this->buflen < 128);
				this->buf[this->buflen++] = this->prev;
				if (this->buflen == 128) {
					// Buffer is full, write out a code
					this->flushBuffer(dst);
				}
				this->count--;
			}
			assert(this->buflen < 128);
			return;
		}

		template<typename Sink>
		void flushBuffer(Sink& dst)
		{
			if (this->buflen > 0) {
				boost::iostreams::put(dst, (char)(0x80 + this->buflen - 1));
				boost::iostreams::write(dst, this->buf, this->buflen);
				this->buflen = 0;
			}
			return;
		}

		template<typename Sink>
		std::streamsize write(Sink& dst, const char* s, std::streamsize n)
		{
			if (n < 1) return 0; // just in case
			const char *start = s;
			const char *end = s + n;
			while (s < end) {
				if (*s == this->prev) {
					this->count++;
					if (count == 130) {
						// If we've reached the maximum repeat amount, write out a code
						boost::iostreams::put(dst, '\x7F');
						boost::iostreams::put(dst, this->prev);
						this->count = 0;
					} else if (count == 3) {
						// If we've reached the point where it is worth writing out the
						// repeat code (eventually), flush the buffer now
						this->flushBuffer(dst);
					}
				} else {
					this->charChanged(dst);
					assert(this->count == 0);
					this->prev = *s;
					this->count = 1;
				}
				s++;
			}
			return s - start;
		}

		template<typename Sink>
		bool flush(Sink& dst)
		{
			// The character hasn't changed, but by calling the char-changed code it
			// will write out any repeated chars to the stream or the buffer.
			this->charChanged(dst);

			// Now flush the buffer if needed.
			this->flushBuffer(dst);

			return true;
		}

};

DDaveRLEFilterType::DDaveRLEFilterType()
	throw ()
{
}

DDaveRLEFilterType::~DDaveRLEFilterType()
	throw ()
{
}

std::string DDaveRLEFilterType::getFilterCode() const
	throw ()
{
	return "rle-ddave";
}

std::string DDaveRLEFilterType::getFriendlyName() const
	throw ()
{
	return "Dangerous Dave RLE";
}

std::vector<std::string> DDaveRLEFilterType::getGameList() const
	throw ()
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Dangerous Dave");
	return vcGames;
}

iostream_sptr DDaveRLEFilterType::apply(iostream_sptr target, FN_TRUNCATE fnTruncate)
	throw (ECorruptedData)
{
	filtered_istream_sptr pinf(new filtered_istream());
	pinf->push(ddave_unrle_filter());

	filtered_ostream_sptr poutf(new filtered_ostream());
	poutf->push(ddave_rle_filter());

	iostream_sptr dec(new filteredstream(target, pinf, poutf));
	return dec;
}

istream_sptr DDaveRLEFilterType::apply(istream_sptr target)
	throw (ECorruptedData)
{
	filtered_istream_sptr pinf(new filtered_istream());
	pinf->push(ddave_unrle_filter());

	pinf->pushShared(target);
	return pinf;
}

ostream_sptr DDaveRLEFilterType::apply(ostream_sptr target, FN_TRUNCATE fnTruncate)
	throw (ECorruptedData)
{
	filtered_ostream_sptr poutf(new filtered_ostream());
	poutf->push(ddave_rle_filter());

	poutf->pushShared(target);
	return poutf;
}

} // namespace gamearchive
} // namespace camoto
