/**
 * @file   test-arch-hog-descent.cpp
 * @brief  Test code for Descent .HOG archives.
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

class test_hog_descent: public test_archive
{
	public:
		test_hog_descent()
		{
			this->type = "hog-descent";
			this->lenMaxFilename = 12;
		}

		void addTests()
		{
			this->test_archive::addTests();

			// c00: Initial state
			this->isInstance(ArchiveType::DefinitelyYes, this->initialstate());

			// c01: Bad signature
			this->isInstance(ArchiveType::DefinitelyNo, STRING_WITH_NULLS(
				"DHL"
				"ONE.DAT\0\0\0\0\0\0" "\x0f\x00\x00\x00"
				"This is one.dat"
				"TWO.DAT\0\0\0\0\0\0" "\x0f\x00\x00\x00"
				"This is two.dat"
			));

			// c02: Too short
			this->isInstance(ArchiveType::DefinitelyNo, STRING_WITH_NULLS(
				"DH"
			));
		}

		virtual std::string initialstate()
		{
			return STRING_WITH_NULLS(
				"DHF"
				"ONE.DAT\0\0\0\0\0\0" "\x0f\x00\x00\x00"
				"This is one.dat"
				"TWO.DAT\0\0\0\0\0\0" "\x0f\x00\x00\x00"
				"This is two.dat"
			);
		}

		virtual std::string rename()
		{
			return STRING_WITH_NULLS(
				"DHF"
				"THREE.DAT\0\0\0\0"   "\x0f\x00\x00\x00"
				"This is one.dat"
				"TWO.DAT\0\0\0\0\0\0" "\x0f\x00\x00\x00"
				"This is two.dat"
			);
		}

		virtual std::string insert_end()
		{
			return STRING_WITH_NULLS(
				"DHF"
				"ONE.DAT\0\0\0\0\0\0" "\x0f\x00\x00\x00"
				"This is one.dat"
				"TWO.DAT\0\0\0\0\0\0" "\x0f\x00\x00\x00"
				"This is two.dat"
				"THREE.DAT\0\0\0\0"   "\x11\x00\x00\x00"
				"This is three.dat"
			);
		}

		virtual std::string insert_mid()
		{
			return STRING_WITH_NULLS(
				"DHF"
				"ONE.DAT\0\0\0\0\0\0" "\x0f\x00\x00\x00"
				"This is one.dat"
				"THREE.DAT\0\0\0\0"   "\x11\x00\x00\x00"
				"This is three.dat"
				"TWO.DAT\0\0\0\0\0\0" "\x0f\x00\x00\x00"
				"This is two.dat"
			);
		}

		virtual std::string insert2()
		{
			return STRING_WITH_NULLS(
				"DHF"
				"ONE.DAT\0\0\0\0\0\0" "\x0f\x00\x00\x00"
				"This is one.dat"
				"THREE.DAT\0\0\0\0"   "\x11\x00\x00\x00"
				"This is three.dat"
				"FOUR.DAT\0\0\0\0\0"  "\x10\x00\x00\x00"
				"This is four.dat"
				"TWO.DAT\0\0\0\0\0\0" "\x0f\x00\x00\x00"
				"This is two.dat"
			);
		}

		virtual std::string remove()
		{
			return STRING_WITH_NULLS(
				"DHF"
				"TWO.DAT\0\0\0\0\0\0" "\x0f\x00\x00\x00"
				"This is two.dat"
			);
		}

		virtual std::string remove2()
		{
			return STRING_WITH_NULLS(
				"DHF"
			);
		}

		virtual std::string insert_remove()
		{
			return STRING_WITH_NULLS(
				"DHF"
				"THREE.DAT\0\0\0\0"   "\x11\x00\x00\x00"
				"This is three.dat"
				"TWO.DAT\0\0\0\0\0\0" "\x0f\x00\x00\x00"
				"This is two.dat"
			);
		}

		virtual std::string move()
		{
			return STRING_WITH_NULLS(
				"DHF"
				"TWO.DAT\0\0\0\0\0\0" "\x0f\x00\x00\x00"
				"This is two.dat"
				"ONE.DAT\0\0\0\0\0\0" "\x0f\x00\x00\x00"
				"This is one.dat"
			);
		}

		virtual std::string resize_larger()
		{
			return STRING_WITH_NULLS(
				"DHF"
				"ONE.DAT\0\0\0\0\0\0" "\x14\x00\x00\x00"
				"This is one.dat\0\0\0\0\0"
				"TWO.DAT\0\0\0\0\0\0" "\x0f\x00\x00\x00"
				"This is two.dat"
			);
		}

		virtual std::string resize_smaller()
		{
			return STRING_WITH_NULLS(
				"DHF"
				"ONE.DAT\0\0\0\0\0\0" "\x0a\x00\x00\x00"
				"This is on"
				"TWO.DAT\0\0\0\0\0\0" "\x0f\x00\x00\x00"
				"This is two.dat"
			);
		}

		virtual std::string resize_write()
		{
			return STRING_WITH_NULLS(
				"DHF"
				"ONE.DAT\0\0\0\0\0\0" "\x17\x00\x00\x00"
				"Now resized to 23 chars"
				"TWO.DAT\0\0\0\0\0\0" "\x0f\x00\x00\x00"
				"This is two.dat"
			);
		}
};

IMPLEMENT_TESTS(hog_descent);
