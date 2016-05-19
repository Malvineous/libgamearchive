/**
 * @file   test-fmt-dat-sango.cpp
 * @brief  Test code for Sango Fighter archives.
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

class test_dat_sango: public test_archive
{
	public:
		test_dat_sango()
		{
			this->type = "dat-sango";
			this->lenMaxFilename = -1; // No filenames
		}

		void addTests()
		{
			this->test_archive::addTests();

			// c00: Initial state
			this->isInstance(ArchiveType::Certainty::DefinitelyYes, this->content_12());

			// c01: File too short
			this->isInstance(ArchiveType::Certainty::DefinitelyNo, STRING_WITH_NULLS(
				"\x00\x00"
			));

			// c02: FAT length larger than archive
			this->isInstance(ArchiveType::Certainty::DefinitelyNo, STRING_WITH_NULLS(
				"\xff\x00\x00\x00"
				"\x1b\x00\x00\x00"
				"\x2a\x00\x00\x00"
				"This is one.dat"
				"This is two.dat"
			));

			// c03: File length larger than archive
			this->isInstance(ArchiveType::Certainty::DefinitelyNo, STRING_WITH_NULLS(
				"\x0c\x00\x00\x00"
				"\xff\x00\x00\x00"
				"\x2a\x00\x00\x00"
				"This is one.dat"
				"This is two.dat"
			));

			// c04: Last offset does not equal archive size
			this->isInstance(ArchiveType::Certainty::DefinitelyNo, STRING_WITH_NULLS(
				"\x0c\x00\x00\x00"
				"\x1b\x00\x00\x00"
				"\x2b\x00\x00\x00"
				"This is one.dat"
				"This is two.dat"
			));
		}

		virtual std::string content_12()
		{
			return STRING_WITH_NULLS(
				"\x0c\x00\x00\x00"
				"\x1b\x00\x00\x00"
				"\x2a\x00\x00\x00"
				"This is one.dat"
				"This is two.dat"
			);
		}

		virtual std::string content_1r2()
		{
			// No filenames to rename
			throw 1;
		}

		virtual std::string content_123()
		{
			return STRING_WITH_NULLS(
				"\x10\x00\x00\x00"
				"\x1f\x00\x00\x00"
				"\x2e\x00\x00\x00"
				"\x3f\x00\x00\x00"
				"This is one.dat"
				"This is two.dat"
				"This is three.dat"
			);
		}

		virtual std::string content_132()
		{
			return STRING_WITH_NULLS(
				"\x10\x00\x00\x00"
				"\x1f\x00\x00\x00"
				"\x30\x00\x00\x00"
				"\x3f\x00\x00\x00"
				"This is one.dat"
				"This is three.dat"
				"This is two.dat"
			);
		}

		virtual std::string content_1342()
		{
			return STRING_WITH_NULLS(
				"\x14\x00\x00\x00"
				"\x23\x00\x00\x00"
				"\x34\x00\x00\x00"
				"\x44\x00\x00\x00"
				"\x53\x00\x00\x00"
				"This is one.dat"
				"This is three.dat"
				"This is four.dat"
				"This is two.dat"
			);
		}

		virtual std::string content_2()
		{
			return STRING_WITH_NULLS(
				"\x08\x00\x00\x00"
				"\x17\x00\x00\x00"
				"This is two.dat"
			);
		}

		virtual std::string content_0()
		{
			return STRING_WITH_NULLS(
				"\x04\x00\x00\x00"
			);
		}

		virtual std::string content_32()
		{
			return STRING_WITH_NULLS(
				"\x0c\x00\x00\x00"
				"\x1d\x00\x00\x00"
				"\x2c\x00\x00\x00"
				"This is three.dat"
				"This is two.dat"
			);
		}

		virtual std::string content_21()
		{
			return STRING_WITH_NULLS(
				"\x0c\x00\x00\x00"
				"\x1b\x00\x00\x00"
				"\x2a\x00\x00\x00"
				"This is two.dat"
				"This is one.dat"
			);
		}

		virtual std::string content_1l2()
		{
			return STRING_WITH_NULLS(
				"\x0c\x00\x00\x00"
				"\x20\x00\x00\x00"
				"\x2f\x00\x00\x00"
				"This is one.dat\0\0\0\0\0"
				"This is two.dat"
			);
		}

		virtual std::string content_1s2()
		{
			return STRING_WITH_NULLS(
				"\x0c\x00\x00\x00"
				"\x16\x00\x00\x00"
				"\x25\x00\x00\x00"
				"This is on"
				"This is two.dat"
			);
		}

		virtual std::string content_1w2()
		{
			return STRING_WITH_NULLS(
				"\x0c\x00\x00\x00"
				"\x23\x00\x00\x00"
				"\x32\x00\x00\x00"
				"Now resized to 23 chars"
				"This is two.dat"
			);
		}
};

IMPLEMENT_TESTS(dat_sango);
