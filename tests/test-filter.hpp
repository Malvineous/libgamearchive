/**
 * @file   test-filter.hpp
 * @brief  Generic test code for filters.
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

#include <camoto/stream_string.hpp>
#include <camoto/stream_filtered.hpp>
#include "tests.hpp"

struct filter_sample: public default_sample {

	camoto::stream::string_sptr in;
	camoto::stream::input_filtered_sptr in_filt;
	camoto::filter_sptr filter;

	filter_sample();
	boost::test_tools::predicate_result is_equal(const std::string& strExpected);
	boost::test_tools::predicate_result should_fail();

};
