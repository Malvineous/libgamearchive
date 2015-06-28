/**
 * @file   test-arch-epf-lionking.cpp
 * @brief  Test code for The Lion King .EPF archives.
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

#include "test-archive.hpp"

class test_epf_lionking: public test_archive
{
	public:
		test_epf_lionking()
		{
			this->type = "epf-lionking";
			this->lenMaxFilename = 12;
			this->hasMetadata[camoto::Metadata::MetadataType::Description] = true;
			this->metadataDesc = "Extra data";
			this->metadataDescLarger = "This is a test";
			this->metadataDescSmaller = "Hello";
		}

		void addTests()
		{
			this->test_archive::addTests();

			// c00: Initial state
			this->isInstance(ArchiveType::DefinitelyYes, this->initialstate());

			// c01: Bad signature
			this->isInstance(ArchiveType::DefinitelyNo, STRING_WITH_NULLS(
				"EPSF"      "\x33\x00\x00\x00" "\x00" "\x02\x00"
				"This is one.dat"
				"This is two.dat"
				"Extra data"
				"ONE.DAT\0\0\0\0\0\0" "\x00" "\x0f\x00\x00\x00" "\x0f\x00\x00\x00"
				"TWO.DAT\0\0\0\0\0\0" "\x00" "\x0f\x00\x00\x00" "\x0f\x00\x00\x00"
			));

			// c02: File too short
			this->isInstance(ArchiveType::DefinitelyNo, STRING_WITH_NULLS(
				"EPF"
			));

			// i01: FAT offset is past EOF
			this->invalidContent(STRING_WITH_NULLS(
				"EPFS"      "\x33\x00\x00\xf0" "\x00" "\x02\x00"
				"This is one.dat"
				"This is two.dat"
				"Extra data"
				"ONE.DAT\0\0\0\0\0\0" "\x00" "\x0f\x00\x00\x00" "\x0f\x00\x00\x00"
				"TWO.DAT\0\0\0\0\0\0" "\x00" "\x0f\x00\x00\x00" "\x0f\x00\x00\x00"
			));

			// i02: Large enough to cause the uint32_t value to wrap
			this->invalidContent(STRING_WITH_NULLS(
				"EPFS"      "\xf0\xff\xff\xff" "\x00" "\x02\x00"
				"This is one.dat"
				"This is two.dat"
				"Extra data"
				"ONE.DAT\0\0\0\0\0\0" "\x00" "\x0f\x00\x00\x00" "\x0f\x00\x00\x00"
				"TWO.DAT\0\0\0\0\0\0" "\x00" "\x0f\x00\x00\x00" "\x0f\x00\x00\x00"
			));
		}

		virtual std::string initialstate()
		{
			return STRING_WITH_NULLS(
				"EPFS"      "\x33\x00\x00\x00" "\x00" "\x02\x00"
				"This is one.dat"
				"This is two.dat"
				"Extra data"
				"ONE.DAT\0\0\0\0\0\0" "\x00" "\x0f\x00\x00\x00" "\x0f\x00\x00\x00"
				"TWO.DAT\0\0\0\0\0\0" "\x00" "\x0f\x00\x00\x00" "\x0f\x00\x00\x00"
			);
		}

		virtual std::string rename()
		{
			return STRING_WITH_NULLS(
				"EPFS"      "\x33\x00\x00\x00" "\x00" "\x02\x00"
				"This is one.dat"
				"This is two.dat"
				"Extra data"
				"THREE.DAT\0\0\0\0"   "\x00" "\x0f\x00\x00\x00" "\x0f\x00\x00\x00"
				"TWO.DAT\0\0\0\0\0\0" "\x00" "\x0f\x00\x00\x00" "\x0f\x00\x00\x00"
			);
		}

		virtual std::string insert_end()
		{
			return STRING_WITH_NULLS(
				"EPFS"      "\x44\x00\x00\x00" "\x00" "\x03\x00"
				"This is one.dat"
				"This is two.dat"
				"This is three.dat"
				"Extra data"
				"ONE.DAT\0\0\0\0\0\0" "\x00" "\x0f\x00\x00\x00" "\x0f\x00\x00\x00"
				"TWO.DAT\0\0\0\0\0\0" "\x00" "\x0f\x00\x00\x00" "\x0f\x00\x00\x00"
				"THREE.DAT\0\0\0\0"   "\x00" "\x11\x00\x00\x00" "\x11\x00\x00\x00"
			);
		}

		virtual std::string insert_mid()
		{
			return STRING_WITH_NULLS(
				"EPFS"      "\x44\x00\x00\x00" "\x00" "\x03\x00"
				"This is one.dat"
				"This is three.dat"
				"This is two.dat"
				"Extra data"
				"ONE.DAT\0\0\0\0\0\0" "\x00" "\x0f\x00\x00\x00" "\x0f\x00\x00\x00"
				"THREE.DAT\0\0\0\0"   "\x00" "\x11\x00\x00\x00" "\x11\x00\x00\x00"
				"TWO.DAT\0\0\0\0\0\0" "\x00" "\x0f\x00\x00\x00" "\x0f\x00\x00\x00"
			);
		}

		virtual std::string insert2()
		{
			return STRING_WITH_NULLS(
				"EPFS"      "\x54\x00\x00\x00" "\x00" "\x04\x00"
				"This is one.dat"
				"This is three.dat"
				"This is four.dat"
				"This is two.dat"
				"Extra data"
				"ONE.DAT\0\0\0\0\0\0" "\x00" "\x0f\x00\x00\x00" "\x0f\x00\x00\x00"
				"THREE.DAT\0\0\0\0"   "\x00" "\x11\x00\x00\x00" "\x11\x00\x00\x00"
				"FOUR.DAT\0\0\0\0\0"  "\x00" "\x10\x00\x00\x00" "\x10\x00\x00\x00"
				"TWO.DAT\0\0\0\0\0\0" "\x00" "\x0f\x00\x00\x00" "\x0f\x00\x00\x00"
			);
		}

		virtual std::string remove()
		{
			return STRING_WITH_NULLS(
				"EPFS"      "\x24\x00\x00\x00" "\x00" "\x01\x00"
				"This is two.dat"
				"Extra data"
				"TWO.DAT\0\0\0\0\0\0" "\x00" "\x0f\x00\x00\x00" "\x0f\x00\x00\x00"
			);
		}

		virtual std::string remove2()
		{
			return STRING_WITH_NULLS(
				"EPFS"      "\x15\x00\x00\x00" "\x00" "\x00\x00"
				"Extra data"
			);
		}

		virtual std::string insert_remove()
		{
			return STRING_WITH_NULLS(
				"EPFS"      "\x35\x00\x00\x00" "\x00" "\x02\x00"
				"This is three.dat"
				"This is two.dat"
				"Extra data"
				"THREE.DAT\0\0\0\0"   "\x00" "\x11\x00\x00\x00" "\x11\x00\x00\x00"
				"TWO.DAT\0\0\0\0\0\0" "\x00" "\x0f\x00\x00\x00" "\x0f\x00\x00\x00"
			);
		}

		virtual std::string move()
		{
			return STRING_WITH_NULLS(
				"EPFS"      "\x33\x00\x00\x00" "\x00" "\x02\x00"
				"This is two.dat"
				"This is one.dat"
				"Extra data"
				"TWO.DAT\0\0\0\0\0\0" "\x00" "\x0f\x00\x00\x00" "\x0f\x00\x00\x00"
				"ONE.DAT\0\0\0\0\0\0" "\x00" "\x0f\x00\x00\x00" "\x0f\x00\x00\x00"
			);
		}

		virtual std::string resize_larger()
		{
			return STRING_WITH_NULLS(
				"EPFS"      "\x38\x00\x00\x00" "\x00" "\x02\x00"
				"This is one.dat\0\0\0\0\0"
				"This is two.dat"
				"Extra data"
				"ONE.DAT\0\0\0\0\0\0" "\x00" "\x14\x00\x00\x00" "\x14\x00\x00\x00"
				"TWO.DAT\0\0\0\0\0\0" "\x00" "\x0f\x00\x00\x00" "\x0f\x00\x00\x00"
			);
		}

		virtual std::string resize_smaller()
		{
			return STRING_WITH_NULLS(
				"EPFS"      "\x2e\x00\x00\x00" "\x00" "\x02\x00"
				"This is on"
				"This is two.dat"
				"Extra data"
				"ONE.DAT\0\0\0\0\0\0" "\x00" "\x0a\x00\x00\x00" "\x0a\x00\x00\x00"
				"TWO.DAT\0\0\0\0\0\0" "\x00" "\x0f\x00\x00\x00" "\x0f\x00\x00\x00"
			);
		}

		virtual std::string resize_write()
		{
			return STRING_WITH_NULLS(
				"EPFS"      "\x3b\x00\x00\x00" "\x00" "\x02\x00"
				"Now resized to 23 chars"
				"This is two.dat"
				"Extra data"
				"ONE.DAT\0\0\0\0\0\0" "\x00" "\x17\x00\x00\x00" "\x17\x00\x00\x00"
				"TWO.DAT\0\0\0\0\0\0" "\x00" "\x0f\x00\x00\x00" "\x0f\x00\x00\x00"
			);
		}

		virtual std::string metadata_set_desc_larger()
		{
			return STRING_WITH_NULLS(
				"EPFS"      "\x37\x00\x00\x00" "\x00" "\x02\x00"
				"This is one.dat"
				"This is two.dat"
				"This is a test"
				"ONE.DAT\0\0\0\0\0\0" "\x00" "\x0f\x00\x00\x00" "\x0f\x00\x00\x00"
				"TWO.DAT\0\0\0\0\0\0" "\x00" "\x0f\x00\x00\x00" "\x0f\x00\x00\x00"
			);
		}

		virtual std::string metadata_set_desc_smaller()
		{
			return STRING_WITH_NULLS(
				"EPFS"      "\x2e\x00\x00\x00" "\x00" "\x02\x00"
				"This is one.dat"
				"This is two.dat"
				"Hello"
				"ONE.DAT\0\0\0\0\0\0" "\x00" "\x0f\x00\x00\x00" "\x0f\x00\x00\x00"
				"TWO.DAT\0\0\0\0\0\0" "\x00" "\x0f\x00\x00\x00" "\x0f\x00\x00\x00"
			);
		}
};

IMPLEMENT_TESTS(epf_lionking);
