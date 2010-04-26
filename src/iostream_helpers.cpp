/*
 * iostream_helpers.cpp - Useful iostream-based functions.
 *
 * Copyright (C) 2010 Adam Nielsen <malvineous@shikadi.net>
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

#include <iosfwd>                           // streamsize, seekdir
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/categories.hpp>   // seekable_device_tag
#include <boost/iostreams/positioning.hpp>  // stream_offset
#include <boost/shared_ptr.hpp>

#include "debug.hpp"
#include "iostream_helpers.hpp"

#ifdef DEBUG
#define BUFFER_SIZE 4
#else
#define BUFFER_SIZE 4096
#endif

namespace camoto {
namespace gamearchive {

void streamMove(std::iostream& ps, io::stream_offset offFrom,
	io::stream_offset offTo, io::stream_offset lenAmount)
{
	assert(offFrom != offTo);

	char buffer[BUFFER_SIZE];
	std::streamsize len;
	std::streamsize szNext;

	io::stream_offset offFromEnd = offFrom + lenAmount;
	io::stream_offset offToEnd = offTo + lenAmount;

#ifdef DEBUG
	// While we can write past the end of the stream, make sure the caller isn't
	// trying to *start* the write after the EOF.
	ps.seekp(0, std::ios::end);
	io::stream_offset size = ps.tellp();
	assert(offFrom < size);
	assert(offTo <= size);
#endif

	if (
		(offFrom > offTo) || // The destination starts before the source
		(offFromEnd <= offTo) || // The source ends before the dest starts (no overlap)
		(offToEnd <= offFrom) // The dest ends before the source starts (no overlap)
	) {
		// Moving data back towards the start of the stream, start at the beginning
		// and work towards the last block.
		do {
			// Figure out how much to read next (a full block or the last partial one)
			if (BUFFER_SIZE <= lenAmount) {
				szNext = BUFFER_SIZE;
			} else {
				szNext = lenAmount;
			}

			// Despite having separate read and write pointers, moving one affects
			// the other so we have to keep seeking all the time.
			ps.seekg(offFrom, std::ios::beg);
			ps.read(buffer, szNext);
			len = ps.gcount();

			ps.seekp(0, std::ios::end);
			ps.seekp(offTo, std::ios::beg);
			ps.write(buffer, len);

			offFrom += len; offTo += len;
			lenAmount -= len;
		} while ((len) && (szNext == BUFFER_SIZE));
	} else {
		// Moving data forwards towards the end of the stream, start at the end
		// and work back towards the first block.

		szNext = BUFFER_SIZE;

		// Check to see if we'll be moving data out past the end of the stream
		ps.seekp(0, std::ios::end);
		io::stream_offset offStreamEnd = ps.tellp();
		if (offStreamEnd < offToEnd) {
			// We have to move data past the end of the stream, but since we can't
			// seek past the end we need to enlarge the stream first.  An easy way
			// of doing this is to use the code above (in the other 'if' condition,
			// moving in the opposite direction) to move just the extra data from
			// start-to-end which will push out the stream to the correct size.
			io::stream_offset offExcess = offToEnd - offStreamEnd;
			streamMove(ps, offStreamEnd - offExcess, offStreamEnd, offExcess);

			// Now we've moved the last offExcess bytes, so shrink the move operation
			// by that amount and continue as before.
			lenAmount -= offExcess;
		}

		do {
			if (offFromEnd - BUFFER_SIZE < offFrom) {
				szNext = offFromEnd - offFrom;
				offFromEnd = offFrom;
				offToEnd = offTo;
			} else {
				offFromEnd -= BUFFER_SIZE;
				offToEnd -= BUFFER_SIZE;
			}

			ps.seekg(offFromEnd, std::ios::beg);

			ps.read(buffer, szNext);
			len = ps.gcount();

			ps.seekp(offToEnd, std::ios::beg);
			ps.write(buffer, len);

		} while (offFromEnd > offFrom);
	}
	return;
}

#define ZEROPAD_BLOCK_SIZE  16
std::ostream& operator << (std::ostream& s, const zeroPad& n)
{
	int lenData = n.data.length();
	assert(lenData <= n.len);

	// Write the content
	s.write(n.data.c_str(), lenData);

	// Pad out to the full length with nulls
	char blank[ZEROPAD_BLOCK_SIZE];
	memset(blank, 0, ZEROPAD_BLOCK_SIZE);
	int lenRemaining = n.len - lenData;
	int amt = ZEROPAD_BLOCK_SIZE;
	while (lenRemaining > 0) {
		if (lenRemaining < ZEROPAD_BLOCK_SIZE) amt = lenRemaining;
		s.write(blank, amt);
		lenRemaining -= amt;
	}
	return s;
}

} // namespace gamearchive
} // namespace camoto
