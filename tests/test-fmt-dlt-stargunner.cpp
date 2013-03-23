/**
 * @file  test-fmt-dlt-stargunner.cpp
 * @brief Test code for DLTArchive class.
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

#define FILENAME1_ENC "O\x1e\x15\x66\x76\x08\x13\x5b\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f"
#define FILENAME2_ENC "T\x02\x16\x7c\x76\x08\x13\x5b\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f"
#define FILENAME3_ENC "T\x1d\x18\x10\x0c\x64p\x0a\x1d]\x0a\x0b\x0c\x0d\x0e\x0f\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f"
#define FILENAME4_ENC "F\x08\x04\x0axw\x0b\x1c\x5c\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f"

#define testdata_initialstate \
	"DAVE" "\x00\x01" "\x02\x00" \
	FILENAME1_ENC "\x00\x00\x00\x00" "\x0f\x00\x00\x00" "This is one.dat" \
	FILENAME2_ENC "\x00\x00\x00\x00" "\x0f\x00\x00\x00" "This is two.dat"

#define testdata_rename \
	"DAVE" "\x00\x01" "\x02\x00" \
	FILENAME3_ENC "\x00\x00\x00\x00" "\x0f\x00\x00\x00" "This is one.dat" \
	FILENAME2_ENC "\x00\x00\x00\x00" "\x0f\x00\x00\x00" "This is two.dat"

#define testdata_insert_end \
	"DAVE" "\x00\x01" "\x03\x00" \
	FILENAME1_ENC "\x00\x00\x00\x00" "\x0f\x00\x00\x00" "This is one.dat" \
	FILENAME2_ENC "\x00\x00\x00\x00" "\x0f\x00\x00\x00" "This is two.dat" \
	FILENAME3_ENC "\x00\x00\x00\x00" "\x11\x00\x00\x00" "This is three.dat"

#define testdata_insert_mid \
	"DAVE" "\x00\x01" "\x03\x00" \
	FILENAME1_ENC "\x00\x00\x00\x00" "\x0f\x00\x00\x00" "This is one.dat" \
	FILENAME3_ENC "\x00\x00\x00\x00" "\x11\x00\x00\x00" "This is three.dat" \
	FILENAME2_ENC "\x00\x00\x00\x00" "\x0f\x00\x00\x00" "This is two.dat"

#define testdata_insert2 \
	"DAVE" "\x00\x01" "\x04\x00" \
	FILENAME1_ENC "\x00\x00\x00\x00" "\x0f\x00\x00\x00" "This is one.dat" \
	FILENAME3_ENC "\x00\x00\x00\x00" "\x11\x00\x00\x00" "This is three.dat" \
	FILENAME4_ENC "\x00\x00\x00\x00" "\x10\x00\x00\x00" "This is four.dat" \
	FILENAME2_ENC "\x00\x00\x00\x00" "\x0f\x00\x00\x00" "This is two.dat"

#define testdata_remove \
	"DAVE" "\x00\x01" "\x01\x00" \
	FILENAME2_ENC "\x00\x00\x00\x00" "\x0f\x00\x00\x00" "This is two.dat"

#define testdata_remove2 \
	"DAVE" "\x00\x01" "\x00\x00"

#define testdata_insert_remove \
	"DAVE" "\x00\x01" "\x02\x00" \
	FILENAME3_ENC "\x00\x00\x00\x00" "\x11\x00\x00\x00" "This is three.dat" \
	FILENAME2_ENC "\x00\x00\x00\x00" "\x0f\x00\x00\x00" "This is two.dat"

#define testdata_move \
	"DAVE" "\x00\x01" "\x02\x00" \
	FILENAME2_ENC "\x00\x00\x00\x00" "\x0f\x00\x00\x00" "This is two.dat" \
	FILENAME1_ENC "\x00\x00\x00\x00" "\x0f\x00\x00\x00" "This is one.dat"

#define testdata_resize_larger \
	"DAVE" "\x00\x01" "\x02\x00" \
	FILENAME1_ENC "\x00\x00\x00\x00" "\x14\x00\x00\x00" "This is one.dat" "\0\0\0\0\0" \
	FILENAME2_ENC "\x00\x00\x00\x00" "\x0f\x00\x00\x00" "This is two.dat"

#define testdata_resize_smaller \
	"DAVE" "\x00\x01" "\x02\x00" \
	FILENAME1_ENC "\x00\x00\x00\x00" "\x0a\x00\x00\x00" "This is on" \
	FILENAME2_ENC "\x00\x00\x00\x00" "\x0f\x00\x00\x00" "This is two.dat"

#define testdata_resize_write \
	"DAVE" "\x00\x01" "\x02\x00" \
	FILENAME1_ENC "\x00\x00\x00\x00" "\x17\x00\x00\x00" "Now resized to 23 chars" \
	FILENAME2_ENC "\x00\x00\x00\x00" "\x0f\x00\x00\x00" "This is two.dat"

#define MAX_FILENAME_LEN  32

#define ARCHIVE_CLASS fmt_dlt_stargunner
#define ARCHIVE_TYPE  "dlt-stargunner"
#include "test-archive.hpp"

// Test some invalid formats to make sure they're not identified as valid
// archives.  Note that they can still be opened though (by 'force'), this
// only checks whether they look like valid files or not.

// The "c00" test has already been performed in test-archive.hpp to ensure the
// initial state is correctly identified as a valid archive.

ISINSTANCE_TEST(c01,
	"DAVY" "\x00\x01" "\x02\x00"
	FILENAME1_ENC "\x00\x00\x00\x00" "\x0F\x00\x00\x00" "This is one.dat"
	FILENAME2_ENC "\x00\x00\x00\x00" "\x0F\x00\x00\x00" "This is two.dat"
	,
	DefinitelyNo
);

ISINSTANCE_TEST(c02,
	"DAVE" "\x00\x01" "\x00"
	,
	DefinitelyNo
);

// Not really possible to do any INVALIDDATA_TEST() tests here, because the
// worst that can happen is it looks like the archive has been truncated.
