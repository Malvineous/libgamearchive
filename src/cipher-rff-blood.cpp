/*
 * cipher-rff-blood.cpp - Class declaration for a C++ iostream that encrypts and
 *   decrypts data in Blood RFF files.
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

#include "cipher-rff-blood.hpp"
#include "debug.hpp"

namespace camoto {
namespace gamearchive {

#define RFF_FILE_CRYPT_LEN 256  // number of bytes encrypted from start of file

refcount_declclass(RFF_FAT_Cipher_device);

RFF_FAT_Cipher_device::RFF_FAT_Cipher_device(iostream_sptr parent, uint8_t seed)
	throw () :
		parent(parent),
		seed(seed),
		curKey(seed),
		keyOffset(0)
{
	assert(parent);
	refcount_qenterclass(RFF_FAT_Cipher_device);
}

RFF_FAT_Cipher_device::RFF_FAT_Cipher_device(const RFF_FAT_Cipher_device& orig)
	throw () :
		parent(orig.parent), // TODO: This won't create an independent file pointer
		seed(orig.seed),
		curKey(orig.curKey),
		keyOffset(orig.keyOffset)
{
	refcount_qenterclass(RFF_FAT_Cipher_device);
}

RFF_FAT_Cipher_device::~RFF_FAT_Cipher_device()
	throw ()
{
	refcount_qexitclass(RFF_FAT_Cipher_device);
}


std::streamsize RFF_FAT_Cipher_device::read(char_type *s, std::streamsize n)
{
	this->parent->read(s, n);
	std::streamsize len = this->parent->gcount();

	for (int i = 0; i < len; i++) {
		s[i] ^= this->curKey;

		// Every two bytes increase the key
		if (this->keyOffset) this->curKey++;
		this->keyOffset ^= 1;
	}
	return len;
}

std::streamsize RFF_FAT_Cipher_device::write(const char_type *s, std::streamsize n)
{
	return n; // TEMP
#define ENC_BLOCKSIZE 2
	char buf[ENC_BLOCKSIZE];
	std::streamsize len = 0;
	std::streamsize left = ENC_BLOCKSIZE;
	while (n > 0) {
		if (n < left) left = n;
		for (int i = 0; i < left; i++, s++) {
			buf[i] = *s ^ this->curKey;

			// Every two bytes increase the key
			if (this->keyOffset) this->curKey++;
			this->keyOffset ^= 1;
		}
		std::streamsize amt = this->parent->rdbuf()->sputn(buf, left);
		if (amt < 0) break; // TODO: This will leave the key corrupted (won't match seek pos)
		len += amt;
		if (amt < left) break; // short write, TODO: will also leave key corrupted
		n -= amt;
	}
	return len;
}

io::stream_offset RFF_FAT_Cipher_device::seek(io::stream_offset off, std::ios_base::seekdir way)
{
	this->parent->seekg(off, way); // use seekp too??
	std::streamsize pos = this->parent->tellg();

	// Calculate the current key from the new offset
	this->curKey = this->seed + pos / 2;
	this->keyOffset = pos % 2;

	return pos;
}

void RFF_FAT_Cipher_device::changeSeed(uint8_t newSeed)
	throw ()
{
	// A transition seed has the effect of decrypting the data with the old key
	// and then re-encrypting with the new key, in the same operation.
	uint8_t transitionSeed = this->seed ^ newSeed;

	// Re-encrypt
	std::streamsize pos = this->parent->tellg();
	this->parent->seekg(0, std::ios::end);
	std::streamsize end = this->parent->tellg();
	this->parent->seekg(0, std::ios::beg);
	std::streamsize cur = 0;
#define TRANSCRYPT_BUFFER_SIZE 3 //64
	char buf[TRANSCRYPT_BUFFER_SIZE];
	std::streamsize len = TRANSCRYPT_BUFFER_SIZE;
	int toggle = 0;
	while (cur < end) {
		if (cur + len > end) len = end - cur;
		this->parent->read(buf, len);
		for (int i = 0; i < len; i++) {
			buf[i] ^= transitionSeed;
			if (toggle) transitionSeed++;
			toggle ^= 1;
		}
		cur += len;
	}

	this->parent->seekg(pos, std::ios::beg);

	this->seed = newSeed;

	return;
}

RFF_FAT_Cipher::RFF_FAT_Cipher(iostream_sptr parent, uint8_t seed)
	throw () :
		io::stream<RFF_FAT_Cipher_device>(parent, seed)
{
}

RFF_FAT_Cipher::RFF_FAT_Cipher(const RFF_FAT_Cipher_device& orig)
	throw () :
		io::stream<RFF_FAT_Cipher_device>(orig)
{
}

void RFF_FAT_Cipher::changeSeed(uint8_t newSeed)
	throw ()
{
	this->io::stream<RFF_FAT_Cipher_device>::operator *().changeSeed(newSeed);
	return;
}

refcount_declclass(RFF_File_Cipher_device);

RFF_File_Cipher_device::RFF_File_Cipher_device(iostream_sptr parent)
	throw () :
		parent(parent),
		pos(0)
{
	assert(parent);
	refcount_qenterclass(RFF_File_Cipher_device);
}

RFF_File_Cipher_device::RFF_File_Cipher_device(const RFF_File_Cipher_device& orig)
	throw () :
		parent(orig.parent), // TODO: This won't create an independent file pointer
		pos(orig.pos)
{
	refcount_qenterclass(RFF_File_Cipher_device);
}

RFF_File_Cipher_device::~RFF_File_Cipher_device()
	throw ()
{
	refcount_qexitclass(RFF_File_Cipher_device);
}


std::streamsize RFF_File_Cipher_device::read(char_type *s, std::streamsize n)
{
	this->parent->read(s, n);
	std::streamsize len = this->parent->gcount();

	for (int i = this->pos; i < RFF_FILE_CRYPT_LEN; i++) {
		s[i] ^= i >> 1;
	}
	this->pos += len;
	return len;
}

std::streamsize RFF_File_Cipher_device::write(const char_type *s, std::streamsize n)
{
	char c;
	std::streamsize p = 0;
	while (p < n) {
		if (this->pos < RFF_FILE_CRYPT_LEN) {
			c = s[p] ^ (this->pos >> 1);
		} else {
			c = s[p];
		}
		//if (this->parent->rdbuf()->sputc(c) < 1) break;
		this->parent->put(c); // TODO: how do faults behave?
		this->pos++;
		p++;
	}
	return p;
}

io::stream_offset RFF_File_Cipher_device::seek(io::stream_offset off, std::ios_base::seekdir way)
{
	this->parent->seekg(off, way); // use seekp too??
	this->pos = this->parent->tellg();

	return this->pos;
}

iostream_sptr RFF_File_Cipher_device::getParentStream() const
	throw ()
{
	return this->parent;
}

RFF_File_Cipher::RFF_File_Cipher(iostream_sptr parent)
	throw () :
		io::stream<RFF_File_Cipher_device>(parent)
{
}

RFF_File_Cipher::RFF_File_Cipher(const RFF_File_Cipher_device& orig)
	throw () :
		io::stream<RFF_File_Cipher_device>(orig)
{
}

iostream_sptr RFF_File_Cipher::getParentStream() const
	throw ()
{
	return const_cast<RFF_File_Cipher *>(this)->io::stream<RFF_File_Cipher_device>::operator *().getParentStream();
}

} // namespace gamearchive
} // namespace camoto
