/*
 * test-fmt-bnk-harry.cpp - test code for BNKArchive class.
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
	"\x04-ID-" "\x07ONE.DAT\0\0\0\0\0" "\x0f\x00\x00\x00" \
	"This is one.dat" \
	"\x04-ID-" "\x07TWO.DAT\0\0\0\0\0" "\x0f\x00\x00\x00" \
	"This is two.dat"

#define testdata_FAT_initialstate \
	"\x07ONE.DAT\0\0\0\0\0" "\x16\x00\x00\x00" "\x0f\x00\x00\x00" \
	"\x07TWO.DAT\0\0\0\0\0" "\x3b\x00\x00\x00" "\x0f\x00\x00\x00"

// This must be a valid file (correct signature) but with invalid content
// Can't really do that here, because the loader will just ignore invalid files
#define testdata_invalidcontent \
	"\x04-ID-" "\x07ONE.DAT\0\0\0\0\0" "\xef\x00\x00\x00" \
	"This is one.dat" \
	"\x04-ID-" "\x07TWO.DAT\0\0\0\0\0" "\x0f\x00\x00\x00" \
	"This is two.dat"
#define testdata_FAT_invalidcontent \
	"\x07ONE.DAT\0\0\0\0\0" "\x16\x00\x00\x00" "\xef\x00\x00\x00" \
	"\x07TWO.DAT\0\0\0\0\0" "\x3b\x00\x00\x00" "\x0f\x00\x00\x00"

#define testdata_rename \
	"\x04-ID-" "\x09THREE.DAT\0\0\0" "\x0f\x00\x00\x00" \
	"This is one.dat" \
	"\x04-ID-" "\x07TWO.DAT\0\0\0\0\0" "\x0f\x00\x00\x00" \
	"This is two.dat"
#define testdata_FAT_rename \
	"\x09THREE.DAT\0\0\0"   "\x16\x00\x00\x00" "\x0f\x00\x00\x00" \
	"\x07TWO.DAT\0\0\0\0\0" "\x3b\x00\x00\x00" "\x0f\x00\x00\x00"

#define testdata_insert_end \
	"\x04-ID-" "\x07ONE.DAT\0\0\0\0\0" "\x0f\x00\x00\x00" \
	"This is one.dat" \
	"\x04-ID-" "\x07TWO.DAT\0\0\0\0\0" "\x0f\x00\x00\x00" \
	"This is two.dat" \
	"\x04-ID-" "\x09THREE.DAT\0\0\0"   "\x11\x00\x00\x00" \
	"This is three.dat"
#define testdata_FAT_insert_end \
	"\x07ONE.DAT\0\0\0\0\0" "\x16\x00\x00\x00" "\x0f\x00\x00\x00" \
	"\x07TWO.DAT\0\0\0\0\0" "\x3b\x00\x00\x00" "\x0f\x00\x00\x00" \
	"\x09THREE.DAT\0\0\0"   "\x60\x00\x00\x00" "\x11\x00\x00\x00"

#define testdata_insert_mid \
	"\x04-ID-" "\x07ONE.DAT\0\0\0\0\0" "\x0f\x00\x00\x00" \
	"This is one.dat" \
	"\x04-ID-" "\x09THREE.DAT\0\0\0"   "\x11\x00\x00\x00" \
	"This is three.dat" \
	"\x04-ID-" "\x07TWO.DAT\0\0\0\0\0" "\x0f\x00\x00\x00" \
	"This is two.dat"
#define testdata_FAT_insert_mid \
	"\x07ONE.DAT\0\0\0\0\0" "\x16\x00\x00\x00" "\x0f\x00\x00\x00" \
	"\x09THREE.DAT\0\0\0"   "\x3b\x00\x00\x00" "\x11\x00\x00\x00" \
	"\x07TWO.DAT\0\0\0\0\0" "\x62\x00\x00\x00" "\x0f\x00\x00\x00"

#define testdata_insert2 \
	"\x04-ID-" "\x07ONE.DAT\0\0\0\0\0" "\x0f\x00\x00\x00" \
	"This is one.dat" \
	"\x04-ID-" "\x09THREE.DAT\0\0\0"   "\x11\x00\x00\x00" \
	"This is three.dat" \
	"\x04-ID-" "\x08""FOUR.DAT\0\0\0\0"  "\x10\x00\x00\x00" \
	"This is four.dat" \
	"\x04-ID-" "\x07TWO.DAT\0\0\0\0\0" "\x0f\x00\x00\x00" \
	"This is two.dat"
#define testdata_FAT_insert2 \
	"\x07ONE.DAT\0\0\0\0\0" "\x16\x00\x00\x00" "\x0f\x00\x00\x00" \
	"\x09THREE.DAT\0\0\0"   "\x3b\x00\x00\x00" "\x11\x00\x00\x00" \
	"\x08""FOUR.DAT\0\0\0\0"  "\x62\x00\x00\x00" "\x10\x00\x00\x00" \
	"\x07TWO.DAT\0\0\0\0\0" "\x88\x00\x00\x00" "\x0f\x00\x00\x00"

#define testdata_remove \
	"\x04-ID-" "\x07TWO.DAT\0\0\0\0\0" "\x0f\x00\x00\x00" \
	"This is two.dat"
#define testdata_FAT_remove \
	"\x07TWO.DAT\0\0\0\0\0" "\x16\x00\x00\x00" "\x0f\x00\x00\x00"

#define testdata_remove2 \
	""
#define testdata_FAT_remove2 \
	""

#define testdata_insert_remove \
	"\x04-ID-" "\x09THREE.DAT\0\0\0"   "\x11\x00\x00\x00" \
	"This is three.dat" \
	"\x04-ID-" "\x07TWO.DAT\0\0\0\0\0" "\x0f\x00\x00\x00" \
	"This is two.dat"
#define testdata_FAT_insert_remove \
	"\x09THREE.DAT\0\0\0"   "\x16\x00\x00\x00" "\x11\x00\x00\x00" \
	"\x07TWO.DAT\0\0\0\0\0" "\x3d\x00\x00\x00" "\x0f\x00\x00\x00"

#define testdata_remove_insert \
	"\x04-ID-" "\x09THREE.DAT\0\0\0"   "\x11\x00\x00\x00" \
	"This is three.dat" \
	"\x04-ID-" "\x07TWO.DAT\0\0\0\0\0" "\x0f\x00\x00\x00" \
	"This is two.dat"
#define testdata_FAT_remove_insert \
	"\x09THREE.DAT\0\0\0"   "\x16\x00\x00\x00" "\x11\x00\x00\x00" \
	"\x07TWO.DAT\0\0\0\0\0" "\x3d\x00\x00\x00" "\x0f\x00\x00\x00"

#define testdata_move \
	"\x04-ID-" "\x07TWO.DAT\0\0\0\0\0" "\x0f\x00\x00\x00" \
	"This is two.dat" \
	"\x04-ID-" "\x07ONE.DAT\0\0\0\0\0" "\x0f\x00\x00\x00" \
	"This is one.dat"
#define testdata_FAT_move \
	"\x07TWO.DAT\0\0\0\0\0" "\x16\x00\x00\x00" "\x0f\x00\x00\x00" \
	"\x07ONE.DAT\0\0\0\0\0" "\x3b\x00\x00\x00" "\x0f\x00\x00\x00"

#define testdata_resize_larger \
	"\x04-ID-" "\x07ONE.DAT\0\0\0\0\0" "\x14\x00\x00\x00" \
	"This is one.dat\0\0\0\0\0" \
	"\x04-ID-" "\x07TWO.DAT\0\0\0\0\0" "\x0f\x00\x00\x00" \
	"This is two.dat"
#define testdata_FAT_resize_larger \
	"\x07ONE.DAT\0\0\0\0\0" "\x16\x00\x00\x00" "\x14\x00\x00\x00" \
	"\x07TWO.DAT\0\0\0\0\0" "\x40\x00\x00\x00" "\x0f\x00\x00\x00"

#define testdata_resize_smaller \
	"\x04-ID-" "\x07ONE.DAT\0\0\0\0\0" "\x0a\x00\x00\x00" \
	"This is on" \
	"\x04-ID-" "\x07TWO.DAT\0\0\0\0\0" "\x0f\x00\x00\x00" \
	"This is two.dat"
#define testdata_FAT_resize_smaller \
	"\x07ONE.DAT\0\0\0\0\0" "\x16\x00\x00\x00" "\x0a\x00\x00\x00" \
	"\x07TWO.DAT\0\0\0\0\0" "\x36\x00\x00\x00" "\x0f\x00\x00\x00"

#define testdata_resize_write \
	"\x04-ID-" "\x07ONE.DAT\0\0\0\0\0" "\x17\x00\x00\x00" \
	"Now resized to 23 chars" \
	"\x04-ID-" "\x07TWO.DAT\0\0\0\0\0" "\x0f\x00\x00\x00" \
	"This is two.dat"
#define testdata_FAT_resize_write \
	"\x07ONE.DAT\0\0\0\0\0" "\x16\x00\x00\x00" "\x17\x00\x00\x00" \
	"\x07TWO.DAT\0\0\0\0\0" "\x43\x00\x00\x00" "\x0f\x00\x00\x00"

#define MAX_FILENAME_LEN  12

// Use all the _FAT_xxx values
#define HAS_FAT

#define ARCHIVE_CLASS fmt_bnk_harry
#define ARCHIVE_TYPE  "bnk-harry"
#include "test-archive.hpp"
