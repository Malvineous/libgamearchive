/**
 * @file   test-filter-bitswap.cpp
 * @brief  Test code for generic bitswap algorithm.
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
#include "test-filter.hpp"
#include "../src/filter-bitswap.hpp"

using namespace camoto::gamearchive;

class test_filter_bitswap: public test_filter
{
	public:
		test_filter_bitswap()
		{
		}

		void addTests()
		{
			this->test_filter::addTests();

			// Swap some data
			this->content("normal", 8, STRING_WITH_NULLS(
				"\x00\x01\x03\x0F\x1E\x55\xAA\xFF"
			), STRING_WITH_NULLS(
				"\x00\x80\xC0\xF0\x78\xAA\x55\xFF"
			));
		}

		std::unique_ptr<stream::input> apply_in(
			std::unique_ptr<stream::input> content)
		{
			return std::make_unique<stream::input_filtered>(
				std::move(content),
				std::make_unique<filter_bitswap>()
			);
		}

		std::unique_ptr<stream::output> apply_out(
			std::unique_ptr<stream::output> content, stream::len *setPrefiltered)
		{
			return std::make_unique<stream::output_filtered>(
				std::move(content),
				std::make_unique<filter_bitswap>(),
				[setPrefiltered](stream::output_filtered* s, stream::len l) {
					if (setPrefiltered) *setPrefiltered = l;
				}
			);
		}

		std::unique_ptr<stream::inout> apply_inout(
			std::unique_ptr<stream::inout> content, stream::len *setPrefiltered)
		{
			return std::make_unique<stream::filtered>(
				std::move(content),
				std::make_unique<filter_bitswap>(),
				std::make_unique<filter_bitswap>(),
				[setPrefiltered](stream::output_filtered* s, stream::len l) {
					if (setPrefiltered) *setPrefiltered = l;
				}
			);
		}
};

IMPLEMENT_TESTS(filter_bitswap);
