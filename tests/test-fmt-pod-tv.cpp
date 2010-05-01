/*
 * test-fmt-pod-tv.cpp - test code for PODArchive class.
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

#define POD_DESC \
	"Startup 1.1 Gold" \
	"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
	"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
	"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
	"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"

#define POD_DESC2_larger \
	"This is a test\0\0" \
	"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
	"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
	"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
	"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"

#define POD_DESC2_smaller \
	"Hello\0\0\0\0\0\0\0\0\0\0\0" \
	"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
	"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
	"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
	"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"

#define testdata_initialstate \
	"\x02\x00\x00\x00" POD_DESC \
	"ONE.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\xa4\x00\x00\x00" \
	"TWO.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\xb3\x00\x00\x00" \
	"This is one.dat" \
	"This is two.dat"

// This must be a valid file (correct signature) but with invalid content.
#define testdata_invalidcontent \
	"\x00\x00\x00\xf0" POD_DESC \
	"ONE.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\xa4\x00\x00\x00" \
	"TWO.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\xb3\x00\x00\x00" \
	"This is one.dat" \
	"This is two.dat"

#define testdata_rename \
	"\x02\x00\x00\x00" POD_DESC \
	"THREE.DAT\0\0\0\0\0\0\0"   "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\xa4\x00\x00\x00" \
	"TWO.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\xb3\x00\x00\x00" \
	"This is one.dat" \
	"This is two.dat"

#define testdata_insert_end \
	"\x03\x00\x00\x00" POD_DESC \
	"ONE.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\xcc\x00\x00\x00" \
	"TWO.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\xdb\x00\x00\x00" \
	"THREE.DAT\0\0\0\0\0\0\0"   "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x11\x00\x00\x00" "\xea\x00\x00\x00" \
	"This is one.dat" \
	"This is two.dat" \
	"This is three.dat"

#define testdata_insert_mid \
	"\x03\x00\x00\x00" POD_DESC \
	"ONE.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\xcc\x00\x00\x00" \
	"THREE.DAT\0\0\0\0\0\0\0"   "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x11\x00\x00\x00" "\xdb\x00\x00\x00" \
	"TWO.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\xec\x00\x00\x00" \
	"This is one.dat" \
	"This is three.dat" \
	"This is two.dat"

#define testdata_insert2 \
	"\x04\x00\x00\x00" POD_DESC \
	"ONE.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\xf4\x00\x00\x00" \
	"THREE.DAT\0\0\0\0\0\0\0"   "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x11\x00\x00\x00" "\x03\x01\x00\x00" \
	"FOUR.DAT\0\0\0\0\0\0\0\0"  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x10\x00\x00\x00" "\x14\x01\x00\x00" \
	"TWO.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\x24\x01\x00\x00" \
	"This is one.dat" \
	"This is three.dat" \
	"This is four.dat" \
	"This is two.dat"

#define testdata_remove \
	"\x01\x00\x00\x00" POD_DESC \
	"TWO.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\x7c\x00\x00\x00" \
	"This is two.dat"

#define testdata_remove2 \
	"\x00\x00\x00\x00" POD_DESC

#define testdata_insert_remove \
	"\x02\x00\x00\x00" POD_DESC \
	"THREE.DAT\0\0\0\0\0\0\0"   "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x11\x00\x00\x00" "\xa4\x00\x00\x00" \
	"TWO.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\xb5\x00\x00\x00" \
	"This is three.dat" \
	"This is two.dat"

#define testdata_remove_insert \
	"\x02\x00\x00\x00" POD_DESC \
	"THREE.DAT\0\0\0\0\0\0\0"   "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x11\x00\x00\x00" "\xa4\x00\x00\x00" \
	"TWO.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\xb5\x00\x00\x00" \
	"This is three.dat" \
	"This is two.dat"

#define testdata_move \
	"\x02\x00\x00\x00" POD_DESC \
	"TWO.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\xa4\x00\x00\x00" \
	"ONE.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\xb3\x00\x00\x00" \
	"This is two.dat" \
	"This is one.dat"

#define testdata_resize_larger \
	"\x02\x00\x00\x00" POD_DESC \
	"ONE.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x14\x00\x00\x00" "\xa4\x00\x00\x00" \
	"TWO.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\xb8\x00\x00\x00" \
	"This is one.dat\0\0\0\0\0" \
	"This is two.dat"

#define testdata_resize_smaller \
	"\x02\x00\x00\x00" POD_DESC \
	"ONE.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0a\x00\x00\x00" "\xa4\x00\x00\x00" \
	"TWO.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\xae\x00\x00\x00" \
	"This is on" \
	"This is two.dat"

#define testdata_resize_write \
	"\x02\x00\x00\x00" POD_DESC \
	"ONE.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x17\x00\x00\x00" "\xa4\x00\x00\x00" \
	"TWO.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\xbb\x00\x00\x00" \
	"Now resized to 23 chars" \
	"This is two.dat"

#define testdata_get_metadata_description \
	"Startup 1.1 Gold"

#define testdata_set_metadata_description_target_larger \
	"This is a test"

#define testdata_set_metadata_description_larger \
	"\x02\x00\x00\x00" POD_DESC2_larger \
	"ONE.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\xa4\x00\x00\x00" \
	"TWO.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\xb3\x00\x00\x00" \
	"This is one.dat" \
	"This is two.dat"

#define testdata_set_metadata_description_target_smaller \
	"Hello"

#define testdata_set_metadata_description_smaller \
	"\x02\x00\x00\x00" POD_DESC2_smaller \
	"ONE.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\xa4\x00\x00\x00" \
	"TWO.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\xb3\x00\x00\x00" \
	"This is one.dat" \
	"This is two.dat"

#define MAX_FILENAME_LEN  32

#define ARCHIVE_CLASS fmt_pod_tv
#define ARCHIVE_TYPE  "pod-tv"
#include "test-archive.hpp"

// Test some invalid formats to make sure they're not identified as valid
// archives.  Note that they can still be opened though (by 'force'), this
// only checks whether they look like valid files or not.

// The "c00" test has already been performed in test-archive.hpp to ensure the
// initial state is correctly identified as a valid archive.

ISINSTANCE_TEST(c01,
	"\x02\x00\x00\x00" POD_DESC
	"ONE.DAT\x05\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\xa4\x00\x00\x00"
	"TWO.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\xb3\x00\x00\x00"
	"This is one.dat"
	"This is two.dat",
	ga::EC_DEFINITELY_NO
);

ISINSTANCE_TEST(c02,
	"\x02\x00\x00\x00" POD_DESC
	"ONE.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\xa4\x01\x00\x00"
	"TWO.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\xb3\x00\x00\x00"
	"This is one.dat"
	"This is two.dat",
	ga::EC_DEFINITELY_NO
);

ISINSTANCE_TEST(c03,
	"\x02\x00\x00\x00" POD_DESC
	"ONE.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x01\x00\x00" "\xa4\x00\x00\x00"
	"TWO.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\xb3\x00\x00\x00"
	"This is one.dat"
	"This is two.dat",
	ga::EC_DEFINITELY_NO
);

ISINSTANCE_TEST(c04,
	"\x02\x00\x00\x00"
	"Startup 1.1 Gold"
	"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x05"
	"ONE.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\xa4\x00\x00\x00"
	"TWO.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\xb3\x00\x00\x00"
	"This is one.dat"
	"This is two.dat",
	ga::EC_DEFINITELY_NO
);
