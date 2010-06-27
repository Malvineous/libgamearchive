/**
 * @file   substream.hpp
 * @brief  Class declaration for a C++ iostream exposing a limited
 *         section of another C++ iostream.
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

/// boost::iostream class for accessing a portion of another stream.
/**
 * This class is never used directly, rather the substream class acts as a
 * wrapper around it.
 *
 * @see substream
 */
class substream_device {
	private:
		/// Parent stream, where the actual data is read from and written to.
		iostream_sptr psParent;

		/// Current offset into parent stream where substream begins.
		/**
		 * \sa relocate(), getOffset()
		 */
		io::stream_offset iOffset;

		/// Length of data exposed from parent stream.
		/**
		 * \sa setSize(), getSize()
		 */
		std::streamsize iLength;

		/// Current seek position (from start of substream).
		io::stream_offset iCurPos;

	public:
		typedef char char_type;
		typedef io::seekable_device_tag category;

		/// Create a substream out of the given stream
		/**
		 * @param psParent Parent stream, where the data comes from
		 * @param iOffset Offset into the parent stream where the substream starts
		 * @param iLength Size of substream in bytes
		 */
		substream_device(iostream_sptr psParent, std::streamsize iOffset, std::streamsize iLength)
			throw ();

		substream_device(const substream_device&)
			throw ();

		~substream_device()
			throw ();

		/// boost::iostream callback function
		std::streamsize read(char_type *s, std::streamsize n);

		/// boost::iostream callback function
		std::streamsize write(const char_type *s, std::streamsize n);

		/// boost::iostream callback function
		io::stream_offset seek(io::stream_offset off, std::ios_base::seekdir way);

	protected:
		/// Move the "window" of data (looking into the parent stream) forward or
		/// back by the given number of bytes.
		/**
		 * This does not flush any write cache, so if you don't call flush() first,
		 * data you wrote previously may end up in the new place in the parent
		 * stream (but it will still be at the same offset within this substream.)
		 * For example:
		 *
		 * @code
		 * new substream(50);   // create new substream at offset 50 in parent
		 * substream->seek(4);
		 * substream->write("hello"); // may write at offset 54, or may cache
		 * //substream->flush();  // would guarantee "hello" at offset 54 in parent
		 *
		 * substream->relocate(10);   // substream is now at offset 60 in parent
		 * substream->flush();        // may cause "hello" at offset 64 in parent
		 * @endcode
		 *
		 * Generally this is only important when the underlying stream needs to be
		 * modified outside of the substream, such as when it is a
		 * segmented_stream.  If you insert a block of data in the segstream, then
		 * relocate the substream by the same amount, any cached writes will end
		 * up where they should go when they are eventually flushed, taking into
		 * account the new block of data inserted into the segstream.  This avoids
		 * the need to flush the substream before inserting data into the
		 * segstream.  To illustrate:
		 *
		 * @code
		 * // parent stream: AAAABBBBCCCCDDDD
		 * new substream(4,8);  // ==> BBBBCCCC
		 * substream->write("hello"); // overwrite the B's and a C with "hello"
		 *   // cached     ==> parent AAAABBBBCCCCDDDD (i.e. unchanged, will write later @ offset 4)
		 *   // not cached ==> parent AAAAhelloCCCDDDD (immediate writethrough @ offset 4)
		 * parent->insert(...); // ==> AAAA___BBBBCCCCDDDD
		 * @endcode
		 *
		 * At this point, the substream wants to write its data at offset 4 in
		 * the parent, but since we've inserted data that location has now
		 * become offset 7.  So we must relocate the substream to take into
		 * account the newly inserted data:
		 *
		 * @code
		 * substream->relocate(3);
		 * substream->flush(); // parent ==> AAAA___helloCCCDDDD
		 * @endcode
		 *
		 * Here, the insert then flush achieved the same result as a flush then
		 * insert would've (which is good.)  If we hadn't done the relocate:
		 *
		 * @code
		 * //substream->relocate(3);
		 * substream->flush(); // parent ==> AAAAhelloBBCCCCDDDD
		 * @endcode
		 * Now the flush has written the data to the wrong place in the parent
		 * stream!  We were supposed to overwrite all the B's and a C but that
		 * didn't happen.
		 *
		 * For this reason, relocating a substream after a major segstream change
		 * is crucial to avoid data corruption.
		 *
		 * @see getOffset()
		 */
		void relocate(io::stream_offset delta);

		/// Change how far into the parent stream data is accessed.
		/**
		 * This does not move any data, it simply sets the point in the parent
		 * stream at which the substream reports reaching EOF.  No data can be
		 * read from or written to the parent stream after this point.
		 *
		 * @see getSize()
		 */
		void setSize(io::stream_offset len);

		/// Return the offset into the parent stream of the substream's first byte.
		/**
		 * This is the offset in the parent stream where the substream begins.
		 * Writing data at offset 0 in the substream will cause it to be written
		 * to the parent stream starting at the offset returned by this function.
		 *
		 * @see relocate()
		 */
		io::stream_offset getOffset() const;

		/// Return the stream size.
		/**
		 * This is the length of the data in the parent stream made available
		 * through the substream.  Accessing data in the parent stream beyond
		 * this length is not possible (unless the substream is resized.)
		 *
		 * @see setSize()
		 */
		io::stream_offset getSize() const;

	friend class substream;

};


/// C++ iostream class for accessing a portion of another stream.
/**
 * This class provides a standard C++ iostream that is used to access a
 * particular area within another C++ iostream.
 *
 * An example is opening a .zip file as a stream, then creating a substream
 * to provide access to a single file's data within the main .zip.  As far as
 * any functions using the substream are concerned, there is no difference
 * between accessing a file on disk and the substream (even though writes to
 * the substream are transparently written back into the appropriate place in
 * the parent stream.)
 *
 * One of the few exceptions to this transparency is that substreams do not
 * automatically resize when data is written past EOF.  If data is written past
 * the end of the stream, an error is returned.
 *
 * \todo Currently because of boost::iostreams caching, an error is not
 *       returned and the data is silently discarded.  The
 *       substream_write_then_move test checks for this, and currently fails.
 *
 * Example use:
 * @code
 * std::fstream file("test.txt");
 * substream sub(file, 10, 20);
 * sub << "hello";  // Write "hello" at offset 10 in test.txt
 *
 * // Since it's a C++ iostream, it can be stored in that type
 * std::iostream standard = sub;
 *
 * // However you will need to convert it back to its native type before you
 * // can call any advanced functions.
 * substream backagain = dynamic_cast<substream>(standard);
 * assert(backagain != NULL);
 * backagain.relocate(5);
 * @endcode
 */
class substream: public io::stream<substream_device>
{
	public:
		/// @copydoc substream_device::substream_device()
		substream(iostream_sptr psParent, std::streamsize iOffset, std::streamsize iLength)
			throw ();

		substream(const substream_device& orig)
			throw ();

		/// @copydoc substream_device::relocate()
		void relocate(io::stream_offset delta);
		/// @copydoc substream_device::setSize()
		void setSize(io::stream_offset len);
		/// @copydoc substream_device::getOffset()
		io::stream_offset getOffset() const;
		/// @copydoc substream_device::getSize()
		io::stream_offset getSize() const;
};

typedef boost::shared_ptr<substream> substream_sptr;

} // namespace gamearchive
} // namespace camoto

#endif // _CAMOTO_SUBSTREAM_HPP_
