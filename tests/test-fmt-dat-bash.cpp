/*
 * test-fmt-dat-wacky.cpp - test code for DAT_WackyArchive class.
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

#define FILENAME1 "ONE.DAT"
#define FILENAME2 "TWO.DAT"
#define FILENAME3 "THREE.DAT"
#define FILENAME4 "FOUR.DAT"

#define testdata_initialstate \
	"\x20\x00" "\x0f\x00" \
		"ONE.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
		"\x00\x00" \
		"This is one.dat" \
	"\x20\x00" "\x0f\x00" \
		"TWO.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
		"\x00\x00" \
		"This is two.dat"

#define testdata_rename \
	"\x20\x00" "\x0f\x00" \
		"THREE.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
		"\x00\x00" \
		"This is one.dat" \
	"\x20\x00" "\x0f\x00" \
		"TWO.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
		"\x00\x00" \
		"This is two.dat"

#define testdata_insert_end \
	"\x20\x00" "\x0f\x00" \
		"ONE.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
		"\x00\x00" \
		"This is one.dat" \
	"\x20\x00" "\x0f\x00" \
		"TWO.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
		"\x00\x00" \
		"This is two.dat" \
	"\x20\x00" "\x11\x00" \
		"THREE.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
		"\x00\x00" \
		"This is three.dat"

#define testdata_insert_mid \
	"\x20\x00" "\x0f\x00" \
		"ONE.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
		"\x00\x00" \
		"This is one.dat" \
	"\x20\x00" "\x11\x00" \
		"THREE.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
		"\x00\x00" \
		"This is three.dat" \
	"\x20\x00" "\x0f\x00" \
		"TWO.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
		"\x00\x00" \
		"This is two.dat"

#define testdata_insert2 \
	"\x20\x00" "\x0f\x00" \
		"ONE.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
		"\x00\x00" \
		"This is one.dat" \
	"\x20\x00" "\x11\x00" \
		"THREE.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
		"\x00\x00" \
		"This is three.dat" \
	"\x20\x00" "\x10\x00" \
		"FOUR.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
		"\x00\x00" \
		"This is four.dat" \
	"\x20\x00" "\x0f\x00" \
		"TWO.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
		"\x00\x00" \
		"This is two.dat"

#define testdata_remove \
	"\x20\x00" "\x0f\x00" \
		"TWO.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
		"\x00\x00" \
		"This is two.dat"

#define testdata_remove2 \
	""

#define testdata_insert_remove \
	"\x20\x00" "\x11\x00" \
		"THREE.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
		"\x00\x00" \
		"This is three.dat" \
	"\x20\x00" "\x0f\x00" \
		"TWO.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
		"\x00\x00" \
		"This is two.dat"

#define testdata_move \
	"\x20\x00" "\x0f\x00" \
		"TWO.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
		"\x00\x00" \
		"This is two.dat" \
	"\x20\x00" "\x0f\x00" \
		"ONE.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
		"\x00\x00" \
		"This is one.dat"

#define testdata_resize_larger \
	"\x20\x00" "\x14\x00" \
		"ONE.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
		"\x00\x00" \
		"This is one.dat\0\0\0\0\0" \
	"\x20\x00" "\x0f\x00" \
		"TWO.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
		"\x00\x00" \
		"This is two.dat"

#define testdata_resize_smaller \
	"\x20\x00" "\x0a\x00" \
		"ONE.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
		"\x00\x00" \
		"This is on" \
	"\x20\x00" "\x0f\x00" \
		"TWO.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
		"\x00\x00" \
		"This is two.dat"

#define testdata_resize_write \
	"\x20\x00" "\x17\x00" \
		"ONE.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
		"\x00\x00" \
		"Now resized to 23 chars" \
	"\x20\x00" "\x0f\x00" \
		"TWO.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
		"\x00\x00" \
		"This is two.dat"

#define MAX_FILENAME_LEN  30

#define ARCHIVE_CLASS fmt_dat_bash
#define ARCHIVE_TYPE  "dat-bash"
#include "test-archive.hpp"

// Test some invalid formats to make sure they're not identified as valid
// archives.  Note that they can still be opened though (by 'force'), this
// only checks whether they look like valid files or not.

// The "c00" test has already been performed in test-archive.hpp to ensure the
// initial state is correctly identified as a valid archive.

ISINSTANCE_TEST(c01,
	"\x20\x00" "\x0f\x00" \
		"ONE.DAT\x05\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
		"\x00\x00" \
		"This is one.dat" \
	"\x20\x00" "\x0f\x00" \
		"TWO.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
		"\x00\x00" \
		"This is two.dat",
	ga::EC_DEFINITELY_NO
);

// Blank archive
ISINSTANCE_TEST(c02,
	"",
	ga::EC_DEFINITELY_YES
);

ISINSTANCE_TEST(c03,
	"\x20\x00" "\x0f\x01" \
		"ONE.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
		"\x00\x00" \
		"This is one.dat" \
	"\x20\x00" "\x0f\x00" \
		"TWO.DAT\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
		"\x00\x00" \
		"This is two.dat",
	ga::EC_DEFINITELY_NO
);
