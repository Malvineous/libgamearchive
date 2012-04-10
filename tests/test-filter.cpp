/**
 * @file   test-filter.cpp
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

#include "test-filter.hpp"

using namespace camoto;

filter_sample::filter_sample()
	: in(new stream::string()),
	  in_filt(new stream::input_filtered())
{
}

boost::test_tools::predicate_result filter_sample::is_equal(const std::string& strExpected)
{
	stream::string_sptr out(new stream::string());
	this->in_filt->open(this->in, this->filter);
	stream::copy(out, this->in_filt);

	// See if the stringstream now matches what we expected
	return this->default_sample::is_equal(strExpected, out->str());
}

boost::test_tools::predicate_result filter_sample::should_fail()
{
	stream::string_sptr out(new stream::string());
	BOOST_REQUIRE_THROW(
		this->in_filt->open(this->in, this->filter);
		stream::copy(out, this->in_filt);
		, filter_error
	);

	// If we made it this far all is good
	return boost::test_tools::predicate_result(true);
}
