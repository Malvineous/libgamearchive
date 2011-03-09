/**
 * @file   test-filter-xor.cpp
 * @brief  Test code for generic XOR encryption algorithm.
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

#include <boost/test/unit_test.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/copy.hpp>
#include <iostream>
#include <sstream>

#include "tests.hpp"
#include "../src/filter-xor.hpp"

using namespace camoto::gamearchive;

struct xor_crypt_sample: public default_sample {

	std::stringstream in;
	std::stringstream out;

	xor_crypt_sample()
	{
	}

	boost::test_tools::predicate_result is_equal(const std::string& strExpected)
	{
		// See if the stringstream now matches what we expected
		return this->default_sample::is_equal(strExpected, out.str());
	}

};

BOOST_FIXTURE_TEST_SUITE(xor_suite, xor_crypt_sample)

BOOST_AUTO_TEST_CASE(xor_read)
{
	BOOST_TEST_MESSAGE("Decode some XOR-encoded data");

	in << makeString("\x00\x01\x02\x03\xFF\xFF\xFF\xFF");

	io::filtering_istream inf;
	inf.push(xor_crypt_filter(0, 0));
	inf.push(in);

	boost::iostreams::copy(inf, out);

	BOOST_CHECK_MESSAGE(is_equal(makeString("\x00\x00\x00\x00\xFB\xFA\xF9\xF8")),
		"Decoding XOR-encoded data failed");
}

BOOST_AUTO_TEST_CASE(xor_partial_read)
{
	BOOST_TEST_MESSAGE("Decode some partially XOR-encoded data");

	in << makeString("\x00\x01\x02\x03\xFF\xFF\xFF\xFF");

	io::filtering_istream inf;
	inf.push(xor_crypt_filter(4, 0));
	inf.push(in);

	boost::iostreams::copy(inf, out);

	BOOST_CHECK_MESSAGE(is_equal(makeString("\x00\x00\x00\x00\xFF\xFF\xFF\xFF")),
		"Decoding partially XOR-encoded data failed");
}

BOOST_AUTO_TEST_CASE(xor_altseed_read)
{
	BOOST_TEST_MESSAGE("Decode some XOR-encoded data with alternate seed");

	in << makeString("\x00\x01\x02\x03\xFF\xFF\xFF\xFF");

	io::filtering_istream inf;
	inf.push(xor_crypt_filter(0, 0xFE));
	inf.push(in);

	boost::iostreams::copy(inf, out);

	BOOST_CHECK_MESSAGE(is_equal(makeString("\xFE\xFE\x02\x02\xFD\xFC\xFB\xFA")),
		"Decoding XOR-encoded data with alternate seed failed");
}

BOOST_AUTO_TEST_SUITE_END()
