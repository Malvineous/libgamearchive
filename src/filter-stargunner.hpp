/**
 * @file   filter-stargunner.hpp
 * @brief  Filter implementation for decompressing Stargunner files.
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

#ifndef _CAMOTO_FILTER_STARGUNNER_HPP_
#define _CAMOTO_FILTER_STARGUNNER_HPP_

#include <stack>
#include <camoto/stream.hpp>
#include <camoto/bitstream.hpp>
#include <camoto/gamearchive/filtertype.hpp>

namespace camoto {
namespace gamearchive {

/// Chunk size used during compression.  Each chunk expands to this amount of data.
#define CHUNK_SIZE 4096

/// Largest possible chunk of compressed data.  (No compression + worst case dictionary size.)
#define CMP_CHUNK_SIZE (CHUNK_SIZE + 256 + 2) // plus 2 for the chunk length

class filter_stargunner_decompress: public filter {

	public:
		filter_stargunner_decompress();

		void explode_chunk(const uint8_t* in, unsigned int expanded_size,
			uint8_t* out);

		void transform(uint8_t *out, stream::len *lenOut,
			const uint8_t *in, stream::len *lenIn);

	protected:
		uint8_t bufIn[CMP_CHUNK_SIZE];  ///< Read (compressed) buffer
		uint8_t bufOut[CHUNK_SIZE]; ///< Output (decompressed) buffer

		bool gotHeader;        ///< Have we read in the file header?
		uint32_t finalSize;    ///< Size of fully decompressed file

		unsigned int lenBufIn; ///< How much data is valid in bufIn
		unsigned int posOut;   ///< How much data has been read out of bufOut
};

/// Stargunner decompression filter.
class StargunnerFilterType: virtual public FilterType {

	public:
		StargunnerFilterType();

		~StargunnerFilterType();

		virtual std::string getFilterCode() const;

		virtual std::string getFriendlyName() const;

		virtual std::vector<std::string> getGameList() const;

		virtual stream::inout_sptr apply(stream::inout_sptr target,
			stream::fn_truncate resize);

		virtual stream::input_sptr apply(stream::input_sptr target);

		virtual stream::output_sptr apply(stream::output_sptr target,
			stream::fn_truncate resize);

};

} // namespace gamearchive
} // namespace camoto

#endif // _CAMOTO_FILTER_STARGUNNER_HPP_
