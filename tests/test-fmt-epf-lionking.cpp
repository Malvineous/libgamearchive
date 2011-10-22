/*
 * test-fmt-epf-lionking.cpp - test code for EPFArchive class.
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
	"EPFS"      "\x33\x00\x00\x00" "\x00" "\x02\x00" \
	"This is one.dat" \
	"This is two.dat" \
	"Extra data" \
	"ONE.DAT\0\0\0\0\0\0" "\x00" "\x0f\x00\x00\x00" "\x0f\x00\x00\x00" \
	"TWO.DAT\0\0\0\0\0\0" "\x00" "\x0f\x00\x00\x00" "\x0f\x00\x00\x00"

#define testdata_rename \
	"EPFS"      "\x33\x00\x00\x00" "\x00" "\x02\x00" \
	"This is one.dat" \
	"This is two.dat" \
	"Extra data" \
	"THREE.DAT\0\0\0\0"   "\x00" "\x0f\x00\x00\x00" "\x0f\x00\x00\x00" \
	"TWO.DAT\0\0\0\0\0\0" "\x00" "\x0f\x00\x00\x00" "\x0f\x00\x00\x00"

#define testdata_insert_end \
	"EPFS"      "\x44\x00\x00\x00" "\x00" "\x03\x00" \
	"This is one.dat" \
	"This is two.dat" \
	"This is three.dat" \
	"Extra data" \
	"ONE.DAT\0\0\0\0\0\0" "\x00" "\x0f\x00\x00\x00" "\x0f\x00\x00\x00" \
	"TWO.DAT\0\0\0\0\0\0" "\x00" "\x0f\x00\x00\x00" "\x0f\x00\x00\x00" \
	"THREE.DAT\0\0\0\0"   "\x00" "\x11\x00\x00\x00" "\x11\x00\x00\x00"

#define testdata_insert_mid \
	"EPFS"      "\x44\x00\x00\x00" "\x00" "\x03\x00" \
	"This is one.dat" \
	"This is three.dat" \
	"This is two.dat" \
	"Extra data" \
	"ONE.DAT\0\0\0\0\0\0" "\x00" "\x0f\x00\x00\x00" "\x0f\x00\x00\x00" \
	"THREE.DAT\0\0\0\0"   "\x00" "\x11\x00\x00\x00" "\x11\x00\x00\x00" \
	"TWO.DAT\0\0\0\0\0\0" "\x00" "\x0f\x00\x00\x00" "\x0f\x00\x00\x00"

#define testdata_insert2 \
	"EPFS"      "\x54\x00\x00\x00" "\x00" "\x04\x00" \
	"This is one.dat" \
	"This is three.dat" \
	"This is four.dat" \
	"This is two.dat" \
	"Extra data" \
	"ONE.DAT\0\0\0\0\0\0" "\x00" "\x0f\x00\x00\x00" "\x0f\x00\x00\x00" \
	"THREE.DAT\0\0\0\0"   "\x00" "\x11\x00\x00\x00" "\x11\x00\x00\x00" \
	"FOUR.DAT\0\0\0\0\0"  "\x00" "\x10\x00\x00\x00" "\x10\x00\x00\x00" \
	"TWO.DAT\0\0\0\0\0\0" "\x00" "\x0f\x00\x00\x00" "\x0f\x00\x00\x00"

#define testdata_remove \
	"EPFS"      "\x24\x00\x00\x00" "\x00" "\x01\x00" \
	"This is two.dat" \
	"Extra data" \
	"TWO.DAT\0\0\0\0\0\0" "\x00" "\x0f\x00\x00\x00" "\x0f\x00\x00\x00"

#define testdata_remove2 \
	"EPFS"      "\x15\x00\x00\x00" "\x00" "\x00\x00" \
	"Extra data"

#define testdata_insert_remove \
	"EPFS"      "\x35\x00\x00\x00" "\x00" "\x02\x00" \
	"This is three.dat" \
	"This is two.dat" \
	"Extra data" \
	"THREE.DAT\0\0\0\0"   "\x00" "\x11\x00\x00\x00" "\x11\x00\x00\x00" \
	"TWO.DAT\0\0\0\0\0\0" "\x00" "\x0f\x00\x00\x00" "\x0f\x00\x00\x00"

#define testdata_move \
	"EPFS"      "\x33\x00\x00\x00" "\x00" "\x02\x00" \
	"This is two.dat" \
	"This is one.dat" \
	"Extra data" \
	"TWO.DAT\0\0\0\0\0\0" "\x00" "\x0f\x00\x00\x00" "\x0f\x00\x00\x00" \
	"ONE.DAT\0\0\0\0\0\0" "\x00" "\x0f\x00\x00\x00" "\x0f\x00\x00\x00"

#define testdata_resize_larger \
	"EPFS"      "\x38\x00\x00\x00" "\x00" "\x02\x00" \
	"This is one.dat\0\0\0\0\0" \
	"This is two.dat" \
	"Extra data" \
	"ONE.DAT\0\0\0\0\0\0" "\x00" "\x14\x00\x00\x00" "\x14\x00\x00\x00" \
	"TWO.DAT\0\0\0\0\0\0" "\x00" "\x0f\x00\x00\x00" "\x0f\x00\x00\x00"

#define testdata_resize_smaller \
	"EPFS"      "\x2e\x00\x00\x00" "\x00" "\x02\x00" \
	"This is on" \
	"This is two.dat" \
	"Extra data" \
	"ONE.DAT\0\0\0\0\0\0" "\x00" "\x0a\x00\x00\x00" "\x0a\x00\x00\x00" \
	"TWO.DAT\0\0\0\0\0\0" "\x00" "\x0f\x00\x00\x00" "\x0f\x00\x00\x00"

#define testdata_resize_write \
	"EPFS"      "\x3b\x00\x00\x00" "\x00" "\x02\x00" \
	"Now resized to 23 chars" \
	"This is two.dat" \
	"Extra data" \
	"ONE.DAT\0\0\0\0\0\0" "\x00" "\x17\x00\x00\x00" "\x17\x00\x00\x00" \
	"TWO.DAT\0\0\0\0\0\0" "\x00" "\x0f\x00\x00\x00" "\x0f\x00\x00\x00"

#define testdata_get_metadata_description \
	"Extra data"

#define testdata_set_metadata_description_target_larger \
	"This is a test"

#define testdata_set_metadata_description_larger \
	"EPFS"      "\x37\x00\x00\x00" "\x00" "\x02\x00" \
	"This is one.dat" \
	"This is two.dat" \
	"This is a test" \
	"ONE.DAT\0\0\0\0\0\0" "\x00" "\x0f\x00\x00\x00" "\x0f\x00\x00\x00" \
	"TWO.DAT\0\0\0\0\0\0" "\x00" "\x0f\x00\x00\x00" "\x0f\x00\x00\x00"

#define testdata_set_metadata_description_target_smaller \
	"Hello"

#define testdata_set_metadata_description_smaller \
	"EPFS"      "\x2e\x00\x00\x00" "\x00" "\x02\x00" \
	"This is one.dat" \
	"This is two.dat" \
	"Hello" \
	"ONE.DAT\0\0\0\0\0\0" "\x00" "\x0f\x00\x00\x00" "\x0f\x00\x00\x00" \
	"TWO.DAT\0\0\0\0\0\0" "\x00" "\x0f\x00\x00\x00" "\x0f\x00\x00\x00"

#define MAX_FILENAME_LEN  12

#define ARCHIVE_CLASS fmt_epf_lionking
#define ARCHIVE_TYPE  "epf-lionking"
#include "test-archive.hpp"

// Test some invalid formats to make sure they're not identified as valid
// archives.  Note that they can still be opened though (by 'force'), this
// only checks whether they look like valid files or not.

// The "c00" test has already been performed in test-archive.hpp to ensure the
// initial state is correctly identified as a valid archive.

ISINSTANCE_TEST(c01,
	"EPSF"      "\x33\x00\x00\x00" "\x00" "\x02\x00"
	"This is one.dat"
	"This is two.dat"
	"Extra data"
	"ONE.DAT\0\0\0\0\0\0" "\x00" "\x0f\x00\x00\x00" "\x0f\x00\x00\x00"
	"TWO.DAT\0\0\0\0\0\0" "\x00" "\x0f\x00\x00\x00" "\x0f\x00\x00\x00",
	DefinitelyNo
);

ISINSTANCE_TEST(c02,
	"EPF",
	DefinitelyNo
);

// Test some valid formats but with corrupted data, to make sure nothing
// strange happens.  These must be valid files (correct signature, so
// isInstance passes them) but with invalid content.
INVALIDDATA_TEST(c01,
	"EPFS"      "\x33\x00\x00\xf0" "\x00" "\x02\x00"
	"This is one.dat"
	"This is two.dat"
	"Extra data"
	"ONE.DAT\0\0\0\0\0\0" "\x00" "\x0f\x00\x00\x00" "\x0f\x00\x00\x00"
	"TWO.DAT\0\0\0\0\0\0" "\x00" "\x0f\x00\x00\x00" "\x0f\x00\x00\x00"
);

// Large enough to cause the uint32_t value to wrap
INVALIDDATA_TEST(c02,
	"EPFS"      "\xf0\xff\xff\xff" "\x00" "\x02\x00"
	"This is one.dat"
	"This is two.dat"
	"Extra data"
	"ONE.DAT\0\0\0\0\0\0" "\x00" "\x0f\x00\x00\x00" "\x0f\x00\x00\x00"
	"TWO.DAT\0\0\0\0\0\0" "\x00" "\x0f\x00\x00\x00" "\x0f\x00\x00\x00"
);
