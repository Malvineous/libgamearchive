/*
 * segmented_stream_device.hpp - Class declaration for a C++ iostream that allows
 *   blocks of data to be inserted or removed at any point in the underlying
 *   stream, shifting data around as necessary.  Data is not modified in the
 *   underlying stream until commit() is called.  Do not modify the underlying
 *   stream while it is being used by segmented_stream_device as some changes will
 *   not be picked up and others will cause data corruption.
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

#ifndef _CAMOTO_SEGMENTED_STREAM_HPP_
#define _CAMOTO_SEGMENTED_STREAM_HPP_

#include <iosfwd>                           // streamsize, seekdir
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/categories.hpp>   // seekable_device_tag
#include <boost/iostreams/positioning.hpp>  // stream_offset
#include <boost/shared_ptr.hpp>

#include <camoto/types.hpp>

#include "debug.hpp"

namespace camoto {
namespace gamearchive {

namespace io = boost::iostreams;

// This class implements a Boost iostreams device, which can act as both a
// Sink and a Source.  It implements additional functions to provide the
// segmented functionality, but these are not public to prevent them from
// being accessed directly.  (If they were accessed directly they would be out
// of sync with the iostreams' build in cache, causing data corruption.)  To
// work around this, a segmented_stream class is defined later which provides
// a wrapper around the functions and flushes the cache as necessary.
class segmented_stream_device
{
	public:
		typedef char char_type;
		typedef io::seekable_device_tag category;

	private:
		io::stream_offset poffFirstStart; // Offsets into parent stream ("poff")
		io::stream_offset poffFirstEnd;
		iostream_sptr psFirst;
		std::vector<uint8_t> vcSecond;
		segmented_stream_device *psegThird;

		// When offPos == 0, the parent stream file pointer is at poffFirstStart
		io::stream_offset offPos;  // Offset into self ("off", starts at 0)

		segmented_stream_device()
			throw ();

	public:
		// psBase now becomes the underlying stream providing data.  This means it
		// shouldn't be accessed while the segmented_stream_device has control of it, or
		// the data will probably be corrupted.
		segmented_stream_device(iostream_sptr psBase)
			throw ();

		segmented_stream_device(const segmented_stream_device&)
			throw ();

		~segmented_stream_device()
			throw ();

		std::streamsize read(char_type *s, std::streamsize n);
		std::streamsize write(const char_type *s, std::streamsize n);
		io::stream_offset seek(io::stream_offset off, std::ios_base::seekdir way);

		std::streamsize getLength();

	protected:
		// Insert a block of data at the current seek position, shifting the rest
		// of the data forward, out of the way.  Seek position remains unchanged,
		// but stream size will have enlarged by lenInsert bytes.
		// Before: AAAABBBB
		// After:  AAAA____BBBB
		//             ^ Seek position, lenInsert == 4
		void insert(std::streamsize lenInsert);

		// Remove a chunk of data from the current seek position, pulling the rest
		// of the data back.  All data from the current seek position to lenRemove
		// bytes after it are lost.  The seek position remains unchanged, but the
		// stream size will have shrunk by lenRemove bytes.
		// Before: AAAAXXXXBBBB
		// After:  AAAABBBB
		//             ^ Seek position, lenRemove == 4
		void remove(std::streamsize lenRemove);

		// Write out all the changes to the underlying stream.  On completion
		// vcSecond and psegThird will be empty.
		void commit(FN_TRUNCATE fnTruncate);

	private:
		// Actual commit function where the destination offset can be specified.
		void commit(io::stream_offset poffWriteFirst, io::stream_offset plenStream);

		// Split the segstream at the current seek position.  Upon return the first
		// data source will only last until the current seek position, the second data
		// source will be empty and the third data source will contain all the data
		// that was originally after the current seek position.
		// Before: AAAABBBB
		//             ^ seek position
		// After:  AAAABBBB
		// first --^   ^-- third (second is empty)
		void split();

	// The segmented_stream class is used to access these protected functions
	// while flushing caches first to ensure no data corruption.
	friend class segmented_stream;
};


// This is the actual Boost iostream class.
//
// All these functions (including c'tors) call their equivalents in the
// segmented_stream_device class.  See the comments in the class above.
class segmented_stream: public io::stream<segmented_stream_device>
{
	public:
		segmented_stream(iostream_sptr psBase)
			throw ();

		segmented_stream(const segmented_stream_device& orig)
			throw ();

		void insert(std::streamsize lenInsert);
		void remove(std::streamsize lenRemove);
		void commit(FN_TRUNCATE fnTruncate);
};

typedef boost::shared_ptr<segmented_stream> segstream_sptr;

} // namespace gamearchive
} // namespace camoto

#endif // _CAMOTO_SEGMENTED_STREAM_HPP_
