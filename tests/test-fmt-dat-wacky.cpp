/*
 * test-fmt-dat-wacky.cpp - test code for DAT_WackyArchive class.
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

#define FILENAME1 "ONE.DAT"
#define FILENAME2 "TWO.DAT"
#define FILENAME3 "THREE.DAT"
#define FILENAME4 "FOUR.DAT"

#define testdata_initialstate \
	"\x02\x00" \
	"ONE.DAT\0\0\0\0\0\0\0"        "\x0f\x00\x00\x00" "\x2c\x00\x00\x00" \
	"TWO.DAT\0\0\0\0\0\0\0"        "\x0f\x00\x00\x00" "\x3b\x00\x00\x00" \
	"This is one.dat" \
	"This is two.dat"

#define testdata_rename \
	"\x02\x00" \
	"THREE.DAT\0\0\0\0\0"          "\x0f\x00\x00\x00" "\x2c\x00\x00\x00" \
	"TWO.DAT\0\0\0\0\0\0\0"        "\x0f\x00\x00\x00" "\x3b\x00\x00\x00" \
	"This is one.dat" \
	"This is two.dat"

#define testdata_insert_end \
	"\x03\x00" \
	"ONE.DAT\0\0\0\0\0\0\0"        "\x0f\x00\x00\x00" "\x42\x00\x00\x00" \
	"TWO.DAT\0\0\0\0\0\0\0"        "\x0f\x00\x00\x00" "\x51\x00\x00\x00" \
	"THREE.DAT\0\0\0\0\0"          "\x11\x00\x00\x00" "\x60\x00\x00\x00" \
	"This is one.dat" \
	"This is two.dat" \
	"This is three.dat"

#define testdata_insert_mid \
	"\x03\x00" \
	"ONE.DAT\0\0\0\0\0\0\0"        "\x0f\x00\x00\x00" "\x42\x00\x00\x00" \
	"THREE.DAT\0\0\0\0\0"          "\x11\x00\x00\x00" "\x51\x00\x00\x00" \
	"TWO.DAT\0\0\0\0\0\0\0"        "\x0f\x00\x00\x00" "\x62\x00\x00\x00" \
	"This is one.dat" \
	"This is three.dat" \
	"This is two.dat"

#define testdata_insert2 \
	"\x04\x00" \
	"ONE.DAT\0\0\0\0\0\0\0"        "\x0f\x00\x00\x00" "\x58\x00\x00\x00" \
	"THREE.DAT\0\0\0\0\0"          "\x11\x00\x00\x00" "\x67\x00\x00\x00" \
	"FOUR.DAT\0\0\0\0\0\0"         "\x10\x00\x00\x00" "\x78\x00\x00\x00" \
	"TWO.DAT\0\0\0\0\0\0\0"        "\x0f\x00\x00\x00" "\x88\x00\x00\x00" \
	"This is one.dat" \
	"This is three.dat" \
	"This is four.dat" \
	"This is two.dat"

#define testdata_remove \
	"\x01\x00" \
	"TWO.DAT\0\0\0\0\0\0\0"        "\x0f\x00\x00\x00" "\x16\x00\x00\x00" \
	"This is two.dat"

#define testdata_remove2 \
	"\x00\x00"

#define testdata_insert_remove \
	"\x02\x00" \
	"THREE.DAT\0\0\0\0\0"          "\x11\x00\x00\x00" "\x2c\x00\x00\x00" \
	"TWO.DAT\0\0\0\0\0\0\0"        "\x0f\x00\x00\x00" "\x3d\x00\x00\x00" \
	"This is three.dat" \
	"This is two.dat"

#define testdata_move \
	"\x02\x00" \
	"TWO.DAT\0\0\0\0\0\0\0"        "\x0f\x00\x00\x00" "\x2c\x00\x00\x00" \
	"ONE.DAT\0\0\0\0\0\0\0"        "\x0f\x00\x00\x00" "\x3b\x00\x00\x00" \
	"This is two.dat" \
	"This is one.dat"

#define testdata_resize_larger \
	"\x02\x00" \
	"ONE.DAT\0\0\0\0\0\0\0"        "\x14\x00\x00\x00" "\x2c\x00\x00\x00" \
	"TWO.DAT\0\0\0\0\0\0\0"        "\x0f\x00\x00\x00" "\x40\x00\x00\x00" \
	"This is one.dat\0\0\0\0\0" \
	"This is two.dat"

#define testdata_resize_smaller \
	"\x02\x00" \
	"ONE.DAT\0\0\0\0\0\0\0"        "\x0a\x00\x00\x00" "\x2c\x00\x00\x00" \
	"TWO.DAT\0\0\0\0\0\0\0"        "\x0f\x00\x00\x00" "\x36\x00\x00\x00" \
	"This is on" \
	"This is two.dat"

#define testdata_resize_write \
	"\x02\x00" \
	"ONE.DAT\0\0\0\0\0\0\0"        "\x17\x00\x00\x00" "\x2c\x00\x00\x00" \
	"TWO.DAT\0\0\0\0\0\0\0"        "\x0f\x00\x00\x00" "\x43\x00\x00\x00" \
	"Now resized to 23 chars" \
	"This is two.dat"

#define MAX_FILENAME_LEN  12

#define ARCHIVE_CLASS fmt_dat_wacky
#define ARCHIVE_TYPE  "dat-wacky"
#include "test-archive.hpp"

// Test some invalid formats to make sure they're not identified as valid
// archives.  Note that they can still be opened though (by 'force'), this
// only checks whether they look like valid files or not.

// The "c00" test has already been performed in test-archive.hpp to ensure the
// initial state is correctly identified as a valid archive.

ISINSTANCE_TEST(c01,
	"\x02\x00" \
	"ONE.DAT\x05\0\0\0\0\0\0"      "\x0f\x00\x00\x00" "\x2c\x00\x00\x00" \
	"TWO.DAT\0\0\0\0\0\0\0"        "\x0f\x00\x00\x00" "\x3b\x00\x00\x00" \
	"This is one.dat" \
	"This is two.dat",
	DefinitelyNo
);

ISINSTANCE_TEST(c02,
	"\x01",
	DefinitelyNo
);

ISINSTANCE_TEST(c03,
	"\x02\x00" \
	"ONE.DAT\0\0\0\0\0\0\0"        "\x0f\x01\x00\x00" "\x2c\x00\x00\x00" \
	"TWO.DAT\0\0\0\0\0\0\0"        "\x0f\x00\x00\x00" "\x3b\x00\x00\x00" \
	"This is one.dat" \
	"This is two.dat",
	DefinitelyNo
);

ISINSTANCE_TEST(c04,
	"\x00\x00" \
	"ONE.DAT\0\0\0\0\0\0\0"        "\x0f\x01\x00\x00" "\x2c\x00\x00\x00" \
	"TWO.DAT\0\0\0\0\0\0\0"        "\x0f\x00\x00\x00" "\x3b\x00\x00\x00" \
	"This is one.dat" \
	"This is two.dat",
	DefinitelyNo
);

// Not really possible to do any INVALIDDATA_TEST() tests here, because the
// worst that can happen is it looks like the archive has been truncated.
