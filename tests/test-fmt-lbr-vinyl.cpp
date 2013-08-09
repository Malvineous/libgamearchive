/*
 * test-fmt-lbr-vinyl.cpp - test code for LBRArchive class.
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
	"\x7c\xff" "\x0e\x00\x00\x00" \
	"\xe0\x97" "\x1d\x00\x00\x00" \
	"This is one.dat" \
	"This is two.dat"

#define testdata_rename \
	"\x02\x00" \
	"\x6d\x99" "\x0e\x00\x00\x00" \
	"\xe0\x97" "\x1d\x00\x00\x00" \
	"This is one.dat" \
	"This is two.dat"

#define testdata_insert_end \
	"\x03\x00" \
	"\x7c\xff" "\x14\x00\x00\x00" \
	"\xe0\x97" "\x23\x00\x00\x00" \
	"\x6d\x99" "\x32\x00\x00\x00" \
	"This is one.dat" \
	"This is two.dat" \
	"This is three.dat"

#define testdata_insert_mid \
	"\x03\x00" \
	"\x7c\xff" "\x14\x00\x00\x00" \
	"\x6d\x99" "\x23\x00\x00\x00" \
	"\xe0\x97" "\x34\x00\x00\x00" \
	"This is one.dat" \
	"This is three.dat" \
	"This is two.dat"

#define testdata_insert2 \
	"\x04\x00" \
	"\x7c\xff" "\x1a\x00\x00\x00" \
	"\x6d\x99" "\x29\x00\x00\x00" \
	"\xcf\x33" "\x3a\x00\x00\x00" \
	"\xe0\x97" "\x4a\x00\x00\x00" \
	"This is one.dat" \
	"This is three.dat" \
	"This is four.dat" \
	"This is two.dat"

#define testdata_remove \
	"\x01\x00" \
	"\xe0\x97" "\x08\x00\x00\x00" \
	"This is two.dat"

#define testdata_remove2 \
	"\x00\x00"

#define testdata_insert_remove \
	"\x02\x00" \
	"\x6d\x99" "\x0e\x00\x00\x00" \
	"\xe0\x97" "\x1f\x00\x00\x00" \
	"This is three.dat" \
	"This is two.dat"

#define testdata_move \
	"\x02\x00" \
	"\xe0\x97" "\x0e\x00\x00\x00" \
	"\x7c\xff" "\x1d\x00\x00\x00" \
	"This is two.dat" \
	"This is one.dat"

#define testdata_resize_larger \
	"\x02\x00" \
	"\x7c\xff" "\x0e\x00\x00\x00" \
	"\xe0\x97" "\x22\x00\x00\x00" \
	"This is one.dat\0\0\0\0\0" \
	"This is two.dat"

#define testdata_resize_smaller \
	"\x02\x00" \
	"\x7c\xff" "\x0e\x00\x00\x00" \
	"\xe0\x97" "\x18\x00\x00\x00" \
	"This is on" \
	"This is two.dat"

#define testdata_resize_write \
	"\x02\x00" \
	"\x7c\xff" "\x0e\x00\x00\x00" \
	"\xe0\x97" "\x25\x00\x00\x00" \
	"Now resized to 23 chars" \
	"This is two.dat"

#define MAX_FILENAME_LEN  0

#define ARCHIVE_CLASS fmt_lbr_vinyl
#define ARCHIVE_TYPE  "lbr-vinyl"
#include "test-archive.hpp"

// Test some invalid formats to make sure they're not identified as valid
// archives.  Note that they can still be opened though (by 'force'), this
// only checks whether they look like valid files or not.

// The "c00" test has already been performed in test-archive.hpp to ensure the
// initial state is correctly identified as a valid archive.

ISINSTANCE_TEST(c01,
	"\x00",
	DefinitelyNo
);

ISINSTANCE_TEST(c02,
	"\x02\x00"
	"\x00\x00" "\x0e\x00\x00\x00"
	"\x00\x00" "\xff\x00\x00\x00"
	"This is one.dat"
	"This is two.dat",
	DefinitelyNo
);

// Handle truncated FAT
ISINSTANCE_TEST(c03,
	"\x02\x00"
	"\x00\x00" "\x0e\x00\x00\x00"
	,
	DefinitelyNo
);

// Offset is inside FAT
ISINSTANCE_TEST(c04,
	"\x02\x00"
	"\x00\x00" "\x0e\x00\x00\x00"
	"\x00\x00" "\x02\x00\x00\x00"
	"This is one.dat"
	"This is two.dat"
	,
	DefinitelyNo
);

// No files but trailing data (which would be data for the first file)
ISINSTANCE_TEST(c05,
	"\x00\x00"
	"\x00\x00" "\x0e\x00\x00\x00"
	"\x00\x00" "\x1d\x00\x00\x00"
	"This is one.dat"
	"This is two.dat"
	,
	DefinitelyNo
);
