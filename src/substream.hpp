/*
 * substream.hpp - Class declaration for a C++ iostream exposing a limited
 *                 section of another C++ iostream.
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

#ifndef _CAMOTO_SUBSTREAM_HPP_
#define _CAMOTO_SUBSTREAM_HPP_

#include <iosfwd>                           // streamsize, seekdir
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/categories.hpp>   // seekable_device_tag
#include <boost/iostreams/positioning.hpp>  // stream_offset

#include <camoto/types.hpp>

namespace camoto {
namespace gamearchive {

namespace io = boost::iostreams;

class substream_device {
	private:
		iostream_sptr psParent;
		io::stream_offset iOffset;
		std::streamsize iLength;

		io::stream_offset iCurPos;

	public:
		typedef char char_type;
		typedef io::seekable_device_tag category;

		substream_device(iostream_sptr psParent, std::streamsize iOffset, std::streamsize iLength)
			throw ();

		substream_device(const substream_device&)
			throw ();

		~substream_device()
			throw ();

		std::streamsize read(char_type *s, std::streamsize n);

		std::streamsize write(const char_type *s, std::streamsize n);

		io::stream_offset seek(io::stream_offset off, std::ios_base::seekdir way);

	protected:
		// Move the "window" of data (looking into the parent stream) forward or
		// back by the given number of bytes.  This does not flush any write cache,
		// so if you don't call flush() first, data you wrote previously may end up
		// in the new place in the parent stream (but it will still be at the
		// same offset within this substream.)  For example:
		//
		//   new substream(50)   # create new substream at offset 50 in parent
		//   substream->seek(4)
		//   substream->write("hello")  # may write at offset 54, or may cache
		//   #substream->flush()  # would guarantee "hello" at offset 54 in parent
		//
		//   substream->relocate(10)  # substream is now at offset 60 in parent
		//   substream->flush()       # may cause "hello" at offset 64 in parent
		//
		// Generally this is only important when the underlying stream needs to be
		// modified outside of the substream, such as when it is a
		// segmented_stream.  If you insert a block of data in the segstream, then
		// relocate the substream by the same amount, any cached writes will end
		// up where they should go, taking into account the new block of data
		// inserted into the segstream.  This avoids the need to flush the
		// substream before inserting data into the segstream.  Another example:
		//
		//   parent stream: AAAABBBBCCCCDDDD
		//   substream(4,8) ==> BBBBCCCC
		//   substream->write("hello")
		//     cached     ==> parent AAAABBBBCCCCDDDD (i.e. unchanged)
		//     not cached ==> parent AAAAhelloCCCDDDD (immediate writethrough)
		//   parent->insert: AAAA____BBBBCCCCDDDD
		//   substream->relocate(4)
		//   substream->flush() ==> parent AAAA____helloCCCDDDD
		//
		// Here the result at the final flush is the same whether the data
		// was flushed first, then the "_" bytes inserted, or if the data was
		// cached, then flushed at the new offset *after* the "_" bytes were
		// inserted.
		void relocate(io::stream_offset delta);

		void setSize(io::stream_offset len);

		io::stream_offset getOffset() const;
		io::stream_offset getSize() const;

	friend class substream;

};


// This is the actual Boost iostream class.
//
// All these functions (including c'tors) call their equivalents in the
// segmented_stream_device class.  See the comments in the class above.
class substream: public io::stream<substream_device>
{
	public:
		substream(iostream_sptr psParent, std::streamsize iOffset, std::streamsize iLength)
			throw ();

		substream(const substream_device& orig)
			throw ();

		void relocate(io::stream_offset delta);
		void setSize(io::stream_offset len);
		io::stream_offset getOffset() const;
		io::stream_offset getSize() const;
};

typedef boost::shared_ptr<substream> substream_sptr;

} // namespace gamearchive
} // namespace camoto

#endif // _CAMOTO_SUBSTREAM_HPP_
