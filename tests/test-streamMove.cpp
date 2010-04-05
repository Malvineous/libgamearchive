/*
 * test-streamMove.cpp - test code for streamMove() function.
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
#include <iostream>

#include "tests.hpp"

// Local headers that will not be installed
#include "../src/iostream_helpers.hpp"

namespace ga = camoto::gamearchive;

struct streamMove_sample: public default_sample {

	std::stringstream data;

	streamMove_sample()
	{
		this->data << "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	}

	boost::test_tools::predicate_result is_equal(const char *cExpected)
	{
		std::string strExpected = cExpected;
		return this->default_sample::is_equal(strExpected, data.str());
	}

};

BOOST_FIXTURE_TEST_SUITE(streamMove_suite, streamMove_sample)

BOOST_AUTO_TEST_CASE(streamMove_fwd)
{
	BOOST_TEST_MESSAGE("Stream move forward");

	ga::streamMove(data, 5, 15, 5);

	BOOST_CHECK_MESSAGE(is_equal("ABCDEFGHIJKLMNOFGHIJUVWXYZ"),
		"Error in stream move forward");
}

BOOST_AUTO_TEST_CASE(streamMove_bk)
{
	BOOST_TEST_MESSAGE("Stream move backward");

	ga::streamMove(data, 15, 5, 5);

	BOOST_CHECK_MESSAGE(is_equal("ABCDEPQRSTKLMNOPQRSTUVWXYZ"),
		"Error in stream move backward");
}

BOOST_AUTO_TEST_CASE(streamMove_fwd_borderline)
{
	BOOST_TEST_MESSAGE("Stream move forward (blocks touching)");

	ga::streamMove(data, 5, 10, 5);

	BOOST_CHECK_MESSAGE(is_equal("ABCDEFGHIJFGHIJPQRSTUVWXYZ"),
		"Error in stream move forward (blocks touching)");
}

BOOST_AUTO_TEST_CASE(streamMove_bk_borderline)
{
	BOOST_TEST_MESSAGE("Stream move backward (blocks touching)");

	ga::streamMove(data, 10, 5, 5);

	BOOST_CHECK_MESSAGE(is_equal("ABCDEKLMNOKLMNOPQRSTUVWXYZ"),
		"Error in stream move backward (blocks touching)");
}

BOOST_AUTO_TEST_CASE(streamMove_fwd_overlap)
{
	BOOST_TEST_MESSAGE("Overlapping stream move forward (process from end to start)");

	ga::streamMove(data, 10, 15, 10);

	BOOST_CHECK_MESSAGE(is_equal("ABCDEFGHIJKLMNOKLMNOPQRSTZ"),
		"Error in overlapping stream move forward (process from end to start)");
}

BOOST_AUTO_TEST_CASE(streamMove_back_overlap)
{
	BOOST_TEST_MESSAGE("Overlapping stream move backward (process from start to end)");

	ga::streamMove(data, 10, 5, 10);

	BOOST_CHECK_MESSAGE(is_equal("ABCDEKLMNOPQRSTPQRSTUVWXYZ"),
		"Error in overlapping stream move backward (process from start to end)");
}

BOOST_AUTO_TEST_CASE(streamMove_fw2)
{
	BOOST_TEST_MESSAGE("Overlapping stream double-move forwards");

	ga::streamMove(data, 5, 10, 15);

	BOOST_CHECK_MESSAGE(is_equal("ABCDEFGHIJFGHIJKLMNOPQRSTZ"),
		"Error in overlapping stream double-move forwards");
}

BOOST_AUTO_TEST_CASE(streamMove_bk2)
{
	BOOST_TEST_MESSAGE("Overlapping stream double-move backwards");

	ga::streamMove(data, 10, 5, 5);
	ga::streamMove(data, 20, 10, 4);

	BOOST_CHECK_MESSAGE(is_equal("ABCDEKLMNOUVWXOPQRSTUVWXYZ"),
		"Error in overlapping stream double-move backwards");
}

BOOST_AUTO_TEST_CASE(streamMove_extend)
{
	BOOST_TEST_MESSAGE("Stream move past EOF");

	ga::streamMove(data, 5, 20, 10);

	BOOST_CHECK_MESSAGE(is_equal("ABCDEFGHIJKLMNOPQRSTFGHIJKLMNO"),
		"Error in stream move past EOF");
}

BOOST_AUTO_TEST_CASE(streamMove_extend_overlap)
{
	BOOST_TEST_MESSAGE("Overlapping stream move past EOF");

	ga::streamMove(data, 5, 15, 20);

	BOOST_CHECK_MESSAGE(is_equal("ABCDEFGHIJKLMNOFGHIJKLMNOPQRSTUVWXY"),
		"Error in overlapping stream move past EOF");
}

BOOST_AUTO_TEST_SUITE_END() // streamMove_suite
