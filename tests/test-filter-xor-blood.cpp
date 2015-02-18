/**
 * @file   test-filter-xor-blood.cpp
 * @brief  Test code for Blood XOR encryption algorithm.
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
#include <camoto/stream_filtered.hpp>
#include "test-filter.hpp"
#include "../src/filter-xor-blood.hpp"

using namespace camoto;
using namespace camoto::gamearchive;

struct rff_decrypt_sample: public test_filter {
	rff_decrypt_sample()
	{
		this->filter.reset(new filter_rff_crypt(0, 0));
	}
};

BOOST_FIXTURE_TEST_SUITE(rff_decrypt_suite, rff_decrypt_sample)

BOOST_AUTO_TEST_CASE(rff_crypt_read)
{
	BOOST_TEST_MESSAGE("Decode some XOR-encoded data");

	*this->in << STRING_WITH_NULLS("\x00\x01\x02\x03\xFF\xFF\xFF\xFF");

	this->filter.reset(new filter_rff_crypt(0, 0));

	BOOST_CHECK_MESSAGE(is_equal(STRING_WITH_NULLS("\x00\x01\x03\x02\xFD\xFD\xFC\xFC")),
		"Decoding XOR-encoded data failed");
}

BOOST_AUTO_TEST_CASE(rff_crypt_partial_read)
{
	BOOST_TEST_MESSAGE("Decode some partially XOR-encoded data");

	*this->in << STRING_WITH_NULLS("\x00\x01\x02\x03\xFF\xFF\xFF\xFF");

	this->filter.reset(new filter_rff_crypt(4, 0));

	BOOST_CHECK_MESSAGE(is_equal(STRING_WITH_NULLS("\x00\x01\x03\x02\xFF\xFF\xFF\xFF")),
		"Decoding partially XOR-encoded data failed");
}

BOOST_AUTO_TEST_CASE(rff_crypt_altseed_read)
{
	BOOST_TEST_MESSAGE("Decode some XOR-encoded data with alternate seed");

	*this->in << STRING_WITH_NULLS("\x00\x01\x02\x03\xFF\xFF\xFF\xFF");

	this->filter.reset(new filter_rff_crypt(0, 0xFE));

	BOOST_CHECK_MESSAGE(is_equal(STRING_WITH_NULLS("\xFE\xFF\xFD\xFC\xFF\xFF\xFE\xFE")),
		"Decoding XOR-encoded data with alternate seed failed");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_FIXTURE_TEST_SUITE(rff_encrypt_suite, test_main)

BOOST_AUTO_TEST_CASE(rff_crypt_write_filteredstream)
{
	BOOST_TEST_MESSAGE("Encode some data through filteredstream");

	auto out = std::make_shared<stream::string>();

	auto f = std::make_shared<stream::filtered>(
		out,
		std::make_shared<filter_rff_crypt>(0, 0),
		std::make_shared<filter_rff_crypt>(0, 0),
		stream::fn_notify_prefiltered_size()
	);

	f->write("\x00\x01\x02\x03\xFF\xFF\xFF\xFF", 8);
	f->flush();

	BOOST_CHECK_MESSAGE(is_equal(
		STRING_WITH_NULLS("\x00\x01\x03\x02\xFD\xFD\xFC\xFC"),
		out->data),
		"Encoding data through filteredstream failed");
}

BOOST_AUTO_TEST_SUITE_END()
