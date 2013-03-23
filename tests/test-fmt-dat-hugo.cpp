/**
 * @file  test-fmt-dat-hugo.cpp
 * @brief Test code for DAT_HugoArchive class.
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
	"\x10\x00\x00\x00" "\x0f\x00\x00\x00" \
	"\x1f\x00\x00\x00" "\x0f\x00\x00\x00" \
	"This is one.dat" \
	"This is two.dat"

#define testdata_insert_end \
	"\x18\x00\x00\x00" "\x0f\x00\x00\x00" \
	"\x27\x00\x00\x00" "\x0f\x00\x00\x00" \
	"\x36\x00\x00\x00" "\x11\x00\x00\x00" \
	"This is one.dat" \
	"This is two.dat" \
	"This is three.dat"

#define testdata_insert_mid \
	"\x18\x00\x00\x00" "\x0f\x00\x00\x00" \
	"\x27\x00\x00\x00" "\x11\x00\x00\x00" \
	"\x38\x00\x00\x00" "\x0f\x00\x00\x00" \
	"This is one.dat" \
	"This is three.dat" \
	"This is two.dat"

#define testdata_insert2 \
	"\x20\x00\x00\x00" "\x0f\x00\x00\x00" \
	"\x2f\x00\x00\x00" "\x11\x00\x00\x00" \
	"\x40\x00\x00\x00" "\x10\x00\x00\x00" \
	"\x50\x00\x00\x00" "\x0f\x00\x00\x00" \
	"This is one.dat" \
	"This is three.dat" \
	"This is four.dat" \
	"This is two.dat"

#define testdata_remove \
	"\x08\x00\x00\x00" "\x0f\x00\x00\x00" \
	"This is two.dat"

#define testdata_remove2 \
	""

#define testdata_insert_remove \
	"\x10\x00\x00\x00" "\x11\x00\x00\x00" \
	"\x21\x00\x00\x00" "\x0f\x00\x00\x00" \
	"This is three.dat" \
	"This is two.dat"

#define testdata_move \
	"\x10\x00\x00\x00" "\x0f\x00\x00\x00" \
	"\x1f\x00\x00\x00" "\x0f\x00\x00\x00" \
	"This is two.dat" \
	"This is one.dat"

#define testdata_resize_larger \
	"\x10\x00\x00\x00" "\x14\x00\x00\x00" \
	"\x24\x00\x00\x00" "\x0f\x00\x00\x00" \
	"This is one.dat\0\0\0\0\0" \
	"This is two.dat"

#define testdata_resize_smaller \
	"\x10\x00\x00\x00" "\x0a\x00\x00\x00" \
	"\x1a\x00\x00\x00" "\x0f\x00\x00\x00" \
	"This is on" \
	"This is two.dat"

#define testdata_resize_write \
	"\x10\x00\x00\x00" "\x17\x00\x00\x00" \
	"\x27\x00\x00\x00" "\x0f\x00\x00\x00" \
	"Now resized to 23 chars" \
	"This is two.dat"

#define NO_FILENAMES 1
#define MAX_FILENAME_LEN  0

#define ARCHIVE_CLASS fmt_dat_hugo
#define ARCHIVE_TYPE  "dat-hugo"
#include "test-archive.hpp"

// Test some invalid formats to make sure they're not identified as valid
// archives.  Note that they can still be opened though (by 'force'), this
// only checks whether they look like valid files or not.

// The "c00" test has already been performed in test-archive.hpp to ensure the
// initial state is correctly identified as a valid archive.

ISINSTANCE_TEST(c01,
	"\x50\x00\x00\x00" "\x0f\x00\x00\x00"
	"This is one.dat",
	DefinitelyNo
);

ISINSTANCE_TEST(c02,
	"\x08\x00\x00\x00" "\x0f\x00\x00",
	DefinitelyNo
);

ISINSTANCE_TEST(c03,
	"\x08\x00\x00\x00" "\x50\x00\x00\x00"
	"This is one.dat",
	DefinitelyNo
);

ISINSTANCE_TEST(c04,
	"",
	PossiblyYes
);

// Not really possible to do any INVALIDDATA_TEST() tests here, because the
// worst that can happen is it looks like the archive has been truncated.
