/**
 * @file   test-fmt-dat-mystic.cpp
 * @brief  Test code for Mystic Towers .DAT archives.
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

class test_dat_mystic: public test_archive
{
	public:
		test_dat_mystic()
		{
			this->type = "dat-mystic";
			this->lenMaxFilename = 12;
		}

		void addTests()
		{
			this->test_archive::addTests();

			// c00: Initial state
			this->isInstance(ArchiveType::Certainty::DefinitelyYes, this->content_12());

			// c01: File too short
			this->isInstance(ArchiveType::Certainty::DefinitelyNo, STRING_WITH_NULLS(
				"\x00"
			));

			// c02: Too many files
			this->isInstance(ArchiveType::Certainty::DefinitelyNo, STRING_WITH_NULLS(
				"This is one.dat"
				"This is two.dat"
				"\x07" "ONE.DAT\0\0\0\0\0" "\x00\x00\x00\x00" "\x0f\x00\x00\x00"
				"\x07" "TWO.DAT\0\0\0\0\0" "\x0f\x00\x00\x00" "\x0f\x00\x00\x00"
				"\x02\xf0"
			));

			// c03: Too small to contain FAT
			this->isInstance(ArchiveType::Certainty::DefinitelyNo, STRING_WITH_NULLS(
				"\x07" "TWO.DAT\0\0\0\0\0" "\x0f\x00\x00\x00" "\x0f\x00\x00\x00"
				"\x02\x00"
			));

			// c04: Filename length longer than field size
			this->isInstance(ArchiveType::Certainty::DefinitelyNo, STRING_WITH_NULLS(
				"This is one.dat"
				"This is two.dat"
				"\x17" "ONE.DAT\0\0\0\0\0" "\x00\x00\x00\x00" "\x0f\x00\x00\x00"
				"\x07" "TWO.DAT\0\0\0\0\0" "\x0f\x00\x00\x00" "\x0f\x00\x00\x00"
				"\x02\x00"
			));

			// c05a: File starts or ends past archive EOF
			this->isInstance(ArchiveType::Certainty::DefinitelyNo, STRING_WITH_NULLS(
				"This is one.dat"
				"This is two.dat"
				"\x07" "ONE.DAT\0\0\0\0\0" "\x00\xf0\x00\x00" "\x0f\x00\x00\x00"
				"\x07" "TWO.DAT\0\0\0\0\0" "\x0f\x00\x00\x00" "\x0f\x00\x00\x00"
				"\x02\x00"
			));

			// c05b: File starts or ends past archive EOF
			this->isInstance(ArchiveType::Certainty::DefinitelyNo, STRING_WITH_NULLS(
				"This is one.dat"
				"This is two.dat"
				"\x07" "ONE.DAT\0\0\0\0\0" "\x00\x00\x00\x00" "\x0f\xf0\x00\x00"
				"\x07" "TWO.DAT\0\0\0\0\0" "\x0f\x00\x00\x00" "\x0f\x00\x00\x00"
				"\x02\x00"
			));

			// c06: File contains extra data beyond what is recorded in the FAT
			this->isInstance(ArchiveType::Certainty::DefinitelyNo, STRING_WITH_NULLS(
				"This is one.dat"
				"This is two.dat"
				"A"
				"\x07" "ONE.DAT\0\0\0\0\0" "\x00\x00\x00\x00" "\x0f\x00\x00\x00"
				"\x07" "TWO.DAT\0\0\0\0\0" "\x0f\x00\x00\x00" "\x0f\x00\x00\x00"
				"\x02\x00"
			));
		}

		virtual std::string content_12()
		{
			return STRING_WITH_NULLS(
				"This is one.dat"
				"This is two.dat"
				"\x07" "ONE.DAT\0\0\0\0\0" "\x00\x00\x00\x00" "\x0f\x00\x00\x00"
				"\x07" "TWO.DAT\0\0\0\0\0" "\x0f\x00\x00\x00" "\x0f\x00\x00\x00"
				"\x02\x00"
			);
		}

		virtual std::string content_1r2()
		{
			return STRING_WITH_NULLS(
				"This is one.dat"
				"This is two.dat"
				"\x09" "THREE.DAT\0\0\0"   "\x00\x00\x00\x00" "\x0f\x00\x00\x00"
				"\x07" "TWO.DAT\0\0\0\0\0" "\x0f\x00\x00\x00" "\x0f\x00\x00\x00"
				"\x02\x00"
			);
		}

		virtual std::string content_123()
		{
			return STRING_WITH_NULLS(
				"This is one.dat"
				"This is two.dat"
				"This is three.dat"
				"\x07" "ONE.DAT\0\0\0\0\0" "\x00\x00\x00\x00" "\x0f\x00\x00\x00"
				"\x07" "TWO.DAT\0\0\0\0\0" "\x0f\x00\x00\x00" "\x0f\x00\x00\x00"
				"\x09" "THREE.DAT\0\0\0"   "\x1e\x00\x00\x00" "\x11\x00\x00\x00"
				"\x03\x00"
			);
		}

		virtual std::string content_132()
		{
			return STRING_WITH_NULLS(
				"This is one.dat"
				"This is three.dat"
				"This is two.dat"
				"\x07" "ONE.DAT\0\0\0\0\0" "\x00\x00\x00\x00" "\x0f\x00\x00\x00"
				"\x09" "THREE.DAT\0\0\0"   "\x0f\x00\x00\x00" "\x11\x00\x00\x00"
				"\x07" "TWO.DAT\0\0\0\0\0" "\x20\x00\x00\x00" "\x0f\x00\x00\x00"
				"\x03\x00"
			);
		}

		virtual std::string content_1342()
		{
			return STRING_WITH_NULLS(
				"This is one.dat"
				"This is three.dat"
				"This is four.dat"
				"This is two.dat"
				"\x07" "ONE.DAT\0\0\0\0\0" "\x00\x00\x00\x00" "\x0f\x00\x00\x00"
				"\x09" "THREE.DAT\0\0\0"   "\x0f\x00\x00\x00" "\x11\x00\x00\x00"
				"\x08" "FOUR.DAT\0\0\0\0"  "\x20\x00\x00\x00" "\x10\x00\x00\x00"
				"\x07" "TWO.DAT\0\0\0\0\0" "\x30\x00\x00\x00" "\x0f\x00\x00\x00"
				"\x04\x00"
			);
		}

		virtual std::string content_2()
		{
			return STRING_WITH_NULLS(
				"This is two.dat"
				"\x07" "TWO.DAT\0\0\0\0\0" "\x00\x00\x00\x00" "\x0f\x00\x00\x00"
				"\x01\x00"
			);
		}

		virtual std::string content_0()
		{
			return STRING_WITH_NULLS(
				"\x00\x00"
			);
		}

		virtual std::string content_32()
		{
			return STRING_WITH_NULLS(
				"This is three.dat"
				"This is two.dat"
				"\x09" "THREE.DAT\0\0\0"   "\x00\x00\x00\x00" "\x11\x00\x00\x00"
				"\x07" "TWO.DAT\0\0\0\0\0" "\x11\x00\x00\x00" "\x0f\x00\x00\x00"
				"\x02\x00"
			);
		}

		virtual std::string content_21()
		{
			return STRING_WITH_NULLS(
				"This is two.dat"
				"This is one.dat"
				"\x07" "TWO.DAT\0\0\0\0\0" "\x00\x00\x00\x00" "\x0f\x00\x00\x00"
				"\x07" "ONE.DAT\0\0\0\0\0" "\x0f\x00\x00\x00" "\x0f\x00\x00\x00"
				"\x02\x00"
			);
		}

		virtual std::string content_1l2()
		{
			return STRING_WITH_NULLS(
				"This is one.dat\0\0\0\0\0"
				"This is two.dat"
				"\x07" "ONE.DAT\0\0\0\0\0" "\x00\x00\x00\x00" "\x14\x00\x00\x00"
				"\x07" "TWO.DAT\0\0\0\0\0" "\x14\x00\x00\x00" "\x0f\x00\x00\x00"
				"\x02\x00"
			);
		}

		virtual std::string content_1s2()
		{
			return STRING_WITH_NULLS(
				"This is on"
				"This is two.dat"
				"\x07" "ONE.DAT\0\0\0\0\0" "\x00\x00\x00\x00" "\x0a\x00\x00\x00"
				"\x07" "TWO.DAT\0\0\0\0\0" "\x0a\x00\x00\x00" "\x0f\x00\x00\x00"
				"\x02\x00"
			);
		}

		virtual std::string content_1w2()
		{
			return STRING_WITH_NULLS(
				"Now resized to 23 chars"
				"This is two.dat"
				"\x07" "ONE.DAT\0\0\0\0\0" "\x00\x00\x00\x00" "\x17\x00\x00\x00"
				"\x07" "TWO.DAT\0\0\0\0\0" "\x17\x00\x00\x00" "\x0f\x00\x00\x00"
				"\x02\x00"
			);
		}
};

IMPLEMENT_TESTS(dat_mystic);
