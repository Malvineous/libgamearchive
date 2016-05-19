/**
 * @file   test-filter-prehistorik.cpp
 * @brief  Test code for Prehistorik compression algorithm.
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

#include <camoto/bitstream.hpp>
#include "test-filter.hpp"

using namespace camoto;
using namespace camoto::gamearchive;

class test_filter_prehistorik: public test_filter
{
	public:
		test_filter_prehistorik()
		{
			this->type = "lzss-prehistorik";
		}

		void addTests()
		{
			this->test_filter::addTests();

			std::shared_ptr<stream::string> exp(new stream::string());
			bitstream bit_exp(exp, bitstream::bigEndian);
			bit_exp.write(8, 0);
			bit_exp.write(8, 0);
			bit_exp.write(8, 0);
			bit_exp.write(8, 18);
			bit_exp.write(9, 'H');
			bit_exp.write(9, 'e');
			bit_exp.write(9, 'l');
			bit_exp.write(9, 'l');
			bit_exp.write(9, 'o');
			bit_exp.write(9, ' ');
			bit_exp.write(9, 'h');
			bit_exp.write(9, 'e');
			bit_exp.write(9, 'l');
			bit_exp.write(9, 'l');
			bit_exp.write(9, 'o');
			bit_exp.write(9, ' ');
			bit_exp.write(9, 'h');
			bit_exp.write(9, 'e');
			bit_exp.write(9, 'l');
			bit_exp.write(9, 'l');
			bit_exp.write(9, 'o');
			bit_exp.write(9, '.');
			bit_exp.flushByte();

			this->content("normal", 18,
				exp->data,
			STRING_WITH_NULLS(
				"Hello hello hello."
			));
		}
};

IMPLEMENT_TESTS(filter_prehistorik);
