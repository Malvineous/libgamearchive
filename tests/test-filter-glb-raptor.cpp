/**
 * @file   test-filter-glb-raptor.cpp
 * @brief  Test code for Raptor GLB encryption algorithm.
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

class test_filter_glb_raptor_file: public test_filter
{
	public:
		test_filter_glb_raptor_file()
		{
			this->type = "glb-raptor";
		}

		void addTests()
		{
			this->test_filter::addTests();

			// Swap some data
			this->content("normal", 36, STRING_WITH_NULLS(
				"\x64\x9B\xD1\x09\x4F\xA0\xE2\x15" "\x47\x7E\xB4\xEC\x33\x7F\xC1\xF4" "\x26\x5D\x93\xCB\x12\x5E\xA0\xD3" "\x05\x3C\x72\xAA"
				"\xF1\x3D\x7F\xB2\xE4\xC3\xF9\x31"
			), STRING_WITH_NULLS(
				"\x00\x00\x00\x00\xFF\x05\x00\x00" "\x00\x00\x00\x00\x00\x00\x00\x00" "\x00\x00\x00\x00\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"\x00\x00\x00\x00\x00\xA8\x00\x00"
			));
		}
};

class test_filter_glb_raptor_fat: public test_filter
{
	public:
		test_filter_glb_raptor_fat()
		{
			this->type = "glb-raptor-fat";
		}

		void addTests()
		{
			this->test_filter::addTests();

			// Swap some data
			this->content("normal", 36, STRING_WITH_NULLS(
				"\x64\x9B\xD1\x09\x4F\xA0\xE2\x15" "\x47\x7E\xB4\xEC\x33\x7F\xC1\xF4" "\x26\x5D\x93\xCB\x12\x5E\xA0\xD3" "\x05\x3C\x72\xAA"
				"\x64\x9B\xD1\x09\x50\x44\x86\xB9"
			), STRING_WITH_NULLS(
				"\x00\x00\x00\x00\xFF\x05\x00\x00" "\x00\x00\x00\x00\x00\x00\x00\x00" "\x00\x00\x00\x00\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"\x00\x00\x00\x00\x00\xA8\x00\x00"
			));
		}
};

IMPLEMENT_TESTS(filter_glb_raptor_file);
IMPLEMENT_TESTS(filter_glb_raptor_fat);
