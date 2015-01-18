/**
 * @file   test-arch-roads-skyroads.cpp
 * @brief  Test code for SkyRoads Roads archives.
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

class test_roads_skyroads: public test_archive
{
	public:
		test_roads_skyroads()
		{
			this->type = "roads-skyroads";
			this->lenMaxFilename = -1; // no filenames
		}

		void addTests()
		{
			this->test_archive::addTests();

			// c00: Initial state
			this->isInstance(ArchiveType::DefinitelyYes, this->initialstate());

			// c01: Lack of a header means an empty file is a valid empty archive.
			this->isInstance(ArchiveType::DefinitelyYes, STRING_WITH_NULLS(
				""
			));

			// c02: FAT is larger than archive
			this->isInstance(ArchiveType::DefinitelyNo, STRING_WITH_NULLS(
				"\x50\x00\x00\x00"
			));

			// c03: FAT is smaller than single entry
			this->isInstance(ArchiveType::DefinitelyNo, STRING_WITH_NULLS(
				"\x01\x00\x00"
			));

			// c04: FAT is truncated
			this->isInstance(ArchiveType::DefinitelyNo, STRING_WITH_NULLS(
				"\x07\x00\x00\x00" "\x07\x00\x00"
			));

			// c05: File offset is beyond EOF
			this->isInstance(ArchiveType::DefinitelyNo, STRING_WITH_NULLS(
				"\x08\x00\x00\x00" "\x50\x00\x00\x00"
				"blahblah"
			));

			// c06: Sequential offsets decrease, resulting in negative file size
			this->isInstance(ArchiveType::DefinitelyNo, STRING_WITH_NULLS(
				"\x08\x00\x00\x00" "\x04\x00\x00\x00"
				"blahblah"
			));
		}

		virtual std::string initialstate()
		{
			return STRING_WITH_NULLS(
				"\x08\x00" "\x0f\x00"
				"\x17\x00" "\x0f\x00"
				"This is one.dat"
				"This is two.dat"
			);
		}

		virtual std::string rename()
		{
			// No filenames to rename
			throw 1;
		}

		virtual std::string insert_end()
		{
			return STRING_WITH_NULLS(
				"\x0c\x00" "\x0f\x00"
				"\x1b\x00" "\x0f\x00"
				"\x2a\x00" "\x11\x00"
				"This is one.dat"
				"This is two.dat"
				"This is three.dat"
			);
		}

		virtual std::string insert_mid()
		{
			return STRING_WITH_NULLS(
				"\x0c\x00" "\x0f\x00"
				"\x1b\x00" "\x11\x00"
				"\x2c\x00" "\x0f\x00"
				"This is one.dat"
				"This is three.dat"
				"This is two.dat"
			);
		}

		virtual std::string insert2()
		{
			return STRING_WITH_NULLS(
				"\x10\x00" "\x0f\x00"
				"\x1f\x00" "\x11\x00"
				"\x30\x00" "\x10\x00"
				"\x40\x00" "\x0f\x00"
				"This is one.dat"
				"This is three.dat"
				"This is four.dat"
				"This is two.dat"
			);
		}

		virtual std::string remove()
		{
			return STRING_WITH_NULLS(
				"\x04\x00" "\x0f\x00"
				"This is two.dat"
			);
		}

		virtual std::string remove2()
		{
			return STRING_WITH_NULLS(
				""
			);
		}

		virtual std::string insert_remove()
		{
			return STRING_WITH_NULLS(
				"\x08\x00" "\x11\x00"
				"\x19\x00" "\x0f\x00"
				"This is three.dat"
				"This is two.dat"
			);
		}

		virtual std::string move()
		{
			return STRING_WITH_NULLS(
				"\x08\x00" "\x0f\x00"
				"\x17\x00" "\x0f\x00"
				"This is two.dat"
				"This is one.dat"
			);
		}

		virtual std::string resize_larger()
		{
			return STRING_WITH_NULLS(
				"\x08\x00" "\x14\x00"
				"\x1c\x00" "\x0f\x00"
				"This is one.dat\0\0\0\0\0"
				"This is two.dat"
			);
		}

		virtual std::string resize_smaller()
		{
			return STRING_WITH_NULLS(
				"\x08\x00" "\x0a\x00"
				"\x12\x00" "\x0f\x00"
				"This is on"
				"This is two.dat"
			);
		}

		virtual std::string resize_write()
		{
			return STRING_WITH_NULLS(
				"\x08\x00" "\x17\x00"
				"\x1f\x00" "\x0f\x00"
				"Now resized to 23 chars"
				"This is two.dat"
			);
		}
};

IMPLEMENT_TESTS(roads_skyroads);
