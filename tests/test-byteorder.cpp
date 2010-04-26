/*
 * test-byteorder.cpp - test code for the endian conversion functions.
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

#include <iostream>
#include <sstream>
#include <boost/test/unit_test.hpp>

#define BYTEORDER_USE_IOSTREAMS
#define BYTEORDER_PROVIDE_TYPED_FUNCTIONS

#include "../src/byteorder.h"

BOOST_AUTO_TEST_SUITE(byteorder)

BOOST_AUTO_TEST_CASE(functions)
{
	uint8_t data[] = {0x01,0x23,0x45,0x67, 0x89,0xAB,0xCD,0xEF};

	BOOST_CHECK_EQUAL(le16toh(*( (uint16_t *)&data )), 0x2301);
	BOOST_CHECK_EQUAL(le32toh(*( (uint32_t *)&data )), 0x67452301);
	BOOST_CHECK_EQUAL(le64toh(*( (uint64_t *)&data )), 0xEFCDAB8967452301);

	BOOST_CHECK_EQUAL(be16toh(*( (uint16_t *)&data )), 0x123);
	BOOST_CHECK_EQUAL(be32toh(*( (uint32_t *)&data )), 0x1234567);
	BOOST_CHECK_EQUAL(be64toh(*( (uint64_t *)&data )), 0x123456789ABCDEF);

	BOOST_CHECK_EQUAL(htole16(0x2301),             *( (uint16_t *)&data ));
	BOOST_CHECK_EQUAL(htole32(0x67452301),         *( (uint32_t *)&data ));
	BOOST_CHECK_EQUAL(htole64(0xEFCDAB8967452301), *( (uint64_t *)&data ));

	BOOST_CHECK_EQUAL(htobe16(0x123),              *( (uint16_t *)&data ));
	BOOST_CHECK_EQUAL(htobe32(0x1234567),          *( (uint32_t *)&data ));
	BOOST_CHECK_EQUAL(htobe64(0x123456789ABCDEF),  *( (uint64_t *)&data ));
}

BOOST_AUTO_TEST_CASE(stream_write)
{
	{
		std::stringstream data;
		data << u16le(0x0123);
		BOOST_CHECK_EQUAL((uint8_t)data.str().at(0), 0x23);
		BOOST_CHECK_EQUAL((uint8_t)data.str().at(1), 0x01);
	}
	{
		std::stringstream data;
		data << u32le(0x01234567);
		BOOST_CHECK_EQUAL((uint8_t)data.str().at(0), 0x67);
		BOOST_CHECK_EQUAL((uint8_t)data.str().at(1), 0x45);
		BOOST_CHECK_EQUAL((uint8_t)data.str().at(2), 0x23);
		BOOST_CHECK_EQUAL((uint8_t)data.str().at(3), 0x01);
	}
	{
		std::stringstream data;
		data << u64le(0x0123456789ABCDEF);
		BOOST_CHECK_EQUAL((uint8_t)data.str().at(0), 0xEF);
		BOOST_CHECK_EQUAL((uint8_t)data.str().at(1), 0xCD);
		BOOST_CHECK_EQUAL((uint8_t)data.str().at(2), 0xAB);
		BOOST_CHECK_EQUAL((uint8_t)data.str().at(3), 0x89);
		BOOST_CHECK_EQUAL((uint8_t)data.str().at(4), 0x67);
		BOOST_CHECK_EQUAL((uint8_t)data.str().at(5), 0x45);
		BOOST_CHECK_EQUAL((uint8_t)data.str().at(6), 0x23);
		BOOST_CHECK_EQUAL((uint8_t)data.str().at(7), 0x01);
	}

	{
		std::stringstream data;
		data << u16be(0x0123);
		BOOST_CHECK_EQUAL((uint8_t)data.str().at(0), 0x01);
		BOOST_CHECK_EQUAL((uint8_t)data.str().at(1), 0x23);
	}
	{
		std::stringstream data;
		data << u32be(0x01234567);
		BOOST_CHECK_EQUAL((uint8_t)data.str().at(0), 0x01);
		BOOST_CHECK_EQUAL((uint8_t)data.str().at(1), 0x23);
		BOOST_CHECK_EQUAL((uint8_t)data.str().at(2), 0x45);
		BOOST_CHECK_EQUAL((uint8_t)data.str().at(3), 0x67);
	}
	{
		std::stringstream data;
		data << u64be(0x0123456789ABCDEF);
		BOOST_CHECK_EQUAL((uint8_t)data.str().at(0), 0x01);
		BOOST_CHECK_EQUAL((uint8_t)data.str().at(1), 0x23);
		BOOST_CHECK_EQUAL((uint8_t)data.str().at(2), 0x45);
		BOOST_CHECK_EQUAL((uint8_t)data.str().at(3), 0x67);
		BOOST_CHECK_EQUAL((uint8_t)data.str().at(4), 0x89);
		BOOST_CHECK_EQUAL((uint8_t)data.str().at(5), 0xAB);
		BOOST_CHECK_EQUAL((uint8_t)data.str().at(6), 0xCD);
		BOOST_CHECK_EQUAL((uint8_t)data.str().at(7), 0xEF);
	}

}

BOOST_AUTO_TEST_CASE(stream_read)
{
	{
		std::stringstream data;
		data << "\x01\x23\x45\x67" "\x89\xAB\xCD\xEF";
		data.seekg(0);
		uint16_t v = 0;
		data >> u16le(v);
		BOOST_CHECK_EQUAL(v, 0x2301);
	}
	{
		std::stringstream data;
		data << "\x01\x23\x45\x67" "\x89\xAB\xCD\xEF";
		data.seekg(0);
		uint32_t v = 0;
		data >> u32le(v);
		BOOST_CHECK_EQUAL(v, 0x67452301);
	}
	{
		std::stringstream data;
		data << "\x01\x23\x45\x67" "\x89\xAB\xCD\xEF";
		data.seekg(0);
		uint64_t v = 0;
		data >> u64le(v);
		BOOST_CHECK_EQUAL(v, 0xEFCDAB8967452301);
	}

	{
		std::stringstream data;
		data << "\x01\x23\x45\x67" "\x89\xAB\xCD\xEF";
		data.seekg(0);
		uint16_t v = 0;
		data >> u16be(v);
		BOOST_CHECK_EQUAL(v, 0x123);
	}
	{
		std::stringstream data;
		data << "\x01\x23\x45\x67" "\x89\xAB\xCD\xEF";
		data.seekg(0);
		uint32_t v = 0;
		data >> u32be(v);
		BOOST_CHECK_EQUAL(v, 0x1234567);
	}
	{
		std::stringstream data;
		data << "\x01\x23\x45\x67" "\x89\xAB\xCD\xEF";
		data.seekg(0);
		uint64_t v = 0;
		data >> u64be(v);
		BOOST_CHECK_EQUAL(v, 0x123456789ABCDEF);
	}
}

BOOST_AUTO_TEST_SUITE_END()
