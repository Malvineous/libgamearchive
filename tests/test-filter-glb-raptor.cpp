/**
 * @file   test-filter-glb-raptor.cpp
 * @brief  Test code for Raptor GLB encryption algorithm.
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

#include "test-filter.hpp"
#include "../src/filter-glb-raptor.hpp"

using namespace camoto;
using namespace camoto::gamearchive;

BOOST_FIXTURE_TEST_SUITE(glb_suite, test_filter)

BOOST_AUTO_TEST_CASE(read_fat)
{
	BOOST_TEST_MESSAGE("Decode some Raptor GLB-encoded FAT data");

	GLBFATFilterType filter;

	BOOST_CHECK_MESSAGE(is_equal_read(&filter,
		STRING_WITH_NULLS(
			"\x64\x9B\xD1\x09\x4F\xA0\xE2\x15" "\x47\x7E\xB4\xEC\x33\x7F\xC1\xF4" "\x26\x5D\x93\xCB\x12\x5E\xA0\xD3" "\x05\x3C\x72\xAA"
			"\x64\x9B\xD1\x09\x50\x44\x86\xB9"
		), STRING_WITH_NULLS(
			"\x00\x00\x00\x00\xFF\x05\x00\x00" "\x00\x00\x00\x00\x00\x00\x00\x00" "\x00\x00\x00\x00\x00\x00\x00\x00" "\x00\x00\x00\x00"
			"\x00\x00\x00\x00\x00\xA8\x00\x00"
		)), "Raptor GLB-decoding FAT failed"
	);
}

BOOST_AUTO_TEST_CASE(write_fat)
{
	BOOST_TEST_MESSAGE("Encode some FAT data using Raptor's GLB cipher");

	GLBFATFilterType filter;

	BOOST_CHECK_MESSAGE(is_equal_write(&filter,
		STRING_WITH_NULLS(
			"\x00\x00\x00\x00\xFF\x05\x00\x00" "\x00\x00\x00\x00\x00\x00\x00\x00" "\x00\x00\x00\x00\x00\x00\x00\x00" "\x00\x00\x00\x00"
			"\x00\x00\x00\x00\x00\xA8\x00\x00"
		), STRING_WITH_NULLS(
			"\x64\x9B\xD1\x09\x4F\xA0\xE2\x15" "\x47\x7E\xB4\xEC\x33\x7F\xC1\xF4" "\x26\x5D\x93\xCB\x12\x5E\xA0\xD3" "\x05\x3C\x72\xAA"
			"\x64\x9B\xD1\x09\x50\x44\x86\xB9"
		)), "Raptor GLB-encoding FAT failed"
	);
}

BOOST_AUTO_TEST_CASE(read_file)
{
	BOOST_TEST_MESSAGE("Decode some Raptor GLB-encoded file data");

	GLBFileFilterType filter;

	BOOST_CHECK_MESSAGE(is_equal_read(&filter,
		STRING_WITH_NULLS(
			"\x64\x9B\xD1\x09\x4F\xA0\xE2\x15" "\x47\x7E\xB4\xEC\x33\x7F\xC1\xF4" "\x26\x5D\x93\xCB\x12\x5E\xA0\xD3" "\x05\x3C\x72\xAA"
			"\xF1\x3D\x7F\xB2\xE4\xC3\xF9\x31"
		), STRING_WITH_NULLS(
			"\x00\x00\x00\x00\xFF\x05\x00\x00" "\x00\x00\x00\x00\x00\x00\x00\x00" "\x00\x00\x00\x00\x00\x00\x00\x00" "\x00\x00\x00\x00"
			"\x00\x00\x00\x00\x00\xA8\x00\x00"
		)), "Raptor GLB-decoding file failed"
	);
}

BOOST_AUTO_TEST_CASE(write_file)
{
	BOOST_TEST_MESSAGE("Encode some file data using Raptor's GLB cipher");

	GLBFileFilterType filter;

	BOOST_CHECK_MESSAGE(is_equal_write(&filter,
		STRING_WITH_NULLS(
			"\x00\x00\x00\x00\xFF\x05\x00\x00" "\x00\x00\x00\x00\x00\x00\x00\x00" "\x00\x00\x00\x00\x00\x00\x00\x00" "\x00\x00\x00\x00"
			"\x00\x00\x00\x00\x00\xA8\x00\x00"
		), STRING_WITH_NULLS(
			"\x64\x9B\xD1\x09\x4F\xA0\xE2\x15" "\x47\x7E\xB4\xEC\x33\x7F\xC1\xF4" "\x26\x5D\x93\xCB\x12\x5E\xA0\xD3" "\x05\x3C\x72\xAA"
			"\xF1\x3D\x7F\xB2\xE4\xC3\xF9\x31"
		)), "Raptor GLB-encoding file failed"
	);
}

BOOST_AUTO_TEST_SUITE_END()
