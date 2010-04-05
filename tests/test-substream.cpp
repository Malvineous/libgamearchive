/*
 * test-substream.cpp - test code for substream class.
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
#include "../src/substream.hpp"

namespace io = boost::iostreams;
namespace ga = camoto::gamearchive;

struct substream_sample: public default_sample {

	boost::shared_ptr<std::stringstream> psstrBase;
	void *_do; // unused var, but allows a statement to run in constructor init
	camoto::iostream_sptr psBase;
	ga::substream_sptr sub;

	substream_sample() :
		psstrBase(new std::stringstream),
		_do((*this->psstrBase) << "ABCDEFGHIJKLMNOPQRSTUVWXYZ"),
		psBase(this->psstrBase),
		sub(new ga::substream(psBase, 0, 26))
	{
		// Make sure the data went in correctly to begin the test
		BOOST_REQUIRE(this->psstrBase->str().compare("ABCDEFGHIJKLMNOPQRSTUVWXYZ") == 0);
	}

	boost::test_tools::predicate_result is_equal(const char *cExpected)
	{
		// See if the stringstream now matches what we expected
		std::string strExpected = cExpected;
		std::stringstream buf;
		boost::iostreams::copy(*sub.get(), buf);
		return this->default_sample::is_equal(strExpected, buf.str());
	}

};

BOOST_FIXTURE_TEST_SUITE(substream_suite, substream_sample)

BOOST_AUTO_TEST_CASE(substream_read)
{
	BOOST_TEST_MESSAGE("Create substream with size and offset");

	sub = ga::substream_sptr(new ga::substream(psBase, 5, 6));

	BOOST_CHECK_MESSAGE(is_equal("FGHIJK"),
		"Substream creation with size and offset failed");
}

BOOST_AUTO_TEST_CASE(substream_change_offset)
{
	BOOST_TEST_MESSAGE("Move substream's offset");

	sub->relocate(10);

	BOOST_CHECK_MESSAGE(is_equal("KLMNOPQRSTUVWXYZ"),
		"Move substream's offset failed");
}

BOOST_AUTO_TEST_SUITE_END()
