/**
 * @file   test-filter-bash-rle.cpp
 * @brief  Test code for Monster Bash RLE packer/unpacker.
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
#include "../src/filter-bash-rle.hpp"

using namespace camoto::gamearchive;

class test_filter_bash_rle: public test_filter
{
	public:
		test_filter_bash_rle()
		{
		}

		void addTests()
		{
			this->test_filter::addTests();

			// Read truncated RLE-escape in Monster Bash RLE-encoded data
			this->invalidContent(STRING_WITH_NULLS(
				"ABC\x90"
			));

			// Decode some Monster Bash RLE-encoded data
			this->content("normal", 8, STRING_WITH_NULLS(
				"ABC\x90\x05""D"
			), STRING_WITH_NULLS(
				"ABCCCCCD"
			));

			// Decode RLE-escape in Monster Bash RLE-encoded data
			this->content("escape", 5, STRING_WITH_NULLS(
				"ABC\x90\x00""D"
			), STRING_WITH_NULLS(
				"ABC\x90""D"
			));

			// RLE decode > 256 bytes (one leftover) in Monster Bash RLE-encoded data
			this->content("read_lots1", 2+255+1, STRING_WITH_NULLS(
				"ABC\x90\xFF""C"
			),
				STRING_WITH_NULLS("AB")
				+ std::string(1+254+1, 'C')
			);

			// RLE decode > 256 bytes (two leftovers) in Monster Bash RLE-encoded data
			this->content("read_lots2", 2+255+2, STRING_WITH_NULLS(
				"ABC\x90\xFF""CC"
			),
				STRING_WITH_NULLS("AB")
				+ std::string(1+254+2, 'C')
			);

			// RLE decode > 256 bytes (three leftovers) in Monster Bash RLE-encoded data
			this->content("read_lots3", 2+255+3, STRING_WITH_NULLS(
				"ABC\x90\xFF\x90\x04"
			),
				STRING_WITH_NULLS("AB")
				+ std::string(1+254+3, 'C')
			);

			// RLE decode > 256 bytes (four leftovers) in Monster Bash RLE-encoded data
			this->content("read_lots4", 2+255+4, STRING_WITH_NULLS(
				"ABC\x90\xFF\x90\x05"
			),
				STRING_WITH_NULLS("AB")
				+ std::string(1+254+4, 'C')
			);

			// RLE decode > 512 bytes in Monster Bash RLE-encoded data
			this->content("read_3lots", 1+5+1+255+254+0x91+1, STRING_WITH_NULLS(
				"AB\x90\x05""CB\x90\xFF\x90\xFF\x90\x92""E"
			),
				STRING_WITH_NULLS("A")
				+ std::string(5, 'B')
				+ STRING_WITH_NULLS("C")
				+ std::string(1+254+254+0x91, 'B')
				+ STRING_WITH_NULLS("E")
			);

			// Unescaping many RLE event chars in Monster Bash RLE-encoded data
			this->content("long_escape", 2+256+1, STRING_WITH_NULLS(
				// Would come out larger post-RLE, so don't bother
				"AB\x90\x00\x90\xFF\x90\x00""D"
			),
				STRING_WITH_NULLS("AB")
				+ std::string(1+254+1, '\x90')
				+ STRING_WITH_NULLS("D")
			);

			// RLE-encode the RLE event byte in Monster Bash RLE-encoded data
			this->content("repeat_escape", 9, STRING_WITH_NULLS(
				"ABC\x90\x00\x90\x05""D"
			), STRING_WITH_NULLS(
				"ABC\x90\x90\x90\x90\x90""D"
			));

			// Write ending with RLE event in Monster Bash RLE-encoded data
			this->content("read_trailing", 8, STRING_WITH_NULLS(
				"ABC\x90\x06"
			), STRING_WITH_NULLS(
				"ABCCCCCC"
			));


			// Write ending with RLE char in Monster Bash RLE-encoded data
			this->content("escape_trailing", 4, STRING_WITH_NULLS(
				"ABC\x90\x00"
			), STRING_WITH_NULLS(
				"ABC\x90"
			));


			// RLE event skipping with doubled data in Monster Bash RLE-encoded data
			this->content("short2", 5, STRING_WITH_NULLS(
				"ABCCD"
			), STRING_WITH_NULLS(
				// Would come out larger post-RLE, so don't bother
				"ABCCD"
			));

			// RLE event skipping with tripled data in Monster Bash RLE-encoded data
			this->content("short3", 6, STRING_WITH_NULLS(
				"ABCCCD"
			), STRING_WITH_NULLS(
				// Would come out the same size post-RLE, so don't bother
				"ABCCCD"
			));

			// Escaping doubled RLE event char in Monster Bash RLE-encoded data
			this->content("short_escape", 5, STRING_WITH_NULLS(
				"AB\x90\x00\x90\x00""D"
			), STRING_WITH_NULLS(
				"AB\x90\x90""D"
			));
		}

		std::unique_ptr<stream::input> apply_in(
			std::unique_ptr<stream::input> content)
		{
			return std::make_unique<stream::input_filtered>(
				std::move(content),
				std::make_unique<filter_bash_unrle>()
			);
		}

		std::unique_ptr<stream::output> apply_out(
			std::unique_ptr<stream::output> content, stream::len *setPrefiltered)
		{
			return std::make_unique<stream::output_filtered>(
				std::move(content),
				std::make_unique<filter_bash_rle>(),
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
				std::make_unique<filter_bash_unrle>(),
				std::make_unique<filter_bash_rle>(),
				[setPrefiltered](stream::output_filtered* s, stream::len l) {
					if (setPrefiltered) *setPrefiltered = l;
				}
			);
		}
};

IMPLEMENT_TESTS(filter_bash_rle);
