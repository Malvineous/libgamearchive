/**
 * @file   test-fmt-dat-hugo.cpp
 * @brief  Test code for Hugo2/3 .DAT archives.
 *
 * Copyright (C) 2010-2016 Adam Nielsen <malvineous@shikadi.net>
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

class test_dat_hugo: public test_archive
{
	public:
		test_dat_hugo()
		{
			this->type = "dat-hugo";
			this->lenMaxFilename = -1; // no filenames
		}

		void addTests()
		{
			this->test_archive::addTests();

			// c00: Initial state
			this->isInstance(ArchiveType::Certainty::DefinitelyYes, this->content_12());

			// c01: File offset/size is past EOF
			this->isInstance(ArchiveType::Certainty::DefinitelyNo, STRING_WITH_NULLS(
				"\x50\x00\x00\x00" "\x0f\x00\x00\x00"
				"This is one.dat"
			));

			// c02: Too short
			this->isInstance(ArchiveType::Certainty::DefinitelyNo, STRING_WITH_NULLS(
				"\x08\x00\x00\x00" "\x0f\x00\x00"
			));

			// c03: First file finishes past EOF
			this->isInstance(ArchiveType::Certainty::DefinitelyNo, STRING_WITH_NULLS(
				"\x08\x00\x00\x00" "\x50\x00\x00\x00"
				"This is one.dat"
			));

			// c04: Empty file can be valid
			this->isInstance(ArchiveType::Certainty::PossiblyYes, STRING_WITH_NULLS(
				""
			));
		}

		virtual std::string content_12()
		{
			return STRING_WITH_NULLS(
				"\x10\x00\x00\x00" "\x0f\x00\x00\x00"
				"\x1f\x00\x00\x00" "\x0f\x00\x00\x00"
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
				"\x18\x00\x00\x00" "\x0f\x00\x00\x00"
				"\x27\x00\x00\x00" "\x0f\x00\x00\x00"
				"\x36\x00\x00\x00" "\x11\x00\x00\x00"
				"This is one.dat"
				"This is two.dat"
				"This is three.dat"
			);
		}

		virtual std::string content_132()
		{
			return STRING_WITH_NULLS(
				"\x18\x00\x00\x00" "\x0f\x00\x00\x00"
				"\x27\x00\x00\x00" "\x11\x00\x00\x00"
				"\x38\x00\x00\x00" "\x0f\x00\x00\x00"
				"This is one.dat"
				"This is three.dat"
				"This is two.dat"
			);
		}

		virtual std::string content_1342()
		{
			return STRING_WITH_NULLS(
				"\x20\x00\x00\x00" "\x0f\x00\x00\x00"
				"\x2f\x00\x00\x00" "\x11\x00\x00\x00"
				"\x40\x00\x00\x00" "\x10\x00\x00\x00"
				"\x50\x00\x00\x00" "\x0f\x00\x00\x00"
				"This is one.dat"
				"This is three.dat"
				"This is four.dat"
				"This is two.dat"
			);
		}

		virtual std::string content_2()
		{
			return STRING_WITH_NULLS(
				"\x08\x00\x00\x00" "\x0f\x00\x00\x00"
				"This is two.dat"
			);
		}

		virtual std::string content_0()
		{
			return STRING_WITH_NULLS(
				""
			);
		}

		virtual std::string content_32()
		{
			return STRING_WITH_NULLS(
				"\x10\x00\x00\x00" "\x11\x00\x00\x00"
				"\x21\x00\x00\x00" "\x0f\x00\x00\x00"
				"This is three.dat"
				"This is two.dat"
			);
		}

		virtual std::string content_21()
		{
			return STRING_WITH_NULLS(
				"\x10\x00\x00\x00" "\x0f\x00\x00\x00"
				"\x1f\x00\x00\x00" "\x0f\x00\x00\x00"
				"This is two.dat"
				"This is one.dat"
			);
		}

		virtual std::string content_1l2()
		{
			return STRING_WITH_NULLS(
				"\x10\x00\x00\x00" "\x14\x00\x00\x00"
				"\x24\x00\x00\x00" "\x0f\x00\x00\x00"
				"This is one.dat\0\0\0\0\0"
				"This is two.dat"
			);
		}

		virtual std::string content_1s2()
		{
			return STRING_WITH_NULLS(
				"\x10\x00\x00\x00" "\x0a\x00\x00\x00"
				"\x1a\x00\x00\x00" "\x0f\x00\x00\x00"
				"This is on"
				"This is two.dat"
			);
		}

		virtual std::string content_1w2()
		{
			return STRING_WITH_NULLS(
				"\x10\x00\x00\x00" "\x17\x00\x00\x00"
				"\x27\x00\x00\x00" "\x0f\x00\x00\x00"
				"Now resized to 23 chars"
				"This is two.dat"
			);
		}
};

IMPLEMENT_TESTS(dat_hugo);
