/**
 * @file   test-arch-bpa-drally.cpp
 * @brief  Test code for Death Rally .BPA archives.
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

#define ONE_DAT   "\xC4\xC0\xB4\x9A\xAD\xA7\xB7"
#define TWO_DAT   "\xC9\xC9\xBE\x9A\xAD\xA7\xB7"
#define THREE_DAT "\xC9\xBA\xC1\xB1\xAE\x94\xA7\xA1\xB1"
#define FOUR_DAT  "\xBB\xC1\xC4\xBE\x97\xAA\xA4\xB4"
/*
#define ONE_DAT   "ONE.DAT"
#define TWO_DAT   "TWO.DAT"
#define THREE_DAT "THREE.DAT"
#define FOUR_DAT  "FOUR.DAT"
*/

class test_bpa_drally: public test_archive
{
	public:
		test_bpa_drally()
		{
			this->type = "bpa-drally";
			this->lenMaxFilename = 12;
			// If we "fix" vol-cosmo so it doesn't detect BPA archives, then there's a
			// chance it won't identify slightly odd VOL archives either.  Since it
			// only picks up BPA archives as "possible", the BPA handler will win out
			// with "definite" for real BPA archives, so we'll leave it as is.
			this->skipInstDetect.push_back("vol-cosmo");
		}

		void addTests()
		{
			this->test_archive::addTests();

			// c00: Initial state
			this->isInstance(ArchiveType::DefinitelyYes, this->initialstate());

			// c01: File too short
			this->isInstance(ArchiveType::DefinitelyNo, STRING_WITH_NULLS(
				"\x00\x00\x00\x00"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x00\x00\x00\x00"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x00\x00\x00\x00"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x00\x00\x00\x00"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x00\x00\x00"
			) + std::string(4267, '\0'));

			// c02: File just long enough
			this->isInstance(ArchiveType::DefinitelyYes, STRING_WITH_NULLS(
				"\x00\x00\x00\x00"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x00\x00\x00\x00"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x00\x00\x00\x00"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x00\x00\x00\x00"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x00\x00\x00\x00"
			) + std::string(4267, '\0'));

			// c03: More than 255 files
			this->isInstance(ArchiveType::DefinitelyNo, STRING_WITH_NULLS(
				"\x00\x01\x00\x00"
				ONE_DAT "\0\0\0\0\0\0"       "\x0f\x00\x00\x00"
				TWO_DAT "\0\0\0\0\0\0"       "\x0f\x00\x00\x00"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x00\x00\x00\x00"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x00\x00\x00\x00"
			) + std::string(4267, '\0') + STRING_WITH_NULLS(
				"This is one.dat"
				"This is two.dat"
			));

			// c04: Control characters in filename
			this->isInstance(ArchiveType::DefinitelyNo, STRING_WITH_NULLS(
				"\x02\x00\x00\x00"
				ONE_DAT "\x60\0\0\0\0\0"     "\x0f\x00\x00\x00"
				TWO_DAT "\0\0\0\0\0\0"       "\x0f\x00\x00\x00"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x00\x00\x00\x00"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x00\x00\x00\x00"
			) + std::string(4267, '\0') + STRING_WITH_NULLS(
				"This is one.dat"
				"This is two.dat"
			));

			// c05: File goes past EOF
			this->isInstance(ArchiveType::DefinitelyNo, STRING_WITH_NULLS(
				"\x02\x00\x00\x00"
				ONE_DAT "\0\0\0\0\0\0"       "\x0f\x00\x00\x00"
				TWO_DAT "\0\0\0\0\0\0"       "\x0f\x00\x01\x00"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x00\x00\x00\x00"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x00\x00\x00\x00"
			) + std::string(4267, '\0') + STRING_WITH_NULLS(
				"This is one.dat"
				"This is two.dat"
			));
		}

		virtual std::string initialstate()
		{
			return STRING_WITH_NULLS(
				"\x02\x00\x00\x00"
				ONE_DAT "\0\0\0\0\0\0"       "\x0f\x00\x00\x00"
				TWO_DAT "\0\0\0\0\0\0"       "\x0f\x00\x00\x00"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x00\x00\x00\x00"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x00\x00\x00\x00"
			) + std::string(4267, '\0') + STRING_WITH_NULLS(
				"This is one.dat"
				"This is two.dat"
			);
		}

		virtual std::string rename()
		{
			return STRING_WITH_NULLS(
				"\x02\x00\x00\x00"
				THREE_DAT "\0\0\0\0"         "\x0f\x00\x00\x00"
				TWO_DAT "\0\0\0\0\0\0"       "\x0f\x00\x00\x00"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x00\x00\x00\x00"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x00\x00\x00\x00"
			) + std::string(4267, '\0') + STRING_WITH_NULLS(
				"This is one.dat"
				"This is two.dat"
			);
		}

		virtual std::string insert_end()
		{
			return STRING_WITH_NULLS(
				"\x03\x00\x00\x00"
				ONE_DAT "\0\0\0\0\0\0"       "\x0f\x00\x00\x00"
				TWO_DAT "\0\0\0\0\0\0"       "\x0f\x00\x00\x00"
				THREE_DAT "\0\0\0\0"         "\x11\x00\x00\x00"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x00\x00\x00\x00"
			) + std::string(4267, '\0') + STRING_WITH_NULLS(
				"This is one.dat"
				"This is two.dat"
				"This is three.dat"
			);
		}

		virtual std::string insert_mid()
		{
			return STRING_WITH_NULLS(
				"\x03\x00\x00\x00"
				ONE_DAT "\0\0\0\0\0\0"       "\x0f\x00\x00\x00"
				THREE_DAT "\0\0\0\0"         "\x11\x00\x00\x00"
				TWO_DAT "\0\0\0\0\0\0"       "\x0f\x00\x00\x00"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x00\x00\x00\x00"
			) + std::string(4267, '\0') + STRING_WITH_NULLS(
				"This is one.dat"
				"This is three.dat"
				"This is two.dat"
			);
		}

		virtual std::string insert2()
		{
			return STRING_WITH_NULLS(
				"\x04\x00\x00\x00"
				ONE_DAT "\0\0\0\0\0\0"       "\x0f\x00\x00\x00"
				THREE_DAT "\0\0\0\0"         "\x11\x00\x00\x00"
				FOUR_DAT "\0\0\0\0\0"        "\x10\x00\x00\x00"
				TWO_DAT "\0\0\0\0\0\0"       "\x0f\x00\x00\x00"
			) + std::string(4267, '\0') + STRING_WITH_NULLS(
				"This is one.dat"
				"This is three.dat"
				"This is four.dat"
				"This is two.dat"
			);
		}

		virtual std::string remove()
		{
			return STRING_WITH_NULLS(
				"\x01\x00\x00\x00"
				TWO_DAT "\0\0\0\0\0\0"       "\x0f\x00\x00\x00"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x00\x00\x00\x00"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x00\x00\x00\x00"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x00\x00\x00\x00"
			) + std::string(4267, '\0') + STRING_WITH_NULLS(
				"This is two.dat"
			);
		}

		virtual std::string remove2()
		{
			return STRING_WITH_NULLS(
				"\x00\x00\x00\x00"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x00\x00\x00\x00"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x00\x00\x00\x00"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x00\x00\x00\x00"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x00\x00\x00\x00"
			) + std::string(4267, '\0');
		}

		virtual std::string insert_remove()
		{
			return STRING_WITH_NULLS(
				"\x02\x00\x00\x00"
				THREE_DAT "\0\0\0\0"         "\x11\x00\x00\x00"
				TWO_DAT "\0\0\0\0\0\0"       "\x0f\x00\x00\x00"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x00\x00\x00\x00"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x00\x00\x00\x00"
			) + std::string(4267, '\0') + STRING_WITH_NULLS(
				"This is three.dat"
				"This is two.dat"
			);
		}

		virtual std::string move()
		{
			return STRING_WITH_NULLS(
				"\x02\x00\x00\x00"
				TWO_DAT "\0\0\0\0\0\0"       "\x0f\x00\x00\x00"
				ONE_DAT "\0\0\0\0\0\0"       "\x0f\x00\x00\x00"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x00\x00\x00\x00"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x00\x00\x00\x00"
			) + std::string(4267, '\0') + STRING_WITH_NULLS(
				"This is two.dat"
				"This is one.dat"
			);
		}

		virtual std::string resize_larger()
		{
			return STRING_WITH_NULLS(
				"\x02\x00\x00\x00"
				ONE_DAT "\0\0\0\0\0\0"       "\x14\x00\x00\x00"
				TWO_DAT "\0\0\0\0\0\0"       "\x0f\x00\x00\x00"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x00\x00\x00\x00"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x00\x00\x00\x00"
			) + std::string(4267, '\0') + STRING_WITH_NULLS(
				"This is one.dat\0\0\0\0\0"
				"This is two.dat"
			);
		}

		virtual std::string resize_smaller()
		{
			return STRING_WITH_NULLS(
				"\x02\x00\x00\x00"
				ONE_DAT "\0\0\0\0\0\0"       "\x0a\x00\x00\x00"
				TWO_DAT "\0\0\0\0\0\0"       "\x0f\x00\x00\x00"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x00\x00\x00\x00"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x00\x00\x00\x00"
			) + std::string(4267, '\0') + STRING_WITH_NULLS(
				"This is on"
				"This is two.dat"
			);
		}

		virtual std::string resize_write()
		{
			return STRING_WITH_NULLS(
				"\x02\x00\x00\x00"
				ONE_DAT "\0\0\0\0\0\0"       "\x17\x00\x00\x00"
				TWO_DAT "\0\0\0\0\0\0"       "\x0f\x00\x00\x00"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x00\x00\x00\x00"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x00\x00\x00\x00"
			) + std::string(4267, '\0') + STRING_WITH_NULLS(
				"Now resized to 23 chars"
				"This is two.dat"
			);
		}
};

IMPLEMENT_TESTS(bpa_drally);
