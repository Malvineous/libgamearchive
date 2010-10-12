/**
 * @file   test-filter-bash-rle.cpp
 * @brief  Test code for Monster Bash RLE packer/unpacker.
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

#include <boost/algorithm/string.hpp> // for case-insensitive string compare
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <iostream>
#include <iomanip>
#include <vector>

#include <camoto/debug.hpp>
#include "../src/filter-bash-rle.hpp"
#include "tests.hpp"

using namespace camoto::gamearchive;

struct bash_rle_sample: public default_sample {

	std::stringstream in;
	std::stringstream out;

	bash_rle_sample()
	{
		//this->in << LZW_DECOMP_OUT;
		// Make sure the data went in correctly to begin the test
		//BOOST_REQUIRE(this->psstrBase->str().compare(LZW_DECOMP_OUT) == 0);
	}

	boost::test_tools::predicate_result is_equal(const std::string& strExpected)
	{
		// See if the stringstream now matches what we expected
		//std::string strExpected = LZW_DECOMP_OUT;
		return this->default_sample::is_equal(strExpected, out.str());
	}

};

BOOST_FIXTURE_TEST_SUITE(bash_unrle_suite, bash_rle_sample)

BOOST_AUTO_TEST_CASE(bash_unrle_read)
{
	BOOST_TEST_MESSAGE("Decode some Monster Bash RLE-encoded data");

	in << "ABC\x90\x05""D";

	io::filtering_istream inf;
	inf.push(bash_unrle_filter());
	inf.push(in);

	boost::iostreams::copy(inf, out);

	BOOST_CHECK_MESSAGE(is_equal("ABCCCCCD"),
		"Decoding Monster Bash RLE-encoded data failed");
}

BOOST_AUTO_TEST_CASE(bash_unrle_escape)
{
	BOOST_TEST_MESSAGE("Decode RLE-escape in Monster Bash RLE-encoded data");

	in << "ABC\x90" << std::ends << "D";

	io::filtering_istream inf;
	inf.push(bash_unrle_filter());
	inf.push(in);

	boost::iostreams::copy(inf, out);

	BOOST_CHECK_MESSAGE(is_equal("ABC\x90""D"),
		"Decoding RLE-escape in Monster Bash RLE-encoded data failed");
}

BOOST_AUTO_TEST_CASE(bash_unrle_truncated_read)
{
	BOOST_TEST_MESSAGE("Read truncated RLE-escape in Monster Bash RLE-encoded data");

	in << "ABC\x90";

	io::filtering_istream inf;
	inf.push(bash_unrle_filter());
	inf.push(in);

	boost::iostreams::copy(inf, out);

	BOOST_CHECK_MESSAGE(is_equal("ABC"),
		"Read truncated RLE-escape in Monster Bash RLE-encoded data failed");
}

BOOST_AUTO_TEST_CASE(bash_rle_read)
{
	BOOST_TEST_MESSAGE("Decode some Monster Bash RLE-encoded data");

	in << "ABCCCCCD";

	io::filtering_istream inf;
	inf.push(bash_rle_filter());
	inf.push(in);

	boost::iostreams::copy(inf, out);

	BOOST_CHECK_MESSAGE(is_equal("ABC\x90\x05""D"),
		"Decoding Monster Bash RLE-encoded data failed");
}

BOOST_AUTO_TEST_CASE(bash_rle_escape)
{
	BOOST_TEST_MESSAGE("Encode RLE-escape in Monster Bash RLE-encoded data");

	in << "ABC\x90""D";

	io::filtering_istream inf;
	inf.push(bash_rle_filter());
	inf.push(in);

	boost::iostreams::copy(inf, out);

	BOOST_CHECK_MESSAGE(is_equal(std::string("ABC\x90\x00""D", 6)),
		"Encoding RLE-escape in Monster Bash RLE-encoded data failed");
}

BOOST_AUTO_TEST_CASE(bash_rle_repeat_escape)
{
	BOOST_TEST_MESSAGE("RLE-encode the RLE event byte in Monster Bash RLE-encoded data");

	in << "ABC\x90\x90\x90\x90\x90""D";

	io::filtering_istream inf;
	inf.push(bash_rle_filter());
	inf.push(in);

	boost::iostreams::copy(inf, out);

	BOOST_CHECK_MESSAGE(is_equal(std::string("ABC\x90\x00\x90\x05""D", 8)),
		"RLE-encode the RLE event byte in Monster Bash RLE-encoded data failed");
}

BOOST_AUTO_TEST_CASE(bash_rle_read_trailing)
{
	BOOST_TEST_MESSAGE("Write ending with RLE event in Monster Bash RLE-encoded data");

	in << "ABCCCCCC";

	io::filtering_istream inf;
	inf.push(bash_rle_filter());
	inf.push(in);

	boost::iostreams::copy(inf, out);

	BOOST_CHECK_MESSAGE(is_equal("ABC\x90\x06"),
		"Write ending with RLE event in Monster Bash RLE-encoded data failed");
}

BOOST_AUTO_TEST_CASE(bash_rle_escape_trailing)
{
	BOOST_TEST_MESSAGE("Write ending with RLE char in Monster Bash RLE-encoded data");

	in << "ABC\x90";

	io::filtering_istream inf;
	inf.push(bash_rle_filter());
	inf.push(in);

	boost::iostreams::copy(inf, out);

	BOOST_CHECK_MESSAGE(is_equal(std::string("ABC\x90\x00", 5)),
		"Write ending with RLE char in Monster Bash RLE-encoded data failed");
}

BOOST_AUTO_TEST_CASE(bash_rle_read_lots1)
{
	BOOST_TEST_MESSAGE("RLE encode > 256 bytes (one leftover) in Monster Bash RLE-encoded data");

	in << "AB" << std::string(256+1, 'C');

	io::filtering_istream inf;
	inf.push(bash_rle_filter());
	inf.push(in);

	boost::iostreams::copy(inf, out);

	BOOST_CHECK_MESSAGE(is_equal("ABC\x90\xFF""C"),
		"RLE encode > 256 bytes (one leftover) in Monster Bash RLE-encoded data failed");
}

BOOST_AUTO_TEST_CASE(bash_rle_read_lots2)
{
	BOOST_TEST_MESSAGE("RLE encode > 256 bytes (two leftovers) in Monster Bash RLE-encoded data");

	in << "AB" << std::string(256+2, 'C');

	io::filtering_istream inf;
	inf.push(bash_rle_filter());
	inf.push(in);

	boost::iostreams::copy(inf, out);

	BOOST_CHECK_MESSAGE(is_equal("ABC\x90\xFF""CC"),
		"RLE encode > 256 bytes (two leftovers) in Monster Bash RLE-encoded data failed");
}

BOOST_AUTO_TEST_CASE(bash_rle_read_lots3)
{
	BOOST_TEST_MESSAGE("RLE encode > 256 bytes (three leftovers) in Monster Bash RLE-encoded data");

	in << "AB" << std::string(256+3, 'C');

	io::filtering_istream inf;
	inf.push(bash_rle_filter());
	inf.push(in);

	boost::iostreams::copy(inf, out);

	BOOST_CHECK_MESSAGE(is_equal("ABC\x90\xFF\x90\x04"),
		"RLE encode > 256 bytes (three leftovers) in Monster Bash RLE-encoded data failed");
}

BOOST_AUTO_TEST_CASE(bash_rle_read_lots4)
{
	BOOST_TEST_MESSAGE("RLE encode > 256 bytes (four leftovers) in Monster Bash RLE-encoded data");

	in << "AB" << std::string(256+4, 'C');

	io::filtering_istream inf;
	inf.push(bash_rle_filter());
	inf.push(in);

	boost::iostreams::copy(inf, out);

	BOOST_CHECK_MESSAGE(is_equal("ABC\x90\xFF\x90\x05"),
		"RLE encode > 256 bytes (four leftovers) in Monster Bash RLE-encoded data failed");
}

BOOST_AUTO_TEST_CASE(bash_rle_short2)
{
	BOOST_TEST_MESSAGE("RLE event skipping with doubled data in Monster Bash RLE-encoded data");

	// Would come out larger post-RLE, so don't bother
	in << "ABCCD";

	io::filtering_istream inf;
	inf.push(bash_rle_filter());
	inf.push(in);

	boost::iostreams::copy(inf, out);

	BOOST_CHECK_MESSAGE(is_equal("ABCCD"),
		"RLE event skipping with doubled data in Monster Bash RLE-encoded data failed");
}

BOOST_AUTO_TEST_CASE(bash_rle_short3)
{
	BOOST_TEST_MESSAGE("RLE event skipping with tripled data in Monster Bash RLE-encoded data");

	// Would come out the same size post-RLE, so don't bother
	in << "ABCCCD";

	io::filtering_istream inf;
	inf.push(bash_rle_filter());
	inf.push(in);

	boost::iostreams::copy(inf, out);

	BOOST_CHECK_MESSAGE(is_equal("ABCCCD"),
		"RLE event skipping with tripled data in Monster Bash RLE-encoded data failed");
}

BOOST_AUTO_TEST_CASE(bash_rle_short_escape)
{
	BOOST_TEST_MESSAGE("Escaping doubled RLE event char in Monster Bash RLE-encoded data");

	// Would come out larger post-RLE, so don't bother
	in << "AB\x90\x90""D";

	io::filtering_istream inf;
	inf.push(bash_rle_filter());
	inf.push(in);

	boost::iostreams::copy(inf, out);

	BOOST_CHECK_MESSAGE(is_equal(std::string("AB\x90\x00\x90\x00""D", 7)),
		"Escaping doubled RLE event char in Monster Bash RLE-encoded data failed");
}

BOOST_AUTO_TEST_CASE(bash_rle_long_escape)
{
	BOOST_TEST_MESSAGE("Escaping many RLE event chars in Monster Bash RLE-encoded data");

	// Would come out larger post-RLE, so don't bother
	in << "AB" << std::string(257, '\x90') << "D";

	io::filtering_istream inf;
	inf.push(bash_rle_filter());
	inf.push(in);

	boost::iostreams::copy(inf, out);

	BOOST_CHECK_MESSAGE(is_equal(std::string("AB\x90\x00\x90\xFF\x90\x00""D", 9)),
		"Escaping doubled RLE event char in Monster Bash RLE-encoded data failed");
}

BOOST_AUTO_TEST_SUITE_END()
