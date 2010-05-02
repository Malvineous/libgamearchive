/*
 * test-fmt-res-stellar7.cpp - test code for RESArchiveFolder class.
 *
 * Copyright (C) 2010 Adam Nielsen <malvineous@shikadi.net>
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

#define FILENAME1 "ONE:"
#define FILENAME2 "TWO:"
#define FILENAME3 "THR:"
#define FILENAME4 "FOU:"

#define testdata_initialstate \
	"ONE:" "\x0f\x00\x00\x00" \
	"This is one.dat" \
	"TWO:" "\x0f\x00\x00\x00" \
	"This is two.dat"

#define testdata_rename \
	"THR:" "\x0f\x00\x00\x00" \
	"This is one.dat" \
	"TWO:" "\x0f\x00\x00\x00" \
	"This is two.dat"

#define testdata_insert_end \
	"ONE:" "\x0f\x00\x00\x00" \
	"This is one.dat" \
	"TWO:" "\x0f\x00\x00\x00" \
	"This is two.dat" \
	"THR:" "\x11\x00\x00\x00" \
	"This is three.dat"

#define testdata_insert_mid \
	"ONE:" "\x0f\x00\x00\x00" \
	"This is one.dat" \
	"THR:" "\x11\x00\x00\x00" \
	"This is three.dat" \
	"TWO:" "\x0f\x00\x00\x00" \
	"This is two.dat"

#define testdata_insert2 \
	"ONE:" "\x0f\x00\x00\x00" \
	"This is one.dat" \
	"THR:" "\x11\x00\x00\x00" \
	"This is three.dat" \
	"FOU:" "\x10\x00\x00\x00" \
	"This is four.dat" \
	"TWO:" "\x0f\x00\x00\x00" \
	"This is two.dat"

#define testdata_remove \
	"TWO:" "\x0f\x00\x00\x00" \
	"This is two.dat"

#define testdata_remove2 \
	""

#define testdata_insert_remove \
	"THR:" "\x11\x00\x00\x00" \
	"This is three.dat" \
	"TWO:" "\x0f\x00\x00\x00" \
	"This is two.dat"

#define testdata_remove_insert \
	"THR:" "\x11\x00\x00\x00" \
	"This is three.dat" \
	"TWO:" "\x0f\x00\x00\x00" \
	"This is two.dat"

#define testdata_move \
	"TWO:" "\x0f\x00\x00\x00" \
	"This is two.dat" \
	"ONE:" "\x0f\x00\x00\x00" \
	"This is one.dat"

#define testdata_resize_larger \
	"ONE:" "\x14\x00\x00\x00" \
	"This is one.dat\0\0\0\0\0" \
	"TWO:" "\x0f\x00\x00\x00" \
	"This is two.dat"

#define testdata_resize_smaller \
	"ONE:" "\x0a\x00\x00\x00" \
	"This is on" \
	"TWO:" "\x0f\x00\x00\x00" \
	"This is two.dat"

#define testdata_resize_write \
	"ONE:" "\x17\x00\x00\x00" \
	"Now resized to 23 chars" \
	"TWO:" "\x0f\x00\x00\x00" \
	"This is two.dat"

#define MAX_FILENAME_LEN  4

#define ARCHIVE_CLASS fmt_res_stellar7
#define ARCHIVE_TYPE  "res-stellar7"
#include "test-archive.hpp"

// Test some invalid formats to make sure they're not identified as valid
// archives.  Note that they can still be opened though (by 'force'), this
// only checks whether they look like valid files or not.

// The "c00" test has already been performed in test-archive.hpp to ensure the
// initial state is correctly identified as a valid archive.

ISINSTANCE_TEST(c01,
	"\x5NE:" "\x0f\x00\x00\x00"
	"This is one.dat"
	"TWO:" "\x0f\x00\x00\x00"
	"This is two.dat",
	ga::EC_DEFINITELY_NO
);

ISINSTANCE_TEST(c02,
	"ONE:" "\xef\x00\x00\x00"
	"This is one.dat"
	"TWO:" "\x0f\x00\x00\x00"
	"This is two.dat",
	ga::EC_DEFINITELY_NO
);

// Not really possible to do any INVALIDDATA_TEST() tests here, because the
// worst that can happen is it looks like the archive has been truncated.
