/**
 * @file  filter-decomp-size.hpp
 * @brief Filter for handing first few bytes as decompressed file size.
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

#ifndef _CAMOTO_FILTER_DECOMP_SIZE_HPP_
#define _CAMOTO_FILTER_DECOMP_SIZE_HPP_

#include <memory>
#include <camoto/filter.hpp>

namespace camoto {
namespace gamearchive {

/// Handle initial UINT32LE as file size.
/**
 * This filter reads the first four bytes as a UINT32LE and stores this as the
 * target file size.  It then passes any remaining data to the supplied filter.
 * After the target file size is reached, no more data is returned (even if
 * there is more to be read.)
 *
 * This filter is intended to handle reading of compressed file formats that
 * store the decompressed size as the first four bytes in the stream.  It will
 * ensure that only the correct number of bytes are read, ignoring any trailing
 * data that could trigger errors in the decompression routine.
 */
class filter_decomp_size_remove: virtual public filter
{
	public:
		/// Remove first four bytes then run rest of data through another filter.
		/**
		 * @param childFilter
		 *   After the first four bytes are removed, the rest of the content is
		 *   run through this filter.
		 */
		filter_decomp_size_remove(std::unique_ptr<filter> childFilter);

		virtual void reset(stream::len lenInput);
		virtual void transform(uint8_t *out, stream::len *lenOut,
			const uint8_t *in, stream::len *lenIn);

	protected:
		/// The decompressed size, or -1 if not read in yet.  We will signal EOF
		/// once this many bytes has been output.
		int lenTarget;

		/// Child stream to filter actual content through.
		std::unique_ptr<filter> childFilter;
};

/// Insert initial UINT32LE as file size.
/**
 * This filter takes the size of the incoming data, and writes it as a UINT32LE
 * in the first four bytes of the stream.  It then passes any remaining data to
 * the supplied filter.  No further padding or truncation is done.
 *
 * This filter is intended to handle writing compressed file formats that store
 * the decompressed size as the first four bytes in the stream.
 */
class filter_decomp_size_insert: virtual public filter
{
	public:
		/// Add original size as first four bytes then run rest of data through
		/// another filter.
		/**
		 * @param childFilter
		 *   After four bytes are inserted at the beginning of the stream, the rest
		 *   of the content is run through this filter.
		 */
		filter_decomp_size_insert(std::unique_ptr<filter> childFilter);

		virtual void reset(stream::len lenInput);
		virtual void transform(uint8_t *out, stream::len *lenOut,
			const uint8_t *in, stream::len *lenIn);

	protected:
		/// Original size of the data before any filtering, to write as a UINT32LE
		/// at the start of the output.
		long lenInput;

		/// Child stream to filter actual content through.
		std::unique_ptr<filter> childFilter;
};

} // namespace gamearchive
} // namespace camoto

#endif // _CAMOTO_FILTER_DECOMP_SIZE_HPP_
