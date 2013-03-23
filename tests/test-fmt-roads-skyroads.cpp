/**
 * @file  test-fmt-roads-skyroads.cpp
 * @brief Test code for SkyRoadsRoadsArchive class.
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

#define testdata_initialstate \
	"\x08\x00" "\x0f\x00" \
	"\x17\x00" "\x0f\x00" \
	"This is one.dat" \
	"This is two.dat"

#define testdata_insert_end \
	"\x0c\x00" "\x0f\x00" \
	"\x1b\x00" "\x0f\x00" \
	"\x2a\x00" "\x11\x00" \
	"This is one.dat" \
	"This is two.dat" \
	"This is three.dat"

#define testdata_insert_mid \
	"\x0c\x00" "\x0f\x00" \
	"\x1b\x00" "\x11\x00" \
	"\x2c\x00" "\x0f\x00" \
	"This is one.dat" \
	"This is three.dat" \
	"This is two.dat"

#define testdata_insert2 \
	"\x10\x00" "\x0f\x00" \
	"\x1f\x00" "\x11\x00" \
	"\x30\x00" "\x10\x00" \
	"\x40\x00" "\x0f\x00" \
	"This is one.dat" \
	"This is three.dat" \
	"This is four.dat" \
	"This is two.dat"

#define testdata_remove \
	"\x04\x00" "\x0f\x00" \
	"This is two.dat"

#define testdata_remove2 \
	""

#define testdata_insert_remove \
	"\x08\x00" "\x11\x00" \
	"\x19\x00" "\x0f\x00" \
	"This is three.dat" \
	"This is two.dat"

#define testdata_move \
	"\x08\x00" "\x0f\x00" \
	"\x17\x00" "\x0f\x00" \
	"This is two.dat" \
	"This is one.dat"

#define testdata_resize_larger \
	"\x08\x00" "\x14\x00" \
	"\x1c\x00" "\x0f\x00" \
	"This is one.dat\0\0\0\0\0" \
	"This is two.dat"

#define testdata_resize_smaller \
	"\x08\x00" "\x0a\x00" \
	"\x12\x00" "\x0f\x00" \
	"This is on" \
	"This is two.dat"

#define testdata_resize_write \
	"\x08\x00" "\x17\x00" \
	"\x1f\x00" "\x0f\x00" \
	"Now resized to 23 chars" \
	"This is two.dat"

#define NO_FILENAMES 1
#define MAX_FILENAME_LEN  0

#define ARCHIVE_CLASS fmt_roads_skyroads
#define ARCHIVE_TYPE  "roads-skyroads"
#include "test-archive.hpp"

// Test some invalid formats to make sure they're not identified as valid
// archives.  Note that they can still be opened though (by 'force'), this
// only checks whether they look like valid files or not.

// The "c00" test has already been performed in test-archive.hpp to ensure the
// initial state is correctly identified as a valid archive.

ISINSTANCE_TEST(c01,
	"",
	DefinitelyYes
);

ISINSTANCE_TEST(c02,
	"\x50\x00\x00\x00",
	DefinitelyNo
);

ISINSTANCE_TEST(c03,
	"\x01\x00\x00",
	DefinitelyNo
);

ISINSTANCE_TEST(c04,
	"\x07\x00\x00\x00" "\x07\x00\x00",
	DefinitelyNo
);

ISINSTANCE_TEST(c05,
	"\x08\x00\x00\x00" "\x50\x00\x00\x00"
	"blahblah",
	DefinitelyNo
);

ISINSTANCE_TEST(c06,
	"\x08\x00\x00\x00" "\x04\x00\x00\x00"
	"blahblah",
	DefinitelyNo
);

// Not really possible to do any INVALIDDATA_TEST() tests here, because the
// worst that can happen is it looks like the archive has been truncated.
