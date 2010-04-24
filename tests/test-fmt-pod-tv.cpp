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

#define fmt_pod_tv_initialstate \
	"\x02\x00\x00\x00" POD_DESC \
	"ONE.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\xa4\x00\x00\x00" \
	"TWO.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\xb3\x00\x00\x00" \
	"This is one.dat" \
	"This is two.dat"

// This must be a valid file (correct signature) but with invalid content.
#define fmt_pod_tv_invalidcontent_result \
	"\x00\x00\x00\xf0" POD_DESC \
	"ONE.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\xa4\x00\x00\x00" \
	"TWO.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\xb3\x00\x00\x00" \
	"This is one.dat" \
	"This is two.dat"

#define fmt_pod_tv_rename_result \
	"\x02\x00\x00\x00" POD_DESC \
	"THREE.DAT\0\0\0\0\0\0\0"   "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\xa4\x00\x00\x00" \
	"TWO.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\xb3\x00\x00\x00" \
	"This is one.dat" \
	"This is two.dat"

#define fmt_pod_tv_insert_end_result \
	"\x03\x00\x00\x00" POD_DESC \
	"ONE.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\xcc\x00\x00\x00" \
	"TWO.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\xdb\x00\x00\x00" \
	"THREE.DAT\0\0\0\0\0\0\0"   "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x11\x00\x00\x00" "\xea\x00\x00\x00" \
	"This is one.dat" \
	"This is two.dat" \
	"This is three.dat"

#define fmt_pod_tv_insert_mid_result \
	"\x03\x00\x00\x00" POD_DESC \
	"ONE.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\xcc\x00\x00\x00" \
	"THREE.DAT\0\0\0\0\0\0\0"   "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x11\x00\x00\x00" "\xdb\x00\x00\x00" \
	"TWO.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\xec\x00\x00\x00" \
	"This is one.dat" \
	"This is three.dat" \
	"This is two.dat"

#define fmt_pod_tv_insert2_result \
	"\x04\x00\x00\x00" POD_DESC \
	"ONE.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\xf4\x00\x00\x00" \
	"THREE.DAT\0\0\0\0\0\0\0"   "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x11\x00\x00\x00" "\x03\x01\x00\x00" \
	"FOUR.DAT\0\0\0\0\0\0\0\0"  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x10\x00\x00\x00" "\x14\x01\x00\x00" \
	"TWO.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\x24\x01\x00\x00" \
	"This is one.dat" \
	"This is three.dat" \
	"This is four.dat" \
	"This is two.dat"

#define fmt_pod_tv_remove_result \
	"\x01\x00\x00\x00" POD_DESC \
	"TWO.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\x7c\x00\x00\x00" \
	"This is two.dat"

#define fmt_pod_tv_remove2_result \
	"\x00\x00\x00\x00" POD_DESC

#define fmt_pod_tv_insert_remove_result \
	"\x02\x00\x00\x00" POD_DESC \
	"THREE.DAT\0\0\0\0\0\0\0"   "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x11\x00\x00\x00" "\xa4\x00\x00\x00" \
	"TWO.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\xb5\x00\x00\x00" \
	"This is three.dat" \
	"This is two.dat"

#define fmt_pod_tv_remove_insert_result \
	"\x02\x00\x00\x00" POD_DESC \
	"THREE.DAT\0\0\0\0\0\0\0"   "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x11\x00\x00\x00" "\xa4\x00\x00\x00" \
	"TWO.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\xb5\x00\x00\x00" \
	"This is three.dat" \
	"This is two.dat"

#define fmt_pod_tv_move_result \
	"\x02\x00\x00\x00" POD_DESC \
	"TWO.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\xa4\x00\x00\x00" \
	"ONE.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\xb3\x00\x00\x00" \
	"This is two.dat" \
	"This is one.dat"

#define fmt_pod_tv_resize_larger_result \
	"\x02\x00\x00\x00" POD_DESC \
	"ONE.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x14\x00\x00\x00" "\xa4\x00\x00\x00" \
	"TWO.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\xb8\x00\x00\x00" \
	"This is one.dat\0\0\0\0\0" \
	"This is two.dat"

#define fmt_pod_tv_resize_smaller_result \
	"\x02\x00\x00\x00" POD_DESC \
	"ONE.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0a\x00\x00\x00" "\xa4\x00\x00\x00" \
	"TWO.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\xae\x00\x00\x00" \
	"This is on" \
	"This is two.dat"

#define fmt_pod_tv_resize_write_result \
	"\x02\x00\x00\x00" POD_DESC \
	"ONE.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x17\x00\x00\x00" "\xa4\x00\x00\x00" \
	"TWO.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\xbb\x00\x00\x00" \
	"Now resized to 23 chars" \
	"This is two.dat"

#define MAX_FILENAME_LEN  32

#define ARCHIVE_CLASS fmt_pod_tv
#define ARCHIVE_TYPE  "pod-tv"
#include "test-archive.hpp"

#define ISINSTANCE_TEST(c, d, r) \
	BOOST_AUTO_TEST_CASE(TEST_NAME(isinstance_ ## c)) \
	{ \
		BOOST_TEST_MESSAGE("isInstance check (" ARCHIVE_TYPE "; " #c ")"); \
		\
		boost::shared_ptr<ga::Manager> pManager(ga::getManager()); \
		ga::Manager::arch_sptr pTestType(pManager->getArchiveTypeByCode(ARCHIVE_TYPE)); \
		\
		boost::shared_ptr<std::stringstream> psstrBase(new std::stringstream); \
		(*psstrBase) << makeString(d); \
		camoto::iostream_sptr psBase(psstrBase); \
		\
		BOOST_CHECK_EQUAL(pTestType->isInstance(psBase), r); \
	}

// Test some invalid formats to make sure they're not identified as valid
// archives.  Note that they can still be opened though (by 'force'), this
// only checks whether they look like valid files or not.

// Make sure the base data is correct, as we're only changing a couple of bytes
// later on and we want to make sure those bytes are causing the failure.
ISINSTANCE_TEST(c00,
	"\x02\x00\x00\x00" POD_DESC
	"ONE.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\xa4\x00\x00\x00"
	"TWO.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\xb3\x00\x00\x00"
	"This is one.dat"
	"This is two.dat",
	ga::EC_DEFINITELY_YES
);

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
