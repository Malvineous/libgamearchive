/*
 * test-fmt-hog-descent.cpp - test code for HOGArchive class.
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
	"DHF" \
	"ONE.DAT\0\0\0\0\0\0" "\x0f\x00\x00\x00" \
	"This is one.dat" \
	"TWO.DAT\0\0\0\0\0\0" "\x0f\x00\x00\x00" \
	"This is two.dat"

// This must be a valid file (correct signature) but with invalid content
/*#define testdata_invalidcontent \
	"DHF" \
	"ONE.DAT\0\0\0\0\0\0" "\x0f\x00\x00\x00" \
	"This is one.dat" \
	"TWO.DAT\0\0\0\0\0\0" "\x0f\x00\x00\x00" \
	"This is two.dat"*/

#define testdata_rename \
	"DHF" \
	"THREE.DAT\0\0\0\0"   "\x0f\x00\x00\x00" \
	"This is one.dat" \
	"TWO.DAT\0\0\0\0\0\0" "\x0f\x00\x00\x00" \
	"This is two.dat"

#define testdata_insert_end \
	"DHF" \
	"ONE.DAT\0\0\0\0\0\0" "\x0f\x00\x00\x00" \
	"This is one.dat" \
	"TWO.DAT\0\0\0\0\0\0" "\x0f\x00\x00\x00" \
	"This is two.dat" \
	"THREE.DAT\0\0\0\0"   "\x11\x00\x00\x00" \
	"This is three.dat"

#define testdata_insert_mid \
	"DHF" \
	"ONE.DAT\0\0\0\0\0\0" "\x0f\x00\x00\x00" \
	"This is one.dat" \
	"THREE.DAT\0\0\0\0"   "\x11\x00\x00\x00" \
	"This is three.dat" \
	"TWO.DAT\0\0\0\0\0\0" "\x0f\x00\x00\x00" \
	"This is two.dat"

#define testdata_insert2 \
	"DHF" \
	"ONE.DAT\0\0\0\0\0\0" "\x0f\x00\x00\x00" \
	"This is one.dat" \
	"THREE.DAT\0\0\0\0"   "\x11\x00\x00\x00" \
	"This is three.dat" \
	"FOUR.DAT\0\0\0\0\0"  "\x10\x00\x00\x00" \
	"This is four.dat" \
	"TWO.DAT\0\0\0\0\0\0" "\x0f\x00\x00\x00" \
	"This is two.dat"

#define testdata_remove \
	"DHF" \
	"TWO.DAT\0\0\0\0\0\0" "\x0f\x00\x00\x00" \
	"This is two.dat"

#define testdata_remove2 \
	"DHF"

#define testdata_insert_remove \
	"DHF" \
	"THREE.DAT\0\0\0\0"   "\x11\x00\x00\x00" \
	"This is three.dat" \
	"TWO.DAT\0\0\0\0\0\0" "\x0f\x00\x00\x00" \
	"This is two.dat"

#define testdata_remove_insert \
	"DHF" \
	"THREE.DAT\0\0\0\0"   "\x11\x00\x00\x00" \
	"This is three.dat" \
	"TWO.DAT\0\0\0\0\0\0" "\x0f\x00\x00\x00" \
	"This is two.dat"

#define testdata_move \
	"DHF" \
	"TWO.DAT\0\0\0\0\0\0" "\x0f\x00\x00\x00" \
	"This is two.dat" \
	"ONE.DAT\0\0\0\0\0\0" "\x0f\x00\x00\x00" \
	"This is one.dat" \

#define testdata_resize_larger \
	"DHF" \
	"ONE.DAT\0\0\0\0\0\0" "\x14\x00\x00\x00" \
	"This is one.dat\0\0\0\0\0" \
	"TWO.DAT\0\0\0\0\0\0" "\x0f\x00\x00\x00" \
	"This is two.dat"

#define testdata_resize_smaller \
	"DHF" \
	"ONE.DAT\0\0\0\0\0\0" "\x0a\x00\x00\x00" \
	"This is on" \
	"TWO.DAT\0\0\0\0\0\0" "\x0f\x00\x00\x00" \
	"This is two.dat"

#define testdata_resize_write \
	"DHF" \
	"ONE.DAT\0\0\0\0\0\0" "\x17\x00\x00\x00" \
	"Now resized to 23 chars" \
	"TWO.DAT\0\0\0\0\0\0" "\x0f\x00\x00\x00" \
	"This is two.dat"

#define MAX_FILENAME_LEN  12

#define ARCHIVE_CLASS fmt_hog_descent
#define ARCHIVE_TYPE  "hog-descent"
#include "test-archive.hpp"

// Test some invalid formats to make sure they're not identified as valid
// archives.  Note that they can still be opened though (by 'force'), this
// only checks whether they look like valid files or not.

// The "c00" test has already been performed in test-archive.hpp to ensure the
// initial state is correctly identified as a valid archive.

ISINSTANCE_TEST(c01,
	"DHL"
	"ONE.DAT\0\0\0\0\0\0" "\x0f\x00\x00\x00"
	"This is one.dat"
	"TWO.DAT\0\0\0\0\0\0" "\x0f\x00\x00\x00"
	"This is two.dat",
	ga::EC_DEFINITELY_NO
);

ISINSTANCE_TEST(c02,
	"DH",
	ga::EC_DEFINITELY_NO
);
