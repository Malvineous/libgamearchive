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

#define fmt_bnk_harry_initialstate \
	"\x04-ID-" "\x07ONE.DAT\0\0\0\0\0" "\x0f\x00\x00\x00" \
	"This is one.dat" \
	"\x04-ID-" "\x07TWO.DAT\0\0\0\0\0" "\x0f\x00\x00\x00" \
	"This is two.dat"

#define fmt_bnk_harry_FAT_initialstate \
	"\x07ONE.DAT\0\0\0\0\0" "\x16\x00\x00\x00" "\x0f\x00\x00\x00" \
	"\x07TWO.DAT\0\0\0\0\0" "\x3b\x00\x00\x00" "\x0f\x00\x00\x00"

// This must be a valid file (correct signature) but with invalid content
// Can't really do that here, because the loader will just ignore invalid files
#define fmt_bnk_harry_invalidcontent_result \
	"\x04-ID-" "\x07ONE.DAT\0\0\0\0\0" "\xef\x00\x00\x00" \
	"This is one.dat" \
	"\x04-ID-" "\x07TWO.DAT\0\0\0\0\0" "\x0f\x00\x00\x00" \
	"This is two.dat"
#define fmt_bnk_harry_FAT_invalidcontent_result \
	"\x07ONE.DAT\0\0\0\0\0" "\x16\x00\x00\x00" "\xef\x00\x00\x00" \
	"\x07TWO.DAT\0\0\0\0\0" "\x3b\x00\x00\x00" "\x0f\x00\x00\x00"

#define fmt_bnk_harry_rename_result \
	"\x04-ID-" "\x09HELLO.BIN\0\0\0" "\x0f\x00\x00\x00" \
	"This is one.dat" \
	"\x04-ID-" "\x07TWO.DAT\0\0\0\0\0" "\x0f\x00\x00\x00" \
	"This is two.dat"
#define fmt_bnk_harry_FAT_rename_result \
	"\x09HELLO.BIN\0\0\0"   "\x16\x00\x00\x00" "\x0f\x00\x00\x00" \
	"\x07TWO.DAT\0\0\0\0\0" "\x3b\x00\x00\x00" "\x0f\x00\x00\x00"

#define fmt_bnk_harry_insert_end_result \
	"\x04-ID-" "\x07ONE.DAT\0\0\0\0\0" "\x0f\x00\x00\x00" \
	"This is one.dat" \
	"\x04-ID-" "\x07TWO.DAT\0\0\0\0\0" "\x0f\x00\x00\x00" \
	"This is two.dat" \
	"\x04-ID-" "\x09THREE.DAT\0\0\0"   "\x11\x00\x00\x00" \
	"This is three.dat"
#define fmt_bnk_harry_FAT_insert_end_result \
	"\x07ONE.DAT\0\0\0\0\0" "\x16\x00\x00\x00" "\x0f\x00\x00\x00" \
	"\x07TWO.DAT\0\0\0\0\0" "\x3b\x00\x00\x00" "\x0f\x00\x00\x00" \
	"\x09THREE.DAT\0\0\0"   "\x60\x00\x00\x00" "\x11\x00\x00\x00"

#define fmt_bnk_harry_insert_mid_result \
	"\x04-ID-" "\x07ONE.DAT\0\0\0\0\0" "\x0f\x00\x00\x00" \
	"This is one.dat" \
	"\x04-ID-" "\x09THREE.DAT\0\0\0"   "\x11\x00\x00\x00" \
	"This is three.dat" \
	"\x04-ID-" "\x07TWO.DAT\0\0\0\0\0" "\x0f\x00\x00\x00" \
	"This is two.dat"
#define fmt_bnk_harry_FAT_insert_mid_result \
	"\x07ONE.DAT\0\0\0\0\0" "\x16\x00\x00\x00" "\x0f\x00\x00\x00" \
	"\x09THREE.DAT\0\0\0"   "\x3b\x00\x00\x00" "\x11\x00\x00\x00" \
	"\x07TWO.DAT\0\0\0\0\0" "\x62\x00\x00\x00" "\x0f\x00\x00\x00"

#define fmt_bnk_harry_insert2_result \
	"\x04-ID-" "\x07ONE.DAT\0\0\0\0\0" "\x0f\x00\x00\x00" \
	"This is one.dat" \
	"\x04-ID-" "\x09THREE.DAT\0\0\0"   "\x11\x00\x00\x00" \
	"This is three.dat" \
	"\x04-ID-" "\x08""FOUR.DAT\0\0\0\0"  "\x10\x00\x00\x00" \
	"This is four.dat" \
	"\x04-ID-" "\x07TWO.DAT\0\0\0\0\0" "\x0f\x00\x00\x00" \
	"This is two.dat"
#define fmt_bnk_harry_FAT_insert2_result \
	"\x07ONE.DAT\0\0\0\0\0" "\x16\x00\x00\x00" "\x0f\x00\x00\x00" \
	"\x09THREE.DAT\0\0\0"   "\x3b\x00\x00\x00" "\x11\x00\x00\x00" \
	"\x08""FOUR.DAT\0\0\0\0"  "\x62\x00\x00\x00" "\x10\x00\x00\x00" \
	"\x07TWO.DAT\0\0\0\0\0" "\x88\x00\x00\x00" "\x0f\x00\x00\x00"

#define fmt_bnk_harry_remove_result \
	"\x04-ID-" "\x07TWO.DAT\0\0\0\0\0" "\x0f\x00\x00\x00" \
	"This is two.dat"
#define fmt_bnk_harry_FAT_remove_result \
	"\x07TWO.DAT\0\0\0\0\0" "\x16\x00\x00\x00" "\x0f\x00\x00\x00"

#define fmt_bnk_harry_remove2_result \
	""
#define fmt_bnk_harry_FAT_remove2_result \
	""

#define fmt_bnk_harry_insert_remove_result \
	"\x04-ID-" "\x09THREE.DAT\0\0\0"   "\x11\x00\x00\x00" \
	"This is three.dat" \
	"\x04-ID-" "\x07TWO.DAT\0\0\0\0\0" "\x0f\x00\x00\x00" \
	"This is two.dat"
#define fmt_bnk_harry_FAT_insert_remove_result \
	"\x09THREE.DAT\0\0\0"   "\x16\x00\x00\x00" "\x11\x00\x00\x00" \
	"\x07TWO.DAT\0\0\0\0\0" "\x3d\x00\x00\x00" "\x0f\x00\x00\x00"

#define fmt_bnk_harry_remove_insert_result \
	"\x04-ID-" "\x09THREE.DAT\0\0\0"   "\x11\x00\x00\x00" \
	"This is three.dat" \
	"\x04-ID-" "\x07TWO.DAT\0\0\0\0\0" "\x0f\x00\x00\x00" \
	"This is two.dat"
#define fmt_bnk_harry_FAT_remove_insert_result \
	"\x09THREE.DAT\0\0\0"   "\x16\x00\x00\x00" "\x11\x00\x00\x00" \
	"\x07TWO.DAT\0\0\0\0\0" "\x3d\x00\x00\x00" "\x0f\x00\x00\x00"

#define fmt_bnk_harry_move_result \
	"\x04-ID-" "\x07TWO.DAT\0\0\0\0\0" "\x0f\x00\x00\x00" \
	"This is two.dat" \
	"\x04-ID-" "\x07ONE.DAT\0\0\0\0\0" "\x0f\x00\x00\x00" \
	"This is one.dat"
#define fmt_bnk_harry_FAT_move_result \
	"\x07TWO.DAT\0\0\0\0\0" "\x16\x00\x00\x00" "\x0f\x00\x00\x00" \
	"\x07ONE.DAT\0\0\0\0\0" "\x3b\x00\x00\x00" "\x0f\x00\x00\x00"

#define fmt_bnk_harry_resize_larger_result \
	"\x04-ID-" "\x07ONE.DAT\0\0\0\0\0" "\x14\x00\x00\x00" \
	"This is one.dat\0\0\0\0\0" \
	"\x04-ID-" "\x07TWO.DAT\0\0\0\0\0" "\x0f\x00\x00\x00" \
	"This is two.dat"
#define fmt_bnk_harry_FAT_resize_larger_result \
	"\x07ONE.DAT\0\0\0\0\0" "\x16\x00\x00\x00" "\x14\x00\x00\x00" \
	"\x07TWO.DAT\0\0\0\0\0" "\x40\x00\x00\x00" "\x0f\x00\x00\x00"

#define fmt_bnk_harry_resize_smaller_result \
	"\x04-ID-" "\x07ONE.DAT\0\0\0\0\0" "\x0a\x00\x00\x00" \
	"This is on" \
	"\x04-ID-" "\x07TWO.DAT\0\0\0\0\0" "\x0f\x00\x00\x00" \
	"This is two.dat"
#define fmt_bnk_harry_FAT_resize_smaller_result \
	"\x07ONE.DAT\0\0\0\0\0" "\x16\x00\x00\x00" "\x0a\x00\x00\x00" \
	"\x07TWO.DAT\0\0\0\0\0" "\x36\x00\x00\x00" "\x0f\x00\x00\x00"

#define fmt_bnk_harry_resize_write_result \
	"\x04-ID-" "\x07ONE.DAT\0\0\0\0\0" "\x17\x00\x00\x00" \
	"Now resized to 23 chars" \
	"\x04-ID-" "\x07TWO.DAT\0\0\0\0\0" "\x0f\x00\x00\x00" \
	"This is two.dat"
#define fmt_bnk_harry_FAT_resize_write_result \
	"\x07ONE.DAT\0\0\0\0\0" "\x16\x00\x00\x00" "\x17\x00\x00\x00" \
	"\x07TWO.DAT\0\0\0\0\0" "\x43\x00\x00\x00" "\x0f\x00\x00\x00"

#define MAX_FILENAME_LEN  12

// Use all the _FAT_xxx values
#define HAS_FAT

#define ARCHIVE_CLASS fmt_bnk_harry
#define ARCHIVE_TYPE  "bnk-harry"
#include "test-archive.hpp"
