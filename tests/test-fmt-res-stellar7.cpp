/**
 * @file   test-arch-res-stellar7.cpp
 * @brief  Test code for Stellar 7 .RES archives.
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

class test_res_stellar7: public test_archive
{
	public:
		test_res_stellar7()
		{
			this->type = "res-stellar7";
			this->filename[0] = "ONE:";
			this->filename[1] = "TWO:";
			this->filename[2] = "THR:";
			this->filename[3] = "FOU:";
			this->filename_shortext = "TS";
			this->lenMaxFilename = 4;
		}

		void addTests()
		{
			this->test_archive::addTests();

			// c00: Initial state
			this->isInstance(ArchiveType::Certainty::DefinitelyYes, this->content_12());

			// c01: Control characters in filename
			this->isInstance(ArchiveType::Certainty::DefinitelyNo, STRING_WITH_NULLS(
				"\x5NE:" "\x0f\x00\x00\x00"
				"This is one.dat"
				"TWO:" "\x0f\x00\x00\x00"
				"This is two.dat"
			));

			// c02: Offset past EOF
			this->isInstance(ArchiveType::Certainty::DefinitelyNo, STRING_WITH_NULLS(
				"ONE:" "\xef\x00\x00\x00"
				"This is one.dat"
				"TWO:" "\x0f\x00\x00\x00"
				"This is two.dat"
			));
		}

		virtual std::string content_12()
		{
			return STRING_WITH_NULLS(
				"ONE:" "\x0f\x00\x00\x00"
				"This is one.dat"
				"TWO:" "\x0f\x00\x00\x00"
				"This is two.dat"
			);
		}

		virtual std::string content_1r2()
		{
			return STRING_WITH_NULLS(
				"THR:" "\x0f\x00\x00\x00"
				"This is one.dat"
				"TWO:" "\x0f\x00\x00\x00"
				"This is two.dat"
			);
		}

		virtual std::string content_123()
		{
			return STRING_WITH_NULLS(
				"ONE:" "\x0f\x00\x00\x00"
				"This is one.dat"
				"TWO:" "\x0f\x00\x00\x00"
				"This is two.dat"
				"THR:" "\x11\x00\x00\x00"
				"This is three.dat"
			);
		}

		virtual std::string content_132()
		{
			return STRING_WITH_NULLS(
				"ONE:" "\x0f\x00\x00\x00"
				"This is one.dat"
				"THR:" "\x11\x00\x00\x00"
				"This is three.dat"
				"TWO:" "\x0f\x00\x00\x00"
				"This is two.dat"
			);
		}

		virtual std::string content_1342()
		{
			return STRING_WITH_NULLS(
				"ONE:" "\x0f\x00\x00\x00"
				"This is one.dat"
				"THR:" "\x11\x00\x00\x00"
				"This is three.dat"
				"FOU:" "\x10\x00\x00\x00"
				"This is four.dat"
				"TWO:" "\x0f\x00\x00\x00"
				"This is two.dat"
			);
		}

		virtual std::string content_2()
		{
			return STRING_WITH_NULLS(
				"TWO:" "\x0f\x00\x00\x00"
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
				"THR:" "\x11\x00\x00\x00"
				"This is three.dat"
				"TWO:" "\x0f\x00\x00\x00"
				"This is two.dat"
			);
		}

		virtual std::string content_21()
		{
			return STRING_WITH_NULLS(
				"TWO:" "\x0f\x00\x00\x00"
				"This is two.dat"
				"ONE:" "\x0f\x00\x00\x00"
				"This is one.dat"
			);
		}

		virtual std::string content_1l2()
		{
			return STRING_WITH_NULLS(
				"ONE:" "\x14\x00\x00\x00"
				"This is one.dat\0\0\0\0\0"
				"TWO:" "\x0f\x00\x00\x00"
				"This is two.dat"
			);
		}

		virtual std::string content_1s2()
		{
			return STRING_WITH_NULLS(
				"ONE:" "\x0a\x00\x00\x00"
				"This is on"
				"TWO:" "\x0f\x00\x00\x00"
				"This is two.dat"
			);
		}

		virtual std::string content_1w2()
		{
			return STRING_WITH_NULLS(
				"ONE:" "\x17\x00\x00\x00"
				"Now resized to 23 chars"
				"TWO:" "\x0f\x00\x00\x00"
				"This is two.dat"
			);
		}
};

IMPLEMENT_TESTS(res_stellar7);
