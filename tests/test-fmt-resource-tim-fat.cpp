/**
 * @file   test-arch-resource-tim-fat.cpp
 * @brief  Test code for The Incredible Machine resource list archives.
 *
 * Copyright (C) 2010-2013 Adam Nielsen <malvineous@shikadi.net>
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

#include "test-archive.hpp"

// This format requires all files to be a multiple of eight bytes in length.
#define CONTENT1 "This is one.dat\0"
#define CONTENT2 "This is two.dat\0"
#define CONTENT3 "This is three.dat\0\0\0\0\0\0\0"
#define CONTENT4 "This is four.dat\0\0\0\0\0\0\0\0"
#define CONTENT1_OVERWRITTEN "Now resized to 24 chars!"

class test_resource_tim_fat: public test_archive
{
	public:
		test_resource_tim_fat()
		{
			this->type = "resource-tim-fat";
			this->filename[0] = "RESOURCE.001";
			this->filename[1] = "RESOURCE.002";
			this->filename[2] = "RESOURCE.003";
			this->filename[3] = "RESOURCE.004";
			this->lenMaxFilename = 12;
			this->content[0] = STRING_WITH_NULLS(CONTENT1);
			this->content[1] = STRING_WITH_NULLS(CONTENT2);
			this->content[2] = STRING_WITH_NULLS(CONTENT3);
			this->content[3] = STRING_WITH_NULLS(CONTENT4);
			this->content0_largeSize = 24;
			this->content0_smallSize = 8;
			this->content0_overwritten = STRING_WITH_NULLS(CONTENT1_OVERWRITTEN);
		}

		void addTests()
		{
			this->test_archive::addTests();

			// c00: Initial state
			this->isInstance(ArchiveType::DefinitelyYes, this->initialstate());

			// c01: File too short
			this->isInstance(ArchiveType::DefinitelyNo, STRING_WITH_NULLS(
				"\x00\x00" "\x00\x00" "\x02"
			));

			// c02: Data trailing after last file
			this->isInstance(ArchiveType::DefinitelyNo, STRING_WITH_NULLS(
				"\x00\x00" "\x00\x00" "\x02\x00"
				"RESOURCE.001\0" "\x02\x00" CONTENT1
				"RESOURCE.002\0" "\x02\x00" CONTENT2
				"blah"
			));

			// c03: Count field truncated
			this->isInstance(ArchiveType::DefinitelyNo, STRING_WITH_NULLS(
				"\x00\x00" "\x00\x00" "\x02\x00"
				"RESOURCE.001\0" "\x02\x00" CONTENT1
				"RESOURCE.002\0" "\x02"
			));
		}

		virtual std::string initialstate()
		{
			return STRING_WITH_NULLS(
				"\x00\x00" "\x00\x00" "\x02\x00"
				"RESOURCE.001\0" "\x02\x00" CONTENT1
				"RESOURCE.002\0" "\x02\x00" CONTENT2
			);
		}

		virtual std::string rename()
		{
			return STRING_WITH_NULLS(
				"\x00\x00" "\x00\x00" "\x02\x00"
				"RESOURCE.003\0" "\x02\x00" CONTENT1
				"RESOURCE.002\0" "\x02\x00" CONTENT2
			);
		}

		virtual std::string insert_end()
		{
			return STRING_WITH_NULLS(
				"\x00\x00" "\x00\x00" "\x03\x00"
				"RESOURCE.001\0" "\x02\x00" CONTENT1
				"RESOURCE.002\0" "\x02\x00" CONTENT2
				"RESOURCE.003\0" "\x03\x00" CONTENT3
			);
		}

		virtual std::string insert_mid()
		{
			return STRING_WITH_NULLS(
				"\x00\x00" "\x00\x00" "\x03\x00"
				"RESOURCE.001\0" "\x02\x00" CONTENT1
				"RESOURCE.003\0" "\x03\x00" CONTENT3
				"RESOURCE.002\0" "\x02\x00" CONTENT2
			);
		}

		virtual std::string insert2()
		{
			return STRING_WITH_NULLS(
				"\x00\x00" "\x00\x00" "\x04\x00"
				"RESOURCE.001\0" "\x02\x00" CONTENT1
				"RESOURCE.003\0" "\x03\x00" CONTENT3
				"RESOURCE.004\0" "\x03\x00" CONTENT4
				"RESOURCE.002\0" "\x02\x00" CONTENT2
			);
		}

		virtual std::string remove()
		{
			return STRING_WITH_NULLS(
				"\x00\x00" "\x00\x00" "\x01\x00"
				"RESOURCE.002\0" "\x02\x00" CONTENT2
			);
		}

		virtual std::string remove2()
		{
			return STRING_WITH_NULLS(
				"\x00\x00" "\x00\x00" "\x00\x00"
			);
		}

		virtual std::string insert_remove()
		{
			return STRING_WITH_NULLS(
				"\x00\x00" "\x00\x00" "\x02\x00"
				"RESOURCE.003\0" "\x03\x00" CONTENT3
				"RESOURCE.002\0" "\x02\x00" CONTENT2
			);
		}

		virtual std::string move()
		{
			return STRING_WITH_NULLS(
				"\x00\x00" "\x00\x00" "\x02\x00"
				"RESOURCE.002\0" "\x02\x00" CONTENT2
				"RESOURCE.001\0" "\x02\x00" CONTENT1
			);
		}

		virtual std::string resize_larger()
		{
			return STRING_WITH_NULLS(
				"\x00\x00" "\x00\x00" "\x02\x00"
				"RESOURCE.001\0" "\x03\x00" CONTENT1 "\0\0\0\0\0\0\0\0"
				"RESOURCE.002\0" "\x02\x00" CONTENT2
			);
		}

		virtual std::string resize_smaller()
		{
			return STRING_WITH_NULLS(
				"\x00\x00" "\x00\x00" "\x02\x00"
				"RESOURCE.001\0" "\x01\x00" "This is "
				"RESOURCE.002\0" "\x02\x00" CONTENT2
			);
		}

		virtual std::string resize_write()
		{
			return STRING_WITH_NULLS(
				"\x00\x00" "\x00\x00" "\x02\x00"
				"RESOURCE.001\0" "\x03\x00" CONTENT1_OVERWRITTEN
				"RESOURCE.002\0" "\x02\x00" CONTENT2
			);
		}
};

IMPLEMENT_TESTS(resource_tim_fat);
