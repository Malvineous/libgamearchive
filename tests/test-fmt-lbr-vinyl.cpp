/**
 * @file   test-arch-lbr-vinyl.cpp
 * @brief  Test code for Vinyl Goddess From Mars .LBR archives.
 *
 * Copyright (C) 2010-2015 Adam Nielsen <malvineous@shikadi.net>
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

class test_lbr_vinyl: public test_archive
{
	public:
		test_lbr_vinyl()
		{
			this->type = "lbr-vinyl";
			this->filename_shortext = "LEVEL1-1.M";
			this->lenMaxFilename = 0; // no limit
		}

		void addTests()
		{
			this->test_archive::addTests();

			// c00: Initial state
			this->isInstance(ArchiveType::DefinitelyYes, this->initialstate());

			// c01: File too short
			this->isInstance(ArchiveType::DefinitelyNo, STRING_WITH_NULLS(
				"\x00"
			));

			// c02: Offset beyond EOF
			this->isInstance(ArchiveType::DefinitelyNo, STRING_WITH_NULLS(
				"\x02\x00"
				"\x00\x00" "\x0e\x00\x00\x00"
				"\x00\x00" "\xff\x00\x00\x00"
				"This is one.dat"
				"This is two.dat"
			));

			// c03: Handle truncated FAT
			this->isInstance(ArchiveType::DefinitelyNo, STRING_WITH_NULLS(
				"\x02\x00"
				"\x00\x00" "\x0e\x00\x00\x00"
			));

			// c04: Offset is inside FAT
			this->isInstance(ArchiveType::DefinitelyNo, STRING_WITH_NULLS(
				"\x02\x00"
				"\x00\x00" "\x0e\x00\x00\x00"
				"\x00\x00" "\x02\x00\x00\x00"
				"This is one.dat"
				"This is two.dat"
			));

			// c05: No files but trailing data (which would be data for the first file)
			this->isInstance(ArchiveType::DefinitelyNo, STRING_WITH_NULLS(
				"\x00\x00"
				"\x00\x00" "\x0e\x00\x00\x00"
				"\x00\x00" "\x1d\x00\x00\x00"
				"This is one.dat"
				"This is two.dat"
			));
		}

		virtual std::string initialstate()
		{
			return STRING_WITH_NULLS(
				"\x02\x00"
				"\x7c\xff" "\x0e\x00\x00\x00"
				"\xe0\x97" "\x1d\x00\x00\x00"
				"This is one.dat"
				"This is two.dat"
			);
		}

		virtual std::string rename()
		{
			return STRING_WITH_NULLS(
				"\x02\x00"
				"\x6d\x99" "\x0e\x00\x00\x00"
				"\xe0\x97" "\x1d\x00\x00\x00"
				"This is one.dat"
				"This is two.dat"
			);
		}

		virtual std::string insert_end()
		{
			return STRING_WITH_NULLS(
				"\x03\x00"
				"\x7c\xff" "\x14\x00\x00\x00"
				"\xe0\x97" "\x23\x00\x00\x00"
				"\x6d\x99" "\x32\x00\x00\x00"
				"This is one.dat"
				"This is two.dat"
				"This is three.dat"
			);
		}

		virtual std::string insert_mid()
		{
			return STRING_WITH_NULLS(
				"\x03\x00"
				"\x7c\xff" "\x14\x00\x00\x00"
				"\x6d\x99" "\x23\x00\x00\x00"
				"\xe0\x97" "\x34\x00\x00\x00"
				"This is one.dat"
				"This is three.dat"
				"This is two.dat"
			);
		}

		virtual std::string insert2()
		{
			return STRING_WITH_NULLS(
				"\x04\x00"
				"\x7c\xff" "\x1a\x00\x00\x00"
				"\x6d\x99" "\x29\x00\x00\x00"
				"\xcf\x33" "\x3a\x00\x00\x00"
				"\xe0\x97" "\x4a\x00\x00\x00"
				"This is one.dat"
				"This is three.dat"
				"This is four.dat"
				"This is two.dat"
			);
		}

		virtual std::string remove()
		{
			return STRING_WITH_NULLS(
				"\x01\x00"
				"\xe0\x97" "\x08\x00\x00\x00"
				"This is two.dat"
			);
		}

		virtual std::string remove2()
		{
			return STRING_WITH_NULLS(
				"\x00\x00"
			);
		}

		virtual std::string insert_remove()
		{
			return STRING_WITH_NULLS(
				"\x02\x00"
				"\x6d\x99" "\x0e\x00\x00\x00"
				"\xe0\x97" "\x1f\x00\x00\x00"
				"This is three.dat"
				"This is two.dat"
			);
		}

		virtual std::string move()
		{
			return STRING_WITH_NULLS(
				"\x02\x00"
				"\xe0\x97" "\x0e\x00\x00\x00"
				"\x7c\xff" "\x1d\x00\x00\x00"
				"This is two.dat"
				"This is one.dat"
			);
		}

		virtual std::string resize_larger()
		{
			return STRING_WITH_NULLS(
				"\x02\x00"
				"\x7c\xff" "\x0e\x00\x00\x00"
				"\xe0\x97" "\x22\x00\x00\x00"
				"This is one.dat\0\0\0\0\0"
				"This is two.dat"
			);
		}

		virtual std::string resize_smaller()
		{
			return STRING_WITH_NULLS(
				"\x02\x00"
				"\x7c\xff" "\x0e\x00\x00\x00"
				"\xe0\x97" "\x18\x00\x00\x00"
				"This is on"
				"This is two.dat"
			);
		}

		virtual std::string resize_write()
		{
			return STRING_WITH_NULLS(
				"\x02\x00"
				"\x7c\xff" "\x0e\x00\x00\x00"
				"\xe0\x97" "\x25\x00\x00\x00"
				"Now resized to 23 chars"
				"This is two.dat"
			);
		}
};

IMPLEMENT_TESTS(lbr_vinyl);
