/**
 * @file   test-fmt-dat-bash-compressed.cpp
 * @brief  Test code for compressed Monster Bash .DAT files.
 *
 * Copyright (C) 2010-2012 Adam Nielsen <malvineous@shikadi.net>
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

#define FILENAME1 "ONE.MBG"
#define FILENAME2 "TWO.DAT"
#define FILENAME3 "THREE.DAT"
#define FILENAME4 "FOUR.DAT"

#define FCONTENT1_SMALL "\x54\xD0\xA4\x99\x03\x22\xCD\x1C" "\x10\x6F\xDC\x94"
#define FCONTENT1 "\x54\xD0\xA4\x99\x03\x22\xCD\x1C" "\x10\x6F\xDC\x94\x71\x41\x26\x0C" "\x1D\x80"
#define FCONTENT2 "\x54\xD0\xA4\x99\x03\x22\xCD\x1C" "\x10\x74\xEE\xBC\x71\x41\x26\x0C" "\x1D\x80"
#define FCONTENT3 "\x54\xD0\xA4\x99\x03\x22\xCD\x1C" "\x10\x74\xD0\xC8\x29\x53\xC6\x05" "\x99\x30\x74\x00\x02"
#define FCONTENT4 "\x54\xD0\xA4\x99\x03\x22\xCD\x1C" "\x10\x66\xDE\xD4\x91\xE3\x82\x4C" "\x18\x3A\x00\x01"
#define FCONTENT_OVERW "\x4E\xDE\xDC\x01\x21\xA7\xCC\x9C" "\x34\x7A\xCA\x90\x01\x41\xE7\x0D" \
	"\x08\x19\x33\x40\x8C\x41\x13\x46" "\xCE\x1C\x80"

#define CONTENT1_LARGESIZE_STORED 23
#define CONTENT1_SMALLSIZE_STORED 12
#define CONTENT1_OVERWSIZE_STORED (sizeof(FCONTENT_OVERW)-1)

#define testdata_initialstate \
	"\x01\x00" "\x12\x00" \
		"ONE\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
		"\x0f\x00" \
		FCONTENT1 \
	"\x20\x00" "\x12\x00" \
		"TWO.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
		"\x0f\x00" \
		FCONTENT2

#define testdata_rename \
	"\x20\x00" "\x12\x00" \
		"THREE.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
		"\x0f\x00" \
		FCONTENT1 \
	"\x20\x00" "\x12\x00" \
		"TWO.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
		"\x0f\x00" \
		FCONTENT2

#define testdata_insert_end \
	"\x01\x00" "\x12\x00" \
		"ONE\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
		"\x0f\x00" \
		FCONTENT1 \
	"\x20\x00" "\x12\x00" \
		"TWO.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
		"\x0f\x00" \
		FCONTENT2 \
	"\x20\x00" "\x15\x00" \
		"THREE.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
		"\x11\x00" \
		FCONTENT3

#define testdata_insert_mid \
	"\x01\x00" "\x12\x00" \
		"ONE\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
		"\x0f\x00" \
		FCONTENT1 \
	"\x20\x00" "\x15\x00" \
		"THREE.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
		"\x11\x00" \
		FCONTENT3 \
	"\x20\x00" "\x12\x00" \
		"TWO.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
		"\x0f\x00" \
		FCONTENT2

#define testdata_insert2 \
	"\x01\x00" "\x12\x00" \
		"ONE\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
		"\x0f\x00" \
		FCONTENT1 \
	"\x20\x00" "\x15\x00" \
		"THREE.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
		"\x11\x00" \
		FCONTENT3 \
	"\x20\x00" "\x14\x00" \
		"FOUR.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
		"\x10\x00" \
		FCONTENT4 \
	"\x20\x00" "\x12\x00" \
		"TWO.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
		"\x0f\x00" \
		FCONTENT2

#define testdata_remove \
	"\x20\x00" "\x12\x00" \
		"TWO.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
		"\x0f\x00" \
		FCONTENT2

#define testdata_remove2 \
	""

#define testdata_insert_remove \
	"\x20\x00" "\x15\x00" \
		"THREE.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
		"\x11\x00" \
		FCONTENT3 \
	"\x20\x00" "\x12\x00" \
		"TWO.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
		"\x0f\x00" \
		FCONTENT2

#define testdata_move \
	"\x20\x00" "\x12\x00" \
		"TWO.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
		"\x0f\x00" \
		FCONTENT2 \
	"\x01\x00" "\x12\x00" \
		"ONE\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
		"\x0f\x00" \
		FCONTENT1

#define testdata_resize_larger \
	"\x01\x00" "\x17\x00" \
		"ONE\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
		"\x14\x00" \
		FCONTENT1 "\0\0\0\0\0" \
	"\x20\x00" "\x12\x00" \
		"TWO.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
		"\x0f\x00" \
		FCONTENT2

#define testdata_resize_smaller \
	"\x01\x00" "\x0c\x00" \
		"ONE\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
		"\x0a\x00" \
		FCONTENT1_SMALL \
	"\x20\x00" "\x12\x00" \
		"TWO.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
		"\x0f\x00" \
		FCONTENT2

#define testdata_resize_write \
	"\x01\x00" "\x1b\x00" \
		"ONE\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
		"\x17\x00" \
		FCONTENT_OVERW \
	"\x20\x00" "\x12\x00" \
		"TWO.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
		"\x0f\x00" \
		FCONTENT2

#define MAX_FILENAME_LEN  30
#define INSERT_ATTRIBUTE EA_COMPRESSED

#define ARCHIVE_CLASS fmt_dat_bash_compressed
#define ARCHIVE_TYPE  "dat-bash"
#include "test-archive.hpp"

// Test some invalid formats to make sure they're not identified as valid
// archives.  Note that they can still be opened though (by 'force'), this
// only checks whether they look like valid files or not.

// The "c00" test has already been performed in test-archive.hpp to ensure the
// initial state is correctly identified as a valid archive.

ISINSTANCE_TEST(c01,
	"\x20\x00" "\x12\x00" \
		"ONE.DAT\x05\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
		"\x0f\x00" \
		FCONTENT1 \
	"\x20\x00" "\x12\x00" \
		"TWO.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
		"\x0f\x00" \
		FCONTENT2,
	DefinitelyNo
);

// Blank archive
ISINSTANCE_TEST(c02,
	"",
	DefinitelyYes
);

ISINSTANCE_TEST(c03,
	"\x20\x00" "\x12\x01" \
		"ONE.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
		"\x0f\x00" \
		FCONTENT1 \
	"\x20\x00" "\x12\x00" \
		"TWO.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
		"\x0f\x00" \
		FCONTENT2,
	DefinitelyNo
);
