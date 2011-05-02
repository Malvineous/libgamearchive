/**
 * @file   tests.cpp
 * @brief  Test code core.
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

#define BOOST_TEST_MODULE libgamearchive
#ifndef __WIN32__
// Dynamically link to the Boost library on non-Windows platforms.
#define BOOST_TEST_DYN_LINK
#endif
#include <boost/test/unit_test.hpp>
#include <boost/algorithm/string.hpp> // for case-insensitive string compare
#include <boost/bind.hpp>
#include <iostream>
#include <iomanip>

#include <camoto/gamearchive/manager.hpp>
#include <camoto/debug.hpp> // for ANSI colours
#include <camoto/util.hpp>  // createString
#include "tests.hpp"

void default_sample::printNice(boost::test_tools::predicate_result& res,
	const std::string& s, const std::string& diff)
{
	const char *c = CLR_YELLOW;
	res.message() << c;
	for (int i = 0; i < s.length(); i++) {
		if ((i > 0) && (i % 16 == 0)) {
			res.message() << CLR_NORM << "\n" << std::setfill('0') << std::setw(3)
				<< std::hex << i << ": " << c;
		}
		if ((i >= diff.length()) || (s[i] != diff[i])) {
			if (c != CLR_MAG) {
				c = CLR_MAG;
				res.message() << CLR_MAG;
			}
		} else {
			if (c != CLR_YELLOW) {
				c = CLR_YELLOW;
				res.message() << CLR_YELLOW;
			}
		}
		if ((s[i] < 32) || (s[i] == 127)) {
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
	res.message() << "\nExp: ";
	this->printNice(res, strExpected, strResult);
	res.message() << CLR_NORM "\n" << "Got: ";
	this->printNice(res, strResult, strExpected);
	res.message() << CLR_NORM;

	return;
}

boost::test_tools::predicate_result default_sample::is_equal(
	const std::string& strExpected, const std::string& strCheck)
{
	if (strExpected.compare(strCheck)) {
		boost::test_tools::predicate_result res(false);
		this->print_wrong(res, strExpected, strCheck);
		return res;
	}

	return true;
}

camoto::iostream_sptr applyFilter(camoto::gamearchive::ArchivePtr arch,
	camoto::gamearchive::Archive::EntryPtr id, camoto::iostream_sptr pStream)
	throw (std::ios::failure)
{
	if (!id->filter.empty()) {
		// The file needs to be filtered first
		camoto::gamearchive::ManagerPtr pManager(camoto::gamearchive::getManager());
		camoto::gamearchive::FilterTypePtr pFilterType(
			pManager->getFilterTypeByCode(id->filter));
		if (!pFilterType) {
			throw std::ios::failure(createString(
				"could not find filter \"" << id->filter << "\""
			));
		}

		// Bind the archive's resize() function to the truncate function.
		camoto::FN_TRUNCATE fnTruncate = boost::bind<void>(
			&camoto::gamearchive::Archive::resize, arch, id, _1, _1);
		return pFilterType->apply(pStream, fnTruncate);
	}
	return pStream; // no filters to apply
}
