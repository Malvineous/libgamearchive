/**
 * @file   test-filter-rff.cpp
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
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/invert.hpp>
#include <iostream>
#include <sstream>
#include <camoto/filteredstream.hpp>

#include "tests.hpp"
#include "../src/filter-xor-blood.hpp"

using namespace camoto::gamearchive;

/// Allow static objects to be passed around in boost:shared_ptr objects.
struct null_deleter
{
    void operator()(void const *) const
    {
    }
};

struct rff_crypt_sample: public default_sample {

	std::stringstream in;
	std::stringstream out;

	rff_crypt_sample()
	{
	}

	boost::test_tools::predicate_result is_equal(const std::string& strExpected)
	{
		// See if the stringstream now matches what we expected
		return this->default_sample::is_equal(strExpected, out.str());
	}

};

BOOST_FIXTURE_TEST_SUITE(rff_crypt_suite, rff_crypt_sample)

BOOST_AUTO_TEST_CASE(rff_crypt_read)
{
	BOOST_TEST_MESSAGE("Decode some XOR-encoded data");

	in << makeString("\x00\x01\x02\x03\xFF\xFF\xFF\xFF");

	io::filtering_istream inf;
	inf.push(rff_crypt_filter(0, 0));
	inf.push(in);

	boost::iostreams::copy(inf, out);

	BOOST_CHECK_MESSAGE(is_equal(makeString("\x00\x01\x03\x02\xFD\xFD\xFC\xFC")),
		"Decoding XOR-encoded data failed");
}

BOOST_AUTO_TEST_CASE(rff_crypt_partial_read)
{
	BOOST_TEST_MESSAGE("Decode some partially XOR-encoded data");

	in << makeString("\x00\x01\x02\x03\xFF\xFF\xFF\xFF");

	io::filtering_istream inf;
	inf.push(rff_crypt_filter(4, 0));
	inf.push(in);

	boost::iostreams::copy(inf, out);

	BOOST_CHECK_MESSAGE(is_equal(makeString("\x00\x01\x03\x02\xFF\xFF\xFF\xFF")),
		"Decoding partially XOR-encoded data failed");
}

BOOST_AUTO_TEST_CASE(rff_crypt_altseed_read)
{
	BOOST_TEST_MESSAGE("Decode some XOR-encoded data with alternate seed");

	in << makeString("\x00\x01\x02\x03\xFF\xFF\xFF\xFF");

	io::filtering_istream inf;
	inf.push(rff_crypt_filter(0, 0xFE));
	inf.push(in);

	boost::iostreams::copy(inf, out);

	BOOST_CHECK_MESSAGE(is_equal(makeString("\xFE\xFF\xFD\xFC\xFF\xFF\xFE\xFE")),
		"Decoding XOR-encoded data with alternate seed failed");
}

BOOST_AUTO_TEST_CASE(rff_crypt_write)
{
	BOOST_TEST_MESSAGE("Encode some data");

	boost::shared_ptr<io::filtering_ostream> poutf(new io::filtering_ostream());
	poutf->push(io::invert(rff_crypt_filter(0, 0)));

	poutf->push(out);

	poutf->write("\x00\x01\x02\x03\xFF\xFF\xFF\xFF", 8);
	poutf->flush();

	BOOST_CHECK_MESSAGE(is_equal(makeString("\x00\x01\x03\x02\xFD\xFD\xFC\xFC")),
		"Encoding data failed");
}

BOOST_AUTO_TEST_CASE(rff_crypt_write_filteredstream)
{
	BOOST_TEST_MESSAGE("Encode some data through filteredstream");

	camoto::filtered_istream_sptr pinf(new camoto::filtered_istream());
	pinf->push(rff_crypt_filter(0, 0));

	camoto::filtered_ostream_sptr poutf(new camoto::filtered_ostream());
	poutf->push(io::invert(rff_crypt_filter(0, 0)));

	camoto::iostream_sptr pout(&out, null_deleter());
	camoto::iostream_sptr dec(new camoto::filteredstream(pout, pinf, poutf));

	dec->write("\x00\x01\x02\x03\xFF\xFF\xFF\xFF", 8);
	dec->flush();

	BOOST_CHECK_MESSAGE(is_equal(makeString("\x00\x01\x03\x02\xFD\xFD\xFC\xFC")),
		"Encoding data through filteredstream failed");
}

BOOST_AUTO_TEST_SUITE_END()
