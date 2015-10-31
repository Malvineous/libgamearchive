/**
 * @file   test-filter.cpp
 * @brief  Generic test code for Filter class implementations.
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

#include <iomanip>
#include <functional>
#include <camoto/util.hpp>
#include "test-filter.hpp"

using namespace camoto;
using namespace camoto::gamearchive;

test_filter::test_filter()
	:	init(false),
		numInvalidContentTests(1)
{
}

void test_filter::addTests()
{
	return;
}

void test_filter::addBoundTest(std::function<void()> fnTest,
	boost::unit_test::const_string file, std::size_t line,
	boost::unit_test::const_string name)
{
	this->ts->add(
		boost::unit_test::make_test_case(
			std::bind(&test_filter::runTest, this, fnTest),
			name, //createString(name << '[' << this->basename << ']'),
			file, line
		)
	);
	return;
}

void test_filter::runTest(std::function<void()> fnTest)
{
	this->prepareTest();
	fnTest();
	return;
}

void test_filter::prepareTest()
{
	if (!this->type.empty()) {
		this->pFilterType = FilterManager::byCode(this->type);
		BOOST_REQUIRE_MESSAGE(this->pFilterType,
			"Could not find filter type " << this->type);
	}

	return;
}

void test_filter::invalidContent(const std::string& content)
{
	auto fnTest = std::bind(&test_filter::test_invalidContent, this, content,
		this->numInvalidContentTests);

	this->addBoundTest(
		std::bind(&test_filter::test_invalidContent, this, content,
			this->numInvalidContentTests),
		__FILE__, __LINE__,
		createString("invalidcontent_i" << std::setfill('0') << std::setw(2)
			<< this->numInvalidContentTests)
	);
/*
	this->ts->add(
		boost::unit_test::make_test_case(
			std::bind(&test_filter::runTest, this, fnTest),
			createString("test_filter[" << this->basename << "]::invalidcontent_i"
				<< std::setfill('0') << std::setw(2) << this->numInvalidContentTests),
			__FILE__, __LINE__
		)
	);
*/
	this->numInvalidContentTests++;
	return;
}

void test_filter::test_invalidContent(const std::string& content,
	unsigned int testNumber)
{
	BOOST_TEST_MESSAGE(this->basename << ": invalidContent_i"
		<< std::setfill('0') << std::setw(2) << testNumber << ")");

	auto input = this->apply_in(
		std::make_unique<stream::input_string>(content)
	);

	auto filterResult = std::make_unique<stream::string>();

	// But that we get an error when trying to open the file
	BOOST_CHECK_THROW(
		stream::copy(*filterResult, *input);
		,
		stream::error
	);

	return;
}

void test_filter::content(const std::string& name, stream::len prefilteredSize,
	const std::string& filtered, const std::string& plain)
{
	this->content_decode(name, filtered, plain);
	this->content_encode(name, prefilteredSize, filtered, plain);
	return;
}

void test_filter::content_decode(const std::string& name,
	const std::string& filtered, const std::string& plain)
{
	// Read through input filter
	this->addBoundTest(
		std::bind(&test_filter::test_content_read_in, this, filtered, plain),
		__FILE__, __LINE__,
		createString("content_read_in/" << name)
	);

	// Read through in/out filter
	this->addBoundTest(
		std::bind(&test_filter::test_content_read_inout, this, filtered, plain),
		__FILE__, __LINE__,
		createString("content_read_inout/" << name)
	);

	return;
}

void test_filter::content_encode(const std::string& name,
	stream::len prefilteredSize, const std::string& filtered,
	const std::string& plain)
{
	// Write through in/out filter
	this->addBoundTest(
		std::bind(&test_filter::test_content_write_out, this,
			filtered, plain, prefilteredSize),
		__FILE__, __LINE__,
		createString("content_write_out/" << name)
	);

	// Write through in/out filter
	this->addBoundTest(
		std::bind(&test_filter::test_content_write_inout, this,
			filtered, plain, prefilteredSize),
		__FILE__, __LINE__,
		createString("content_write_inout/" << name)
	);

	return;
}

void test_filter::test_content_read_in(const std::string& filtered,
	const std::string& plain)
{
	BOOST_TEST_MESSAGE(this->basename << ": "
		<< boost::unit_test::framework::current_test_case().p_name);

	auto input = this->apply_in(
		std::make_unique<stream::input_string>(filtered)
	);

	BOOST_TEST_CHECKPOINT("Read through input filter");
	auto filterResult = std::make_unique<stream::string>();
	stream::copy(*filterResult, *input);

	BOOST_REQUIRE_MESSAGE(
		this->is_equal(plain, filterResult->data),
		"Reading through filter produced incorrect result"
	);

	return;
}

void test_filter::test_content_read_inout(const std::string& filtered,
	const std::string& plain)
{
	BOOST_TEST_MESSAGE(this->basename << ": "
		<< boost::unit_test::framework::current_test_case().p_name);

	auto input = this->apply_inout(
		std::make_unique<stream::string>(filtered),
		nullptr
	);

	BOOST_TEST_CHECKPOINT("Read through in/out filter");
	auto filterResult = std::make_unique<stream::string>();
	stream::copy(*filterResult, *input);

	BOOST_REQUIRE_MESSAGE(
		this->is_equal(plain, filterResult->data),
		"Reading through in/out filter produced incorrect result"
	);

	return;
}

void test_filter::test_content_write_out(const std::string& filtered,
	const std::string& plain, stream::len prefilteredSize)
{
	BOOST_TEST_MESSAGE(this->basename << ": "
		<< boost::unit_test::framework::current_test_case().p_name);

	auto filterResult = std::make_unique<stream::output_string>();
	auto& filterResult_data = filterResult->data;

	stream::len setPrefiltered = 0;

	auto output = this->apply_out(std::move(filterResult), &setPrefiltered);

	BOOST_TEST_CHECKPOINT("Write through in/out filter");
	output->write(plain);
	output->flush();

	// Make sure the prefiltered size set by the filter is what we are expecting
	BOOST_CHECK_EQUAL(setPrefiltered, prefilteredSize);

	BOOST_REQUIRE_MESSAGE(
		this->is_equal(filtered, filterResult_data),
		"Writing through in/out filter produced incorrect result"
	);

	return;
}

void test_filter::test_content_write_inout(const std::string& filtered,
	const std::string& plain, stream::len prefilteredSize)
{
	BOOST_TEST_MESSAGE(this->basename << ": "
		<< boost::unit_test::framework::current_test_case().p_name);

	auto filterResult = std::make_unique<stream::string>();
	auto& filterResult_data = filterResult->data;

	stream::len setPrefiltered = 0;

	auto output = this->apply_inout(std::move(filterResult), &setPrefiltered);

	BOOST_TEST_CHECKPOINT("Write through in/out filter");
	output->write(plain);
	output->flush(); // updates setPrefiltered

	// Make sure the prefiltered size set by the filter is what we are expecting
	BOOST_CHECK_EQUAL(setPrefiltered, prefilteredSize);

	BOOST_REQUIRE_MESSAGE(
		this->is_equal(filtered, filterResult_data),
		"Writing through in/out filter produced incorrect result"
	);

	return;
}

std::unique_ptr<stream::input> test_filter::apply_in(
	std::unique_ptr<stream::input> content)
{
	BOOST_REQUIRE_MESSAGE(this->pFilterType, "Must specify type in test case, "
		"or override filter apply functions.");

	return this->pFilterType->apply(
		std::move(content)
	);
}

std::unique_ptr<stream::output> test_filter::apply_out(
	std::unique_ptr<stream::output> content, stream::len *setPrefiltered)
{
	BOOST_REQUIRE_MESSAGE(this->pFilterType, "Must specify type in test case, "
		"or override filter apply functions.");

	return this->pFilterType->apply(
		std::move(content),
		[setPrefiltered](stream::output_filtered* s, stream::len l) {
			if (setPrefiltered) *setPrefiltered = l;
		}
	);
}

std::unique_ptr<stream::inout> test_filter::apply_inout(
	std::unique_ptr<stream::inout> content, stream::len *setPrefiltered)
{
	BOOST_REQUIRE_MESSAGE(this->pFilterType, "Must specify type in test case, "
		"or override filter apply functions.");

	return this->pFilterType->apply(
		std::move(content),
		[setPrefiltered](stream::output_filtered* s, stream::len l) {
			if (setPrefiltered) *setPrefiltered = l;
		}
	);
}
