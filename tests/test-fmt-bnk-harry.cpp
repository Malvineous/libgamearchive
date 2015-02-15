/**
 * @file   test-arch-bnk-harry.cpp
 * @brief  Test code for Halloween Harry .BNK archives.
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

class test_suppfat_bnk_harry: public test_archive
{
	public:
		test_suppfat_bnk_harry()
		{
			this->type = "bnk-harry.fat";
		}

		virtual std::string initialstate()
		{
			return STRING_WITH_NULLS(
				"\x07ONE.DAT\0\0\0\0\0" "\x16\x00\x00\x00" "\x0f\x00\x00\x00"
				"\x07TWO.DAT\0\0\0\0\0" "\x3b\x00\x00\x00" "\x0f\x00\x00\x00"
			);
		}

		virtual std::string rename()
		{
			return STRING_WITH_NULLS(
				"\x09THREE.DAT\0\0\0"   "\x16\x00\x00\x00" "\x0f\x00\x00\x00"
				"\x07TWO.DAT\0\0\0\0\0" "\x3b\x00\x00\x00" "\x0f\x00\x00\x00"
			);
		}

		virtual std::string insert_end()
		{
			return STRING_WITH_NULLS(
				"\x07ONE.DAT\0\0\0\0\0" "\x16\x00\x00\x00" "\x0f\x00\x00\x00"
				"\x07TWO.DAT\0\0\0\0\0" "\x3b\x00\x00\x00" "\x0f\x00\x00\x00"
				"\x09THREE.DAT\0\0\0"   "\x60\x00\x00\x00" "\x11\x00\x00\x00"
			);
		}

		virtual std::string insert_mid()
		{
			return STRING_WITH_NULLS(
				"\x07ONE.DAT\0\0\0\0\0" "\x16\x00\x00\x00" "\x0f\x00\x00\x00"
				"\x09THREE.DAT\0\0\0"   "\x3b\x00\x00\x00" "\x11\x00\x00\x00"
				"\x07TWO.DAT\0\0\0\0\0" "\x62\x00\x00\x00" "\x0f\x00\x00\x00"
			);
		}

		virtual std::string insert2()
		{
			return STRING_WITH_NULLS(
				"\x07ONE.DAT\0\0\0\0\0" "\x16\x00\x00\x00" "\x0f\x00\x00\x00"
				"\x09THREE.DAT\0\0\0"   "\x3b\x00\x00\x00" "\x11\x00\x00\x00"
				"\x08""FOUR.DAT\0\0\0\0"  "\x62\x00\x00\x00" "\x10\x00\x00\x00"
				"\x07TWO.DAT\0\0\0\0\0" "\x88\x00\x00\x00" "\x0f\x00\x00\x00"
			);
		}

		virtual std::string remove()
		{
			return STRING_WITH_NULLS(
				"\x07TWO.DAT\0\0\0\0\0" "\x16\x00\x00\x00" "\x0f\x00\x00\x00"
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
				"\x09THREE.DAT\0\0\0"   "\x16\x00\x00\x00" "\x11\x00\x00\x00"
				"\x07TWO.DAT\0\0\0\0\0" "\x3d\x00\x00\x00" "\x0f\x00\x00\x00"
			);
		}

		virtual std::string move()
		{
			return STRING_WITH_NULLS(
				"\x07TWO.DAT\0\0\0\0\0" "\x16\x00\x00\x00" "\x0f\x00\x00\x00"
				"\x07ONE.DAT\0\0\0\0\0" "\x3b\x00\x00\x00" "\x0f\x00\x00\x00"
			);
		}

		virtual std::string resize_larger()
		{
			return STRING_WITH_NULLS(
				"\x07ONE.DAT\0\0\0\0\0" "\x16\x00\x00\x00" "\x14\x00\x00\x00"
				"\x07TWO.DAT\0\0\0\0\0" "\x40\x00\x00\x00" "\x0f\x00\x00\x00"
			);
		}

		virtual std::string resize_smaller()
		{
			return STRING_WITH_NULLS(
				"\x07ONE.DAT\0\0\0\0\0" "\x16\x00\x00\x00" "\x0a\x00\x00\x00"
				"\x07TWO.DAT\0\0\0\0\0" "\x36\x00\x00\x00" "\x0f\x00\x00\x00"
			);
		}

		virtual std::string resize_write()
		{
			return STRING_WITH_NULLS(
				"\x07ONE.DAT\0\0\0\0\0" "\x16\x00\x00\x00" "\x17\x00\x00\x00"
				"\x07TWO.DAT\0\0\0\0\0" "\x43\x00\x00\x00" "\x0f\x00\x00\x00"
			);
		}
};

class test_bnk_harry: public test_archive
{
	public:
		test_bnk_harry()
		{
			this->type = "bnk-harry";
			this->lenMaxFilename = 12;
			this->suppResult[SuppItem::FAT] = std::make_unique<test_suppfat_bnk_harry>();
		}

		void addTests()
		{
			this->test_archive::addTests();

			// c00: Initial state
			this->isInstance(ArchiveType::DefinitelyYes, this->initialstate());

			// c01: Bad signature
			this->isInstance(ArchiveType::DefinitelyNo, STRING_WITH_NULLS(
				"\x05-ID-" "\x07ONE.DAT\0\0\0\0\0" "\x0f\x00\x00\x00"
				"This is one.dat"
				"\x04-ID-" "\x07TWO.DAT\0\0\0\0\0" "\x0f\x00\x00\x00"
				"This is two.dat"
			));
		}

		virtual std::string initialstate()
		{
			return STRING_WITH_NULLS(
				"\x04-ID-" "\x07ONE.DAT\0\0\0\0\0" "\x0f\x00\x00\x00"
				"This is one.dat"
				"\x04-ID-" "\x07TWO.DAT\0\0\0\0\0" "\x0f\x00\x00\x00"
				"This is two.dat"
			);
		}

		virtual std::string rename()
		{
			return STRING_WITH_NULLS(
				"\x04-ID-" "\x09THREE.DAT\0\0\0" "\x0f\x00\x00\x00"
				"This is one.dat"
				"\x04-ID-" "\x07TWO.DAT\0\0\0\0\0" "\x0f\x00\x00\x00"
				"This is two.dat"
			);
		}

		virtual std::string insert_end()
		{
			return STRING_WITH_NULLS(
				"\x04-ID-" "\x07ONE.DAT\0\0\0\0\0" "\x0f\x00\x00\x00"
				"This is one.dat"
				"\x04-ID-" "\x07TWO.DAT\0\0\0\0\0" "\x0f\x00\x00\x00"
				"This is two.dat"
				"\x04-ID-" "\x09THREE.DAT\0\0\0"   "\x11\x00\x00\x00"
				"This is three.dat"
			);
		}

		virtual std::string insert_mid()
		{
			return STRING_WITH_NULLS(
				"\x04-ID-" "\x07ONE.DAT\0\0\0\0\0" "\x0f\x00\x00\x00"
				"This is one.dat"
				"\x04-ID-" "\x09THREE.DAT\0\0\0"   "\x11\x00\x00\x00"
				"This is three.dat"
				"\x04-ID-" "\x07TWO.DAT\0\0\0\0\0" "\x0f\x00\x00\x00"
				"This is two.dat"
			);
		}

		virtual std::string insert2()
		{
			return STRING_WITH_NULLS(
				"\x04-ID-" "\x07ONE.DAT\0\0\0\0\0" "\x0f\x00\x00\x00"
				"This is one.dat"
				"\x04-ID-" "\x09THREE.DAT\0\0\0"   "\x11\x00\x00\x00"
				"This is three.dat"
				"\x04-ID-" "\x08""FOUR.DAT\0\0\0\0"  "\x10\x00\x00\x00"
				"This is four.dat"
				"\x04-ID-" "\x07TWO.DAT\0\0\0\0\0" "\x0f\x00\x00\x00"
				"This is two.dat"
			);
		}

		virtual std::string remove()
		{
			return STRING_WITH_NULLS(
				"\x04-ID-" "\x07TWO.DAT\0\0\0\0\0" "\x0f\x00\x00\x00"
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
				"\x04-ID-" "\x09THREE.DAT\0\0\0"   "\x11\x00\x00\x00"
				"This is three.dat"
				"\x04-ID-" "\x07TWO.DAT\0\0\0\0\0" "\x0f\x00\x00\x00"
				"This is two.dat"
			);
		}

		virtual std::string move()
		{
			return STRING_WITH_NULLS(
				"\x04-ID-" "\x07TWO.DAT\0\0\0\0\0" "\x0f\x00\x00\x00"
				"This is two.dat"
				"\x04-ID-" "\x07ONE.DAT\0\0\0\0\0" "\x0f\x00\x00\x00"
				"This is one.dat"
			);
		}

		virtual std::string resize_larger()
		{
			return STRING_WITH_NULLS(
				"\x04-ID-" "\x07ONE.DAT\0\0\0\0\0" "\x14\x00\x00\x00"
				"This is one.dat\0\0\0\0\0"
				"\x04-ID-" "\x07TWO.DAT\0\0\0\0\0" "\x0f\x00\x00\x00"
				"This is two.dat"
			);
		}

		virtual std::string resize_smaller()
		{
			return STRING_WITH_NULLS(
				"\x04-ID-" "\x07ONE.DAT\0\0\0\0\0" "\x0a\x00\x00\x00"
				"This is on"
				"\x04-ID-" "\x07TWO.DAT\0\0\0\0\0" "\x0f\x00\x00\x00"
				"This is two.dat"
			);
		}

		virtual std::string resize_write()
		{
			return STRING_WITH_NULLS(
				"\x04-ID-" "\x07ONE.DAT\0\0\0\0\0" "\x17\x00\x00\x00"
				"Now resized to 23 chars"
				"\x04-ID-" "\x07TWO.DAT\0\0\0\0\0" "\x0f\x00\x00\x00"
				"This is two.dat"
			);
		}
};

IMPLEMENT_TESTS(bnk_harry);
