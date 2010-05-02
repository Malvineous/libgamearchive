/*
 * test-fmt-grp-duke3d.cpp - test code for GRPArchive class.
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
	"KenSilverman"      "\x02\x00\x00\x00" \
	"ONE.DAT\0\0\0\0\0" "\x0f\x00\x00\x00" \
	"TWO.DAT\0\0\0\0\0" "\x0f\x00\x00\x00" \
	"This is one.dat" \
	"This is two.dat"

#define testdata_rename \
	"KenSilverman"      "\x02\x00\x00\x00" \
	"THREE.DAT\0\0\0"   "\x0f\x00\x00\x00" \
	"TWO.DAT\0\0\0\0\0" "\x0f\x00\x00\x00" \
	"This is one.dat" \
	"This is two.dat" \

#define testdata_insert_end \
	"KenSilverman"      "\x03\x00\x00\x00" \
	"ONE.DAT\0\0\0\0\0" "\x0f\x00\x00\x00" \
	"TWO.DAT\0\0\0\0\0" "\x0f\x00\x00\x00" \
	"THREE.DAT\0\0\0"   "\x11\x00\x00\x00" \
	"This is one.dat" \
	"This is two.dat" \
	"This is three.dat"

#define testdata_insert_mid \
	"KenSilverman"      "\x03\x00\x00\x00" \
	"ONE.DAT\0\0\0\0\0" "\x0f\x00\x00\x00" \
	"THREE.DAT\0\0\0"   "\x11\x00\x00\x00" \
	"TWO.DAT\0\0\0\0\0" "\x0f\x00\x00\x00" \
	"This is one.dat" \
	"This is three.dat" \
	"This is two.dat"

#define testdata_insert2 \
	"KenSilverman"      "\x04\x00\x00\x00" \
	"ONE.DAT\0\0\0\0\0" "\x0f\x00\x00\x00" \
	"THREE.DAT\0\0\0"   "\x11\x00\x00\x00" \
	"FOUR.DAT\0\0\0\0"  "\x10\x00\x00\x00" \
	"TWO.DAT\0\0\0\0\0" "\x0f\x00\x00\x00" \
	"This is one.dat" \
	"This is three.dat" \
	"This is four.dat" \
	"This is two.dat"

#define testdata_remove \
	"KenSilverman"      "\x01\x00\x00\x00" \
	"TWO.DAT\0\0\0\0\0" "\x0f\x00\x00\x00" \
	"This is two.dat"

#define testdata_remove2 \
	"KenSilverman"      "\x00\x00\x00\x00"

#define testdata_insert_remove \
	"KenSilverman"      "\x02\x00\x00\x00" \
	"THREE.DAT\0\0\0"   "\x11\x00\x00\x00" \
	"TWO.DAT\0\0\0\0\0" "\x0f\x00\x00\x00" \
	"This is three.dat" \
	"This is two.dat" \

#define testdata_remove_insert \
	"KenSilverman"      "\x02\x00\x00\x00" \
	"THREE.DAT\0\0\0"   "\x11\x00\x00\x00" \
	"TWO.DAT\0\0\0\0\0" "\x0f\x00\x00\x00" \
	"This is three.dat" \
	"This is two.dat"

#define testdata_move \
	"KenSilverman"      "\x02\x00\x00\x00" \
	"TWO.DAT\0\0\0\0\0" "\x0f\x00\x00\x00" \
	"ONE.DAT\0\0\0\0\0" "\x0f\x00\x00\x00" \
	"This is two.dat" \
	"This is one.dat"

#define testdata_resize_larger \
	"KenSilverman"      "\x02\x00\x00\x00" \
	"ONE.DAT\0\0\0\0\0" "\x14\x00\x00\x00" \
	"TWO.DAT\0\0\0\0\0" "\x0f\x00\x00\x00" \
	"This is one.dat\0\0\0\0\0" \
	"This is two.dat"

#define testdata_resize_smaller \
	"KenSilverman"      "\x02\x00\x00\x00" \
	"ONE.DAT\0\0\0\0\0" "\x0a\x00\x00\x00" \
	"TWO.DAT\0\0\0\0\0" "\x0f\x00\x00\x00" \
	"This is on" \
	"This is two.dat"

#define testdata_resize_write \
	"KenSilverman"      "\x02\x00\x00\x00" \
	"ONE.DAT\0\0\0\0\0" "\x17\x00\x00\x00" \
	"TWO.DAT\0\0\0\0\0" "\x0f\x00\x00\x00" \
	"Now resized to 23 chars" \
	"This is two.dat"

#define MAX_FILENAME_LEN  12

#define ARCHIVE_CLASS fmt_grp_duke3d
#define ARCHIVE_TYPE  "grp-duke3d"
#include "test-archive.hpp"

// Test some invalid formats to make sure they're not identified as valid
// archives.  Note that they can still be opened though (by 'force'), this
// only checks whether they look like valid files or not.

// The "c00" test has already been performed in test-archive.hpp to ensure the
// initial state is correctly identified as a valid archive.

ISINSTANCE_TEST(c01,
	"KenSliverman"      "\x02\x00\x00\x00"
	"ONE.DAT\0\0\0\0\0" "\x0f\x00\x00\x00"
	"TWO.DAT\0\0\0\0\0" "\x0f\x00\x00\x00"
	"This is one.dat"
	"This is two.dat",
	ga::EC_DEFINITELY_NO
);

ISINSTANCE_TEST(c02,
	"KenGoldman",
	ga::EC_DEFINITELY_NO
);

// Test some valid formats but with corrupted data, to make sure nothing
// strange happens.  These must be valid files (correct signature, so
// isInstance passes them) but with invalid content.
INVALIDDATA_TEST(c01,
	"KenSilverman"      "\xff\xff\xff\xf0" \
	"ONE.DAT\0\0\0\0\0" "\x0f\x00\x00\x00" \
	"This is one.dat"
);
