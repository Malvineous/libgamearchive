/**
 * @file  test-fmt-resource-tim-fat.cpp
 * @brief Test code for TIMResourceFATArchive class.
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

#define FILENAME1 "RESOURCE.001"
#define FILENAME2 "RESOURCE.002"
#define FILENAME3 "RESOURCE.003"
#define FILENAME4 "RESOURCE.004"

#define CONTENT1 "This is one.dat\0"
#define CONTENT2 "This is two.dat\0"
#define CONTENT3 "This is three.dat\0\0\0\0\0\0\0"
#define CONTENT4 "This is four.dat\0\0\0\0\0\0\0\0"
#define CONTENT1_NORMALSIZE 16
#define CONTENT1_LARGESIZE 24
#define CONTENT1_SMALLSIZE 8
#define CONTENT1_OVERWRITTEN "Now resized to 24 chars!"
#define CONTENT1_OVERWSIZE (sizeof(CONTENT1_OVERWRITTEN)-1)

#define testdata_initialstate \
	"\x00\x00" "\x00\x00" "\x02\x00" \
	"RESOURCE.001\0" "\x02\x00" CONTENT1 \
	"RESOURCE.002\0" "\x02\x00" CONTENT2

#define testdata_rename \
	"\x00\x00" "\x00\x00" "\x02\x00" \
	"RESOURCE.003\0" "\x02\x00" CONTENT1 \
	"RESOURCE.002\0" "\x02\x00" CONTENT2

#define testdata_insert_end \
	"\x00\x00" "\x00\x00" "\x03\x00" \
	"RESOURCE.001\0" "\x02\x00" CONTENT1 \
	"RESOURCE.002\0" "\x02\x00" CONTENT2 \
	"RESOURCE.003\0" "\x03\x00" CONTENT3

#define testdata_insert_mid \
	"\x00\x00" "\x00\x00" "\x03\x00" \
	"RESOURCE.001\0" "\x02\x00" CONTENT1 \
	"RESOURCE.003\0" "\x03\x00" CONTENT3 \
	"RESOURCE.002\0" "\x02\x00" CONTENT2

#define testdata_insert2 \
	"\x00\x00" "\x00\x00" "\x04\x00" \
	"RESOURCE.001\0" "\x02\x00" CONTENT1 \
	"RESOURCE.003\0" "\x03\x00" CONTENT3 \
	"RESOURCE.004\0" "\x03\x00" CONTENT4 \
	"RESOURCE.002\0" "\x02\x00" CONTENT2

#define testdata_remove \
	"\x00\x00" "\x00\x00" "\x01\x00" \
	"RESOURCE.002\0" "\x02\x00" CONTENT2

#define testdata_remove2 \
	"\x00\x00" "\x00\x00" "\x00\x00"

#define testdata_insert_remove \
	"\x00\x00" "\x00\x00" "\x02\x00" \
	"RESOURCE.003\0" "\x03\x00" CONTENT3 \
	"RESOURCE.002\0" "\x02\x00" CONTENT2

#define testdata_move \
	"\x00\x00" "\x00\x00" "\x02\x00" \
	"RESOURCE.002\0" "\x02\x00" CONTENT2 \
	"RESOURCE.001\0" "\x02\x00" CONTENT1

#define testdata_resize_larger \
	"\x00\x00" "\x00\x00" "\x02\x00" \
	"RESOURCE.001\0" "\x03\x00" CONTENT1 "\0\0\0\0\0\0\0\0" \
	"RESOURCE.002\0" "\x02\x00" CONTENT2

#define testdata_resize_smaller \
	"\x00\x00" "\x00\x00" "\x02\x00" \
	"RESOURCE.001\0" "\x01\x00" "This is " \
	"RESOURCE.002\0" "\x02\x00" CONTENT2

#define testdata_resize_write \
	"\x00\x00" "\x00\x00" "\x02\x00" \
	"RESOURCE.001\0" "\x03\x00" CONTENT1_OVERWRITTEN \
	"RESOURCE.002\0" "\x02\x00" CONTENT2

#define MAX_FILENAME_LEN  12

#define ARCHIVE_CLASS fmt_resource_tim_fat
#define ARCHIVE_TYPE  "resource-tim-fat"
#include "test-archive.hpp"

// Test some invalid formats to make sure they're not identified as valid
// archives.  Note that they can still be opened though (by 'force'), this
// only checks whether they look like valid files or not.

// The "c00" test has already been performed in test-archive.hpp to ensure the
// initial state is correctly identified as a valid archive.

ISINSTANCE_TEST(c01,
	"\x00\x00" "\x00\x00" "\x02"
	,
	DefinitelyNo
);

ISINSTANCE_TEST(c02,
	"\x00\x00" "\x00\x00" "\x02\x00"
	"RESOURCE.001\0" "\x02\x00" CONTENT1
	"RESOURCE.002\0" "\x02\x00" CONTENT2
	"blah"
	,
	DefinitelyNo
);

ISINSTANCE_TEST(c03,
	"\x00\x00" "\x00\x00" "\x02\x00"
	"RESOURCE.001\0" "\x02\x00" CONTENT1
	"RESOURCE.002\0" "\x02"
	,
	DefinitelyNo
);

// Not really possible to do any INVALIDDATA_TEST() tests here, because the
// worst that can happen is it looks like the archive has been truncated.
