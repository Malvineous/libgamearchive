/**
 * @file   test-filter-bash-rle.cpp
 * @brief  Test code for Monster Bash RLE packer/unpacker.
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
#include "../src/filter-bash-rle.hpp"
#include "test-filter.hpp"

using namespace camoto;
using namespace camoto::gamearchive;

struct bash_rle_sample: public test_filter {
	bash_rle_sample()
	{
		this->filter.reset(new filter_bash_rle());
	}
};

struct bash_unrle_sample: public test_filter {
	bash_unrle_sample()
	{
		this->filter.reset(new filter_bash_unrle());
	}
};

BOOST_FIXTURE_TEST_SUITE(bash_unrle_suite, bash_unrle_sample)

BOOST_AUTO_TEST_CASE(bash_unrle_read)
{
	BOOST_TEST_MESSAGE("Decode some Monster Bash RLE-encoded data");

	*this->in << "ABC\x90\x05""D";

	BOOST_CHECK_MESSAGE(is_equal("ABCCCCCD"),
		"Decoding Monster Bash RLE-encoded data failed");
}

BOOST_AUTO_TEST_CASE(bash_unrle_escape)
{
	BOOST_TEST_MESSAGE("Decode RLE-escape in Monster Bash RLE-encoded data");

	this->in->write("ABC\x90\x00""D", 6);

	BOOST_CHECK_MESSAGE(is_equal("ABC\x90""D"),
		"Decoding RLE-escape in Monster Bash RLE-encoded data failed");
}

BOOST_AUTO_TEST_CASE(bash_unrle_truncated_read)
{
	BOOST_TEST_MESSAGE("Read truncated RLE-escape in Monster Bash RLE-encoded data");

	*this->in << "ABC\x90";

	BOOST_CHECK_MESSAGE(should_fail(),
		"Read truncated RLE-escape in Monster Bash RLE-encoded data incorrectly succeeded");
}

BOOST_AUTO_TEST_CASE(bash_unrle_read_lots1)
{
	BOOST_TEST_MESSAGE("RLE decode > 256 bytes (one leftover) in Monster Bash RLE-encoded data");

	*this->in << "ABC\x90\xFF""C";

	BOOST_CHECK_MESSAGE(is_equal(createString("AB" << std::string(1+254+1, 'C'))),
		"RLE decode > 256 bytes (one leftover) in Monster Bash RLE-encoded data failed");
}

BOOST_AUTO_TEST_CASE(bash_unrle_read_lots2)
{
	BOOST_TEST_MESSAGE("RLE decode > 256 bytes (two leftovers) in Monster Bash RLE-encoded data");

	*this->in << "ABC\x90\xFF""CC";

	BOOST_CHECK_MESSAGE(is_equal(createString("AB" << std::string(1+254+2, 'C'))),
		"RLE decode > 256 bytes (two leftovers) in Monster Bash RLE-encoded data failed");
}

BOOST_AUTO_TEST_CASE(bash_unrle_read_lots3)
{
	BOOST_TEST_MESSAGE("RLE decode > 256 bytes (three leftovers) in Monster Bash RLE-encoded data");

	*this->in << "ABC\x90\xFF\x90\x04";

	BOOST_CHECK_MESSAGE(is_equal(createString("AB" << std::string(1+254+3, 'C'))),
		"RLE decode > 256 bytes (three leftovers) in Monster Bash RLE-encoded data failed");
}

BOOST_AUTO_TEST_CASE(bash_unrle_read_lots4)
{
	BOOST_TEST_MESSAGE("RLE decode > 256 bytes (four leftovers) in Monster Bash RLE-encoded data");

	*this->in << "ABC\x90\xFF\x90\x05";

	BOOST_CHECK_MESSAGE(is_equal(createString("AB" << std::string(1+254+4, 'C'))),
		"RLE decode > 256 bytes (four leftovers) in Monster Bash RLE-encoded data failed");
}

BOOST_AUTO_TEST_CASE(bash_unrle_read_3lots)
{
	BOOST_TEST_MESSAGE("RLE decode > 512 bytes in Monster Bash RLE-encoded data");

	*this->in << "AB\x90\x05""CB\x90\xFF\x90\xFF\x90\x92""E";

	BOOST_CHECK_MESSAGE(is_equal(createString(
		"A" << std::string(5, 'B') << "C" << std::string(1+254+254+0x91, 'B') << "E"
		)),
		"RLE decode > 256 bytes (four leftovers) in Monster Bash RLE-encoded data failed");
}

BOOST_AUTO_TEST_CASE(bash_unrle_long_escape)
{
	BOOST_TEST_MESSAGE("Unescaping many RLE event chars in Monster Bash RLE-encoded data");

	// Would come out larger post-RLE, so don't bother
	*this->in << STRING_WITH_NULLS("AB\x90\x00\x90\xFF\x90\x00""D");

	BOOST_CHECK_MESSAGE(is_equal(createString("AB" << std::string(1+254+1, '\x90') << "D")),
		"Unescaping doubled RLE event char in Monster Bash RLE-encoded data failed");
}

BOOST_AUTO_TEST_SUITE_END()


BOOST_FIXTURE_TEST_SUITE(bash_rle_suite, bash_rle_sample)

BOOST_AUTO_TEST_CASE(bash_rle_read)
{
	BOOST_TEST_MESSAGE("Decode some Monster Bash RLE-encoded data");

	*this->in << "ABCCCCCD";

	BOOST_CHECK_MESSAGE(is_equal("ABC\x90\x05""D"),
		"Decoding Monster Bash RLE-encoded data failed");
}

BOOST_AUTO_TEST_CASE(bash_rle_escape)
{
	BOOST_TEST_MESSAGE("Encode RLE-escape in Monster Bash RLE-encoded data");

	*this->in << "ABC\x90""D";

	BOOST_CHECK_MESSAGE(is_equal(STRING_WITH_NULLS("ABC\x90\x00""D")),
		"Encoding RLE-escape in Monster Bash RLE-encoded data failed");
}

BOOST_AUTO_TEST_CASE(bash_rle_repeat_escape)
{
	BOOST_TEST_MESSAGE("RLE-encode the RLE event byte in Monster Bash RLE-encoded data");

	*this->in << "ABC\x90\x90\x90\x90\x90""D";

	BOOST_CHECK_MESSAGE(is_equal(std::string("ABC\x90\x00\x90\x05""D", 8)),
		"RLE-encode the RLE event byte in Monster Bash RLE-encoded data failed");
}

BOOST_AUTO_TEST_CASE(bash_rle_read_trailing)
{
	BOOST_TEST_MESSAGE("Write ending with RLE event in Monster Bash RLE-encoded data");

	*this->in << "ABCCCCCC";

	BOOST_CHECK_MESSAGE(is_equal("ABC\x90\x06"),
		"Write ending with RLE event in Monster Bash RLE-encoded data failed");
}

BOOST_AUTO_TEST_CASE(bash_rle_escape_trailing)
{
	BOOST_TEST_MESSAGE("Write ending with RLE char in Monster Bash RLE-encoded data");

	*this->in << "ABC\x90";

	BOOST_CHECK_MESSAGE(is_equal(STRING_WITH_NULLS("ABC\x90\x00")),
		"Write ending with RLE char in Monster Bash RLE-encoded data failed");
}

BOOST_AUTO_TEST_CASE(bash_rle_read_lots1)
{
	BOOST_TEST_MESSAGE("RLE encode > 256 bytes (one leftover) in Monster Bash RLE-encoded data");

	*this->in << "AB" << std::string(1+254+1, 'C');

	BOOST_CHECK_MESSAGE(is_equal("ABC\x90\xFF""C"),
		"RLE encode > 256 bytes (one leftover) in Monster Bash RLE-encoded data failed");
}

BOOST_AUTO_TEST_CASE(bash_rle_read_lots2)
{
	BOOST_TEST_MESSAGE("RLE encode > 256 bytes (two leftovers) in Monster Bash RLE-encoded data");

	*this->in << "AB" << std::string(1+254+2, 'C');

	BOOST_CHECK_MESSAGE(is_equal("ABC\x90\xFF""CC"),
		"RLE encode > 256 bytes (two leftovers) in Monster Bash RLE-encoded data failed");
}

BOOST_AUTO_TEST_CASE(bash_rle_read_lots3)
{
	BOOST_TEST_MESSAGE("RLE encode > 256 bytes (three leftovers) in Monster Bash RLE-encoded data");

	*this->in << "AB" << std::string(1+254+3, 'C');

	BOOST_CHECK_MESSAGE(is_equal("ABC\x90\xFF\x90\x04"),
		"RLE encode > 256 bytes (three leftovers) in Monster Bash RLE-encoded data failed");
}

BOOST_AUTO_TEST_CASE(bash_rle_read_lots4)
{
	BOOST_TEST_MESSAGE("RLE encode > 256 bytes (four leftovers) in Monster Bash RLE-encoded data");

	*this->in << "AB" << std::string(1+254+4, 'C');

	BOOST_CHECK_MESSAGE(is_equal("ABC\x90\xFF\x90\x05"),
		"RLE encode > 256 bytes (four leftovers) in Monster Bash RLE-encoded data failed");
}

BOOST_AUTO_TEST_CASE(bash_rle_read_3lots)
{
	BOOST_TEST_MESSAGE("RLE encode > 512 bytes in Monster Bash RLE-encoded data");

	*this->in << "A" << std::string(5, 'B') << "C"
		<< std::string(255+254+0x91, 'B') << "E";

	BOOST_CHECK_MESSAGE(is_equal("AB\x90\x05""CB\x90\xFF\x90\xFF\x90\x92""E"),
		"RLE encode > 256 bytes (four leftovers) in Monster Bash RLE-encoded data failed");
}

BOOST_AUTO_TEST_CASE(bash_rle_short2)
{
	BOOST_TEST_MESSAGE("RLE event skipping with doubled data in Monster Bash RLE-encoded data");

	// Would come out larger post-RLE, so don't bother
	*this->in << "ABCCD";

	BOOST_CHECK_MESSAGE(is_equal("ABCCD"),
		"RLE event skipping with doubled data in Monster Bash RLE-encoded data failed");
}

BOOST_AUTO_TEST_CASE(bash_rle_short3)
{
	BOOST_TEST_MESSAGE("RLE event skipping with tripled data in Monster Bash RLE-encoded data");

	// Would come out the same size post-RLE, so don't bother
	*this->in << "ABCCCD";

	BOOST_CHECK_MESSAGE(is_equal("ABCCCD"),
		"RLE event skipping with tripled data in Monster Bash RLE-encoded data failed");
}

BOOST_AUTO_TEST_CASE(bash_rle_short_escape)
{
	BOOST_TEST_MESSAGE("Escaping doubled RLE event char in Monster Bash RLE-encoded data");

	// Would come out larger post-RLE, so don't bother
	*this->in << "AB\x90\x90""D";

	BOOST_CHECK_MESSAGE(is_equal(std::string("AB\x90\x00\x90\x00""D", 7)),
		"Escaping doubled RLE event char in Monster Bash RLE-encoded data failed");
}

BOOST_AUTO_TEST_CASE(bash_rle_long_escape)
{
	BOOST_TEST_MESSAGE("Escaping many RLE event chars in Monster Bash RLE-encoded data");

	// Would come out larger post-RLE, so don't bother
	*this->in << "AB" << std::string(1+254+1, '\x90') << "D";

	BOOST_CHECK_MESSAGE(is_equal(STRING_WITH_NULLS("AB\x90\x00\x90\xFF\x90\x00""D")),
		"Escaping doubled RLE event char in Monster Bash RLE-encoded data failed");
}

BOOST_AUTO_TEST_SUITE_END()
