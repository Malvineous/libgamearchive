/**
 * @file   test-arch-dat-bash-compressed.cpp
 * @brief  Test code for compressed Monster Bash .DAT files.
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

#define FCONTENT1_SMALL "\x54\xD0\xA4\x99\x03\x22\xCD\x1C" "\x10\x6F\xDC\x94"
#define FCONTENT1 "\x54\xD0\xA4\x99\x03\x22\xCD\x1C" "\x10\x6F\xDC\x94\x71\x41\x26\x0C" "\x1D\x80"
#define FCONTENT2 "\x54\xD0\xA4\x99\x03\x22\xCD\x1C" "\x10\x74\xEE\xBC\x71\x41\x26\x0C" "\x1D\x80"
#define FCONTENT3 "\x54\xD0\xA4\x99\x03\x22\xCD\x1C" "\x10\x74\xD0\xC8\x29\x53\xC6\x05" "\x99\x30\x74\x00\x02"
#define FCONTENT4 "\x54\xD0\xA4\x99\x03\x22\xCD\x1C" "\x10\x66\xDE\xD4\x91\xE3\x82\x4C" "\x18\x3A\x00\x01"
#define FCONTENT_OVERW "\x4E\xDE\xDC\x01\x21\xA7\xCC\x9C" "\x34\x7A\xCA\x90\x01\x41\xE7\x0D" \
	"\x08\x19\x33\x40\x8C\x41\x13\x46" "\xCE\x1C\x80"

class test_dat_bash_compressed: public test_archive
{
	public:
		test_dat_bash_compressed()
		{
			this->type = "dat-bash";
			this->filename[0] = "ONE.MBG";
			this->lenMaxFilename = 30;
			this->insertAttr = Archive::File::Attribute::Compressed;
			this->content0_largeSize = 23;
			this->content0_smallSize = 12;
		}

		void addTests()
		{
			this->test_archive::addTests();

			// c00: Initial state
			this->isInstance(ArchiveType::DefinitelyYes, this->initialstate());

			// c01: access charst in by filename
			this->isInstance(ArchiveType::DefinitelyNo, STRING_WITH_NULLS(
				"\x20\x00" "\x12\x00"
					"ONE.DAT\x05\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
					"\x0f\x00"
					FCONTENT1
				"\x20\x00" "\x12\x00"
					"TWO.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
					"\x0f\x00"
					FCONTENT2
			));

			// c02: Blank archive
			this->isInstance(ArchiveType::DefinitelyYes, STRING_WITH_NULLS(
				""
			));

			// c03: File ends past EOF
			this->isInstance(ArchiveType::DefinitelyNo, STRING_WITH_NULLS(
				"\x20\x00" "\x12\x01"
					"ONE.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
					"\x0f\x00"
					FCONTENT1
				"\x20\x00" "\x12\x00"
					"TWO.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
					"\x0f\x00"
					FCONTENT2
			));

			// c04: Truncated FAT entry
			this->isInstance(ArchiveType::DefinitelyNo, STRING_WITH_NULLS(
				"\x20\x00" "\x12\x00"
					"ONE.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
					"\x0f\x00"
					FCONTENT1
				"\x20\x00" "\x12\x00"
					"TWO.DA"
			));
		}

		virtual std::string initialstate()
		{
			return STRING_WITH_NULLS(
				"\x01\x00" "\x12\x00"
					"ONE\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
					"\x0f\x00"
					FCONTENT1
				"\x20\x00" "\x12\x00"
					"TWO.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
					"\x0f\x00"
					FCONTENT2
			);
		}

		virtual std::string rename()
		{
			return STRING_WITH_NULLS(
				"\x20\x00" "\x12\x00"
					"THREE.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
					"\x0f\x00"
					FCONTENT1
				"\x20\x00" "\x12\x00"
					"TWO.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
					"\x0f\x00"
					FCONTENT2
			);
		}

		virtual std::string insert_end()
		{
			return STRING_WITH_NULLS(
				"\x01\x00" "\x12\x00"
					"ONE\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
					"\x0f\x00"
					FCONTENT1
				"\x20\x00" "\x12\x00"
					"TWO.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
					"\x0f\x00"
					FCONTENT2
				"\x20\x00" "\x15\x00"
					"THREE.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
					"\x11\x00"
					FCONTENT3
			);
		}

		virtual std::string insert_mid()
		{
			return STRING_WITH_NULLS(
				"\x01\x00" "\x12\x00"
					"ONE\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
					"\x0f\x00"
					FCONTENT1
				"\x20\x00" "\x15\x00"
					"THREE.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
					"\x11\x00"
					FCONTENT3
				"\x20\x00" "\x12\x00"
					"TWO.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
					"\x0f\x00"
					FCONTENT2
			);
		}

		virtual std::string insert2()
		{
			return STRING_WITH_NULLS(
				"\x01\x00" "\x12\x00"
					"ONE\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
					"\x0f\x00"
					FCONTENT1
				"\x20\x00" "\x15\x00"
					"THREE.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
					"\x11\x00"
					FCONTENT3
				"\x20\x00" "\x14\x00"
					"FOUR.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
					"\x10\x00"
					FCONTENT4
				"\x20\x00" "\x12\x00"
					"TWO.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
					"\x0f\x00"
					FCONTENT2
			);
		}

		virtual std::string remove()
		{
			return STRING_WITH_NULLS(
				"\x20\x00" "\x12\x00"
					"TWO.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
					"\x0f\x00"
					FCONTENT2
			);
		}

		virtual std::string remove2()
		{
			return STRING_WITH_NULLS(
				""
			);
		}

		virtual std::string insert_remove()
		{
			return STRING_WITH_NULLS(
				"\x20\x00" "\x15\x00"
					"THREE.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
					"\x11\x00"
					FCONTENT3
				"\x20\x00" "\x12\x00"
					"TWO.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
					"\x0f\x00"
					FCONTENT2
			);
		}

		virtual std::string move()
		{
			return STRING_WITH_NULLS(
				"\x20\x00" "\x12\x00"
					"TWO.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
					"\x0f\x00"
					FCONTENT2
				"\x01\x00" "\x12\x00"
					"ONE\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
					"\x0f\x00"
					FCONTENT1
			);
		}

		virtual std::string resize_larger()
		{
			return STRING_WITH_NULLS(
				"\x01\x00" "\x17\x00"
					"ONE\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
					"\x14\x00"
					FCONTENT1 "\0\0\0\0\0"
				"\x20\x00" "\x12\x00"
					"TWO.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
					"\x0f\x00"
					FCONTENT2
			);
		}

		virtual std::string resize_smaller()
		{
			return STRING_WITH_NULLS(
				"\x01\x00" "\x0c\x00"
					"ONE\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
					"\x0a\x00"
					FCONTENT1_SMALL
				"\x20\x00" "\x12\x00"
					"TWO.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
					"\x0f\x00"
					FCONTENT2
			);
		}

		virtual std::string resize_write()
		{
			return STRING_WITH_NULLS(
				"\x01\x00" "\x1b\x00"
					"ONE\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
					"\x17\x00"
					FCONTENT_OVERW
				"\x20\x00" "\x12\x00"
					"TWO.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
					"\x0f\x00"
					FCONTENT2
			);
		}
};

IMPLEMENT_TESTS(dat_bash_compressed);
