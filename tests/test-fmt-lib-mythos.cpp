/**
 * @file   test-arch-lib-mythos.cpp
 * @brief  Test code for Mythos Software .LIB archives.
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

class test_lib_mythos: public test_archive
{
	public:
		test_lib_mythos()
		{
			this->type = "lib-mythos";
			this->lenMaxFilename = 12;
			this->outputWidth = 17;
		}

		void addTests()
		{
			this->test_archive::addTests();

			// c00: Initial state
			this->isInstance(ArchiveType::DefinitelyYes, this->initialstate());

			// c01: Bad signature
			this->isInstance(ArchiveType::DefinitelyNo, STRING_WITH_NULLS(
				"LIC\x1A" "\x02\x00"
				"ONE.DAT\0\0\0\0\0\0"          "\x39\x00\x00\x00"
				"TWO.DAT\0\0\0\0\0\0"          "\x48\x00\x00\x00"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0"   "\x57\x00\x00\x00"
				"This is one.dat"
				"This is two.dat"
			));

			// c02: File too short
			this->isInstance(ArchiveType::DefinitelyNo, STRING_WITH_NULLS(
				"LIB\x1A" "\x00"
			));
		}

		virtual std::string initialstate()
		{
			return STRING_WITH_NULLS(
				"LIB\x1A" "\x02\x00"
				"ONE.DAT\0\0\0\0\0\0"          "\x39\x00\x00\x00"
				"TWO.DAT\0\0\0\0\0\0"          "\x48\x00\x00\x00"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0"   "\x57\x00\x00\x00"
				"This is one.dat"
				"This is two.dat"
			);
		}

		virtual std::string rename()
		{
			return STRING_WITH_NULLS(
				"LIB\x1A" "\x02\x00"
				"THREE.DAT\0\0\0\0"            "\x39\x00\x00\x00"
				"TWO.DAT\0\0\0\0\0\0"          "\x48\x00\x00\x00"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0"   "\x57\x00\x00\x00"
				"This is one.dat"
				"This is two.dat"
			);
		}

		virtual std::string insert_end()
		{
			return STRING_WITH_NULLS(
				"LIB\x1A" "\x03\x00"
				"ONE.DAT\0\0\0\0\0\0"          "\x4A\x00\x00\x00"
				"TWO.DAT\0\0\0\0\0\0"          "\x59\x00\x00\x00"
				"THREE.DAT\0\0\0\0"            "\x68\x00\x00\x00"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0"   "\x79\x00\x00\x00"
				"This is one.dat"
				"This is two.dat"
				"This is three.dat"
			);
		}

		virtual std::string insert_mid()
		{
			return STRING_WITH_NULLS(
				"LIB\x1A" "\x03\x00"
				"ONE.DAT\0\0\0\0\0\0"          "\x4A\x00\x00\x00"
				"THREE.DAT\0\0\0\0"            "\x59\x00\x00\x00"
				"TWO.DAT\0\0\0\0\0\0"          "\x6A\x00\x00\x00"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0"   "\x79\x00\x00\x00"
				"This is one.dat"
				"This is three.dat"
				"This is two.dat"
			);
		}

		virtual std::string insert2()
		{
			return STRING_WITH_NULLS(
				"LIB\x1A" "\x04\x00"
				"ONE.DAT\0\0\0\0\0\0"          "\x5B\x00\x00\x00"
				"THREE.DAT\0\0\0\0"            "\x6A\x00\x00\x00"
				"FOUR.DAT\0\0\0\0\0"           "\x7B\x00\x00\x00"
				"TWO.DAT\0\0\0\0\0\0"          "\x8B\x00\x00\x00"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0"   "\x9A\x00\x00\x00"
				"This is one.dat"
				"This is three.dat"
				"This is four.dat"
				"This is two.dat"
			);
		}

		virtual std::string remove()
		{
			return STRING_WITH_NULLS(
				"LIB\x1A" "\x01\x00"
				"TWO.DAT\0\0\0\0\0\0"          "\x28\x00\x00\x00"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0"   "\x37\x00\x00\x00"
				"This is two.dat"
			);
		}

		virtual std::string remove2()
		{
			return STRING_WITH_NULLS(
				"LIB\x1A" "\x00\x00"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0"   "\x17\x00\x00\x00"
			);
		}

		virtual std::string insert_remove()
		{
			return STRING_WITH_NULLS(
				"LIB\x1A" "\x02\x00"
				"THREE.DAT\0\0\0\0"            "\x39\x00\x00\x00"
				"TWO.DAT\0\0\0\0\0\0"          "\x4A\x00\x00\x00"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0"   "\x59\x00\x00\x00"
				"This is three.dat"
				"This is two.dat"
			);
		}

		virtual std::string move()
		{
			return STRING_WITH_NULLS(
				"LIB\x1A" "\x02\x00"
				"TWO.DAT\0\0\0\0\0\0"          "\x39\x00\x00\x00"
				"ONE.DAT\0\0\0\0\0\0"          "\x48\x00\x00\x00"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0"   "\x57\x00\x00\x00"
				"This is two.dat"
				"This is one.dat"
			);
		}

		virtual std::string resize_larger()
		{
			return STRING_WITH_NULLS(
				"LIB\x1A" "\x02\x00"
				"ONE.DAT\0\0\0\0\0\0"          "\x39\x00\x00\x00"
				"TWO.DAT\0\0\0\0\0\0"          "\x4D\x00\x00\x00"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0"   "\x5C\x00\x00\x00"
				"This is one.dat\0\0\0\0\0"
				"This is two.dat"
			);
		}

		virtual std::string resize_smaller()
		{
			return STRING_WITH_NULLS(
				"LIB\x1A" "\x02\x00"
				"ONE.DAT\0\0\0\0\0\0"          "\x39\x00\x00\x00"
				"TWO.DAT\0\0\0\0\0\0"          "\x43\x00\x00\x00"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0"   "\x52\x00\x00\x00"
				"This is on"
				"This is two.dat"
			);
		}

		virtual std::string resize_write()
		{
			return STRING_WITH_NULLS(
				"LIB\x1A" "\x02\x00"
				"ONE.DAT\0\0\0\0\0\0"          "\x39\x00\x00\x00"
				"TWO.DAT\0\0\0\0\0\0"          "\x50\x00\x00\x00"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0"   "\x5F\x00\x00\x00"
				"Now resized to 23 chars"
				"This is two.dat"
			);
		}
};

IMPLEMENT_TESTS(lib_mythos);
