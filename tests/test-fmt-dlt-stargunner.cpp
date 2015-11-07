/**
 * @file   test-arch-dlt-stargunner.cpp
 * @brief  Test code for Stargunner .DLT archives.
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

#define FILENAME1_ENC "O\x1e\x15\x66\x76\x08\x13\x5b\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f"
#define FILENAME2_ENC "T\x02\x16\x7c\x76\x08\x13\x5b\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f"
#define FILENAME3_ENC "T\x1d\x18\x10\x0c\x64p\x0a\x1d]\x0a\x0b\x0c\x0d\x0e\x0f\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f"
#define FILENAME4_ENC "F\x08\x04\x0axw\x0b\x1c\x5c\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f"

class test_dlt_stargunner: public test_archive
{
	public:
		test_dlt_stargunner()
		{
			this->type = "dlt-stargunner";
			this->lenMaxFilename = 32;
		}

		void addTests()
		{
			this->test_archive::addTests();

			// c00: Initial state
			this->isInstance(ArchiveType::DefinitelyYes, this->content_12());

			// c01: Bad signature
			this->isInstance(ArchiveType::DefinitelyNo, STRING_WITH_NULLS(
				"DAVY" "\x00\x01" "\x02\x00"
				FILENAME1_ENC "\x00\x00\x00\x00" "\x0F\x00\x00\x00" "This is one.dat"
				FILENAME2_ENC "\x00\x00\x00\x00" "\x0F\x00\x00\x00" "This is two.dat"
			));

			// c02: Too short
			this->isInstance(ArchiveType::DefinitelyNo, STRING_WITH_NULLS(
				"DAVE" "\x00\x01" "\x00"
			));
		}

		virtual std::string content_12()
		{
			return STRING_WITH_NULLS(
				"DAVE" "\x00\x01" "\x02\x00"
				FILENAME1_ENC "\x00\x00\x00\x00" "\x0f\x00\x00\x00" "This is one.dat"
				FILENAME2_ENC "\x00\x00\x00\x00" "\x0f\x00\x00\x00" "This is two.dat"
			);
		}

		virtual std::string content_1r2()
		{
			return STRING_WITH_NULLS(
				"DAVE" "\x00\x01" "\x02\x00"
				FILENAME3_ENC "\x00\x00\x00\x00" "\x0f\x00\x00\x00" "This is one.dat"
				FILENAME2_ENC "\x00\x00\x00\x00" "\x0f\x00\x00\x00" "This is two.dat"
			);
		}

		virtual std::string content_123()
		{
			return STRING_WITH_NULLS(
				"DAVE" "\x00\x01" "\x03\x00"
				FILENAME1_ENC "\x00\x00\x00\x00" "\x0f\x00\x00\x00" "This is one.dat"
				FILENAME2_ENC "\x00\x00\x00\x00" "\x0f\x00\x00\x00" "This is two.dat"
				FILENAME3_ENC "\x00\x00\x00\x00" "\x11\x00\x00\x00" "This is three.dat"
			);
		}

		virtual std::string content_132()
		{
			return STRING_WITH_NULLS(
				"DAVE" "\x00\x01" "\x03\x00"
				FILENAME1_ENC "\x00\x00\x00\x00" "\x0f\x00\x00\x00" "This is one.dat"
				FILENAME3_ENC "\x00\x00\x00\x00" "\x11\x00\x00\x00" "This is three.dat"
				FILENAME2_ENC "\x00\x00\x00\x00" "\x0f\x00\x00\x00" "This is two.dat"
			);
		}

		virtual std::string content_1342()
		{
			return STRING_WITH_NULLS(
				"DAVE" "\x00\x01" "\x04\x00"
				FILENAME1_ENC "\x00\x00\x00\x00" "\x0f\x00\x00\x00" "This is one.dat"
				FILENAME3_ENC "\x00\x00\x00\x00" "\x11\x00\x00\x00" "This is three.dat"
				FILENAME4_ENC "\x00\x00\x00\x00" "\x10\x00\x00\x00" "This is four.dat"
				FILENAME2_ENC "\x00\x00\x00\x00" "\x0f\x00\x00\x00" "This is two.dat"
			);
		}

		virtual std::string content_2()
		{
			return STRING_WITH_NULLS(
				"DAVE" "\x00\x01" "\x01\x00"
				FILENAME2_ENC "\x00\x00\x00\x00" "\x0f\x00\x00\x00" "This is two.dat"
			);
		}

		virtual std::string content_0()
		{
			return STRING_WITH_NULLS(
				"DAVE" "\x00\x01" "\x00\x00"
			);
		}

		virtual std::string content_32()
		{
			return STRING_WITH_NULLS(
				"DAVE" "\x00\x01" "\x02\x00"
				FILENAME3_ENC "\x00\x00\x00\x00" "\x11\x00\x00\x00" "This is three.dat"
				FILENAME2_ENC "\x00\x00\x00\x00" "\x0f\x00\x00\x00" "This is two.dat"
			);
		}

		virtual std::string content_21()
		{
			return STRING_WITH_NULLS(
				"DAVE" "\x00\x01" "\x02\x00"
				FILENAME2_ENC "\x00\x00\x00\x00" "\x0f\x00\x00\x00" "This is two.dat"
				FILENAME1_ENC "\x00\x00\x00\x00" "\x0f\x00\x00\x00" "This is one.dat"
			);
		}

		virtual std::string content_1l2()
		{
			return STRING_WITH_NULLS(
				"DAVE" "\x00\x01" "\x02\x00"
				FILENAME1_ENC "\x00\x00\x00\x00" "\x14\x00\x00\x00" "This is one.dat" "\0\0\0\0\0"
				FILENAME2_ENC "\x00\x00\x00\x00" "\x0f\x00\x00\x00" "This is two.dat"
			);
		}

		virtual std::string content_1s2()
		{
			return STRING_WITH_NULLS(
				"DAVE" "\x00\x01" "\x02\x00"
				FILENAME1_ENC "\x00\x00\x00\x00" "\x0a\x00\x00\x00" "This is on"
				FILENAME2_ENC "\x00\x00\x00\x00" "\x0f\x00\x00\x00" "This is two.dat"
			);
		}

		virtual std::string content_1w2()
		{
			return STRING_WITH_NULLS(
				"DAVE" "\x00\x01" "\x02\x00"
				FILENAME1_ENC "\x00\x00\x00\x00" "\x17\x00\x00\x00" "Now resized to 23 chars"
				FILENAME2_ENC "\x00\x00\x00\x00" "\x0f\x00\x00\x00" "This is two.dat"
			);
		}
};

IMPLEMENT_TESTS(dlt_stargunner);
