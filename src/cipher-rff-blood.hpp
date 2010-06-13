/*
 * cipher-rff-blood.hpp - Class declaration for a C++ iostream that encrypts and
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

#ifndef _CAMOTO_CIPHER_RFF_BLOOD_HPP_
#define _CAMOTO_CIPHER_RFF_BLOOD_HPP_

#include <iosfwd>                           // streamsize, seekdir
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/categories.hpp>   // seekable_device_tag
#include <boost/iostreams/positioning.hpp>  // stream_offset

#include <camoto/types.hpp>

namespace camoto {
namespace gamearchive {

namespace io = boost::iostreams;

class RFF_FAT_Cipher_device {
	private:
		iostream_sptr parent;
		uint8_t seed;
		uint8_t curKey, keyOffset;

	public:
		typedef char char_type;
		typedef io::seekable_device_tag category;

		RFF_FAT_Cipher_device(iostream_sptr parent, uint8_t seed)
			throw ();

		RFF_FAT_Cipher_device(const RFF_FAT_Cipher_device&)
			throw ();

		~RFF_FAT_Cipher_device()
			throw ();

		std::streamsize read(char_type *s, std::streamsize n);

		std::streamsize write(const char_type *s, std::streamsize n);

		io::stream_offset seek(io::stream_offset off, std::ios_base::seekdir way);

	protected:
		void changeSeed(uint8_t newSeed)
			throw ();

	friend class RFF_FAT_Cipher;

};


// This is the actual Boost iostream class.
//
// All these functions (including c'tors) call their equivalents in the
// segmented_stream_device class.  See the comments in the class above.
class RFF_FAT_Cipher: public io::stream<RFF_FAT_Cipher_device>
{
	public:
		RFF_FAT_Cipher(iostream_sptr parent, uint8_t seed)
			throw ();

		RFF_FAT_Cipher(const RFF_FAT_Cipher_device& orig)
			throw ();

		void changeSeed(uint8_t newSeed)
			throw ();

};

class RFF_File_Cipher_device {
	private:
		iostream_sptr parent;
		io::stream_offset pos;

	public:
		typedef char char_type;
		typedef io::seekable_device_tag category;

		RFF_File_Cipher_device(iostream_sptr parent)
			throw ();

		RFF_File_Cipher_device(const RFF_File_Cipher_device&)
			throw ();

		~RFF_File_Cipher_device()
			throw ();

		std::streamsize read(char_type *s, std::streamsize n);

		std::streamsize write(const char_type *s, std::streamsize n);

		io::stream_offset seek(io::stream_offset off, std::ios_base::seekdir way);

	protected:
		iostream_sptr getParentStream() const
			throw ();

	friend class RFF_File_Cipher;

};


// This is the actual Boost iostream class.
//
// All these functions (including c'tors) call their equivalents in the
// segmented_stream_device class.  See the comments in the class above.
class RFF_File_Cipher: public io::stream<RFF_File_Cipher_device>
{
	public:
		RFF_File_Cipher(iostream_sptr parent)
			throw ();

		RFF_File_Cipher(const RFF_File_Cipher_device& orig)
			throw ();

		iostream_sptr getParentStream() const
			throw ();

};

} // namespace gamearchive
} // namespace camoto

#endif // _CAMOTO_CIPHER_RFF_BLOOD_HPP_
