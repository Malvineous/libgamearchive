/**
 * @file   test-fmt-dat-bash.cpp
 * @brief  Test code for uncompressed Monster Bash .DAT files.
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

class test_dat_bash: public test_archive
{
	public:
		test_dat_bash()
		{
			this->type = "dat-bash";
			this->filename[0] = "ONE.MBG";
			this->lenMaxFilename = 30;
		}

		void addTests()
		{
			this->test_archive::addTests();

			// c00: Initial state
			this->isInstance(ArchiveType::Certainty::DefinitelyYes, this->content_12());

			// c01: access charst in by filename
			this->isInstance(ArchiveType::Certainty::DefinitelyNo, STRING_WITH_NULLS(
				"\x20\x00" "\x0f\x00"
					"ONE.DAT\x05\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
					"\x00\x00"
					"This is one.dat"
				"\x20\x00" "\x0f\x00"
					"TWO.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
					"\x00\x00"
					"This is two.dat"
			));

			// c02: Blank archive
			this->isInstance(ArchiveType::Certainty::DefinitelyYes, STRING_WITH_NULLS(
				""
			));

			// c03: File ends past EOF
			this->isInstance(ArchiveType::Certainty::DefinitelyNo, STRING_WITH_NULLS(
				"\x20\x00" "\x0f\x01"
					"ONE.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
					"\x00\x00"
					"This is one.dat"
				"\x20\x00" "\x0f\x00"
					"TWO.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
					"\x00\x00"
					"This is two.dat"
			));

			// c04: Truncated FAT entry
			this->isInstance(ArchiveType::Certainty::DefinitelyNo, STRING_WITH_NULLS(
				"\x20\x00" "\x0f\x00"
					"ONE.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
					"\x00\x00"
					"This is one.dat"
				"\x20\x00" "\x0f\x00"
					"TWO.DA"
			));
		}

		virtual std::string content_12()
		{
			return STRING_WITH_NULLS(
				"\x01\x00" "\x0f\x00"
					"ONE\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
					"\x00\x00"
					"This is one.dat"
				"\x20\x00" "\x0f\x00"
					"TWO.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
					"\x00\x00"
					"This is two.dat"
			);
		}

		virtual std::string content_1r2()
		{
			return STRING_WITH_NULLS(
				"\x20\x00" "\x0f\x00"
					"THREE.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
					"\x00\x00"
					"This is one.dat"
				"\x20\x00" "\x0f\x00"
					"TWO.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
					"\x00\x00"
					"This is two.dat"
			);
		}

		virtual std::string content_123()
		{
			return STRING_WITH_NULLS(
				"\x01\x00" "\x0f\x00"
					"ONE\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
					"\x00\x00"
					"This is one.dat"
				"\x20\x00" "\x0f\x00"
					"TWO.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
					"\x00\x00"
					"This is two.dat"
				"\x20\x00" "\x11\x00"
					"THREE.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
					"\x00\x00"
					"This is three.dat"
			);
		}

		virtual std::string content_132()
		{
			return STRING_WITH_NULLS(
				"\x01\x00" "\x0f\x00"
					"ONE\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
					"\x00\x00"
					"This is one.dat"
				"\x20\x00" "\x11\x00"
					"THREE.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
					"\x00\x00"
					"This is three.dat"
				"\x20\x00" "\x0f\x00"
					"TWO.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
					"\x00\x00"
					"This is two.dat"
			);
		}

		virtual std::string content_1342()
		{
			return STRING_WITH_NULLS(
				"\x01\x00" "\x0f\x00"
					"ONE\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
					"\x00\x00"
					"This is one.dat"
				"\x20\x00" "\x11\x00"
					"THREE.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
					"\x00\x00"
					"This is three.dat"
				"\x20\x00" "\x10\x00"
					"FOUR.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
					"\x00\x00"
					"This is four.dat"
				"\x20\x00" "\x0f\x00"
					"TWO.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
					"\x00\x00"
					"This is two.dat"
			);
		}

		virtual std::string content_2()
		{
			return STRING_WITH_NULLS(
				"\x20\x00" "\x0f\x00"
					"TWO.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
					"\x00\x00"
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
				"\x20\x00" "\x11\x00"
					"THREE.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
					"\x00\x00"
					"This is three.dat"
				"\x20\x00" "\x0f\x00"
					"TWO.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
					"\x00\x00"
					"This is two.dat"
			);
		}

		virtual std::string content_21()
		{
			return STRING_WITH_NULLS(
				"\x20\x00" "\x0f\x00"
					"TWO.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
					"\x00\x00"
					"This is two.dat"
				"\x01\x00" "\x0f\x00"
					"ONE\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
					"\x00\x00"
					"This is one.dat"
			);
		}

		virtual std::string content_1l2()
		{
			return STRING_WITH_NULLS(
				"\x01\x00" "\x14\x00"
					"ONE\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
					"\x00\x00"
					"This is one.dat\0\0\0\0\0"
				"\x20\x00" "\x0f\x00"
					"TWO.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
					"\x00\x00"
					"This is two.dat"
			);
		}

		virtual std::string content_1s2()
		{
			return STRING_WITH_NULLS(
				"\x01\x00" "\x0a\x00"
					"ONE\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
					"\x00\x00"
					"This is on"
				"\x20\x00" "\x0f\x00"
					"TWO.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
					"\x00\x00"
					"This is two.dat"
			);
		}

		virtual std::string content_1w2()
		{
			return STRING_WITH_NULLS(
				"\x01\x00" "\x17\x00"
					"ONE\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
					"\x00\x00"
					"Now resized to 23 chars"
				"\x20\x00" "\x0f\x00"
					"TWO.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
					"\x00\x00"
					"This is two.dat"
			);
		}
};

IMPLEMENT_TESTS(dat_bash);
