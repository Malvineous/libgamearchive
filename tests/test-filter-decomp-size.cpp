/**
 * @file   test-filter-decomp-size.cpp
 * @brief  Test code for decompression-size prefix filter.
 *
 * Copyright (C) 2010-2016 Adam Nielsen <malvineous@shikadi.net>
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

#include <camoto/filter-dummy.hpp>
#include <camoto/util.hpp>
#include "test-filter.hpp"
#include "../src/filter-decomp-size.hpp"

using namespace camoto;
using namespace camoto::gamearchive;

class test_filter_decomp_size: public test_filter
{
	public:
		test_filter_decomp_size()
		{
		}

		void addTests()
		{
			this->test_filter::addTests();

			// Remove length field from some data
			this->content("normal", 5, STRING_WITH_NULLS(
				"\x05\x00\x00\x00"
				"hello"
			),
				"hello"
			);

			// Ignore trailing data
			this->content_decode("ignore_trailing", STRING_WITH_NULLS(
				"\x05\x00\x00\x00"
				"hellogoodbye"
			),
				"hello"
			);

			// Pad after running out of source data
			this->content_decode("pad", STRING_WITH_NULLS(
				"\x06\x00\x00\x00"
				"hello"
			), STRING_WITH_NULLS(
				"hello\x00"
			));

			// Pad after running out of source data
			this->content_decode("pad", STRING_WITH_NULLS(
				"\x06\x00\x00\x00"
				"hello"
			), STRING_WITH_NULLS(
				"hello\x00"
			));

			// Empty
			this->content_decode("empty", STRING_WITH_NULLS(
				"\x00\x00\x00\x00"
			), STRING_WITH_NULLS(
				""
			));
		}

		std::unique_ptr<stream::input> apply_in(
			std::unique_ptr<stream::input> content)
		{
			return std::make_unique<stream::input_filtered>(
				std::move(content),
				std::make_unique<filter_decomp_size_remove>(
					std::make_unique<filter_dummy>()
				)
			);
		}

		std::unique_ptr<stream::output> apply_out(
			std::unique_ptr<stream::output> content, stream::len *setPrefiltered)
		{
			return std::make_unique<stream::output_filtered>(
				std::move(content),
				std::make_unique<filter_decomp_size_insert>(
					std::make_unique<filter_dummy>()
				),
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
				std::make_unique<filter_decomp_size_remove>(
					std::make_unique<filter_dummy>()
				),
				std::make_unique<filter_decomp_size_insert>(
					std::make_unique<filter_dummy>()
				),
				[setPrefiltered](stream::output_filtered* s, stream::len l) {
					if (setPrefiltered) *setPrefiltered = l;
				}
			);
		}
};

IMPLEMENT_TESTS(filter_decomp_size);
