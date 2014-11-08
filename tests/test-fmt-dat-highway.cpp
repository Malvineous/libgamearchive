/**
 * @file   test-fmt-dat-highway.cpp
 * @brief  Test code for Highway Hunter .DAT archives.
 *
 * Copyright (C) 2010-2014 Adam Nielsen <malvineous@shikadi.net>
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

class test_dat_highway: public test_archive
{
	public:
		test_dat_highway()
		{
			this->type = "dat-highway";
			this->lenMaxFilename = 12;
			this->outputWidth = 17;
			this->filename[0] = "one.dat";
			this->filename[1] = "two.dat";
			this->filename[2] = "three.dat";
			this->filename[3] = "four.dat";
		}

		void addTests()
		{
			this->test_archive::addTests();

			// c00: Initial state
			this->isInstance(ArchiveType::DefinitelyYes, this->initialstate());

			// c01: File too short
			this->isInstance(ArchiveType::DefinitelyNo, STRING_WITH_NULLS(
				"\x11\x00"
				"\x00\x00\x00\x00" "\0\0\0\0\0\0\0\0\0\0\0\0" // "\0"
			));

			// c02: FAT is not a multiple of the FAT entry length
			this->isInstance(ArchiveType::DefinitelyNo, STRING_WITH_NULLS(
				"\x34\x00"
				"\x36\x00\x00\x00" "one.dat\0\0\0\0\0\0"
				"\x49\x00\x00\x00" "two.dat\0\0\0\0\0\0"
				"\x00\x00\x00\x00" "\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\x00"
				"\x0F\x00\x00\x00" "This is one.dat"
				"\x0F\x00\x00\x00" "This is two.dat"
			));

			// c03: Offset past EOF
			this->isInstance(ArchiveType::DefinitelyNo, STRING_WITH_NULLS(
				"\x33\x00"
				"\x35\x10\x00\x00" "one.dat\0\0\0\0\0\0"
				"\x48\x00\x00\x00" "two.dat\0\0\0\0\0\0"
				"\x00\x00\x00\x00" "\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\x0F\x00\x00\x00" "This is one.dat"
				"\x0F\x00\x00\x00" "This is two.dat"
			));

			// c04: File starts inside FAT
			this->isInstance(ArchiveType::DefinitelyNo, STRING_WITH_NULLS(
				"\x33\x00"
				"\x35\x00\x00\x00" "one.dat\0\0\0\0\0\0"
				"\x04\x00\x00\x00" "two.dat\0\0\0\0\0\0"
				"\x00\x00\x00\x00" "\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\x0F\x00\x00\x00" "This is one.dat"
				"\x0F\x00\x00\x00" "This is two.dat"
			));

			// c05: Filename isn't null terminated
			this->isInstance(ArchiveType::DefinitelyNo, STRING_WITH_NULLS(
				"\x33\x00"
				"\x35\x00\x00\x00" "one.dat\0\0\0\0\0*"
				"\x48\x00\x00\x00" "two.dat\0\0\0\0\0\0"
				"\x00\x00\x00\x00" "\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\x0F\x00\x00\x00" "This is one.dat"
				"\x0F\x00\x00\x00" "This is two.dat"
			));

			// c06: Final file must be empty
			this->isInstance(ArchiveType::DefinitelyNo, STRING_WITH_NULLS(
				"\x33\x00"
				"\x35\x00\x00\x00" "one.dat\0\0\0\0\0\0"
				"\x48\x00\x00\x00" "two.dat\0\0\0\0\0\0"
				"\x45\x00\x00\x00" "\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\x0F\x00\x00\x00" "This is one.dat"
				"\x0F\x00\x00\x00" "This is two.dat"
			));

			// c07: FAT length too small to hold final null entry
			this->isInstance(ArchiveType::DefinitelyNo, STRING_WITH_NULLS(
				"\x00\x00" // Don't use 0x10 because it's not a multiple of 0x11
				"\x00\x00\x00\x00" "\0\0\0\0\0\0\0\0\0\0\0\0\0"
			));
		}

		virtual std::string initialstate()
		{
			return STRING_WITH_NULLS(
				"\x33\x00"
				"\x35\x00\x00\x00" "one.dat\0\0\0\0\0\0"
				"\x48\x00\x00\x00" "two.dat\0\0\0\0\0\0"
				"\x00\x00\x00\x00" "\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\x0F\x00\x00\x00" "This is one.dat"
				"\x0F\x00\x00\x00" "This is two.dat"
			);
		}

		virtual std::string rename()
		{
			return STRING_WITH_NULLS(
				"\x33\x00"
				"\x35\x00\x00\x00" "three.dat\0\0\0\0"
				"\x48\x00\x00\x00" "two.dat\0\0\0\0\0\0"
				"\x00\x00\x00\x00" "\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\x0F\x00\x00\x00" "This is one.dat"
				"\x0F\x00\x00\x00" "This is two.dat"
			);
		}

		virtual std::string insert_end()
		{
			return STRING_WITH_NULLS(
				"\x44\x00"
				"\x46\x00\x00\x00" "one.dat\0\0\0\0\0\0"
				"\x59\x00\x00\x00" "two.dat\0\0\0\0\0\0"
				"\x6C\x00\x00\x00" "three.dat\0\0\0\0"
				"\x00\x00\x00\x00" "\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\x0F\x00\x00\x00" "This is one.dat"
				"\x0F\x00\x00\x00" "This is two.dat"
				"\x11\x00\x00\x00" "This is three.dat"
			);
		}

		virtual std::string insert_mid()
		{
			return STRING_WITH_NULLS(
				"\x44\x00"
				"\x46\x00\x00\x00" "one.dat\0\0\0\0\0\0"
				"\x59\x00\x00\x00" "three.dat\0\0\0\0"
				"\x6E\x00\x00\x00" "two.dat\0\0\0\0\0\0"
				"\x00\x00\x00\x00" "\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\x0F\x00\x00\x00" "This is one.dat"
				"\x11\x00\x00\x00" "This is three.dat"
				"\x0F\x00\x00\x00" "This is two.dat"
			);
		}

		virtual std::string insert2()
		{
			return STRING_WITH_NULLS(
				"\x55\x00"
				"\x57\x00\x00\x00" "one.dat\0\0\0\0\0\0"
				"\x6A\x00\x00\x00" "three.dat\0\0\0\0"
				"\x7F\x00\x00\x00" "four.dat\0\0\0\0\0"
				"\x93\x00\x00\x00" "two.dat\0\0\0\0\0\0"
				"\x00\x00\x00\x00" "\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\x0F\x00\x00\x00" "This is one.dat"
				"\x11\x00\x00\x00" "This is three.dat"
				"\x10\x00\x00\x00" "This is four.dat"
				"\x0F\x00\x00\x00" "This is two.dat"
			);
		}

		virtual std::string remove()
		{
			return STRING_WITH_NULLS(
				"\x22\x00"
				"\x24\x00\x00\x00" "two.dat\0\0\0\0\0\0"
				"\x00\x00\x00\x00" "\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\x0F\x00\x00\x00" "This is two.dat"
			);
		}

		virtual std::string remove2()
		{
			return STRING_WITH_NULLS(
				"\x11\x00"
				"\x00\x00\x00\x00" "\0\0\0\0\0\0\0\0\0\0\0\0\0"
			);
		}

		virtual std::string insert_remove()
		{
			return STRING_WITH_NULLS(
				"\x33\x00"
				"\x35\x00\x00\x00" "three.dat\0\0\0\0"
				"\x4A\x00\x00\x00" "two.dat\0\0\0\0\0\0"
				"\x00\x00\x00\x00" "\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\x11\x00\x00\x00" "This is three.dat"
				"\x0F\x00\x00\x00" "This is two.dat"
			);
		}

		virtual std::string move()
		{
			return STRING_WITH_NULLS(
				"\x33\x00"
				"\x35\x00\x00\x00" "two.dat\0\0\0\0\0\0"
				"\x48\x00\x00\x00" "one.dat\0\0\0\0\0\0"
				"\x00\x00\x00\x00" "\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\x0F\x00\x00\x00" "This is two.dat"
				"\x0F\x00\x00\x00" "This is one.dat"
			);
		}

		virtual std::string resize_larger()
		{
			return STRING_WITH_NULLS(
				"\x33\x00"
				"\x35\x00\x00\x00" "one.dat\0\0\0\0\0\0"
				"\x4D\x00\x00\x00" "two.dat\0\0\0\0\0\0"
				"\x00\x00\x00\x00" "\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\x14\x00\x00\x00" "This is one.dat\0\0\0\0\0"
				"\x0F\x00\x00\x00" "This is two.dat"
			);
		}

		virtual std::string resize_smaller()
		{
			return STRING_WITH_NULLS(
				"\x33\x00"
				"\x35\x00\x00\x00" "one.dat\0\0\0\0\0\0"
				"\x43\x00\x00\x00" "two.dat\0\0\0\0\0\0"
				"\x00\x00\x00\x00" "\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\x0A\x00\x00\x00" "This is on"
				"\x0F\x00\x00\x00" "This is two.dat"
			);
		}

		virtual std::string resize_write()
		{
			return STRING_WITH_NULLS(
				"\x33\x00"
				"\x35\x00\x00\x00" "one.dat\0\0\0\0\0\0"
				"\x50\x00\x00\x00" "two.dat\0\0\0\0\0\0"
				"\x00\x00\x00\x00" "\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\x17\x00\x00\x00" "Now resized to 23 chars"
				"\x0F\x00\x00\x00" "This is two.dat"
			);
		}
};

IMPLEMENT_TESTS(dat_highway);
