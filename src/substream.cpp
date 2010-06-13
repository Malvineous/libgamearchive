/*
 * substream.cpp - Class declaration for a C++ iostream exposing a limited
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

#include "substream.hpp"
#include "debug.hpp"

namespace camoto {
namespace gamearchive {

refcount_declclass(substream_device);

substream_device::substream_device(iostream_sptr psParent, std::streamsize iOffset, std::streamsize iLength)
	throw () :
		psParent(psParent),
		iOffset(iOffset),
		iLength(iLength),
		iCurPos(0)
{
	assert(psParent);
	refcount_qenterclass(substream_device);
}

substream_device::substream_device(const substream_device& orig)
	throw () :
		psParent(orig.psParent),
		iOffset(orig.iOffset),
		iLength(orig.iLength),
		iCurPos(orig.iCurPos)
{
	refcount_qenterclass(substream_device);
}

substream_device::~substream_device()
	throw ()
{
	refcount_qexitclass(substream_device);
}


std::streamsize substream_device::read(char_type *s, std::streamsize n)
{
	if (this->iCurPos >= this->iLength) return -1; // EOF

	// Make sure we can't read past the end of the file
	if ((this->iCurPos + n) > this->iLength) {
		n = this->iLength - this->iCurPos;
	}

	assert(this->psParent->good());

#ifdef DEBUG
	// Make sure we don't read past the end of the parent stream
	this->psParent->seekg(0, std::ios::end);
	io::stream_offset lenParent = this->psParent->tellg();
	assert(lenParent >= this->iOffset + this->iCurPos + n);
#endif

	this->psParent->seekg(this->iOffset + this->iCurPos);

	this->psParent->read(s, n);
	std::streamsize iLen = this->psParent->gcount();
	this->iCurPos += iLen;
	return iLen;
}

std::streamsize substream_device::write(const char_type *s, std::streamsize n)
{
	if (this->iCurPos >= this->iLength)
		throw std::ios_base::failure("no space left in archive slot");

	if ((this->iCurPos + n) > this->iLength) {
		n = this->iLength - this->iCurPos;
	}

	assert(this->psParent->good());
	this->psParent->seekp(this->iOffset + this->iCurPos);
	std::streamsize iLen = this->psParent->rdbuf()->sputn(s, n);
	this->iCurPos += iLen;

	// Make sure these bytes actually were written to the stream
	assert(this->psParent->tellp() == this->iOffset + this->iCurPos);
	return iLen;
}

io::stream_offset substream_device::seek(io::stream_offset off, std::ios_base::seekdir way)
{
	io::stream_offset iBase = 0;
	switch (way) {
		case std::ios_base::beg:
			//iBase = 0;
			break;
		case std::ios_base::cur:
			iBase = this->iCurPos;
			break;
		case std::ios_base::end:
			iBase = this->iLength;
			break;
	}
	iBase += off;
	if (iBase > this->iLength) {
		// Can't seek past EOF
		iBase = this->iLength;
	} else if (iBase < 0) {
		// Can't see past SOF
		iBase = 0;
	}
	this->iCurPos = iBase;
	return this->iCurPos;
}

// Move the "window" of data (looking into the parent stream) forward or
// back by the given number of bytes.
void substream_device::relocate(io::stream_offset delta)
{
	this->iOffset += delta;
	return;
}

void substream_device::setSize(io::stream_offset len)
{
	this->iLength = len;

	if (this->iCurPos > this->iLength) {
		// Can't seek past EOF
		this->iCurPos = this->iLength;
	}

	// Sanity check: Make sure the size doesn't go past the parent's EOF
	this->psParent->seekp(0, std::ios::end);
	assert(this->psParent->tellp() >= this->iOffset + this->iLength);

	return;
}

io::stream_offset substream_device::getOffset() const
{
	return this->iOffset;
}

io::stream_offset substream_device::getSize() const
{
	return this->iLength;
}


substream::substream(iostream_sptr psParent, std::streamsize iOffset, std::streamsize iLength)
	throw () :
		io::stream<substream_device>(psParent, iOffset, iLength)
{
}

substream::substream(const substream_device& orig)
	throw () :
		io::stream<substream_device>(orig)
{
}

void substream::relocate(io::stream_offset delta)
{
	// We can't flush before the relocate() because if data has been cached,
	// the underlying stream may have since been modified.  Calling flush() now
	// would cause the data to be written to the wrong spot in the underlying
	// stream.  If this is important, the caller must call flush() themselves.
	//this->flush();
	this->io::stream<substream_device>::operator *().relocate(delta);
}

void substream::setSize(io::stream_offset len)
{
	this->io::stream<substream_device>::operator *().setSize(len);
}

io::stream_offset substream::getOffset() const
{
	return const_cast<substream *>(this)->io::stream<substream_device>::operator *().getOffset();
}

io::stream_offset substream::getSize() const
{
	return const_cast<substream *>(this)->io::stream<substream_device>::operator *().getSize();
}

} // namespace gamearchive
} // namespace camoto
