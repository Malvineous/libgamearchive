/*
 * tests.cpp - test code core.
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

#define BOOST_TEST_MODULE libgamearchive
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <boost/algorithm/string.hpp> // for case-insensitive string compare
#include <iostream>
#include <iomanip>

#include "../src/debug.hpp"
#include "tests.hpp"

void default_sample::printNice(boost::test_tools::predicate_result& res, const std::string& s)
{
	for (int i = 0; i < s.length(); i++) {
		if (s[i] < 32) {
			res.message() << "\\x" << std::setfill('0') << std::setw(2)
				<< std::hex << (int)((uint8_t)s[i]);
		} else {
			res.message() << s[i];
		}
	}
	return;
}

void default_sample::print_wrong(boost::test_tools::predicate_result& res,
	const std::string& strExpected, const std::string& strResult)
{
	res.message() << "\nExp: " CLR_YELLOW;
	this->printNice(res, strExpected);
	res.message() << CLR_NORM "\n" << "Got: " CLR_YELLOW;
	this->printNice(res, strResult);
	res.message() << CLR_NORM;

	return;
}

boost::test_tools::predicate_result default_sample::is_equal(
	const std::string& strExpected, const std::string& strCheck)
{
	std::string strResult = strCheck.substr(0, strCheck.find_last_not_of('\0') + 1); /* TEMP: trim off trailing nulls */
	if (strResult.length() < strExpected.length()) strResult = strCheck.substr(0, strExpected.length()); // don't trim off too much in the case of trailing nulls

	if (strExpected.compare(strResult)) {
		boost::test_tools::predicate_result res(false);
		this->print_wrong(res, strExpected, strResult);
		return res;
	}

	return true;
}
