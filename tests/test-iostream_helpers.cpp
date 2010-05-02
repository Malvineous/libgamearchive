/*
 * test-byteorder.cpp - test code for the endian conversion functions.
 *
 * Copyright (C) 2010 Adam Nielsen <malvineous@shikadi.net>
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

#include <iostream>
#include <sstream>
#include <boost/test/unit_test.hpp>

#define BYTEORDER_USE_IOSTREAMS
#define BYTEORDER_PROVIDE_TYPED_FUNCTIONS

#include "../src/iostream_helpers.hpp"

using namespace camoto::gamearchive;

BOOST_AUTO_TEST_SUITE(iostream_helpers)

BOOST_AUTO_TEST_CASE(null_padded_write)
{
	{
		std::stringstream data;
		data << nullPadded("AB", 4);
		BOOST_REQUIRE_EQUAL(data.str().length(), 4);
		BOOST_CHECK_EQUAL((uint8_t)data.str().at(0), 0x41);
		BOOST_CHECK_EQUAL((uint8_t)data.str().at(1), 0x42);
		BOOST_CHECK_EQUAL((uint8_t)data.str().at(2), 0x00);
		BOOST_CHECK_EQUAL((uint8_t)data.str().at(3), 0x00);
	}
}

BOOST_AUTO_TEST_CASE(null_padded_read)
{
	{
		std::stringstream data;
		data << "ABC\0EFGHIJKL";
		data.seekg(0);
		std::string v;
		data >> nullPadded(v, 8);
		BOOST_CHECK(v.compare("ABC") == 0);
	}
}

BOOST_AUTO_TEST_CASE(fixed_length_read)
{
	{
		std::stringstream data;
		data << std::string("ABC\0EFGHIJKL", 12);
		data.seekg(0);
		std::string v;
		data >> fixedLength(v, 8);
		BOOST_REQUIRE_EQUAL(v.length(), 8);
		BOOST_CHECK_EQUAL((uint8_t)v.c_str()[0], 0x41);
		BOOST_CHECK_EQUAL((uint8_t)v.c_str()[1], 0x42);
		BOOST_CHECK_EQUAL((uint8_t)v.c_str()[2], 0x43);
		BOOST_CHECK_EQUAL((uint8_t)v.c_str()[3], 0x00);
		BOOST_CHECK_EQUAL((uint8_t)v.c_str()[4], 0x45);
		BOOST_CHECK_EQUAL((uint8_t)v.c_str()[5], 0x46);
		BOOST_CHECK_EQUAL((uint8_t)v.c_str()[6], 0x47);
		BOOST_CHECK_EQUAL((uint8_t)v.c_str()[7], 0x48);
		BOOST_CHECK_EQUAL((uint8_t)v.c_str()[8], 0x00);
	}
}

BOOST_AUTO_TEST_SUITE_END()
