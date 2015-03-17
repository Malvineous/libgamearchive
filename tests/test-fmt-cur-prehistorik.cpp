/**
 * @file   test-fmt-cur-prehistorik.cpp
 * @brief  Test code for Prehistorik .CUR/.VGA archives.
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

class test_cur_prehistorik: public test_archive
{
	public:
		test_cur_prehistorik()
		{
			this->type = "cur-prehistorik";
			this->lenMaxFilename = 32;
		}

		void addTests()
		{
			this->test_archive::addTests();

			// c00: Initial state
			this->isInstance(ArchiveType::DefinitelyYes, this->initialstate());

			// c01: File too short
			this->isInstance(ArchiveType::DefinitelyNo, STRING_WITH_NULLS(
				"\x06\x00"
				"\x00\x00\x00"
			));
			// c01a: File not too short
			this->isInstance(ArchiveType::DefinitelyYes, STRING_WITH_NULLS(
				"\x06\x00"
				"\x00\x00\x00\x00"
			));

			// c02: FAT is too short
			this->isInstance(ArchiveType::DefinitelyNo, STRING_WITH_NULLS(
				"\x05\x00"
				"\x0f\x00\x00\x00" "ONE.DAT\0"
				"\x0f\x00\x00\x00" "TWO.DAT\0"
				"\x00\x00\x00\x00"
				"This is one.dat"
				"This is two.dat"
			));

			// c03: FAT ends past EOF
			this->isInstance(ArchiveType::DefinitelyNo, STRING_WITH_NULLS(
				"\xFF\x00"
				"\x0f\x00\x00\x00" "ONE.DAT\0"
				"\x0f\x00\x00\x00" "TWO.DAT\0"
				"\x00\x00\x00\x00"
				"This is one.dat"
				"This is two.dat"
			));

			// c04: Filename too long
			this->isInstance(ArchiveType::DefinitelyNo, STRING_WITH_NULLS(
				"\x1E\x00"
				"\x1f\x00\x00\x00" "ONE.DAT0123456789ABCDEF\0"
				"\x0f\x00\x00\x00" "TWO.DAT\0"
				"\x00\x00\x00\x00"
				"This is one.dat"
				"This is two.dat"
			));

			// c05: Control char in filename
			this->isInstance(ArchiveType::DefinitelyNo, STRING_WITH_NULLS(
				"\x1E\x00"
				"\x0f\x00\x00\x00" "ON\x05.DAT\0"
				"\x0f\x00\x00\x00" "TWO.DAT\0"
				"\x00\x00\x00\x00"
				"This is one.dat"
				"This is two.dat"
			));

			// c06: FAT ends mid-filename
			this->isInstance(ArchiveType::DefinitelyNo, STRING_WITH_NULLS(
				"\x18\x00"
				"\x0f\x00\x00\x00" "ONE.DAT\0"
				"\x0f\x00\x00\x00" "TWO.DA"
				"This is one.dat"
				"This is two.dat"
			));

			// c07: File goes past archive EOF
			this->isInstance(ArchiveType::DefinitelyNo, STRING_WITH_NULLS(
				"\x1E\x00"
				"\x0f\x00\x00\x00" "ONE.DAT\0"
				"\xff\x00\x00\x00" "TWO.DAT\0"
				"\x00\x00\x00\x00"
				"This is one.dat"
				"This is two.dat"
			));

			// c08: Last file doesn't end at archive EOF
			this->isInstance(ArchiveType::DefinitelyNo, STRING_WITH_NULLS(
				"\x1E\x00"
				"\x0f\x00\x00\x00" "ONE.DAT\0"
				"\x0e\x00\x00\x00" "TWO.DAT\0"
				"\x00\x00\x00\x00"
				"This is one.dat"
				"This is two.dat"
			));
		}

		virtual std::string initialstate()
		{
			return STRING_WITH_NULLS(
				"\x1E\x00"
				"\x0f\x00\x00\x00" "ONE.DAT\0"
				"\x0f\x00\x00\x00" "TWO.DAT\0"
				"\x00\x00\x00\x00"
				"This is one.dat"
				"This is two.dat"
			);
		}

		virtual std::string rename()
		{
			return STRING_WITH_NULLS(
				"\x20\x00"
				"\x0f\x00\x00\x00" "THREE.DAT\0"
				"\x0f\x00\x00\x00" "TWO.DAT\0"
				"\x00\x00\x00\x00"
				"This is one.dat"
				"This is two.dat"
			);
		}

		virtual std::string insert_end()
		{
			return STRING_WITH_NULLS(
				"\x2C\x00"
				"\x0f\x00\x00\x00" "ONE.DAT\0"
				"\x0f\x00\x00\x00" "TWO.DAT\0"
				"\x11\x00\x00\x00" "THREE.DAT\0"
				"\x00\x00\x00\x00"
				"This is one.dat"
				"This is two.dat"
				"This is three.dat"
			);
		}

		virtual std::string insert_mid()
		{
			return STRING_WITH_NULLS(
				"\x2C\x00"
				"\x0f\x00\x00\x00" "ONE.DAT\0"
				"\x11\x00\x00\x00" "THREE.DAT\0"
				"\x0f\x00\x00\x00" "TWO.DAT\0"
				"\x00\x00\x00\x00"
				"This is one.dat"
				"This is three.dat"
				"This is two.dat"
			);
		}

		virtual std::string insert2()
		{
			return STRING_WITH_NULLS(
				"\x39\x00"
				"\x0f\x00\x00\x00" "ONE.DAT\0"
				"\x11\x00\x00\x00" "THREE.DAT\0"
				"\x10\x00\x00\x00" "FOUR.DAT\0"
				"\x0f\x00\x00\x00" "TWO.DAT\0"
				"\x00\x00\x00\x00"
				"This is one.dat"
				"This is three.dat"
				"This is four.dat"
				"This is two.dat"
			);
		}

		virtual std::string remove()
		{
			return STRING_WITH_NULLS(
				"\x12\x00"
				"\x0f\x00\x00\x00" "TWO.DAT\0"
				"\x00\x00\x00\x00"
				"This is two.dat"
			);
		}

		virtual std::string remove2()
		{
			return STRING_WITH_NULLS(
				"\x06\x00"
				"\x00\x00\x00\x00"
			);
		}

		virtual std::string insert_remove()
		{
			return STRING_WITH_NULLS(
				"\x20\x00"
				"\x11\x00\x00\x00" "THREE.DAT\0"
				"\x0f\x00\x00\x00" "TWO.DAT\0"
				"\x00\x00\x00\x00"
				"This is three.dat"
				"This is two.dat"
			);
		}

		virtual std::string move()
		{
			return STRING_WITH_NULLS(
				"\x1E\x00"
				"\x0f\x00\x00\x00" "TWO.DAT\0"
				"\x0f\x00\x00\x00" "ONE.DAT\0"
				"\x00\x00\x00\x00"
				"This is two.dat"
				"This is one.dat"
			);
		}

		virtual std::string resize_larger()
		{
			return STRING_WITH_NULLS(
				"\x1E\x00"
				"\x14\x00\x00\x00" "ONE.DAT\0"
				"\x0f\x00\x00\x00" "TWO.DAT\0"
				"\x00\x00\x00\x00"
				"This is one.dat\0\0\0\0\0"
				"This is two.dat"
			);
		}

		virtual std::string resize_smaller()
		{
			return STRING_WITH_NULLS(
				"\x1E\x00"
				"\x0a\x00\x00\x00" "ONE.DAT\0"
				"\x0f\x00\x00\x00" "TWO.DAT\0"
				"\x00\x00\x00\x00"
				"This is on"
				"This is two.dat"
			);
		}

		virtual std::string resize_write()
		{
			return STRING_WITH_NULLS(
				"\x1E\x00"
				"\x17\x00\x00\x00" "ONE.DAT\0"
				"\x0f\x00\x00\x00" "TWO.DAT\0"
				"\x00\x00\x00\x00"
				"Now resized to 23 chars"
				"This is two.dat"
			);
		}
};

IMPLEMENT_TESTS(cur_prehistorik);
