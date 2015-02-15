/**
 * @file   test-fmt-dat-hocus.cpp
 * @brief  Test code for Hocus Pocus .DAT archives.
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

/// Number of entries in the FAT
#define FAT_SIZE 16

class test_suppfat_dat_hocus: public test_archive
{
	public:
		test_suppfat_dat_hocus()
		{
			this->type = "dat-hocus.fat";
			this->outputWidth = 8;
		}

		std::string emptyFAT(unsigned int count)
		{
			std::string rows;
			while (count--) {
				rows.append("\x00\x00\x00\x00" "\x00\x00\x00\x00", 8);
			}
			return rows;
		}

		virtual std::string initialstate()
		{
			return STRING_WITH_NULLS(
				"\x00\x00\x00\x00" "\x0f\x00\x00\x00"
				"\x0f\x00\x00\x00" "\x0f\x00\x00\x00"
			) + this->emptyFAT(FAT_SIZE - 2);
		}

		virtual std::string rename()
		{
			return STRING_WITH_NULLS(
				"\x00\x00\x00\x00" "\x0f\x00\x00\x00"
				"\x0f\x00\x00\x00" "\x0f\x00\x00\x00"
			) + this->emptyFAT(FAT_SIZE - 2);
		}

		virtual std::string insert_end()
		{
			return STRING_WITH_NULLS(
				"\x00\x00\x00\x00" "\x0f\x00\x00\x00"
				"\x0f\x00\x00\x00" "\x0f\x00\x00\x00"
				"\x1e\x00\x00\x00" "\x11\x00\x00\x00"
			) + this->emptyFAT(FAT_SIZE - 3);
		}

		virtual std::string insert_mid()
		{
			return STRING_WITH_NULLS(
				"\x00\x00\x00\x00" "\x0f\x00\x00\x00"
				"\x0f\x00\x00\x00" "\x11\x00\x00\x00"
				"\x20\x00\x00\x00" "\x0f\x00\x00\x00"
			) + this->emptyFAT(FAT_SIZE - 3);
		}

		virtual std::string insert2()
		{
			return STRING_WITH_NULLS(
				"\x00\x00\x00\x00" "\x0f\x00\x00\x00"
				"\x0f\x00\x00\x00" "\x11\x00\x00\x00"
				"\x20\x00\x00\x00" "\x10\x00\x00\x00"
				"\x30\x00\x00\x00" "\x0f\x00\x00\x00"
			) + this->emptyFAT(FAT_SIZE - 4);
		}

		virtual std::string remove()
		{
			return STRING_WITH_NULLS(
				"\x00\x00\x00\x00" "\x0f\x00\x00\x00"
			) + this->emptyFAT(FAT_SIZE - 1);
		}

		virtual std::string remove2()
		{
			return STRING_WITH_NULLS(
				""
			) + this->emptyFAT(FAT_SIZE - 0);
		}

		virtual std::string insert_remove()
		{
			return STRING_WITH_NULLS(
				"\x00\x00\x00\x00" "\x11\x00\x00\x00"
				"\x11\x00\x00\x00" "\x0f\x00\x00\x00"
			) + this->emptyFAT(FAT_SIZE - 2);
		}

		virtual std::string move()
		{
			return STRING_WITH_NULLS(
				"\x00\x00\x00\x00" "\x0f\x00\x00\x00"
				"\x0f\x00\x00\x00" "\x0f\x00\x00\x00"
			) + this->emptyFAT(FAT_SIZE - 2);
		}

		virtual std::string resize_larger()
		{
			return STRING_WITH_NULLS(
				"\x00\x00\x00\x00" "\x14\x00\x00\x00"
				"\x14\x00\x00\x00" "\x0f\x00\x00\x00"
			) + this->emptyFAT(FAT_SIZE - 2);
		}

		virtual std::string resize_smaller()
		{
			return STRING_WITH_NULLS(
				"\x00\x00\x00\x00" "\x0a\x00\x00\x00"
				"\x0a\x00\x00\x00" "\x0f\x00\x00\x00"
			) + this->emptyFAT(FAT_SIZE - 2);
		}

		virtual std::string resize_write()
		{
			return STRING_WITH_NULLS(
				"\x00\x00\x00\x00" "\x17\x00\x00\x00"
				"\x17\x00\x00\x00" "\x0f\x00\x00\x00"
			) + this->emptyFAT(FAT_SIZE - 2);
		}
};

class test_dat_hocus: public test_archive
{
	public:
		test_dat_hocus()
		{
			this->type = "dat-hocus";
			this->create = false;
			this->lenMaxFilename = -1;
			this->suppResult[SuppItem::FAT] = std::make_unique<test_suppfat_dat_hocus>();
		}

		void addTests()
		{
			this->test_archive::addTests();

			// c00: Initial state
			this->isInstance(ArchiveType::Unsure, this->initialstate());
		}

		virtual std::string initialstate()
		{
			return STRING_WITH_NULLS(
				"This is one.dat"
				"This is two.dat"
			);
		}

		virtual std::string rename()
		{
			return STRING_WITH_NULLS(
				"This is one.dat"
				"This is two.dat"
			);
		}

		virtual std::string insert_end()
		{
			return STRING_WITH_NULLS(
				"This is one.dat"
				"This is two.dat"
				"This is three.dat"
			);
		}

		virtual std::string insert_mid()
		{
			return STRING_WITH_NULLS(
				"This is one.dat"
				"This is three.dat"
				"This is two.dat"
			);
		}

		virtual std::string insert2()
		{
			return STRING_WITH_NULLS(
				"This is one.dat"
				"This is three.dat"
				"This is four.dat"
				"This is two.dat"
			);
		}

		virtual std::string remove()
		{
			return STRING_WITH_NULLS(
				"This is two.dat"
			);
		}

		virtual std::string remove2()
		{
			return STRING_WITH_NULLS(
				""
			);
		}

		virtual std::string insert_remove()
		{
			return STRING_WITH_NULLS(
				"This is three.dat"
				"This is two.dat"
			);
		}

		virtual std::string move()
		{
			return STRING_WITH_NULLS(
				"This is two.dat"
				"This is one.dat"
			);
		}

		virtual std::string resize_larger()
		{
			return STRING_WITH_NULLS(
				"This is one.dat\0\0\0\0\0"
				"This is two.dat"
			);
		}

		virtual std::string resize_smaller()
		{
			return STRING_WITH_NULLS(
				"This is on"
				"This is two.dat"
			);
		}

		virtual std::string resize_write()
		{
			return STRING_WITH_NULLS(
				"Now resized to 23 chars"
				"This is two.dat"
			);
		}

		virtual std::string insert_unknown_type()
		{
			return STRING_WITH_NULLS(
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"This is one.dat"
				"This is two.dat"
			);
		}
};

IMPLEMENT_TESTS(dat_hocus);
