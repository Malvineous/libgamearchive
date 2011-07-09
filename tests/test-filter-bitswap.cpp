/**
 * @file   test-filter-bitswap.cpp
 * @brief  Test code for generic bitswap algorithm.
 *
 * Copyright (C) 2010-2011 Adam Nielsen <malvineous@shikadi.net>
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
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/copy.hpp>
#include <iostream>
#include <sstream>

#include "tests.hpp"
#include "../src/filter-bitswap.hpp"

using namespace camoto::gamearchive;

struct bitswap_sample: public default_sample {

	std::stringstream in;
	std::stringstream out;

	bitswap_sample()
	{
	}

	boost::test_tools::predicate_result is_equal(const std::string& strExpected)
	{
		// See if the stringstream now matches what we expected
		return this->default_sample::is_equal(strExpected, out.str());
	}

};

BOOST_FIXTURE_TEST_SUITE(bitswap_suite, bitswap_sample)

BOOST_AUTO_TEST_CASE(bitswap_read)
{
	BOOST_TEST_MESSAGE("Decode some bitswapped data");

	in << makeString("\x00\x01\x03\x0F\x1E\x55\xAA\xFF");

	io::filtering_istream inf;
	inf.push(bitswap_filter());
	inf.push(in);

	boost::iostreams::copy(inf, out);

	BOOST_CHECK_MESSAGE(is_equal(makeString("\x00\x80\xC0\xF0\x78\xAA\x55\xFF")),
		"Decoding bitswapped data failed");
}

BOOST_AUTO_TEST_SUITE_END()
