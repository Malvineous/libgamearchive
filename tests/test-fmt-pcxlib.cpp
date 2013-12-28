/**
 * @file   test-arch-pcxlib.cpp
 * @brief  Test code for PCX library files.
 *
 * Copyright (C) 2010-2013 Adam Nielsen <malvineous@shikadi.net>
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

#include "test-archive.hpp"

// 94-byte header
#define VER "\x01\xCA"
#define HEADER \
	"Copyright (c) Genus Microprogramming, Inc. 1988-90" \
	"\x00\x00\x00\x00\x00\x00\x00\x00" \
	"\x00\x00\x00\x00\x00\x00\x00\x00" \
	"\x00\x00\x00\x00\x00\x00\x00\x00" \
	"\x00\x00\x00\x00\x00\x00\x00\x00" \
	"\x00\x00\x00\x00\x00\x00\x00\x00" \
	"\x00\x00"

// 32-byte header end
#define TRAIL \
	"\x00\x00\x00\x00\x00\x00\x00\x00" \
	"\x00\x00\x00\x00\x00\x00\x00\x00" \
	"\x00\x00\x00\x00\x00\x00\x00\x00" \
	"\x00\x00\x00\x00\x00\x00\x00\x00"

// Header + file count + trail = 128 bytes
// FAT entry is 26 bytes

class test_pcxlib: public test_archive
{
	public:
		test_pcxlib()
		{
			this->type = "pcxlib";
			this->filename[1] = "TWO.DA";
			this->filename[2] = "THREE.D";
			this->lenMaxFilename = 12;
		}

		void addTests()
		{
			this->test_archive::addTests();

			// c00: Initial state
			this->isInstance(ArchiveType::DefinitelyYes, this->initialstate());

			// c01: File too short for signature
			this->isInstance(ArchiveType::DefinitelyNo, STRING_WITH_NULLS(
				VER HEADER "\x00\x00"
			));

			// c02: Unsupported version
			this->isInstance(ArchiveType::DefinitelyNo, STRING_WITH_NULLS(
				"\xff\xff" HEADER "\x02\x00" TRAIL
				"\x00" "ONE     .DAT\0" "\xd6\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00"
				"\x00" "TWO     .DA \0" "\xe5\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00"
				"This is one.dat"
				"This is two.dat"
			));

			// c03: File too short for FAT
			this->isInstance(ArchiveType::DefinitelyNo, STRING_WITH_NULLS(
				VER HEADER "\x02\x00" TRAIL
				"\x00" "ONE     .DAT\0" "\xd6\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00"
			));

			// c04: No/invalid sync byte
			this->isInstance(ArchiveType::DefinitelyNo, STRING_WITH_NULLS(
				VER HEADER "\x02\x00" TRAIL
				"\x00" "ONE     .DAT\0" "\xd6\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00"
				"\x00" "TWO     .DA \0" "\xe5\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00"
				"This is one.dat"
				"This is two.dat"
			));

			// c05: Bad filename
			this->isInstance(ArchiveType::DefinitelyNo, STRING_WITH_NULLS(
				VER HEADER "\x02\x00" TRAIL
				"\x00" "ONE     .DAT\0" "\xd6\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00"
				"\x00" "TWO      DA \0" "\xe5\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00"
				"This is one.dat"
				"This is two.dat"
			));

			// c06: File inside FAT
			this->isInstance(ArchiveType::DefinitelyNo, STRING_WITH_NULLS(
				VER HEADER "\x02\x00" TRAIL
				"\x00" "ONE     .DAT\0" "\xd6\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00"
				"\x00" "TWO     .DA \0" "\x05\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00"
				"This is one.dat"
				"This is two.dat"
			));

			// c07: Truncated file
			this->isInstance(ArchiveType::DefinitelyNo, STRING_WITH_NULLS(
				VER HEADER "\x02\x00" TRAIL
				"\x00" "ONE     .DAT\0" "\xd6\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00"
				"\x00" "TWO     .DA \0" "\xe5\x00\x00\x00" "\xff\x00\x00\x00" "\x00\x00" "\x00\x00"
				"This is one.dat"
				"This is two.dat"
			));
		}

		virtual std::string initialstate()
		{
			return STRING_WITH_NULLS(
				VER HEADER "\x02\x00" TRAIL
				"\x00" "ONE     .DAT\0" "\xb4\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00"
				"\x00" "TWO     .DA \0" "\xc3\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00"
				"This is one.dat"
				"This is two.dat"
			);
		}

		virtual std::string rename()
		{
			return STRING_WITH_NULLS(
				VER HEADER "\x02\x00" TRAIL
				"\x00" "THREE   .D  \0" "\xb4\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00"
				"\x00" "TWO     .DA \0" "\xc3\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00"
				"This is one.dat"
				"This is two.dat"
			);
		}

		virtual std::string insert_end()
		{
			return STRING_WITH_NULLS(
				VER HEADER "\x03\x00" TRAIL
				"\x00" "ONE     .DAT\0" "\xce\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00"
				"\x00" "TWO     .DA \0" "\xdd\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00"
				"\x00" "THREE   .D  \0" "\xec\x00\x00\x00" "\x11\x00\x00\x00" "\x00\x00" "\x00\x00"
				"This is one.dat"
				"This is two.dat"
				"This is three.dat"
			);
		}

		virtual std::string insert_mid()
		{
			return STRING_WITH_NULLS(
				VER HEADER "\x03\x00" TRAIL
				"\x00" "ONE     .DAT\0" "\xce\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00"
				"\x00" "THREE   .D  \0" "\xdd\x00\x00\x00" "\x11\x00\x00\x00" "\x00\x00" "\x00\x00"
				"\x00" "TWO     .DA \0" "\xee\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00"
				"This is one.dat"
				"This is three.dat"
				"This is two.dat"
			);
		}

		virtual std::string insert2()
		{
			return STRING_WITH_NULLS(
				VER HEADER "\x04\x00" TRAIL
				"\x00" "ONE     .DAT\0" "\xe8\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00"
				"\x00" "THREE   .D  \0" "\xf7\x00\x00\x00" "\x11\x00\x00\x00" "\x00\x00" "\x00\x00"
				"\x00" "FOUR    .DAT\0" "\x08\x01\x00\x00" "\x10\x00\x00\x00" "\x00\x00" "\x00\x00"
				"\x00" "TWO     .DA \0" "\x18\x01\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00"
				"This is one.dat"
				"This is three.dat"
				"This is four.dat"
				"This is two.dat"
			);
		}

		virtual std::string remove()
		{
			return STRING_WITH_NULLS(
				VER HEADER "\x01\x00" TRAIL
				"\x00" "TWO     .DA \0" "\x9a\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00"
				"This is two.dat"
			);
		}

		virtual std::string remove2()
		{
			return STRING_WITH_NULLS(
				VER HEADER "\x00\x00" TRAIL
			);
		}

		virtual std::string insert_remove()
		{
			return STRING_WITH_NULLS(
				VER HEADER "\x02\x00" TRAIL
				"\x00" "THREE   .D  \0" "\xb4\x00\x00\x00" "\x11\x00\x00\x00" "\x00\x00" "\x00\x00"
				"\x00" "TWO     .DA \0" "\xc5\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00"
				"This is three.dat"
				"This is two.dat"
			);
		}

		virtual std::string move()
		{
			return STRING_WITH_NULLS(
				VER HEADER "\x02\x00" TRAIL
				"\x00" "TWO     .DA \0" "\xb4\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00"
				"\x00" "ONE     .DAT\0" "\xc3\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00"
				"This is two.dat"
				"This is one.dat"
			);
		}

		virtual std::string resize_larger()
		{
			return STRING_WITH_NULLS(
				VER HEADER "\x02\x00" TRAIL
				"\x00" "ONE     .DAT\0" "\xb4\x00\x00\x00" "\x14\x00\x00\x00" "\x00\x00" "\x00\x00"
				"\x00" "TWO     .DA \0" "\xc8\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00"
				"This is one.dat\0\0\0\0\0"
				"This is two.dat"
			);
		}

		virtual std::string resize_smaller()
		{
			return STRING_WITH_NULLS(
				VER HEADER "\x02\x00" TRAIL
				"\x00" "ONE     .DAT\0" "\xb4\x00\x00\x00" "\x0a\x00\x00\x00" "\x00\x00" "\x00\x00"
				"\x00" "TWO     .DA \0" "\xbe\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00"
				"This is on"
				"This is two.dat"
			);
		}

		virtual std::string resize_write()
		{
			return STRING_WITH_NULLS(
				VER HEADER "\x02\x00" TRAIL
				"\x00" "ONE     .DAT\0" "\xb4\x00\x00\x00" "\x17\x00\x00\x00" "\x00\x00" "\x00\x00"
				"\x00" "TWO     .DA \0" "\xcb\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00"
				"Now resized to 23 chars"
				"This is two.dat"
			);
		}
};

IMPLEMENT_TESTS(pcxlib);
