/*
 * test-fmt-dat-sango.cpp - test code for GRPArchive class.
 *
 * Copyright (C) 2010-2011 Adam Nielsen <malvineous@shikadi.net>
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

#define FILENAME1 "ONE.DAT"
#define FILENAME2 "TWO.DAT"
#define FILENAME3 "THREE.DAT"
#define FILENAME4 "FOUR.DAT"

#define testdata_initialstate \
	"\x0c\x00\x00\x00" \
	"\x1b\x00\x00\x00" \
	"\x2a\x00\x00\x00" \
	"This is one.dat" \
	"This is two.dat"

#define testdata_insert_end \
	"\x10\x00\x00\x00" \
	"\x1f\x00\x00\x00" \
	"\x2e\x00\x00\x00" \
	"\x3f\x00\x00\x00" \
	"This is one.dat" \
	"This is two.dat" \
	"This is three.dat"

#define testdata_insert_mid \
	"\x10\x00\x00\x00" \
	"\x1f\x00\x00\x00" \
	"\x30\x00\x00\x00" \
	"\x3f\x00\x00\x00" \
	"This is one.dat" \
	"This is three.dat" \
	"This is two.dat"

#define testdata_insert2 \
	"\x14\x00\x00\x00" \
	"\x23\x00\x00\x00" \
	"\x34\x00\x00\x00" \
	"\x44\x00\x00\x00" \
	"\x53\x00\x00\x00" \
	"This is one.dat" \
	"This is three.dat" \
	"This is four.dat" \
	"This is two.dat"

#define testdata_remove \
	"\x08\x00\x00\x00" \
	"\x17\x00\x00\x00" \
	"This is two.dat"

#define testdata_remove2 \
	"\x04\x00\x00\x00"

#define testdata_insert_remove \
	"\x0c\x00\x00\x00" \
	"\x1d\x00\x00\x00" \
	"\x2c\x00\x00\x00" \
	"This is three.dat" \
	"This is two.dat" \

#define testdata_move \
	"\x0c\x00\x00\x00" \
	"\x1b\x00\x00\x00" \
	"\x2a\x00\x00\x00" \
	"This is two.dat" \
	"This is one.dat"

#define testdata_resize_larger \
	"\x0c\x00\x00\x00" \
	"\x20\x00\x00\x00" \
	"\x2f\x00\x00\x00" \
	"This is one.dat\0\0\0\0\0" \
	"This is two.dat"

#define testdata_resize_smaller \
	"\x0c\x00\x00\x00" \
	"\x16\x00\x00\x00" \
	"\x25\x00\x00\x00" \
	"This is on" \
	"This is two.dat"

#define testdata_resize_write \
	"\x0c\x00\x00\x00" \
	"\x23\x00\x00\x00" \
	"\x32\x00\x00\x00" \
	"Now resized to 23 chars" \
	"This is two.dat"

#define NO_FILENAMES 1
#define MAX_FILENAME_LEN  0

#define ARCHIVE_CLASS fmt_dat_sango
#define ARCHIVE_TYPE  "dat-sango"
#include "test-archive.hpp"

// Test some invalid formats to make sure they're not identified as valid
// archives.  Note that they can still be opened though (by 'force'), this
// only checks whether they look like valid files or not.

// The "c00" test has already been performed in test-archive.hpp to ensure the
// initial state is correctly identified as a valid archive.

ISINSTANCE_TEST(c01,
	"\x00\x00",
	DefinitelyNo
);

ISINSTANCE_TEST(c02,
	"\xff\x00\x00\x00" \
	"\x1b\x00\x00\x00" \
	"\x2a\x00\x00\x00" \
	"This is one.dat" \
	"This is two.dat",
	DefinitelyNo
);

ISINSTANCE_TEST(c03,
	"\x0c\x00\x00\x00" \
	"\xff\x00\x00\x00" \
	"\x2a\x00\x00\x00" \
	"This is one.dat" \
	"This is two.dat",
	DefinitelyNo
);

ISINSTANCE_TEST(c04,
	"\x0c\x00\x00\x00" \
	"\x1b\x00\x00\x00" \
	"\x2b\x00\x00\x00" \
	"This is one.dat" \
	"This is two.dat",
	DefinitelyNo
);
