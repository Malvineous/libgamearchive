/**
 * @file   test-filter.cpp
 * @brief  Generic test code for filters.
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

#include <memory>
#include <camoto/util.hpp> // std::make_unique
#include "test-filter.hpp"

test_filter::test_filter()
	:	in(std::make_unique<stream::string>())
{
}

boost::test_tools::predicate_result test_filter::is_equal(const std::string& strExpected)
{
	auto in_filt = std::make_unique<stream::input_filtered>(
		std::move(this->in),
		std::move(this->filter)
	);

	stream::string out;
	stream::copy(out, *in_filt);

	// See if the stringstream now matches what we expected
	return this->test_main::is_equal(strExpected, out.data);
}

void test_filter::test_equal_read(FilterType *ft, const std::string& strInput,
	const std::string& strExpected)
{
	{
	auto ss = std::make_unique<stream::string>(strInput);
	auto filteredStream = ft->apply(
		std::unique_ptr<stream::inout>(std::move(ss)),
		stream::fn_notify_prefiltered_size()
	);

	stream::string out;
	stream::copy(out, *filteredStream);
	out.flush();

	// See if the stringstream now matches what we expected
	BOOST_CHECK_MESSAGE(this->test_main::is_equal(strExpected, out.data),
		"Reading data through input/output filter failed");
	}
	{
	auto ss = std::make_unique<stream::input_string>(strInput);
	auto filteredStream = ft->apply(
		std::unique_ptr<stream::input>(std::move(ss))
	);

	stream::string out;
	stream::copy(out, *filteredStream);
	out.flush();

	// See if the stringstream now matches what we expected
	BOOST_CHECK_MESSAGE(this->test_main::is_equal(strExpected, out.data),
		"Reading data through input-only filter failed");
	}
	return;
}

void test_filter::test_equal_write(FilterType *ft, const std::string& strInput,
	const std::string& strExpected)
{
	{
	auto ss = std::make_unique<stream::string>();
	const std::string *indata = &ss->data;
	auto filteredStream = ft->apply(
		std::unique_ptr<stream::inout>(std::move(ss)),
		stream::fn_notify_prefiltered_size()
	);

	*filteredStream << strInput;
	filteredStream->truncate_here();
	filteredStream->flush();

	// See if the stringstream now matches what we expected
	BOOST_CHECK_MESSAGE(this->test_main::is_equal(strExpected, *indata),
		"Writing data through input/output filter failed");
	}
	{
	auto ss = std::make_unique<stream::output_string>();
	const std::string *indata = &ss->data;
	auto filteredStream = ft->apply(
		std::unique_ptr<stream::output>(std::move(ss)),
		stream::fn_notify_prefiltered_size()
	);

	*filteredStream << strInput;
	filteredStream->truncate_here();
	filteredStream->flush();

	// See if the stringstream now matches what we expected
	BOOST_CHECK_MESSAGE(this->test_main::is_equal(strExpected, *indata),
		"Writing data through output-only filter failed");
	}
	return;
}

boost::test_tools::predicate_result test_filter::should_fail()
{
	try {
		auto in_filt = std::make_shared<stream::input_filtered>(
			std::move(this->in),
			std::move(this->filter)
		);
		stream::string out;
		stream::copy(out, *in_filt);
	} catch (filter_error) {
		// If we made it this far all is good
		return boost::test_tools::predicate_result(true);
	} catch (...) {
		BOOST_TEST_CHECKPOINT("Got exception but wasn't filter_error");
		return boost::test_tools::predicate_result(false);
	}
	BOOST_TEST_CHECKPOINT("filter_error exception was not raised");
	return boost::test_tools::predicate_result(false);
}
