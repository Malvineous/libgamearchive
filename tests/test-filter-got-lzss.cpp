/**
 * @file   test-filter-got-lzss.cpp
 * @brief  Test code for God of Thunder LZSS packer/unpacker.
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

#include <boost/test/unit_test.hpp>
#include <camoto/stream_string.hpp>
#include <camoto/stream_filtered.hpp>
#include <camoto/util.hpp>
#include "../src/filter-got-lzss.hpp"
#include "test-filter.hpp"

using namespace camoto;
using namespace camoto::gamearchive;

struct got_lzss_sample: public test_filter {
	got_lzss_sample()
	{
		this->filter.reset(new filter_got_lzss());
	}
};

struct got_unlzss_sample: public test_filter {
	got_unlzss_sample()
	{
		this->filter.reset(new filter_got_unlzss());
	}
};

BOOST_FIXTURE_TEST_SUITE(got_unlzss_suite, got_unlzss_sample)

BOOST_AUTO_TEST_CASE(got_unlzss_read)
{
	BOOST_TEST_MESSAGE("Decompress some GoT data");

	*this->in << STRING_WITH_NULLS(
		"\x10\x00\x01\x00"
		"\xFF""ABCDEFGH"
		"\xFF""IJKLMNOP"
	);

	BOOST_CHECK_MESSAGE(is_equal("ABCDEFGHIJKLMNOP"),
		"Decompressing GoT data failed");
}

BOOST_AUTO_TEST_CASE(got_unlzss_read_short)
{
	BOOST_TEST_MESSAGE("Decompress a little GoT data");

	*this->in << STRING_WITH_NULLS("\x05\x00\x01\x00" "\xFF""ABCDE");

	BOOST_CHECK_MESSAGE(is_equal("ABCDE"),
		"Decompressing a little GoT data failed");
}

BOOST_AUTO_TEST_SUITE_END()


BOOST_FIXTURE_TEST_SUITE(got_lzss_suite, got_lzss_sample)

BOOST_AUTO_TEST_CASE(got_lzss_read)
{
	BOOST_TEST_MESSAGE("Compress some GoT data");

	*this->in << "ABCDEFGHIJKLMNOP";

	BOOST_CHECK_MESSAGE(is_equal(STRING_WITH_NULLS(
		"\x10\x00\x01\x00"
		"\xFF""ABCDEFGH"
		"\xFF""IJKLMNOP"
	)),
		"Compressing GoT data failed");
}

BOOST_AUTO_TEST_CASE(got_lzss_read_short)
{
	BOOST_TEST_MESSAGE("Compress a little GoT data");

	*this->in << "ABCDE";

	BOOST_CHECK_MESSAGE(is_equal(STRING_WITH_NULLS("\x05\x00\x01\x00" "\xFF""ABCDE")),
		"Compressing a little GoT data failed");
}

BOOST_AUTO_TEST_SUITE_END()
