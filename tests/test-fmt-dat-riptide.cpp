/**
 * @file   test-fmt-dat-riptide.cpp
 * @brief  Test code for Dr. Riptide .DAT archives.
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

class test_dat_riptide: public test_archive
{
	public:
		test_dat_riptide()
		{
			this->type = "dat-riptide";
			this->lenMaxFilename = 12;
		}

		void addTests()
		{
			this->test_archive::addTests();

			// c00: Initial state
			this->isInstance(ArchiveType::DefinitelyYes, this->initialstate());

			// c01: File too short
			this->isInstance(ArchiveType::DefinitelyNo, STRING_WITH_NULLS(
				"\x02"
			));

			// c02: If the file count is zero, the archive must be only two bytes long
			this->isInstance(ArchiveType::DefinitelyNo, STRING_WITH_NULLS(
				"\x00\x00"
				"\x0F\x00\x00\x00" "\x00\x00\x00\x00" "\x34\x00\x00\x00" "ONE.DAT\0\0\0\0\0\0"
				"\x0F\x00\x00\x00" "\x00\x00\x00\x00" "\x43\x00\x00\x00" "TWO.DAT\0\0\0\0\0\0"
				"This is one.dat"
				"This is two.dat"
			));

			// c03: FAT too short
			this->isInstance(ArchiveType::DefinitelyNo, STRING_WITH_NULLS(
				"\x02\x00"
				"\x0F\x00\x00\x00" "\x00\x00\x00\x00" "\x34\x00\x00\x00" "ONE.DAT\0\0\0\0\0\0"
			));

			// c04: Offset past EOF
			this->isInstance(ArchiveType::DefinitelyNo, STRING_WITH_NULLS(
				"\x02\x00"
				"\x0F\x00\x00\x00" "\x00\x00\x00\x00" "\x34\x0F\x00\x00" "ONE.DAT\0\0\0\0\0\0"
				"\x0F\x00\x00\x00" "\x00\x00\x00\x00" "\x43\x00\x00\x00" "TWO.DAT\0\0\0\0\0\0"
				"This is one.dat"
				"This is two.dat"
			));

			// c05: File starts inside FAT
			this->isInstance(ArchiveType::DefinitelyNo, STRING_WITH_NULLS(
				"\x02\x00"
				"\x0F\x00\x00\x00" "\x00\x00\x00\x00" "\x03\x00\x00\x00" "ONE.DAT\0\0\0\0\0\0"
				"\x0F\x00\x00\x00" "\x00\x00\x00\x00" "\x43\x00\x00\x00" "TWO.DAT\0\0\0\0\0\0"
				"This is one.dat"
				"This is two.dat"
			));

			// c06: Filename isn't null terminated
			this->isInstance(ArchiveType::DefinitelyNo, STRING_WITH_NULLS(
				"\x02\x00"
				"\x0F\x00\x00\x00" "\x00\x00\x00\x00" "\x34\x00\x00\x00" "ONE.DATXXXXXX"
				"\x0F\x00\x00\x00" "\x00\x00\x00\x00" "\x43\x00\x00\x00" "TWO.DAT\0\0\0\0\0\0"
				"This is one.dat"
				"This is two.dat"
			));
		}

		virtual std::string initialstate()
		{
			return STRING_WITH_NULLS(
				"\x02\x00"
				"\x0F\x00\x00\x00" "\x00\x00\x00\x00" "\x34\x00\x00\x00" "ONE.DAT\0\0\0\0\0\0"
				"\x0F\x00\x00\x00" "\x00\x00\x00\x00" "\x43\x00\x00\x00" "TWO.DAT\0\0\0\0\0\0"
				"This is one.dat"
				"This is two.dat"
			);
		}

		virtual std::string rename()
		{
			return STRING_WITH_NULLS(
				"\x02\x00"
				"\x0F\x00\x00\x00" "\x00\x00\x00\x00" "\x34\x00\x00\x00" "THREE.DAT\0\0\0\0"
				"\x0F\x00\x00\x00" "\x00\x00\x00\x00" "\x43\x00\x00\x00" "TWO.DAT\0\0\0\0\0\0"
				"This is one.dat"
				"This is two.dat"
			);
		}

		virtual std::string insert_end()
		{
			return STRING_WITH_NULLS(
				"\x03\x00"
				"\x0F\x00\x00\x00" "\x00\x00\x00\x00" "\x4D\x00\x00\x00" "ONE.DAT\0\0\0\0\0\0"
				"\x0F\x00\x00\x00" "\x00\x00\x00\x00" "\x5C\x00\x00\x00" "TWO.DAT\0\0\0\0\0\0"
				"\x11\x00\x00\x00" "\x00\x00\x00\x00" "\x6B\x00\x00\x00" "THREE.DAT\0\0\0\0"
				"This is one.dat"
				"This is two.dat"
				"This is three.dat"
			);
		}

		virtual std::string insert_mid()
		{
			return STRING_WITH_NULLS(
				"\x03\x00"
				"\x0F\x00\x00\x00" "\x00\x00\x00\x00" "\x4D\x00\x00\x00" "ONE.DAT\0\0\0\0\0\0"
				"\x11\x00\x00\x00" "\x00\x00\x00\x00" "\x5C\x00\x00\x00" "THREE.DAT\0\0\0\0"
				"\x0F\x00\x00\x00" "\x00\x00\x00\x00" "\x6D\x00\x00\x00" "TWO.DAT\0\0\0\0\0\0"
				"This is one.dat"
				"This is three.dat"
				"This is two.dat"
			);
		}

		virtual std::string insert2()
		{
			return STRING_WITH_NULLS(
				"\x04\x00"
				"\x0F\x00\x00\x00" "\x00\x00\x00\x00" "\x66\x00\x00\x00" "ONE.DAT\0\0\0\0\0\0"
				"\x11\x00\x00\x00" "\x00\x00\x00\x00" "\x75\x00\x00\x00" "THREE.DAT\0\0\0\0"
				"\x10\x00\x00\x00" "\x00\x00\x00\x00" "\x86\x00\x00\x00" "FOUR.DAT\0\0\0\0\0"
				"\x0F\x00\x00\x00" "\x00\x00\x00\x00" "\x96\x00\x00\x00" "TWO.DAT\0\0\0\0\0\0"
				"This is one.dat"
				"This is three.dat"
				"This is four.dat"
				"This is two.dat"
			);
		}

		virtual std::string remove()
		{
			return STRING_WITH_NULLS(
				"\x01\x00"
				"\x0F\x00\x00\x00" "\x00\x00\x00\x00" "\x1B\x00\x00\x00" "TWO.DAT\0\0\0\0\0\0"
				"This is two.dat"
			);
		}

		virtual std::string remove2()
		{
			return STRING_WITH_NULLS(
				"\x00\x00"
			);
		}

		virtual std::string insert_remove()
		{
			return STRING_WITH_NULLS(
				"\x02\x00"
				"\x11\x00\x00\x00" "\x00\x00\x00\x00" "\x34\x00\x00\x00" "THREE.DAT\0\0\0\0"
				"\x0F\x00\x00\x00" "\x00\x00\x00\x00" "\x45\x00\x00\x00" "TWO.DAT\0\0\0\0\0\0"
				"This is three.dat"
				"This is two.dat"
			);
		}

		virtual std::string move()
		{
			return STRING_WITH_NULLS(
				"\x02\x00"
				"\x0F\x00\x00\x00" "\x00\x00\x00\x00" "\x34\x00\x00\x00" "TWO.DAT\0\0\0\0\0\0"
				"\x0F\x00\x00\x00" "\x00\x00\x00\x00" "\x43\x00\x00\x00" "ONE.DAT\0\0\0\0\0\0"
				"This is two.dat"
				"This is one.dat"
			);
		}

		virtual std::string resize_larger()
		{
			return STRING_WITH_NULLS(
				"\x02\x00"
				"\x14\x00\x00\x00" "\x00\x00\x00\x00" "\x34\x00\x00\x00" "ONE.DAT\0\0\0\0\0\0"
				"\x0F\x00\x00\x00" "\x00\x00\x00\x00" "\x48\x00\x00\x00" "TWO.DAT\0\0\0\0\0\0"
				"This is one.dat\0\0\0\0\0"
				"This is two.dat"
			);
		}

		virtual std::string resize_smaller()
		{
			return STRING_WITH_NULLS(
				"\x02\x00"
				"\x0A\x00\x00\x00" "\x00\x00\x00\x00" "\x34\x00\x00\x00" "ONE.DAT\0\0\0\0\0\0"
				"\x0F\x00\x00\x00" "\x00\x00\x00\x00" "\x3E\x00\x00\x00" "TWO.DAT\0\0\0\0\0\0"
				"This is on"
				"This is two.dat"
			);
		}

		virtual std::string resize_write()
		{
			return STRING_WITH_NULLS(
				"\x02\x00"
				"\x17\x00\x00\x00" "\x00\x00\x00\x00" "\x34\x00\x00\x00" "ONE.DAT\0\0\0\0\0\0"
				"\x0F\x00\x00\x00" "\x00\x00\x00\x00" "\x4B\x00\x00\x00" "TWO.DAT\0\0\0\0\0\0"
				"Now resized to 23 chars"
				"This is two.dat"
			);
		}
};

IMPLEMENT_TESTS(dat_riptide);
