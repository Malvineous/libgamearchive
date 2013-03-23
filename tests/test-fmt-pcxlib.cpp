/*
 * test-fmt-pcxlib.cpp - test code for PCXLibArchive class.
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
#define FILENAME2 "TWO.DA"
#define FILENAME3 "THREE.D"
#define FILENAME4 "FOUR.DAT"


// 94-byte header
#define VER "\x01\xCA"
#define HEADER \
	"Copyright (c) Genus Microprogramming, Inc. 1988-90" \
	"\x00\x00\x00\x00\x00\x00\x00\x00" \
	"\x00\x00\x00\x00\x00\x00\x00\x00" \
	"\x00\x00\x00\x00\x00\x00\x00\x00" \
	"\x00\x00\x00\x00\x00\x00\x00\x00" \
	"\x00\x00\x00\x00\x00\x00\x00\x00" \
	"\x00\x00"

// 32-byte header end
#define TRAIL \
	"\x00\x00\x00\x00\x00\x00\x00\x00" \
	"\x00\x00\x00\x00\x00\x00\x00\x00" \
	"\x00\x00\x00\x00\x00\x00\x00\x00" \
	"\x00\x00\x00\x00\x00\x00\x00\x00"

// Header + file count + trail = 128 bytes
// FAT entry is 26 bytes

#define testdata_initialstate \
	VER HEADER "\x02\x00" TRAIL \
	"\x00" "ONE     .DAT\0" "\xb4\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00" \
	"\x00" "TWO     .DA \0" "\xc3\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00" \
	"This is one.dat" \
	"This is two.dat"

#define testdata_rename \
	VER HEADER "\x02\x00" TRAIL \
	"\x00" "THREE   .D  \0" "\xb4\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00" \
	"\x00" "TWO     .DA \0" "\xc3\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00" \
	"This is one.dat" \
	"This is two.dat"

#define testdata_insert_end \
	VER HEADER "\x03\x00" TRAIL \
	"\x00" "ONE     .DAT\0" "\xce\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00" \
	"\x00" "TWO     .DA \0" "\xdd\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00" \
	"\x00" "THREE   .D  \0" "\xec\x00\x00\x00" "\x11\x00\x00\x00" "\x00\x00" "\x00\x00" \
	"This is one.dat" \
	"This is two.dat" \
	"This is three.dat"

#define testdata_insert_mid \
	VER HEADER "\x03\x00" TRAIL \
	"\x00" "ONE     .DAT\0" "\xce\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00" \
	"\x00" "THREE   .D  \0" "\xdd\x00\x00\x00" "\x11\x00\x00\x00" "\x00\x00" "\x00\x00" \
	"\x00" "TWO     .DA \0" "\xee\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00" \
	"This is one.dat" \
	"This is three.dat" \
	"This is two.dat"

#define testdata_insert2 \
	VER HEADER "\x04\x00" TRAIL \
	"\x00" "ONE     .DAT\0" "\xe8\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00" \
	"\x00" "THREE   .D  \0" "\xf7\x00\x00\x00" "\x11\x00\x00\x00" "\x00\x00" "\x00\x00" \
	"\x00" "FOUR    .DAT\0" "\x08\x01\x00\x00" "\x10\x00\x00\x00" "\x00\x00" "\x00\x00" \
	"\x00" "TWO     .DA \0" "\x18\x01\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00" \
	"This is one.dat" \
	"This is three.dat" \
	"This is four.dat" \
	"This is two.dat"

#define testdata_remove \
	VER HEADER "\x01\x00" TRAIL \
	"\x00" "TWO     .DA \0" "\x9a\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00" \
	"This is two.dat"

#define testdata_remove2 \
	VER HEADER "\x00\x00" TRAIL \

#define testdata_insert_remove \
	VER HEADER "\x02\x00" TRAIL \
	"\x00" "THREE   .D  \0" "\xb4\x00\x00\x00" "\x11\x00\x00\x00" "\x00\x00" "\x00\x00" \
	"\x00" "TWO     .DA \0" "\xc5\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00" \
	"This is three.dat" \
	"This is two.dat"

#define testdata_move \
	VER HEADER "\x02\x00" TRAIL \
	"\x00" "TWO     .DA \0" "\xb4\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00" \
	"\x00" "ONE     .DAT\0" "\xc3\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00" \
	"This is two.dat" \
	"This is one.dat"

#define testdata_resize_larger \
	VER HEADER "\x02\x00" TRAIL \
	"\x00" "ONE     .DAT\0" "\xb4\x00\x00\x00" "\x14\x00\x00\x00" "\x00\x00" "\x00\x00" \
	"\x00" "TWO     .DA \0" "\xc8\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00" \
	"This is one.dat\0\0\0\0\0" \
	"This is two.dat"

#define testdata_resize_smaller \
	VER HEADER "\x02\x00" TRAIL \
	"\x00" "ONE     .DAT\0" "\xb4\x00\x00\x00" "\x0a\x00\x00\x00" "\x00\x00" "\x00\x00" \
	"\x00" "TWO     .DA \0" "\xbe\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00" \
	"This is on" \
	"This is two.dat"

#define testdata_resize_write \
	VER HEADER "\x02\x00" TRAIL \
	"\x00" "ONE     .DAT\0" "\xb4\x00\x00\x00" "\x17\x00\x00\x00" "\x00\x00" "\x00\x00" \
	"\x00" "TWO     .DA \0" "\xcb\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00" \
	"Now resized to 23 chars" \
	"This is two.dat"

#define MAX_FILENAME_LEN  12

#define ARCHIVE_CLASS fmt_pcxlib
#define ARCHIVE_TYPE  "pcxlib"
#include "test-archive.hpp"

// Test some invalid formats to make sure they're not identified as valid
// archives.  Note that they can still be opened though (by 'force'), this
// only checks whether they look like valid files or not.

// The "c00" test has already been performed in test-archive.hpp to ensure the
// initial state is correctly identified as a valid archive.

ISINSTANCE_TEST(c01,
	VER HEADER "\x00\x00",
	DefinitelyNo
);

ISINSTANCE_TEST(c02,
	"\xff\xff" HEADER "\x02\x00" TRAIL
	"\x00" "ONE     .DAT\0" "\xd6\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00"
	"\x00" "TWO     .DA \0" "\xe5\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00"
	"This is one.dat"
	"This is two.dat",
	DefinitelyNo
);

ISINSTANCE_TEST(c03,
	VER HEADER "\x02\x00" TRAIL
	"\x00" "ONE     .DAT\0" "\xd6\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00",
	DefinitelyNo
);

ISINSTANCE_TEST(c04,
	VER HEADER "\x02\x00" TRAIL
	"\x00" "ONE     .DAT\0" "\xd6\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00"
	"\x00" "TWO     .DA \0" "\xe5\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00"
	"This is one.dat"
	"This is two.dat",
	DefinitelyNo
);

ISINSTANCE_TEST(c05,
	VER HEADER "\x02\x00" TRAIL
	"\x00" "ONE     .DAT\0" "\xd6\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00"
	"\x00" "TWO      DA \0" "\xe5\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00"
	"This is one.dat"
	"This is two.dat",
	DefinitelyNo
);

ISINSTANCE_TEST(c06,
	VER HEADER "\x02\x00" TRAIL
	"\x00" "ONE     .DAT\0" "\xd6\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00"
	"\x00" "TWO     .DA \0" "\x05\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00"
	"This is one.dat"
	"This is two.dat",
	DefinitelyNo
);

ISINSTANCE_TEST(c07,
	VER HEADER "\x02\x00" TRAIL
	"\x00" "ONE     .DAT\0" "\xd6\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00" "\x00\x00"
	"\x00" "TWO     .DA \0" "\xe5\x00\x00\x00" "\xff\x00\x00\x00" "\x00\x00" "\x00\x00"
	"This is one.dat"
	"This is two.dat",
	DefinitelyNo
);

// Not really possible to do any INVALIDDATA_TEST() tests here, because the
// worst that can happen is it looks like the archive has been truncated.
