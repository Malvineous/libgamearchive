/*
 * segmented_stream_device.hpp - Class declaration for a C++ iostream that allows
 *   blocks of data to be inserted or removed at any point in the underlying
 *   stream, shifting data around as necessary.  Data is not modified in the
 *   underlying stream until commit() is called.
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
#include <vector>

#include "iostream_helpers.hpp"  // streamMove
#include "segmented_stream.hpp"
#include "debug.hpp"

namespace camoto {
namespace gamearchive {

refcount_declclass(segmented_stream_device);

segmented_stream_device::segmented_stream_device()
	throw ()
{
	refcount_qenterclass(segmented_stream_device);
}

segmented_stream_device::segmented_stream_device(iostream_sptr psBase)
	throw () :
		poffFirstStart(0),
		psFirst(psBase),
		vcSecond(),
		psegThird(NULL),
		offPos(0)
{
	this->psFirst->seekg(0, std::ios::end);
	this->poffFirstEnd = this->psFirst->tellg();
	refcount_qenterclass(segmented_stream_device);
}

segmented_stream_device::segmented_stream_device(const segmented_stream_device& orig)
	throw () :
		poffFirstStart(orig.poffFirstStart),
		poffFirstEnd(orig.poffFirstEnd),
		psFirst(orig.psFirst),
		vcSecond(orig.vcSecond),
		psegThird(orig.psegThird),
		offPos(orig.offPos)
{
	refcount_qenterclass(segmented_stream_device);
	assert(this->poffFirstStart <= this->poffFirstEnd);
}

segmented_stream_device::~segmented_stream_device()
	throw ()
{
	refcount_qexitclass(segmented_stream_device);
	if (this->psegThird) delete this->psegThird;
}

std::streamsize segmented_stream_device::read(char_type *s, std::streamsize n)
{
	// Read the first stream
	io::stream_offset lenReadFirst; // How much we read from the first stream
	io::stream_offset lenRemaining = n; // How much left to read from second/third streams

	// This is the "reported" end of the stream (what we tell anyone using this
	// class.)
	io::stream_offset offFirstReportedEnd = this->poffFirstEnd - this->poffFirstStart;

	if (this->offPos < offFirstReportedEnd) {
		// Some of the read will happen in the first stream
		io::stream_offset lenFirst;
		if (this->offPos + n > offFirstReportedEnd) {
			lenFirst = offFirstReportedEnd - this->offPos;
			lenRemaining -= lenFirst;
		} else {
			lenFirst = lenRemaining;
			lenRemaining = 0;
		}
		this->psFirst->seekg(this->offPos + this->poffFirstStart);
		lenReadFirst = this->psFirst->rdbuf()->sgetn(s, lenFirst);
		this->offPos += lenReadFirst;
		if (lenReadFirst < lenFirst) {
			// Didn't read the full amount from the first stream for some reason,
			// this shouldn't happen unless there's a major problem with the
			// underlying stream.
			return lenReadFirst;
		}
		s += lenReadFirst;
	} else {
		lenReadFirst = 0;
	}

	// Read the second stream (the vector)
	io::stream_offset lenReadSecond;
	io::stream_offset offSecondEnd = offFirstReportedEnd + this->vcSecond.size();
	if ((lenRemaining > 0) && (this->offPos < offSecondEnd)) {
		// Some of the read will happen in the second stream
		io::stream_offset lenSecond;
		if (this->offPos + lenRemaining > offSecondEnd) {
			lenSecond = offSecondEnd - this->offPos;
			lenRemaining -= lenSecond;
		} else {
			lenSecond = lenRemaining;
		}
		io::stream_offset offSecond = this->offPos - offFirstReportedEnd;
		assert(offSecond < this->vcSecond.size());
		memcpy(s, &this->vcSecond[offSecond], lenSecond);
		this->offPos += lenSecond;
		s += lenSecond;
		lenReadSecond = lenSecond;
	} else {
		lenReadSecond = 0;
		// lenRemaining remains unchanged as == n
	}

	// Read the third stream (the child segmented_stream_device)
	io::stream_offset lenReadThird;
	if ((lenRemaining > 0) && (this->psegThird)) {
		// Some of the read will happen in the third stream
		//lenReadThird = this->psegThird->rdbuf()->sgetn(s, lenRemaining);
		lenReadThird = this->psegThird->read(s, lenRemaining);
		this->offPos += lenReadThird;
	} else {
		lenReadThird = 0;
		// lenRemaining remains unchanged
	}

	// Return the number of bytes read
	if (lenReadFirst + lenReadSecond + lenReadThird == 0) return -1; // EOF
	return lenReadFirst + lenReadSecond + lenReadThird;
}

std::streamsize segmented_stream_device::write(const char_type *s, std::streamsize n)
{
	// Write to the first stream
	io::stream_offset lenWroteFirst; // How much we wrote to the first source
	io::stream_offset lenRemaining = n; // How much left to write to the second/third sources
	io::stream_offset offFirstReportedEnd = this->poffFirstEnd - this->poffFirstStart;

	if (this->offPos < offFirstReportedEnd) {
		// Some of the write will happen in the first source
		io::stream_offset lenFirst;
		if (this->offPos + n > offFirstReportedEnd) {
			lenFirst = offFirstReportedEnd - this->offPos;
			lenRemaining -= lenFirst;
		} else {
			lenFirst = lenRemaining;
			lenRemaining = 0;
		}
		this->psFirst->seekp(this->offPos + this->poffFirstStart);
		lenWroteFirst = this->psFirst->rdbuf()->sputn(s, lenFirst);
		this->offPos += lenWroteFirst;
		s += lenWroteFirst;
		if (lenWroteFirst < lenFirst) {
			// Didn't write the full amount from the first source for some reason
			return lenWroteFirst;
		}
	} else {
		lenWroteFirst = 0;
	}

	// Write to the second stream (the vector)
	io::stream_offset lenWroteSecond;
	io::stream_offset offSecondEnd = offFirstReportedEnd + this->vcSecond.size();
	if ((lenRemaining > 0) && (this->offPos < offSecondEnd)) {
		// Some of the write will happen in the second source
		io::stream_offset lenSecond;
		if (this->offPos + lenRemaining > offSecondEnd) {
			// The write will go past the end of the second source
			lenSecond = offSecondEnd - this->offPos;
			lenRemaining -= lenSecond;
		} else {
			lenSecond = lenRemaining;
			lenRemaining = 0;
		}
		io::stream_offset offSecond = this->offPos - offFirstReportedEnd;
		assert(offSecond + lenSecond <= this->vcSecond.size());
		memcpy(&this->vcSecond[offSecond], s, lenSecond);
		this->offPos += lenSecond;
		s += lenSecond;
		lenWroteSecond = lenSecond;
	} else {
		lenWroteSecond = 0;
		// lenRemaining remains unchanged as == n
	}

	// Write to the third source (the child segmented_stream_device)
	int lenWroteThird;
	if ((lenRemaining > 0) && (this->psegThird)) {
		// Some of the write will happen in the third stream
		// No need to seek here, the segstream will do it when it realises the
		// write is in its own first data source.
		lenWroteThird = this->psegThird->write(s, lenRemaining);
		this->offPos += lenWroteThird;
	} else {
		lenWroteThird = 0;
		// lenRemaining remains unchanged
	}

	// Return the number of bytes written
	return lenWroteFirst + lenWroteSecond + lenWroteThird;
}

io::stream_offset segmented_stream_device::seek(io::stream_offset off, std::ios_base::seekdir way)
{
	// Seek to position off and return the new stream
	// position. The argument way indicates how off is
	// interpretted:
	//    - std::ios_base::beg indicates an offset from the
	//      sequence beginning
	//    - std::ios_base::cur indicates an offset from the
	//      current character position
	//    - std::ios_base::end indicates an offset from the
	//      sequence end
	io::stream_offset lenFirst = this->poffFirstEnd - this->poffFirstStart;
	io::stream_offset lenTotal = lenFirst;
	//if (!this->vcSecond.empty())
	lenTotal += this->vcSecond.size();
	io::stream_offset offSecondEnd = lenTotal;
	if (this->psegThird) lenTotal += this->psegThird->getLength();

	io::stream_offset iBase;
	switch (way) {
		case std::ios_base::beg:
			iBase = 0;
			break;
		case std::ios_base::cur:
			iBase = this->offPos;
			break;
		case std::ios_base::end:
			iBase = lenTotal;
			break;
		default:
			assert(false);
	}
	iBase += off;
	if (iBase > lenTotal) {
		// Can't seek past EOF
		iBase = lenTotal;
	} else if (iBase < 0) {
		// Can't see past SOF
		iBase = 0;
	}
	this->offPos = iBase;

	// The seek pointer can't be updated here, because it's shared by all the
	// descendent psegThird elements.
	/*
	if (this->offPos < lenFirst) {
		// Within first stream
		this->psFirst->seekg(this->offPos + this->poffFirstStart);
		this->psFirst->seekp(this->offPos + this->poffFirstStart);
		if (this->psegThird) {
			// Prepare the third stream for when the write overflows into it
			this->psegThird->seek(0, std::ios_base::beg);
		}
	} else if (this->offPos < offSecondEnd) {
		// Within second stream (vector)
		if (this->psegThird) {
			// Prepare the third stream for when the write overflows into it
			this->psegThird->seek(0, std::ios_base::beg);
		}
	} else {
		// Within third stream (segstream)
		assert(this->psegThird);

		// Prepare the third stream for when the write overflows into it
		this->psegThird->seek(this->offPos - offSecondEnd, std::ios_base::beg);
	}*/

	// But we can let the third source know where we'll come in when we read
	// straight through later.
	if ((this->offPos >= offSecondEnd) && (this->psegThird)) {
		this->psegThird->seek(this->offPos - offSecondEnd, std::ios::beg);
	}

	return this->offPos;
}

std::streamsize segmented_stream_device::getLength()
{
	std::streamsize lenTotal = this->poffFirstEnd - this->poffFirstStart
		+ this->vcSecond.size();
	if (this->psegThird) lenTotal += this->psegThird->getLength();
	return lenTotal;
}

// The Boost iostream must be flushed before this function is called, otherwise
// it may move data around while writes are pending, causing those writes to
// end up in the wrong place.  The segmented_stream wrapper class takes care
// of this.
void segmented_stream_device::insert(std::streamsize lenInsert)
{
	io::stream_offset lenFirst = this->poffFirstEnd - this->poffFirstStart;
	if (this->offPos < lenFirst) {
		// The extra data is to be inserted within the first source
		// TESTED BY: segstream_insert_c01

		this->split();
		// Make our second source lenInsert bytes long so it will become the newly
		// inserted block of data.
		this->vcSecond.resize(lenInsert);
		assert(this->vcSecond.size() == lenInsert);
	} else {
		io::stream_offset offSecondEnd = lenFirst + this->vcSecond.size();
		if (this->offPos <= offSecondEnd) {
			// Extra data is to be inserted in the middle of the second source
			// TESTED BY: segstream_insert_c02

			this->vcSecond.insert(this->vcSecond.begin() + (this->offPos
				- lenFirst), lenInsert, '\0');
		} else {
			// Extra data is to be inserted in the third source
			// TESTED BY: segstream_insert_c03

			// Rebase the offset so that zero is the start of the child segstream
			assert(this->psegThird);
			this->psegThird->insert(lenInsert);
		}
	}
	return;
}

// The Boost iostream must be flushed before this function is called, otherwise
// it may move data around while writes are pending, causing those writes to
// end up in the wrong place.  The segmented_stream wrapper class takes care
// of this.
void segmented_stream_device::remove(std::streamsize lenRemove)
{
	if (lenRemove == 0) return;

	io::stream_offset lenFirst = this->poffFirstEnd - this->poffFirstStart;
	if (this->offPos < lenFirst) {
		// The data to be removed is contained (or at least starts) in the first
		// source.
		if (this->offPos + lenRemove >= lenFirst) {
			// The block to remove goes past the end of the first data source, so
			// we can just trim the first block, leaving the remainder to be handled
			// below.
			// TESTED BY: segstream_remove_c04
			lenRemove -= lenFirst - this->offPos;
			lenFirst = this->offPos;
			this->poffFirstEnd = this->poffFirstStart + lenFirst;

		} else if (this->offPos == 0) {
			// The remove is contained entirely within the first block, starting
			// at the beginning.  So cut data off the start of the block.
			// TESTED BY: segstream_remove_c01
			this->poffFirstStart += lenRemove;
			assert(this->poffFirstStart <= this->poffFirstEnd);
			return;
		} else {
			// The remove is contained entirely within the first block, so we'll have
			// to split the first block and remove the data from the front of the
			// new third block.
			// TESTED BY: segstream_remove_c02
			this->split();
			this->psegThird->poffFirstStart += lenRemove;
			assert(this->psegThird->poffFirstStart < this->psegThird->poffFirstEnd);
			return;
		}
	} // else none of the remove is contained in the first source

	if (lenRemove == 0) return; // No more data to remove

	// This can't be possible, otherwise we haven't removed data from the first
	// source when we should have.
	assert(this->offPos >= lenFirst);

	io::stream_offset lenSecond = this->vcSecond.size();
	io::stream_offset offSecondEnd = lenFirst + lenSecond;
	if (this->offPos < offSecondEnd) {
		// There is some data to remove from the second source

		if (this->offPos == lenFirst) {
			// The block to remove crosses the start of the second source, so we can
			// just truncate data off the front.
			if (lenRemove >= lenSecond) {
				// The block to remove also reaches or crosses the end of the second
				// source, i.e. the entire second source is to be removed.
				// TESTED BY: segstream_remove_c05
				this->vcSecond.clear();
				lenRemove -= lenSecond;  // in case there's any leftovers
			} else {
				// Just some data off the front is to go
				// TESTED BY: segstream_remove_c06
				this->vcSecond.erase(this->vcSecond.begin(),
					this->vcSecond.begin() + lenRemove);
				lenRemove = 0;
			}
		} else {
			// The remove doesn't start until somewhere in the middle of the second
			// source.
			io::stream_offset offCropStart = this->offPos - lenFirst;
			std::vector<uint8_t>::iterator itCropStart =
				this->vcSecond.begin() + offCropStart;
			std::vector<uint8_t>::iterator itCropEnd;
			if (offCropStart + lenRemove >= lenSecond) {
				// It goes past the end though, so truncate some data off the end of
				// the second source
				// TESTED BY: segstream_remove_c07
				itCropEnd = this->vcSecond.end();
				lenRemove -= lenSecond - offCropStart;
			} else {
				// Removal is contained entirely within the second source
				// TESTED BY: segstream_remove_c08
				itCropEnd = itCropStart + lenRemove;
				lenRemove = 0;
			}
			this->vcSecond.erase(itCropStart, itCropEnd);
		}
	}

	if (lenRemove == 0) return; // No more data to remove

	// If we've gotten this far there's still some data to remove from
	// the third source.  We must have a third source, otherwise the caller is
	// trying to remove too much data.
	// TESTED BY: segstream_remove_c03
	assert(this->psegThird);
	this->psegThird->remove(lenRemove);

	return;
}

// Write out all the changes to the underlying stream.  On completion
// vcSecond and psegThird will be empty, and the file pointer in psFirst
// is undefined (which is fine, because the read/write functions reset it
// anyway.)  The file pointer from the user's point of view doesn't change.
//
// The Boost iostream must be flushed before this function is called, otherwise
// it may move data around while writes are pending, causing those writes to
// end up in the wrong place.  The segmented_stream wrapper class takes care
// of this.
void segmented_stream_device::commit()
{
	this->psFirst->seekp(0, std::ios::end);
	io::stream_offset plenStream = this->psFirst->tellp();

	// Call private commit()
	this->commit(0, plenStream);

	assert(this->poffFirstStart == 0);
	assert(this->vcSecond.empty());
	assert(this->psegThird == NULL);

	// Until I can find a cross-platform way of truncating a file, this just
	// write out zeroes in the extra space.
	this->psFirst->seekp(0, std::ios::end);
	plenStream = this->psFirst->tellp();

	// Work around C++ stringstream bug that returns invalid offset when empty.
	// https://issues.apache.org/jira/browse/STDCXX-332
	if (plenStream == (io::stream_offset)-1) plenStream = 0;

	// Now that the data has been committed to the underlying stream, we only
	// have a single source (confirmed above), which should hold all our data.
	// This check makes sure the stream isn't too small, because if it is we've
	// lost some data off the end!
	// This can also happen (in the testing code) when something tries to seek
	// past the end of a stringstream and then perform a write.
	assert(plenStream >= this->poffFirstEnd);

	// If the stream is larger than it should be, zero out the excess.  This will
	// be removed once a cross-platform way of truncating a stream comes about.
	if (plenStream > this->poffFirstEnd) {
		this->psFirst->seekp(this->poffFirstEnd, std::ios::beg);
		plenStream -= this->poffFirstEnd;
		while (plenStream--) {
			this->psFirst->rdbuf()->sputn("\0", 1);
		}
	}
	return;
}

// Commit the data to the underlying stream.  This moves the first segment
// around as necessary, then writes the third segment (which often shares the
// same underlying stream as the first segment) and lastly writes out the
// second segment in the middle.  It has to be done in this order so that no
// data we need gets overwritten before it has been moved out of the way.
void segmented_stream_device::commit(io::stream_offset poffWriteFirst,
	io::stream_offset plenStream)
{
	assert(this->poffFirstStart <= this->poffFirstEnd);

	io::stream_offset lenFirst = this->poffFirstEnd - this->poffFirstStart;
	io::stream_offset lenSecond = this->vcSecond.size();
	io::stream_offset offWriteSecond = poffWriteFirst + lenFirst;
	io::stream_offset offWriteThird = offWriteSecond + lenSecond;

	if (poffWriteFirst > plenStream) {
		// We're going to start writing data past the end of the stream, so we
		// need to enlarge the stream by writing out some dummy data.
		this->psFirst->seekp(plenStream, std::ios::beg);
		for (int i = plenStream; i < poffWriteFirst; i++) {
			this->psFirst->write("\0", 1);
		}
		plenStream = poffWriteFirst;
	}

	if (this->poffFirstStart > poffWriteFirst) {
		// There's data off the front that needs to be trimmed, so move the
		// first source back a bit.
		streamMove(*this->psFirst, this->poffFirstStart, poffWriteFirst, lenFirst);

		this->poffFirstStart = poffWriteFirst;
		this->poffFirstEnd = poffWriteFirst + lenFirst;

		if (this->psegThird) this->psegThird->commit(offWriteThird, plenStream);

	} else if (this->poffFirstStart < poffWriteFirst) {
		// Data has been inserted before us, so we need to push the first source
		// further into the file a bit.

		// Before we do that, we need to make sure the third source has been
		// moved out of the way or we'll overwrite it!
		if (this->psegThird) this->psegThird->commit(offWriteThird, plenStream);

		// Then move the first source forward a bit
		streamMove(*this->psFirst, this->poffFirstStart, poffWriteFirst, lenFirst);

		this->poffFirstStart = poffWriteFirst;
		this->poffFirstEnd = poffWriteFirst + lenFirst;

	} else {
		// First source isn't moving, so (possibly) make room for the second
		// source and commit the third source.

		// offWriteSecond doesn't need to change

		// Write out the third source straight after where the second one will end.
		if (this->psegThird) this->psegThird->commit(offWriteThird, plenStream);

	}

	// Write out the second source to the underlying stream
	if (lenSecond) {
		this->psFirst->seekp(offWriteSecond, std::ios::beg);
		this->psFirst->write((const char *)&this->vcSecond[0], lenSecond);
		this->vcSecond.clear();
		this->poffFirstEnd += lenSecond;
	}

	if (this->psegThird) {
		this->poffFirstEnd += this->psegThird->getLength();
		delete this->psegThird;
		this->psegThird = NULL;
	}

	return;
}

// Split the segstream at the current seek position.  Upon return the first
// data source will only last until the current seek position, the second data
// source will be empty and the third data source will contain all the data
// that was originally after the current seek position.
// Before: AAAABBBB
//             ^ seek position
// After:  AAAABBBB
// first --^   ^-- third (second is empty)
void segmented_stream_device::split()
{
	assert(this->offPos < (this->poffFirstEnd - this->poffFirstStart));

	// Create child segstream
	segmented_stream_device *psegNew = new segmented_stream_device();
	psegNew->offPos = 0;
	// Copy psFirst to segstream's psFirst
	psegNew->psFirst = psFirst;
	// The new one ends where we used to end
	psegNew->poffFirstEnd = this->poffFirstEnd;
	// It starts at the current file pointer
	psegNew->poffFirstStart = this->poffFirstStart + this->offPos;
	// And we now end at the current file pointer
	this->poffFirstEnd = psegNew->poffFirstStart;
	// Move our vcSecond to child segstream's vcSecond
	psegNew->vcSecond = this->vcSecond;
	this->vcSecond.clear();
	// Move our psegThird to child segstream's psegThird
	psegNew->psegThird = this->psegThird; // possibly NULL
	// Make child segstream our psegThird
	this->psegThird = psegNew;
	return;
}


segmented_stream::segmented_stream(iostream_sptr psBase)
	throw () :
		io::stream<segmented_stream_device>(psBase)
{
}

segmented_stream::segmented_stream(const segmented_stream_device& orig)
	throw () :
		io::stream<segmented_stream_device>(orig)
{
}

void segmented_stream::insert(std::streamsize lenInsert)
{
	this->flush();
	this->io::stream<segmented_stream_device>::operator *().insert(lenInsert);
}

void segmented_stream::remove(std::streamsize lenRemove)
{
	this->flush();
	this->io::stream<segmented_stream_device>::operator *().remove(lenRemove);
}

void segmented_stream::commit()
{
	this->flush();
	this->io::stream<segmented_stream_device>::operator *().commit();
}

} // namespace gamearchive
} // namespace camoto
