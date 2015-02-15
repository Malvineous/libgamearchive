/**
 * @file   test-fmt-gd-doofus.cpp
 * @brief  Test code for Doofus .G-D archives.
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

#define FAT_SIZE 64
class test_suppfat_gd_doofus: public test_archive
{
	public:
		test_suppfat_gd_doofus()
		{
			this->type = "gd-doofus.fat";
			this->outputWidth = 8;
		}

		std::string emptyFAT(unsigned int count)
		{
			std::string rows;
			while (count--) {
				rows.append("\x00\x00" "\x00\x00" "\x00\x00\x00\x00", 8);
			}
			return rows;
		}

		virtual std::string initialstate()
		{
			return STRING_WITH_NULLS(
				"\x0f\x00" "\xEE\x59" "\x00\x00\x00\x00"
				"\x0f\x00" "\xEE\x59" "\x00\x00\x00\x00"
			) + this->emptyFAT(FAT_SIZE - 2);
		}

		virtual std::string rename()
		{
			return STRING_WITH_NULLS(
				"\x0f\x00" "\xEE\x59" "\x00\x00\x00\x00"
				"\x0f\x00" "\xEE\x59" "\x00\x00\x00\x00"
			) + this->emptyFAT(FAT_SIZE - 2);
		}

		virtual std::string insert_end()
		{
			return STRING_WITH_NULLS(
				"\x0f\x00" "\xEE\x59" "\x00\x00\x00\x00"
				"\x0f\x00" "\xEE\x59" "\x00\x00\x00\x00"
				"\x11\x00" "\xEE\x59" "\x00\x00\x00\x00"
			) + this->emptyFAT(FAT_SIZE - 3);
		}

		virtual std::string insert_mid()
		{
			return STRING_WITH_NULLS(
				"\x0f\x00" "\xEE\x59" "\x00\x00\x00\x00"
				"\x11\x00" "\xEE\x59" "\x00\x00\x00\x00"
				"\x0f\x00" "\xEE\x59" "\x00\x00\x00\x00"
			) + this->emptyFAT(FAT_SIZE - 3);
		}

		virtual std::string insert2()
		{
			return STRING_WITH_NULLS(
				"\x0f\x00" "\xEE\x59" "\x00\x00\x00\x00"
				"\x11\x00" "\xEE\x59" "\x00\x00\x00\x00"
				"\x10\x00" "\xEE\x59" "\x00\x00\x00\x00"
				"\x0f\x00" "\xEE\x59" "\x00\x00\x00\x00"
			) + this->emptyFAT(FAT_SIZE - 4);
		}

		virtual std::string remove()
		{
			return STRING_WITH_NULLS(
				"\x0f\x00" "\xEE\x59" "\x00\x00\x00\x00"
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
				"\x11\x00" "\xEE\x59" "\x00\x00\x00\x00"
				"\x0f\x00" "\xEE\x59" "\x00\x00\x00\x00"
			) + this->emptyFAT(FAT_SIZE - 2);
		}

		virtual std::string move()
		{
			return STRING_WITH_NULLS(
				"\x0f\x00" "\xEE\x59" "\x00\x00\x00\x00"
				"\x0f\x00" "\xEE\x59" "\x00\x00\x00\x00"
			) + this->emptyFAT(FAT_SIZE - 2);
		}

		virtual std::string resize_larger()
		{
			return STRING_WITH_NULLS(
				"\x14\x00" "\xEE\x59" "\x00\x00\x00\x00"
				"\x0f\x00" "\xEE\x59" "\x00\x00\x00\x00"
			) + this->emptyFAT(FAT_SIZE - 2);
		}

		virtual std::string resize_smaller()
		{
			return STRING_WITH_NULLS(
				"\x0a\x00" "\xEE\x59" "\x00\x00\x00\x00"
				"\x0f\x00" "\xEE\x59" "\x00\x00\x00\x00"
			) + this->emptyFAT(FAT_SIZE - 2);
		}

		virtual std::string resize_write()
		{
			return STRING_WITH_NULLS(
				"\x17\x00" "\xEE\x59" "\x00\x00\x00\x00"
				"\x0f\x00" "\xEE\x59" "\x00\x00\x00\x00"
			) + this->emptyFAT(FAT_SIZE - 2);
		}

		virtual std::string insert_unknown_type()
		{
			return STRING_WITH_NULLS(
				"\x0f\x00" "\x34\x12" "\x00\x00\x00\x00"
				"\x0f\x00" "\xEE\x59" "\x00\x00\x00\x00"
				"\x0f\x00" "\xEE\x59" "\x00\x00\x00\x00"
			) + this->emptyFAT(FAT_SIZE - 3);
		}
};

class test_gd_doofus: public test_archive
{
	public:
		test_gd_doofus()
		{
			this->type = "gd-doofus";
			this->create = false;
			this->lenMaxFilename = -1;
			this->insertType = "music/tbsa";
			this->suppResult[SuppItem::FAT] = std::make_unique<test_suppfat_gd_doofus>();
		}

		void addTests()
		{
			this->test_archive::addTests();

			ADD_ARCH_TEST(false, &test_gd_doofus::test_insert_unknown_type);

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

		void test_insert_unknown_type()
		{
			BOOST_TEST_MESSAGE("Inserting file with unknown file type");

			Archive::FileHandle epb = this->findFile(0);

			// Insert a file with a numeric typecode ("unknown/doofus-1234")
			Archive::FileHandle ep =
				this->pArchive->insert(epb, "", 0x0f, "unknown/doofus-1234", EA_NONE);

			BOOST_CHECK_MESSAGE(
				this->is_content_equal(this->insert_unknown_type()),
				"Inserting file with known type wrote wrong filetype code"
			);

			auto supp = static_cast<test_suppfat_gd_doofus *>(this->suppResult[camoto::SuppItem::FAT].get());
			BOOST_CHECK_MESSAGE(
				this->is_supp_equal(camoto::SuppItem::FAT, supp->insert_unknown_type()),
				"[SuppItem::FAT] Inserting file with known type wrote wrong filetype code"
			);
		}
};

IMPLEMENT_TESTS(gd_doofus);
