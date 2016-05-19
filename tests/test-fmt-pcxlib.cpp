/**
 * @file   test-fmt-pcxlib.cpp
 * @brief  Test code for PCX library files.
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

#define START_PAD \
	"\x00\x00\x00\x00\x00\x00\x00\x00" \
	"\x00\x00\x00\x00\x00\x00\x00\x00" \
	"\x00\x00\x00\x00\x00\x00\x00\x00" \
	"\x00\x00\x00\x00\x00\x00\x00\x00" \
	"\x00\x00\x00\x00\x00\x00\x00\x00" \

// 32-byte header end
#define END_PAD \
	"\x00\x00\x00\x00\x00\x00\x00\x00" \
	"\x00\x00\x00\x00\x00\x00\x00\x00" \
	"\x00\x00\x00\x00\x00\x00\x00\x00" \
	"\x00\x00\x00\x00\x00\x00\x00\x00"

class test_pcxlib: public test_archive
{
	public:
		test_pcxlib()
		{
			this->type = "pcxlib";
			this->filename[1] = "TWO.DA";
			this->filename[2] = "THREE.D";
			this->lenMaxFilename = 12;

			Attribute copyright;
			copyright.type = Attribute::Type::Text;
			copyright.textValue = "Copyright (c) Genus Microprogramming, Inc. 1988-90";
			copyright.textMaxLength = 50;
			this->attributes.push_back(copyright);

			Attribute vollabel;
			vollabel.type = Attribute::Type::Text;
			vollabel.textValue = "";
			vollabel.textMaxLength = 40;
			this->attributes.push_back(vollabel);
		}

		void addTests()
		{
			this->test_archive::addTests();

			// c00: Initial state
			this->isInstance(ArchiveType::Certainty::DefinitelyYes, this->content_12());

			// c01: File too short for signature
			this->isInstance(ArchiveType::Certainty::DefinitelyNo, STRING_WITH_NULLS(
				"\x01\xCA"
				"Copyright (c) Genus Microprogramming, Inc. 1988-90"
				"\x64\x00"
				START_PAD
				"\x00\x00"
			));

			// c02: Bad signature
			this->isInstance(ArchiveType::Certainty::DefinitelyNo, STRING_WITH_NULLS(
				"\xff\xff"
				"Copyright (c) Genus Microprogramming, Inc. 1988-90"
				"\x64\x00"
				START_PAD
				"\x02\x00"
				END_PAD
				"\x00" "ONE     .DAT\0" "\xd6\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00"
				"\x00" "TWO     .DA \0" "\xe5\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00"
				"This is one.dat"
				"This is two.dat"
			));

			// c03: File too short for FAT
			this->isInstance(ArchiveType::Certainty::DefinitelyNo, STRING_WITH_NULLS(
				"\x01\xCA"
				"Copyright (c) Genus Microprogramming, Inc. 1988-90"
				"\x64\x00"
				START_PAD
				"\x02\x00"
				END_PAD
				"\x00" "ONE     .DAT\0" "\xd6\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00"
			));

			// c04: No/invalid sync byte
			this->isInstance(ArchiveType::Certainty::DefinitelyNo, STRING_WITH_NULLS(
				"\x01\xCA"
				"Copyright (c) Genus Microprogramming, Inc. 1988-90"
				"\x64\x00"
				START_PAD
				"\x02\x00"
				END_PAD
				"\x00" "ONE     .DAT\0" "\xd6\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00"
				"\x00" "TWO     .DA \0" "\xe5\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00"
				"This is one.dat"
				"This is two.dat"
			));

			// c05: Bad filename
			this->isInstance(ArchiveType::Certainty::DefinitelyNo, STRING_WITH_NULLS(
				"\x01\xCA"
				"Copyright (c) Genus Microprogramming, Inc. 1988-90"
				"\x64\x00"
				START_PAD
				"\x02\x00"
				END_PAD
				"\x00" "ONE     .DAT\0" "\xd6\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00"
				"\x00" "TWO      DA \0" "\xe5\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00"
				"This is one.dat"
				"This is two.dat"
			));

			// c06: File inside FAT
			this->isInstance(ArchiveType::Certainty::DefinitelyNo, STRING_WITH_NULLS(
				"\x01\xCA"
				"Copyright (c) Genus Microprogramming, Inc. 1988-90"
				"\x64\x00"
				START_PAD
				"\x02\x00"
				END_PAD
				"\x00" "ONE     .DAT\0" "\xd6\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00"
				"\x00" "TWO     .DA \0" "\x05\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00"
				"This is one.dat"
				"This is two.dat"
			));

			// c07: Truncated file
			this->isInstance(ArchiveType::Certainty::DefinitelyNo, STRING_WITH_NULLS(
				"\x01\xCA"
				"Copyright (c) Genus Microprogramming, Inc. 1988-90"
				"\x64\x00"
				START_PAD
				"\x02\x00"
				END_PAD
				"\x00" "ONE     .DAT\0" "\xd6\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00"
				"\x00" "TWO     .DA \0" "\xe5\x00\x00\x00" "\xff\x00\x00\x00" "\x00\x00" "\x00\x00"
				"This is one.dat"
				"This is two.dat"
			));

			// c08: Wrong version
			this->isInstance(ArchiveType::Certainty::DefinitelyNo, STRING_WITH_NULLS(
				"\xff\xff"
				"Copyright (c) Genus Microprogramming, Inc. 1988-90"
				"\x64\x00"
				START_PAD
				"\x02\x00"
				END_PAD
				"\x00" "ONE     .DAT\0" "\xd6\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00"
				"\x00" "TWO     .DA \0" "\xe5\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00"
				"This is one.dat"
				"This is two.dat"
			));

			// a01: Change comment
			this->changeAttribute(0, "Hello", STRING_WITH_NULLS(
				"\x01\xCA"
				"Hello\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\x64\x00"
				START_PAD
				"\x02\x00"
				END_PAD
				"\x00" "ONE     .DAT\0" "\xb4\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00"
				"\x00" "TWO     .DA \0" "\xc3\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00"
				"This is one.dat"
				"This is two.dat"
			));

			// a02: Change label
			this->changeAttribute(1, "Hello", STRING_WITH_NULLS(
				"\x01\xCA"
				"Copyright (c) Genus Microprogramming, Inc. 1988-90"
				"\x64\x00"
				"Hello"
			) + std::string(40-5, '\0') + STRING_WITH_NULLS(
				"\x02\x00"
				END_PAD
				"\x00" "ONE     .DAT\0" "\xb4\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00"
				"\x00" "TWO     .DA \0" "\xc3\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00"
				"This is one.dat"
				"This is two.dat"
			));
		}

		virtual std::string content_12()
		{
			return STRING_WITH_NULLS(
				"\x01\xCA"
				"Copyright (c) Genus Microprogramming, Inc. 1988-90"
				"\x64\x00"
				START_PAD
				"\x02\x00"
				END_PAD
				"\x00" "ONE     .DAT\0" "\xb4\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00"
				"\x00" "TWO     .DA \0" "\xc3\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00"
				"This is one.dat"
				"This is two.dat"
			);
		}

		virtual std::string content_1r2()
		{
			return STRING_WITH_NULLS(
				"\x01\xCA"
				"Copyright (c) Genus Microprogramming, Inc. 1988-90"
				"\x64\x00"
				START_PAD
				"\x02\x00"
				END_PAD
				"\x00" "THREE   .D  \0" "\xb4\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00"
				"\x00" "TWO     .DA \0" "\xc3\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00"
				"This is one.dat"
				"This is two.dat"
			);
		}

		virtual std::string content_123()
		{
			return STRING_WITH_NULLS(
				"\x01\xCA"
				"Copyright (c) Genus Microprogramming, Inc. 1988-90"
				"\x64\x00"
				START_PAD
				 "\x03\x00" END_PAD
				"\x00" "ONE     .DAT\0" "\xce\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00"
				"\x00" "TWO     .DA \0" "\xdd\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00"
				"\x00" "THREE   .D  \0" "\xec\x00\x00\x00" "\x11\x00\x00\x00" "\x00\x00" "\x00\x00"
				"This is one.dat"
				"This is two.dat"
				"This is three.dat"
			);
		}

		virtual std::string content_132()
		{
			return STRING_WITH_NULLS(
				"\x01\xCA"
				"Copyright (c) Genus Microprogramming, Inc. 1988-90"
				"\x64\x00"
				START_PAD
				 "\x03\x00" END_PAD
				"\x00" "ONE     .DAT\0" "\xce\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00"
				"\x00" "THREE   .D  \0" "\xdd\x00\x00\x00" "\x11\x00\x00\x00" "\x00\x00" "\x00\x00"
				"\x00" "TWO     .DA \0" "\xee\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00"
				"This is one.dat"
				"This is three.dat"
				"This is two.dat"
			);
		}

		virtual std::string content_1342()
		{
			return STRING_WITH_NULLS(
				"\x01\xCA"
				"Copyright (c) Genus Microprogramming, Inc. 1988-90"
				"\x64\x00"
				START_PAD
				 "\x04\x00" END_PAD
				"\x00" "ONE     .DAT\0" "\xe8\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00"
				"\x00" "THREE   .D  \0" "\xf7\x00\x00\x00" "\x11\x00\x00\x00" "\x00\x00" "\x00\x00"
				"\x00" "FOUR    .DAT\0" "\x08\x01\x00\x00" "\x10\x00\x00\x00" "\x00\x00" "\x00\x00"
				"\x00" "TWO     .DA \0" "\x18\x01\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00"
				"This is one.dat"
				"This is three.dat"
				"This is four.dat"
				"This is two.dat"
			);
		}

		virtual std::string content_2()
		{
			return STRING_WITH_NULLS(
				"\x01\xCA"
				"Copyright (c) Genus Microprogramming, Inc. 1988-90"
				"\x64\x00"
				START_PAD
				 "\x01\x00" END_PAD
				"\x00" "TWO     .DA \0" "\x9a\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00"
				"This is two.dat"
			);
		}

		virtual std::string content_0()
		{
			return STRING_WITH_NULLS(
				"\x01\xCA"
				"Copyright (c) Genus Microprogramming, Inc. 1988-90"
				"\x64\x00"
				START_PAD
				 "\x00\x00" END_PAD
			);
		}

		virtual std::string content_32()
		{
			return STRING_WITH_NULLS(
				"\x01\xCA"
				"Copyright (c) Genus Microprogramming, Inc. 1988-90"
				"\x64\x00"
				START_PAD
				"\x02\x00"
				END_PAD
				"\x00" "THREE   .D  \0" "\xb4\x00\x00\x00" "\x11\x00\x00\x00" "\x00\x00" "\x00\x00"
				"\x00" "TWO     .DA \0" "\xc5\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00"
				"This is three.dat"
				"This is two.dat"
			);
		}

		virtual std::string content_21()
		{
			return STRING_WITH_NULLS(
				"\x01\xCA"
				"Copyright (c) Genus Microprogramming, Inc. 1988-90"
				"\x64\x00"
				START_PAD
				"\x02\x00"
				END_PAD
				"\x00" "TWO     .DA \0" "\xb4\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00"
				"\x00" "ONE     .DAT\0" "\xc3\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00"
				"This is two.dat"
				"This is one.dat"
			);
		}

		virtual std::string content_1l2()
		{
			return STRING_WITH_NULLS(
				"\x01\xCA"
				"Copyright (c) Genus Microprogramming, Inc. 1988-90"
				"\x64\x00"
				START_PAD
				"\x02\x00"
				END_PAD
				"\x00" "ONE     .DAT\0" "\xb4\x00\x00\x00" "\x14\x00\x00\x00" "\x00\x00" "\x00\x00"
				"\x00" "TWO     .DA \0" "\xc8\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00"
				"This is one.dat\0\0\0\0\0"
				"This is two.dat"
			);
		}

		virtual std::string content_1s2()
		{
			return STRING_WITH_NULLS(
				"\x01\xCA"
				"Copyright (c) Genus Microprogramming, Inc. 1988-90"
				"\x64\x00"
				START_PAD
				"\x02\x00"
				END_PAD
				"\x00" "ONE     .DAT\0" "\xb4\x00\x00\x00" "\x0a\x00\x00\x00" "\x00\x00" "\x00\x00"
				"\x00" "TWO     .DA \0" "\xbe\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00"
				"This is on"
				"This is two.dat"
			);
		}

		virtual std::string content_1w2()
		{
			return STRING_WITH_NULLS(
				"\x01\xCA"
				"Copyright (c) Genus Microprogramming, Inc. 1988-90"
				"\x64\x00"
				START_PAD
				"\x02\x00"
				END_PAD
				"\x00" "ONE     .DAT\0" "\xb4\x00\x00\x00" "\x17\x00\x00\x00" "\x00\x00" "\x00\x00"
				"\x00" "TWO     .DA \0" "\xcb\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00"
				"Now resized to 23 chars"
				"This is two.dat"
			);
		}
};

IMPLEMENT_TESTS(pcxlib);
