/**
 * @file   test-filter.hpp
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

#ifndef _CAMOTO_GAMEARCHIVE_TEST_FILTER_HPP_
#define _CAMOTO_GAMEARCHIVE_TEST_FILTER_HPP_

#include <map>
#include <functional>
#include <boost/test/unit_test.hpp>
#include <camoto/stream_string.hpp>
#include <camoto/gamearchive.hpp>
#include "tests.hpp"

// This header will only be used by test implementations.
using namespace camoto;
using namespace camoto::gamearchive;

class test_filter: public test_main
{
	public:
		/// Constructor sets some default values.
		test_filter();

		/// Add all the standard tests.
		/**
		 * This can be overridden by descendent classes to add more tests for
		 * particular file formats.  If this is done, remember to call this
		 * function from the overridden one or the standard tests won't get run.
		 */
		virtual void addTests();

		/// Reset content back to a known state.
		virtual void prepareTest();

	protected:
		/// Add a test to the suite.  Used by ADD_IMAGE_TEST().
		void addBoundTest(std::function<void()> fnTest,
			boost::unit_test::const_string file, std::size_t line,
			boost::unit_test::const_string name);

		/// Reset the image to the initial state and run the given test.
		/**
		 * @param empty
		 *   true resets to an empty image (via ImageType::create()) while
		 *   false resets to initialstate() and calls ImageType::open().
		 *
		 * @param fnTest
		 *   Function to call once image is back to initial state.
		 */
		void runTest(std::function<void()> fnTest);

		/// Add an invalidContent check to run later.
		/**
		 * These checks make sure files that are in the correct format
		 * don't cause segfaults or infinite loops if the data is corrupted.
		 *
		 * @param content
		 *   Content to pass as an archive to ImageType::isInstance() where
		 *   it will be reported as a valid instance, then passed to
		 *   ImageType::open(), where an exception should be thrown.
		 */
		void invalidContent(const std::string& content);

		/// Add another read/write content filtering test.
		/**
		 * @param name
		 *   Name to identify the test in error messages.
		 *
		 * @param filtered
		 *   Filtered content (e.g. compressed, encrypted).
		 *
		 * @param plain
		 *   Unfiltered content (e.g. uncompressed, plaintext).
		 */
		void content(const std::string& name, const std::string& filtered,
			const std::string& plain, stream::len prefilteredSize);

	protected:
		/// Perform an invalidContent check now.
		void test_invalidContent(const std::string& content,
			unsigned int testNumber);

		/// Perform a content check now, reading the data through a stream::input.
		void test_content_read_in(const std::string& filtered,
			const std::string& plain);

		/// Perform a content check now, reading the data through a stream::inout.
		void test_content_read_inout(const std::string& filtered,
			const std::string& plain);

		/// Perform a content check now, writing the data through a stream::output.
		void test_content_write_out(const std::string& filtered,
			const std::string& plain, stream::len prefilteredSize);

		/// Perform a content check now, writing the data through a stream::inout.
		void test_content_write_inout(const std::string& filtered,
			const std::string& plain, stream::len prefilteredSize);

		/// Factory class used to open images in this format.
		FilterManager::handler_t pFilterType;

	private:
		/// Have we allocated pFilterType yet?
		bool init;

		/// Number of invalidData tests, used to number them sequentially.
		unsigned int numInvalidContentTests;

	public:
		/// File type code for this format.
		std::string type;
};

/// Add a test_image member function to the test suite
#define ADD_FILTER_TEST(fn) \
	this->test_filter::addBoundTest( \
		std::bind(fn, this), \
		__FILE__, __LINE__, \
		BOOST_TEST_STRINGIZE(fn) \
	);

#endif // _CAMOTO_GAMEARCHIVE_TEST_FILTER_HPP_
