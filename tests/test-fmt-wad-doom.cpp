/**
 * @file   test-arch-wad-doom.cpp
 * @brief  Test code for Doom .WAD archives.
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

class test_wad_doom: public test_archive
{
	public:
		test_wad_doom()
		{
			this->type = "wad-doom";
			this->filename[2] = "THREE";
			this->lenMaxFilename = 8;

			this->hasMetadata[camoto::Metadata::MetadataType::Version] = true;
			this->metadataVer = "I"; // matches initialstate
		}

		void addTests()
		{
			this->test_archive::addTests();

			// c00: Initial state
			this->isInstance(ArchiveType::DefinitelyYes, this->initialstate());

			// c01: Alt sig
			this->isInstance(ArchiveType::DefinitelyYes, STRING_WITH_NULLS(
				"PWAD" "\x02\x00\x00\x00" "\x0c\x00\x00\x00"
				"\x2c\x00\x00\x00" "\x0f\x00\x00\x00" "ONE.DAT\0"
				"\x3b\x00\x00\x00" "\x0f\x00\x00\x00" "TWO.DAT\0"
				"This is one.dat"
				"This is two.dat"
			));

			// c02: Bad signature
			this->isInstance(ArchiveType::DefinitelyNo, STRING_WITH_NULLS(
				"XWAD" "\x02\x00\x00\x00" "\x0c\x00\x00\x00"
				"\x2c\x00\x00\x00" "\x0f\x00\x00\x00" "ONE.DAT\0"
				"\x3b\x00\x00\x00" "\x0f\x00\x00\x00" "TWO.DAT\0"
				"This is one.dat"
				"This is two.dat"
			));

			// c03: File too short
			this->isInstance(ArchiveType::DefinitelyNo, STRING_WITH_NULLS(
				"IWAD" "\x00\x00\x00\x00" "\x0b\x00\x00"
			));

			// i01: Too many files
			this->invalidContent(STRING_WITH_NULLS(
				"IWAD" "\x01\x00\xf0\x00" "\x0c\x00\x00\x00"
				"\x2c\x00\x00\x00" "\x0f\x00\x00\x00" "ONE.DAT\0"
				"This is one.dat"
			));

			// test_wad_doom_changemetadata_c01
			this->changeMetadata(camoto::Metadata::MetadataType::Version, "P", STRING_WITH_NULLS(
				"PWAD" "\x02\x00\x00\x00" "\x0c\x00\x00\x00"
				"\x2c\x00\x00\x00" "\x0f\x00\x00\x00" "ONE.DAT\0"
				"\x3b\x00\x00\x00" "\x0f\x00\x00\x00" "TWO.DAT\0"
				"This is one.dat"
				"This is two.dat"
			));
		}

		virtual std::string initialstate()
		{
			return STRING_WITH_NULLS(
				"IWAD" "\x02\x00\x00\x00" "\x0c\x00\x00\x00"
				"\x2c\x00\x00\x00" "\x0f\x00\x00\x00" "ONE.DAT\0"
				"\x3b\x00\x00\x00" "\x0f\x00\x00\x00" "TWO.DAT\0"
				"This is one.dat"
				"This is two.dat"
			);
		}

		virtual std::string rename()
		{
			return STRING_WITH_NULLS(
				"IWAD" "\x02\x00\x00\x00" "\x0c\x00\x00\x00"
				"\x2c\x00\x00\x00" "\x0f\x00\x00\x00" "THREE\0\0\0"
				"\x3b\x00\x00\x00" "\x0f\x00\x00\x00" "TWO.DAT\0"
				"This is one.dat"
				"This is two.dat"
			);
		}

		virtual std::string insert_end()
		{
			return STRING_WITH_NULLS(
				"IWAD" "\x03\x00\x00\x00" "\x0c\x00\x00\x00"
				"\x3c\x00\x00\x00" "\x0f\x00\x00\x00" "ONE.DAT\0"
				"\x4b\x00\x00\x00" "\x0f\x00\x00\x00" "TWO.DAT\0"
				"\x5a\x00\x00\x00" "\x11\x00\x00\x00" "THREE\0\0\0"
				"This is one.dat"
				"This is two.dat"
				"This is three.dat"
			);
		}

		virtual std::string insert_mid()
		{
			return STRING_WITH_NULLS(
				"IWAD" "\x03\x00\x00\x00" "\x0c\x00\x00\x00"
				"\x3c\x00\x00\x00" "\x0f\x00\x00\x00" "ONE.DAT\0"
				"\x4b\x00\x00\x00" "\x11\x00\x00\x00" "THREE\0\0\0"
				"\x5c\x00\x00\x00" "\x0f\x00\x00\x00" "TWO.DAT\0"
				"This is one.dat"
				"This is three.dat"
				"This is two.dat"
			);
		}

		virtual std::string insert2()
		{
			return STRING_WITH_NULLS(
				"IWAD" "\x04\x00\x00\x00" "\x0c\x00\x00\x00"
				"\x4c\x00\x00\x00" "\x0f\x00\x00\x00" "ONE.DAT\0"
				"\x5b\x00\x00\x00" "\x11\x00\x00\x00" "THREE\0\0\0"
				"\x6c\x00\x00\x00" "\x10\x00\x00\x00" "FOUR.DAT"
				"\x7c\x00\x00\x00" "\x0f\x00\x00\x00" "TWO.DAT\0"
				"This is one.dat"
				"This is three.dat"
				"This is four.dat"
				"This is two.dat"
			);
		}

		virtual std::string remove()
		{
			return STRING_WITH_NULLS(
				"IWAD" "\x01\x00\x00\x00" "\x0c\x00\x00\x00"
				"\x1c\x00\x00\x00" "\x0f\x00\x00\x00" "TWO.DAT\0"
				"This is two.dat"
			);
		}

		virtual std::string remove2()
		{
			return STRING_WITH_NULLS(
				"IWAD" "\x00\x00\x00\x00" "\x0c\x00\x00\x00"
			);
		}

		virtual std::string insert_remove()
		{
			return STRING_WITH_NULLS(
				"IWAD" "\x02\x00\x00\x00" "\x0c\x00\x00\x00"
				"\x2c\x00\x00\x00" "\x11\x00\x00\x00" "THREE\0\0\0"
				"\x3d\x00\x00\x00" "\x0f\x00\x00\x00" "TWO.DAT\0"
				"This is three.dat"
				"This is two.dat"
			);
		}

		virtual std::string move()
		{
			return STRING_WITH_NULLS(
				"IWAD" "\x02\x00\x00\x00" "\x0c\x00\x00\x00"
				"\x2c\x00\x00\x00" "\x0f\x00\x00\x00" "TWO.DAT\0"
				"\x3b\x00\x00\x00" "\x0f\x00\x00\x00" "ONE.DAT\0"
				"This is two.dat"
				"This is one.dat"
			);
		}

		virtual std::string resize_larger()
		{
			return STRING_WITH_NULLS(
				"IWAD" "\x02\x00\x00\x00" "\x0c\x00\x00\x00"
				"\x2c\x00\x00\x00" "\x14\x00\x00\x00" "ONE.DAT\0"
				"\x40\x00\x00\x00" "\x0f\x00\x00\x00" "TWO.DAT\0"
				"This is one.dat\0\0\0\0\0"
				"This is two.dat"
			);
		}

		virtual std::string resize_smaller()
		{
			return STRING_WITH_NULLS(
				"IWAD" "\x02\x00\x00\x00" "\x0c\x00\x00\x00"
				"\x2c\x00\x00\x00" "\x0a\x00\x00\x00" "ONE.DAT\0"
				"\x36\x00\x00\x00" "\x0f\x00\x00\x00" "TWO.DAT\0"
				"This is on"
				"This is two.dat"
			);
		}

		virtual std::string resize_write()
		{
			return STRING_WITH_NULLS(
				"IWAD" "\x02\x00\x00\x00" "\x0c\x00\x00\x00"
				"\x2c\x00\x00\x00" "\x17\x00\x00\x00" "ONE.DAT\0"
				"\x43\x00\x00\x00" "\x0f\x00\x00\x00" "TWO.DAT\0"
				"Now resized to 23 chars"
				"This is two.dat"
			);
		}
};

IMPLEMENT_TESTS(wad_doom);
