/**
 * @file   test-arch-vol-cosmo.cpp
 * @brief  Test code for Cosmo's Cosmic Adventure .VOL archives.
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

class test_vol_cosmo: public test_archive
{
	public:
		test_vol_cosmo()
		{
			this->type = "vol-cosmo";
			this->lenMaxFilename = 12;
		}

		void addTests()
		{
			this->test_archive::addTests();

			// c00: Initial state
			this->isInstance(ArchiveType::DefinitelyYes, this->content_12());

			// c01: Control characters in filename
			this->isInstance(ArchiveType::DefinitelyNo, STRING_WITH_NULLS(
				"ONE.DAT\x05\0\0\0\0"      "\xa0\x0f\x00\x00" "\x0f\x00\x00\x00"
			) + std::string(3980, '\0') + STRING_WITH_NULLS(
				"This is one.dat"
			));

			// c02: First file offset is within fixed-length FAT
			this->isInstance(ArchiveType::DefinitelyNo, STRING_WITH_NULLS(
				"ONE.DAT\0\0\0\0\0"        "\x05\x00\x00\x00" "\x0f\x00\x00\x00"
			) + std::string(3980, '\0') + STRING_WITH_NULLS(
				"This is one.dat"
			));

			// c03: File length is past EOF
			this->isInstance(ArchiveType::DefinitelyNo, STRING_WITH_NULLS(
				"ONE.DAT\0\0\0\0\0"        "\xa0\x0f\x00\x00" "\x1f\x00\x00\x00"
			) + std::string(3980, '\0') + STRING_WITH_NULLS(
				"This is one.dat"
			));

			// c04: FAT is larger than entire archive
			this->isInstance(ArchiveType::DefinitelyNo, STRING_WITH_NULLS(
				"ONE.DAT\0\0\0\0\0"        "\xff\xff\x00\x00" "\x0f\x00\x00\x00"
			) + std::string(3980, '\0') + STRING_WITH_NULLS(
				"This is one.dat"
			));

			// c05: Empty file is valid
			this->isInstance(ArchiveType::DefinitelyYes, std::string(4000, '\0'));
		}

		virtual std::string content_12()
		{
			return STRING_WITH_NULLS(
				"ONE.DAT\0\0\0\0\0"        "\xa0\x0f\x00\x00" "\x0f\x00\x00\x00"
				"TWO.DAT\0\0\0\0\0"        "\xaf\x0f\x00\x00" "\x0f\x00\x00\x00"
				"\0\0\0\0\0\0\0\0\0\0\0\0" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"\0\0\0\0\0\0\0\0\0\0\0\0" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
			) + std::string(3920, '\0') + STRING_WITH_NULLS(
				"This is one.dat"
				"This is two.dat"
			);
		}

		virtual std::string content_1r2()
		{
			return STRING_WITH_NULLS(
				"THREE.DAT\0\0\0"          "\xa0\x0f\x00\x00" "\x0f\x00\x00\x00"
				"TWO.DAT\0\0\0\0\0"        "\xaf\x0f\x00\x00" "\x0f\x00\x00\x00"
				"\0\0\0\0\0\0\0\0\0\0\0\0" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"\0\0\0\0\0\0\0\0\0\0\0\0" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
			) + std::string(3920, '\0') + STRING_WITH_NULLS(
				"This is one.dat"
				"This is two.dat"
			);
		}

		virtual std::string content_123()
		{
			return STRING_WITH_NULLS(
				"ONE.DAT\0\0\0\0\0"        "\xa0\x0f\x00\x00" "\x0f\x00\x00\x00"
				"TWO.DAT\0\0\0\0\0"        "\xaf\x0f\x00\x00" "\x0f\x00\x00\x00"
				"THREE.DAT\0\0\0"          "\xbe\x0f\x00\x00" "\x11\x00\x00\x00"
				"\0\0\0\0\0\0\0\0\0\0\0\0" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
			) + std::string(3920, '\0') + STRING_WITH_NULLS(
				"This is one.dat"
				"This is two.dat"
				"This is three.dat"
			);
		}

		virtual std::string content_132()
		{
			return STRING_WITH_NULLS(
				"ONE.DAT\0\0\0\0\0"        "\xa0\x0f\x00\x00" "\x0f\x00\x00\x00"
				"THREE.DAT\0\0\0"          "\xaf\x0f\x00\x00" "\x11\x00\x00\x00"
				"TWO.DAT\0\0\0\0\0"        "\xc0\x0f\x00\x00" "\x0f\x00\x00\x00"
				"\0\0\0\0\0\0\0\0\0\0\0\0" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
			) + std::string(3920, '\0') + STRING_WITH_NULLS(
				"This is one.dat"
				"This is three.dat"
				"This is two.dat"
			);
		}

		virtual std::string content_1342()
		{
			return STRING_WITH_NULLS(
				"ONE.DAT\0\0\0\0\0"        "\xa0\x0f\x00\x00" "\x0f\x00\x00\x00"
				"THREE.DAT\0\0\0"          "\xaf\x0f\x00\x00" "\x11\x00\x00\x00"
				"FOUR.DAT\0\0\0\0"         "\xc0\x0f\x00\x00" "\x10\x00\x00\x00"
				"TWO.DAT\0\0\0\0\0"        "\xd0\x0f\x00\x00" "\x0f\x00\x00\x00"
			) + std::string(3920, '\0') + STRING_WITH_NULLS(
				"This is one.dat"
				"This is three.dat"
				"This is four.dat"
				"This is two.dat"
			);
		}

		virtual std::string content_2()
		{
			return STRING_WITH_NULLS(
				"TWO.DAT\0\0\0\0\0"        "\xa0\x0f\x00\x00" "\x0f\x00\x00\x00"
				"\0\0\0\0\0\0\0\0\0\0\0\0" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"\0\0\0\0\0\0\0\0\0\0\0\0" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"\0\0\0\0\0\0\0\0\0\0\0\0" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
			) + std::string(3920, '\0') + STRING_WITH_NULLS(
				"This is two.dat"
			);
		}

		virtual std::string content_0()
		{
			return STRING_WITH_NULLS(
				"\0\0\0\0\0\0\0\0\0\0\0\0" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"\0\0\0\0\0\0\0\0\0\0\0\0" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"\0\0\0\0\0\0\0\0\0\0\0\0" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"\0\0\0\0\0\0\0\0\0\0\0\0" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
			) + std::string(3920, '\0');
		}

		virtual std::string content_32()
		{
			return STRING_WITH_NULLS(
				"THREE.DAT\0\0\0"          "\xa0\x0f\x00\x00" "\x11\x00\x00\x00"
				"TWO.DAT\0\0\0\0\0"        "\xb1\x0f\x00\x00" "\x0f\x00\x00\x00"
				"\0\0\0\0\0\0\0\0\0\0\0\0" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"\0\0\0\0\0\0\0\0\0\0\0\0" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
			) + std::string(3920, '\0') + STRING_WITH_NULLS(
				"This is three.dat"
				"This is two.dat"
			);
		}

		virtual std::string content_21()
		{
			return STRING_WITH_NULLS(
				"TWO.DAT\0\0\0\0\0"        "\xa0\x0f\x00\x00" "\x0f\x00\x00\x00"
				"ONE.DAT\0\0\0\0\0"        "\xaf\x0f\x00\x00" "\x0f\x00\x00\x00"
				"\0\0\0\0\0\0\0\0\0\0\0\0" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"\0\0\0\0\0\0\0\0\0\0\0\0" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
			) + std::string(3920, '\0') + STRING_WITH_NULLS(
				"This is two.dat"
				"This is one.dat"
			);
		}

		virtual std::string content_1l2()
		{
			return STRING_WITH_NULLS(
				"ONE.DAT\0\0\0\0\0"        "\xa0\x0f\x00\x00" "\x14\x00\x00\x00"
				"TWO.DAT\0\0\0\0\0"        "\xb4\x0f\x00\x00" "\x0f\x00\x00\x00"
				"\0\0\0\0\0\0\0\0\0\0\0\0" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"\0\0\0\0\0\0\0\0\0\0\0\0" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
			) + std::string(3920, '\0') + STRING_WITH_NULLS(
				"This is one.dat\0\0\0\0\0"
				"This is two.dat"
			);
		}

		virtual std::string content_1s2()
		{
			return STRING_WITH_NULLS(
				"ONE.DAT\0\0\0\0\0"        "\xa0\x0f\x00\x00" "\x0a\x00\x00\x00"
				"TWO.DAT\0\0\0\0\0"        "\xaa\x0f\x00\x00" "\x0f\x00\x00\x00"
				"\0\0\0\0\0\0\0\0\0\0\0\0" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"\0\0\0\0\0\0\0\0\0\0\0\0" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
			) + std::string(3920, '\0') + STRING_WITH_NULLS(
				"This is on"
				"This is two.dat"
			);
		}

		virtual std::string content_1w2()
		{
			return STRING_WITH_NULLS(
				"ONE.DAT\0\0\0\0\0"        "\xa0\x0f\x00\x00" "\x17\x00\x00\x00"
				"TWO.DAT\0\0\0\0\0"        "\xb7\x0f\x00\x00" "\x0f\x00\x00\x00"
				"\0\0\0\0\0\0\0\0\0\0\0\0" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"\0\0\0\0\0\0\0\0\0\0\0\0" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
			) + std::string(3920, '\0') + STRING_WITH_NULLS(
				"Now resized to 23 chars"
				"This is two.dat"
			);
		}
};

IMPLEMENT_TESTS(vol_cosmo);
