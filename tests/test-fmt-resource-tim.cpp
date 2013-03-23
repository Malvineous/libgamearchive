/**
 * @file  test-fmt-resource-tim.cpp
 * @brief Test code for TIMResourceArchive class.
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
	"ONE.DAT\0\0\0\0\0\0" "\x0f\x00\x00\x00" "This is one.dat" \
	"TWO.DAT\0\0\0\0\0\0" "\x0f\x00\x00\x00" "This is two.dat"
#define testdata_FAT_initialstate \
	"\x00\x00" "\x00\x00" "\x00\x00\x00\x00" \
	"\x00\x00" "\x00\x00" "\x20\x00\x00\x00"

#define testdata_rename \
	"THREE.DAT\0\0\0\0"   "\x0f\x00\x00\x00" "This is one.dat" \
	"TWO.DAT\0\0\0\0\0\0" "\x0f\x00\x00\x00" "This is two.dat"
#define testdata_FAT_rename \
	"\x00\x00" "\x00\x00" "\x00\x00\x00\x00" \
	"\x00\x00" "\x00\x00" "\x20\x00\x00\x00"

#define testdata_insert_end \
	"ONE.DAT\0\0\0\0\0\0" "\x0f\x00\x00\x00" "This is one.dat" \
	"TWO.DAT\0\0\0\0\0\0" "\x0f\x00\x00\x00" "This is two.dat" \
	"THREE.DAT\0\0\0\0"   "\x11\x00\x00\x00" "This is three.dat"
#define testdata_FAT_insert_end \
	"\x00\x00" "\x00\x00" "\x00\x00\x00\x00" \
	"\x00\x00" "\x00\x00" "\x20\x00\x00\x00" \
	"\x00\x00" "\x00\x00" "\x40\x00\x00\x00"

#define testdata_insert_mid \
	"ONE.DAT\0\0\0\0\0\0" "\x0f\x00\x00\x00" "This is one.dat" \
	"THREE.DAT\0\0\0\0"   "\x11\x00\x00\x00" "This is three.dat" \
	"TWO.DAT\0\0\0\0\0\0" "\x0f\x00\x00\x00" "This is two.dat"
#define testdata_FAT_insert_mid \
	"\x00\x00" "\x00\x00" "\x00\x00\x00\x00" \
	"\x00\x00" "\x00\x00" "\x20\x00\x00\x00" \
	"\x00\x00" "\x00\x00" "\x42\x00\x00\x00"

#define testdata_insert2 \
	"ONE.DAT\0\0\0\0\0\0" "\x0f\x00\x00\x00" "This is one.dat" \
	"THREE.DAT\0\0\0\0"   "\x11\x00\x00\x00" "This is three.dat" \
	"FOUR.DAT\0\0\0\0\0"  "\x10\x00\x00\x00" "This is four.dat" \
	"TWO.DAT\0\0\0\0\0\0" "\x0f\x00\x00\x00" "This is two.dat"
#define testdata_FAT_insert2 \
	"\x00\x00" "\x00\x00" "\x00\x00\x00\x00" \
	"\x00\x00" "\x00\x00" "\x20\x00\x00\x00" \
	"\x00\x00" "\x00\x00" "\x42\x00\x00\x00" \
	"\x00\x00" "\x00\x00" "\x63\x00\x00\x00"

#define testdata_remove \
	"TWO.DAT\0\0\0\0\0\0" "\x0f\x00\x00\x00" "This is two.dat"
#define testdata_FAT_remove \
	"\x00\x00" "\x00\x00" "\x00\x00\x00\x00"

#define testdata_remove2 \
	""
#define testdata_FAT_remove2 \
	""

#define testdata_insert_remove \
	"THREE.DAT\0\0\0\0"   "\x11\x00\x00\x00" "This is three.dat" \
	"TWO.DAT\0\0\0\0\0\0" "\x0f\x00\x00\x00" "This is two.dat"
#define testdata_FAT_insert_remove \
	"\x00\x00" "\x00\x00" "\x00\x00\x00\x00" \
	"\x00\x00" "\x00\x00" "\x22\x00\x00\x00"

#define testdata_move \
	"TWO.DAT\0\0\0\0\0\0" "\x0f\x00\x00\x00" "This is two.dat" \
	"ONE.DAT\0\0\0\0\0\0" "\x0f\x00\x00\x00" "This is one.dat"
#define testdata_FAT_move \
	"\x00\x00" "\x00\x00" "\x00\x00\x00\x00" \
	"\x00\x00" "\x00\x00" "\x20\x00\x00\x00"

#define testdata_resize_larger \
	"ONE.DAT\0\0\0\0\0\0" "\x14\x00\x00\x00" "This is one.dat\0\0\0\0\0" \
	"TWO.DAT\0\0\0\0\0\0" "\x0f\x00\x00\x00" "This is two.dat"
#define testdata_FAT_resize_larger \
	"\x00\x00" "\x00\x00" "\x00\x00\x00\x00" \
	"\x00\x00" "\x00\x00" "\x25\x00\x00\x00" \

#define testdata_resize_smaller \
	"ONE.DAT\0\0\0\0\0\0" "\x0a\x00\x00\x00" "This is on" \
	"TWO.DAT\0\0\0\0\0\0" "\x0f\x00\x00\x00" "This is two.dat"
#define testdata_FAT_resize_smaller \
	"\x00\x00" "\x00\x00" "\x00\x00\x00\x00" \
	"\x00\x00" "\x00\x00" "\x1b\x00\x00\x00" \

#define testdata_resize_write \
	"ONE.DAT\0\0\0\0\0\0" "\x17\x00\x00\x00" "Now resized to 23 chars" \
	"TWO.DAT\0\0\0\0\0\0" "\x0f\x00\x00\x00" "This is two.dat"
#define testdata_FAT_resize_write \
	"\x00\x00" "\x00\x00" "\x00\x00\x00\x00" \
	"\x00\x00" "\x00\x00" "\x28\x00\x00\x00" \

#define MAX_FILENAME_LEN  12

// Use all the _FAT_xxx values
#define HAS_FAT

#define ARCHIVE_CLASS fmt_resource_tim
#define ARCHIVE_TYPE  "resource-tim"
#include "test-archive.hpp"

// Test some invalid formats to make sure they're not identified as valid
// archives.  Note that they can still be opened though (by 'force'), this
// only checks whether they look like valid files or not.

// The "c00" test has already been performed in test-archive.hpp to ensure the
// initial state is correctly identified as a valid archive.

// File too short
ISINSTANCE_TEST(c01,
	"ONE.DAT\0\0\0\0\0\0" "\x0f\x00"
	,
	DefinitelyNo
);

// File too large
ISINSTANCE_TEST(c02,
	"ONE.DAT\0\0\0\0\0\0" "\x1f\x00\x00\x00" "This is one.dat"
	"TWO.DAT\0\0\0\0\0\0" "\x0f\x00\x00\x00" "This is two.dat"
	,
	DefinitelyNo
);

// File truncated
ISINSTANCE_TEST(c03,
	"ONE.DAT\0\0\0\0\0\0" "\x1f\x00\x00\x00" "This is one.dat"
	"TWO.DAT\0\0\0\0\0\0" "\x0f\x00"
	,
	DefinitelyNo
);

// Not really possible to do any INVALIDDATA_TEST() tests here, because the
// worst that can happen is it looks like the archive has been truncated.
