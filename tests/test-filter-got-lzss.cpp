/**
 * @file   test-filter-got-lzss.cpp
 * @brief  Test code for God of Thunder LZSS packer/unpacker.
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

#include "test-filter.hpp"

using namespace camoto::gamearchive;

class test_filter_got_lzss: public test_filter
{
	public:
		test_filter_got_lzss()
		{
			this->type = "lzss-got";
		}

		void addTests()
		{
			this->test_filter::addTests();

			this->content("normal", 16, STRING_WITH_NULLS(
				"\x10\x00\x01\x00"
				"\xFF""ABCDEFGH"
				"\xFF""IJKLMNOP"
			), STRING_WITH_NULLS(
				"ABCDEFGHIJKLMNOP"
			));

			this->content("short", 5, STRING_WITH_NULLS(
				"\x05\x00\x01\x00" "\xFF""ABCDE"
			), STRING_WITH_NULLS(
				"ABCDE"
			));
		}
};

IMPLEMENT_TESTS(filter_got_lzss);
