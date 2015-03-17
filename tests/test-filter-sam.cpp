/**
 * @file   test-filter-sam.cpp
 * @brief  Test code for Secret Agent XOR encryption algorithm.
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
#include "../src/filter-xor-sagent.hpp"

using namespace camoto;
using namespace camoto::gamearchive;

BOOST_FIXTURE_TEST_SUITE(sam_suite, test_filter)

BOOST_AUTO_TEST_CASE(sam_read)
{
	BOOST_TEST_MESSAGE("Decode some Secret Agent XOR-encoded data");

	FilterType_SAM_Map filter;

	this->test_equal_read(&filter,
		STRING_WITH_NULLS("\xC2\x76\x4E\x5E\xB1\x69\x19\xE9"),
		STRING_WITH_NULLS("\x00\x01\x02\x03\xFF\xFF\xFF\xFF")
	);
}

BOOST_AUTO_TEST_CASE(sam_write)
{
	BOOST_TEST_MESSAGE("Encode some data using Secret Agent XOR cipher");

	FilterType_SAM_Map filter;

	this->test_equal_write(&filter,
		STRING_WITH_NULLS("\x00\x01\x02\x03\xFF\xFF\xFF\xFF"),
		STRING_WITH_NULLS("\xC2\x76\x4E\x5E\xB1\x69\x19\xE9")
	);
}

BOOST_AUTO_TEST_SUITE_END()
