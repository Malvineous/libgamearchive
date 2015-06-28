/**
 * @file  filter-stargunner.cpp
 * @brief Filter implementation for decompressing Stargunner files.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/DLT_Format
 *
 * Copyright (C) 2010-2015 Adam Nielsen <malvineous@shikadi.net>
 * Decompression algorithm by The_coder
 *   <http://code.google.com/p/tombexcavator/source/browse/branches/v0/src/providers/stargunner/read.cc>
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

#include <cassert>
#include <functional>
#include <stack>
#include <camoto/filter.hpp>
#include <camoto/stream_filtered.hpp>
#include <camoto/util.hpp> // std::make_unique
#include <camoto/bitstream.hpp>
#include <camoto/util.hpp>

#include "filter-stargunner.hpp"

namespace camoto {
namespace gamearchive {

void filter_stargunner_decompress::reset(stream::len lenInput)
{
	this->gotHeader = false;
	this->lenBufIn = 0;
	this->posOut = CHUNK_SIZE;
	return;
}

void filter_stargunner_decompress::explode_chunk(const uint8_t* in,
	unsigned int expanded_size, uint8_t* out)
{
	uint8_t tableA[256], tableB[256];
	unsigned int inpos = 0;
	unsigned int outpos = 0;

	while (outpos < expanded_size) {
		// Initialise the dictionary so that no bytes are codewords (or if you
		// prefer, each byte expands to itself only.)
		for (int i = 0; i < 256; i++) tableA[i] = i;

		//
		// Read in the dictionary
		//

		uint8_t code;
		unsigned int tablepos = 0;
		do {
			code = in[inpos++];

			// If the code has the high bit set, the lower 7 bits plus one is the
			// number of codewords that will be skipped from the dictionary.  (Those
			// codewords were initialised to expand to themselves in the loop above.)
			if (code > 127) {
				tablepos += code - 127;
				code = 0;
			}
			if (tablepos == 256) break;

			// Read in the indicated number of codewords.
			for (int i = 0; i <= code; i++) {
				if (tablepos >= 256) {
					throw filter_error("Dictionary was larger than 256 bytes");
				}
				uint8_t data = in[inpos++];
				tableA[tablepos] = data;
				if (tablepos != data) {
					// If this codeword didn't expand to itself, store the second byte
					// of the expansion pair.
					tableB[tablepos] = in[inpos++];
				}
				tablepos++;
			}
		} while (tablepos < 256);

		// Read the length of the data encoded with this dictionary
		int len = in[inpos++];
		len |= in[inpos++] << 8;

		//
		// Decompress the data
		//

		int expbufpos = 0;
		// This is the maximum number of bytes a single codeword can expand to.
		uint8_t expbuf[32];
		while (1) {
			if (expbufpos) {
				// There is data in the expansion buffer, use that
				code = expbuf[--expbufpos];
			} else {
				// There is no data in the expansion buffer, use the input data
				if (--len == -1) break; // no more input data
				code = in[inpos++];
			}

			if (code == tableA[code]) {
				// This byte is itself, write this to the output
				out[outpos++] = code;
			} else {
				// This byte is actually a codeword, expand it into the expansion buffer
				if (expbufpos >= (signed)sizeof(expbuf) - 2) {
					throw filter_error("Codeword expanded to more than "
						TOSTRING(sizeof(expbuf)) " bytes");
				}
				expbuf[expbufpos++] = tableB[code];
				expbuf[expbufpos++] = tableA[code];
			}
		}
	}
	return;
}

void filter_stargunner_decompress::transform(uint8_t *out, stream::len *lenOut,
	const uint8_t *in, stream::len *lenIn)
{
	stream::len amtRead = 0;

	if (!this->gotHeader) {
		// Check for a valid header
		if (*lenIn < 8) {
			throw filter_error("Not enough data");
		}
		if (
			(in[0] == 'P') &&
			(in[1] == 'G') &&
			(in[2] == 'B') &&
			(in[3] == 'P')
		) {
			this->gotHeader = true;
			this->finalSize =
				 in[4] |
				(in[5] << 8) |
				(in[6] << 16) |
				(in[7] << 24)
			;
			in += 8; // skip these bytes
			*lenIn -= 8;
			amtRead += 8;
		} else {
			throw filter_error("Data is not compressed in Stargunner format");
		}
	}

	// Fill up the input buffer
	if (this->lenBufIn < CMP_CHUNK_SIZE) {
		// Read more data to fill up the chunk
		unsigned int amt = CMP_CHUNK_SIZE - this->lenBufIn; // fill up the buffer
		if (amt > *lenIn) amt = *lenIn; // or as much as we can, anyway

		memcpy(this->bufIn + this->lenBufIn, in, amt);
		amtRead += amt;
		this->lenBufIn += amt;
	}
	*lenIn = amtRead; // store how much we read (if anything)

	// If the output buffer is empty, and the input one contains at least one
	// chunk, explode it.
	if ((this->posOut == CHUNK_SIZE) && (this->lenBufIn > 2)) {
		unsigned int lenChunk = this->bufIn[0] | (this->bufIn[1] << 8);
		if (lenChunk + 2 <= this->lenBufIn) {
			// Have all the data from one chunk
			unsigned int chunkSize;
			if (this->finalSize < CHUNK_SIZE) chunkSize = this->finalSize;
			else chunkSize = CHUNK_SIZE;
			this->explode_chunk(this->bufIn + 2, chunkSize, this->bufOut);
			this->finalSize -= chunkSize;
			if (chunkSize < CHUNK_SIZE) {
				// This was a partial chunk so 'right-justify' it to the end of the
				// buffer, so the read code below doesn't store data past the end.
				memmove(this->bufOut + CHUNK_SIZE - chunkSize, this->bufOut, chunkSize);
				this->posOut = CHUNK_SIZE - chunkSize;
			} else {
				this->posOut = 0;
			}
			// Remove this chunk, shifting the rest of the data up
			this->lenBufIn -= 2 + lenChunk;
			memmove(this->bufIn, this->bufIn + 2 + lenChunk, this->lenBufIn);
		} // else haven't yet got a whole chunk's worth of data
	}

	// Fill up the output buffer
	if (this->posOut < CHUNK_SIZE) {
		unsigned int amt = CHUNK_SIZE - this->posOut; // read the rest of the buffer
		if (amt > *lenOut) amt = *lenOut; // or as much as will fit

		memcpy(out, this->bufOut + this->posOut, amt);
		*lenOut = amt; // store how much we wrote
		this->posOut += amt;

		assert(this->posOut <= CHUNK_SIZE);
	} else {
		*lenOut = 0;
	}
	return;
}


FilterType_Stargunner::FilterType_Stargunner()
{
}

FilterType_Stargunner::~FilterType_Stargunner()
{
}

std::string FilterType_Stargunner::code() const
{
	return "bpe-stargunner";
}

std::string FilterType_Stargunner::friendlyName() const
{
	return "Stargunner compression";
}

std::vector<std::string> FilterType_Stargunner::games() const
{
	return {
		"Stargunner",
	};
}

std::unique_ptr<stream::inout> FilterType_Stargunner::apply(
	std::unique_ptr<stream::inout> target, stream::fn_notify_prefiltered_size resize)
	const
{
	return std::make_unique<stream::filtered>(
		std::move(target),
		std::make_shared<filter_stargunner_decompress>(),
		/// @todo Implement Stargunner compression
		std::unique_ptr<filter>(),//std::make_shared<filter_stargunner_compress>(),
		resize
	);
}

std::unique_ptr<stream::input> FilterType_Stargunner::apply(
	std::unique_ptr<stream::input> target) const
{
	return std::make_unique<stream::input_filtered>(
		std::move(target),
		std::make_shared<filter_stargunner_decompress>()
	);
}

std::unique_ptr<stream::output> FilterType_Stargunner::apply(
	std::unique_ptr<stream::output> target,  stream::fn_notify_prefiltered_size resize)
	const
{
	return std::make_unique<stream::output_filtered>(
		std::move(target),
		/// @todo Implement Stargunner compression
		std::unique_ptr<filter>(),//std::make_shared<filter_stargunner_compress>(),
		resize
	);
}

} // namespace gamearchive
} // namespace camoto
