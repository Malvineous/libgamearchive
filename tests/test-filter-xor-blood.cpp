/**
 * @file   test-filter-xor-blood.cpp
 * @brief  Test code for Blood XOR encryption algorithm.
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

#include "test-filter.hpp"
#include "../src/filter-xor-blood.hpp"

using namespace camoto::gamearchive;

class test_filter_xor_blood: public test_filter
{
	public:
		test_filter_xor_blood()
		{
		}

		void addTests()
		{
			this->test_filter::addTests();

			this->content("normal", 8, STRING_WITH_NULLS(
				"\x00\x01\x02\x03\xFF\xFF\xFF\xFF"
			), STRING_WITH_NULLS(
				"\x00\x01\x03\x02\xFD\xFD\xFC\xFC"
			));
		}

		std::unique_ptr<stream::input> apply_in(
			std::unique_ptr<stream::input> content)
		{
			return std::make_unique<stream::input_filtered>(
				std::move(content),
				std::make_unique<filter_rff_crypt>(0, 0)
			);
		}

		std::unique_ptr<stream::output> apply_out(
			std::unique_ptr<stream::output> content, stream::len *setPrefiltered)
		{
			return std::make_unique<stream::output_filtered>(
				std::move(content),
				std::make_unique<filter_rff_crypt>(0, 0),
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
				std::make_unique<filter_rff_crypt>(0, 0),
				std::make_unique<filter_rff_crypt>(0, 0),
				[setPrefiltered](stream::output_filtered* s, stream::len l) {
					if (setPrefiltered) *setPrefiltered = l;
				}
			);
		}
};

class test_filter_xor_blood_partial: public test_filter
{
	public:
		test_filter_xor_blood_partial()
		{
		}

		void addTests()
		{
			this->test_filter::addTests();

			this->content("normal", 8, STRING_WITH_NULLS(
				"\x00\x01\x02\x03\xFF\xFF\xFF\xFF"
			), STRING_WITH_NULLS(
				"\x00\x01\x03\x02\xFF\xFF\xFF\xFF"
			));
		}

		std::unique_ptr<stream::input> apply_in(
			std::unique_ptr<stream::input> content)
		{
			return std::make_unique<stream::input_filtered>(
				std::move(content),
				std::make_unique<filter_rff_crypt>(4, 0)
			);
		}

		std::unique_ptr<stream::output> apply_out(
			std::unique_ptr<stream::output> content, stream::len *setPrefiltered)
		{
			return std::make_unique<stream::output_filtered>(
				std::move(content),
				std::make_unique<filter_rff_crypt>(4, 0),
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
				std::make_unique<filter_rff_crypt>(4, 0),
				std::make_unique<filter_rff_crypt>(4, 0),
				[setPrefiltered](stream::output_filtered* s, stream::len l) {
					if (setPrefiltered) *setPrefiltered = l;
				}
			);
		}
};

class test_filter_xor_blood_altseed: public test_filter
{
	public:
		test_filter_xor_blood_altseed()
		{
		}

		void addTests()
		{
			this->test_filter::addTests();

			this->content("normal", 8, STRING_WITH_NULLS(
				"\x00\x01\x02\x03\xFF\xFF\xFF\xFF"
			), STRING_WITH_NULLS(
				"\xFE\xFF\xFD\xFC\xFF\xFF\xFE\xFE"
			));
		}

		std::unique_ptr<stream::input> apply_in(
			std::unique_ptr<stream::input> content)
		{
			return std::make_unique<stream::input_filtered>(
				std::move(content),
				std::make_unique<filter_rff_crypt>(0, 0xFE)
			);
		}

		std::unique_ptr<stream::output> apply_out(
			std::unique_ptr<stream::output> content, stream::len *setPrefiltered)
		{
			return std::make_unique<stream::output_filtered>(
				std::move(content),
				std::make_unique<filter_rff_crypt>(0, 0xFE),
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
				std::make_unique<filter_rff_crypt>(0, 0xFE),
				std::make_unique<filter_rff_crypt>(0, 0xFE),
				[setPrefiltered](stream::output_filtered* s, stream::len l) {
					if (setPrefiltered) *setPrefiltered = l;
				}
			);
		}
};

class test_filter_xor_blood_stream: public test_filter
{
	public:
		test_filter_xor_blood_stream()
		{
			this->type = "xor-blood";
		}

		void addTests()
		{
			this->test_filter::addTests();

			this->content("normal", 8, STRING_WITH_NULLS(
				"\x00\x01\x02\x03\xFF\xFF\xFF\xFF"
			), STRING_WITH_NULLS(
				"\x00\x01\x03\x02\xFD\xFD\xFC\xFC"
			));
		}
};

IMPLEMENT_TESTS(filter_xor_blood);
IMPLEMENT_TESTS(filter_xor_blood_partial);
IMPLEMENT_TESTS(filter_xor_blood_altseed);
IMPLEMENT_TESTS(filter_xor_blood_stream);
