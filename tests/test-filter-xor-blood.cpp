/**
 * @file   test-filter-rff.cpp
 * @brief  Test code for generic XOR encryption algorithm.
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
#include <camoto/stream_filtered.hpp>

#include "tests.hpp"
#include "test-filter.hpp"
#include "../src/filter-xor-blood.hpp"

using namespace camoto;
using namespace camoto::gamearchive;

struct rff_decrypt_sample: public filter_sample {
	rff_decrypt_sample()
	{
		this->filter.reset(new filter_rff_crypt(0, 0));
	}
};

BOOST_FIXTURE_TEST_SUITE(rff_decrypt_suite, rff_decrypt_sample)

BOOST_AUTO_TEST_CASE(rff_crypt_read)
{
	BOOST_TEST_MESSAGE("Decode some XOR-encoded data");

	in << makeString("\x00\x01\x02\x03\xFF\xFF\xFF\xFF");

	this->filter.reset(new filter_rff_crypt(0, 0));

	BOOST_CHECK_MESSAGE(is_equal(makeString("\x00\x01\x03\x02\xFD\xFD\xFC\xFC")),
		"Decoding XOR-encoded data failed");
}

BOOST_AUTO_TEST_CASE(rff_crypt_partial_read)
{
	BOOST_TEST_MESSAGE("Decode some partially XOR-encoded data");

	in << makeString("\x00\x01\x02\x03\xFF\xFF\xFF\xFF");

	this->filter.reset(new filter_rff_crypt(4, 0));

	BOOST_CHECK_MESSAGE(is_equal(makeString("\x00\x01\x03\x02\xFF\xFF\xFF\xFF")),
		"Decoding partially XOR-encoded data failed");
}

BOOST_AUTO_TEST_CASE(rff_crypt_altseed_read)
{
	BOOST_TEST_MESSAGE("Decode some XOR-encoded data with alternate seed");

	in << makeString("\x00\x01\x02\x03\xFF\xFF\xFF\xFF");

	this->filter.reset(new filter_rff_crypt(0, 0xFE));

	BOOST_CHECK_MESSAGE(is_equal(makeString("\xFE\xFF\xFD\xFC\xFF\xFF\xFE\xFE")),
		"Decoding XOR-encoded data with alternate seed failed");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_FIXTURE_TEST_SUITE(rff_encrypt_suite, default_sample)

BOOST_AUTO_TEST_CASE(rff_crypt_write_filteredstream)
{
	BOOST_TEST_MESSAGE("Encode some data through filteredstream");

	stream::string_sptr out(new stream::string());

	filter_sptr in_filt(new filter_rff_crypt(0, 0));
	filter_sptr out_filt(new filter_rff_crypt(0, 0));

	stream::filtered_sptr f(new stream::filtered());
	f->open(out, in_filt, out_filt, NULL);

	f->write("\x00\x01\x02\x03\xFF\xFF\xFF\xFF", 8);
	f->flush();

	BOOST_CHECK_MESSAGE(is_equal(makeString("\x00\x01\x03\x02\xFD\xFD\xFC\xFC"),
		out->str()),
		"Encoding data through filteredstream failed");
}

BOOST_AUTO_TEST_SUITE_END()
