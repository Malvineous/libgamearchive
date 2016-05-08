/**
 * @file   test-fmt-dat-zool.cpp
 * @brief  Test code for Zool .DAT archives.
 *
 * Copyright (C) 2010-2016 Adam Nielsen <malvineous@shikadi.net>
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

#include "test-archive.hpp"

class test_dat_zool: public test_archive
{
	public:
		test_dat_zool()
		{
			this->type = "dat-zool";
			this->lenMaxFilename = 8;
			this->content0_overwritten = std::string("Now resized to 513 chars")
				+ std::string(513 - 24, '\0');
			assert(this->content0_overwritten.length() == 513);

			// Technically the files aren't a fixed size, but this avoids a bunch of
			// extra special cases in the generic test code.
			this->lenFilesizeFixed = 512;
			this->content[0].append(512-15, '\0');

			// Since we only have eight chars for filenames and the original game
			// doesn't use filename extensions, don't use them here either.
			this->filename[0] = "ONE";
			this->filename[1] = "TWO";
			this->filename[2] = "THREE";
			this->filename[3] = "FOUR";
		}

		void addTests()
		{
			this->test_archive::addTests();

			ADD_ARCH_TEST(false, &test_dat_zool::test_noexpand_fat);
			ADD_ARCH_TEST(false, &test_dat_zool::test_expand_fat_chunk);
			ADD_ARCH_TEST(false, &test_dat_zool::test_expand_fat_chunk_with_entry);
			ADD_ARCH_TEST(false, &test_dat_zool::test_shrink_fat);

			// c00: Initial state
			this->isInstance(ArchiveType::Certainty::DefinitelyYes, this->content_12());

			// c01: File too short
			this->isInstance(ArchiveType::Certainty::DefinitelyNo, STRING_WITH_NULLS(
				"\x01\x00"
				"ONE     " "\x01\x00"
				"TWO     " "\x02\x00"
				"\x00"
			));

			// c02: Incorrect archive size
			this->isInstance(ArchiveType::Certainty::DefinitelyNo, STRING_WITH_NULLS(
				"\x0F\x00"
				"ONE     " "\x01\x00"
				"TWO     " "\x02\x00"
				"\x00") + std::string(512 - (2+1 + 10*2), '\0') + STRING_WITH_NULLS(
				"This is one.dat") + std::string(512 - 15, '\0') + STRING_WITH_NULLS(
				"This is two.dat") + std::string(512 - 15, '\0')
			);
		}

		virtual std::string content_12()
		{
			return STRING_WITH_NULLS(
				"\x03\x00"
				"ONE     " "\x01\x00"
				"TWO     " "\x02\x00"
				"\x00") + std::string(512 - (2+1 + 10*2), '\0') + STRING_WITH_NULLS(
				"This is one.dat") + std::string(512 - 15, '\0') + STRING_WITH_NULLS(
				"This is two.dat") + std::string(512 - 15, '\0')
			;
		}

		virtual std::string content_1r2()
		{
			return STRING_WITH_NULLS(
				"\x03\x00"
				"THREE   " "\x01\x00"
				"TWO     " "\x02\x00"
				"\x00") + std::string(512 - (2+1 + 10*2), '\0') + STRING_WITH_NULLS(
				"This is one.dat") + std::string(512 - 15, '\0') + STRING_WITH_NULLS(
				"This is two.dat") + std::string(512 - 15, '\0')
			;
		}

		virtual std::string content_123()
		{
			return STRING_WITH_NULLS(
				"\x04\x00"
				"ONE     " "\x01\x00"
				"TWO     " "\x02\x00"
				"THREE   " "\x03\x00"
				"\x00") + std::string(512 - (2+1 + 10*3), '\0') + STRING_WITH_NULLS(
				"This is one.dat") + std::string(512 - 15, '\0') + STRING_WITH_NULLS(
				"This is two.dat") + std::string(512 - 15, '\0') + STRING_WITH_NULLS(
				"This is three.dat") + std::string(512 - 17, '\0')
			;
		}

		virtual std::string content_132()
		{
			return STRING_WITH_NULLS(
				"\x04\x00"
				"ONE     " "\x01\x00"
				"THREE   " "\x02\x00"
				"TWO     " "\x03\x00"
				"\x00") + std::string(512 - (2+1 + 10*3), '\0') + STRING_WITH_NULLS(
				"This is one.dat") + std::string(512 - 15, '\0') + STRING_WITH_NULLS(
				"This is three.dat") + std::string(512 - 17, '\0') + STRING_WITH_NULLS(
				"This is two.dat") + std::string(512 - 15, '\0')
			;
		}

		virtual std::string content_1342()
		{
			return STRING_WITH_NULLS(
				"\x05\x00"
				"ONE     " "\x01\x00"
				"THREE   " "\x02\x00"
				"FOUR    " "\x03\x00"
				"TWO     " "\x04\x00"
				"\x00") + std::string(512 - (2+1 + 10*4), '\0') + STRING_WITH_NULLS(
				"This is one.dat") + std::string(512 - 15, '\0') + STRING_WITH_NULLS(
				"This is three.dat") + std::string(512 - 17, '\0') + STRING_WITH_NULLS(
				"This is four.dat") + std::string(512 - 16, '\0') + STRING_WITH_NULLS(
				"This is two.dat") + std::string(512 - 15, '\0')
			;
		}

		virtual std::string content_2()
		{
			return STRING_WITH_NULLS(
				"\x02\x00"
				"TWO     " "\x01\x00"
				"\x00") + std::string(512 - (2+1 + 10*1), '\0') + STRING_WITH_NULLS(
				"This is two.dat") + std::string(512 - 15, '\0')
			;
		}

		virtual std::string content_0()
		{
			return STRING_WITH_NULLS(
				"\x01\x00"
				"\x00") + std::string(512 - (2+1), '\0')
			;
		}

		virtual std::string content_32()
		{
			return STRING_WITH_NULLS(
				"\x03\x00"
				"THREE   " "\x01\x00"
				"TWO     " "\x02\x00"
				"\x00") + std::string(512 - (2+1 + 10*2), '\0') + STRING_WITH_NULLS(
				"This is three.dat") + std::string(512 - 17, '\0') + STRING_WITH_NULLS(
				"This is two.dat") + std::string(512 - 15, '\0')
			;
		}

		virtual std::string content_21()
		{
			return STRING_WITH_NULLS(
				"\x03\x00"
				"TWO     " "\x01\x00"
				"ONE     " "\x02\x00"
				"\x00") + std::string(512 - (2+1 + 10*2), '\0') + STRING_WITH_NULLS(
				"This is two.dat") + std::string(512 - 15, '\0') + STRING_WITH_NULLS(
				"This is one.dat") + std::string(512 - 15, '\0')
			;
		}

		virtual std::string content_1l2()
		{
			return STRING_WITH_NULLS(
				"\x03\x00"
				"ONE     " "\x01\x00"
				"TWO     " "\x02\x00"
				"\x00") + std::string(512 - (2+1 + 10*2), '\0') + STRING_WITH_NULLS(
				"This is one.dat\0\0\0\0\0") + std::string(512 - 20, '\0') + STRING_WITH_NULLS(
				"This is two.dat") + std::string(512 - 15, '\0')
			;
		}

		virtual std::string content_1s2()
		{
			return STRING_WITH_NULLS(
				"\x03\x00"
				"ONE     " "\x01\x00"
				"TWO     " "\x02\x00"
				"\x00") + std::string(512 - (2+1 + 10*2), '\0') + STRING_WITH_NULLS(
				"This is on") + std::string(512 - 10, '\0') + STRING_WITH_NULLS(
				"This is two.dat") + std::string(512 - 15, '\0')
			;
		}

		virtual std::string content_1w2()
		{
			return STRING_WITH_NULLS(
				"\x04\x00"
				"ONE     " "\x01\x00"
				"TWO     " "\x03\x00"
				"\x00") + std::string(512 - (2+1 + 10*2), '\0') + STRING_WITH_NULLS(
				"Now resized to 513 chars") + std::string(1024 - 24, '\0') + STRING_WITH_NULLS(
				"This is two.dat") + std::string(512 - 15, '\0')
			;
		}

		virtual std::string content_63_files()
		{
			return STRING_WITH_NULLS(
				"\x33\x00"
				"ONE     " "\x01\x00"
				"TWO     " "\x02\x00"
				"        " "\x03\x00"
				"        " "\x04\x00"
				"        " "\x05\x00"
				"        " "\x06\x00"
				"        " "\x07\x00"
				"        " "\x08\x00"
				"        " "\x09\x00"
				"        " "\x0A\x00" // 10
				"        " "\x0B\x00"
				"        " "\x0C\x00"
				"        " "\x0D\x00"
				"        " "\x0E\x00"
				"        " "\x0F\x00"
				"        " "\x10\x00"
				"        " "\x11\x00"
				"        " "\x12\x00"
				"        " "\x13\x00"
				"        " "\x14\x00" // 20
				"        " "\x15\x00"
				"        " "\x16\x00"
				"        " "\x17\x00"
				"        " "\x18\x00"
				"        " "\x19\x00"
				"        " "\x1A\x00"
				"        " "\x1B\x00"
				"        " "\x1C\x00"
				"        " "\x1D\x00"
				"        " "\x1E\x00" // 30
				"        " "\x1F\x00"
				"        " "\x20\x00"
				"        " "\x21\x00"
				"        " "\x22\x00"
				"        " "\x23\x00"
				"        " "\x24\x00"
				"        " "\x25\x00"
				"        " "\x26\x00"
				"        " "\x27\x00"
				"        " "\x28\x00" // 40
				"        " "\x29\x00"
				"        " "\x2A\x00"
				"        " "\x2B\x00"
				"        " "\x2C\x00"
				"        " "\x2D\x00"
				"        " "\x2E\x00"
				"        " "\x2F\x00"
				"        " "\x30\x00"
				"        " "\x31\x00"
				"        " "\x32\x00" // 50
				"\x00") + std::string(512 - (2+1 + 10*50), '\0') + STRING_WITH_NULLS(
				"This is one.dat") + std::string(512 - 15, '\0') + STRING_WITH_NULLS(
				"This is two.dat") + std::string(512 - 15, '\0')
				+ std::string(512 * (50 - 2), '\0')
			;
		}

		virtual std::string content_64_files()
		{
			return STRING_WITH_NULLS(
				"\x35\x00"
				"ONE     " "\x02\x00"
				"TWO     " "\x03\x00"
				"        " "\x04\x00"
				"        " "\x05\x00"
				"        " "\x06\x00"
				"        " "\x07\x00"
				"        " "\x08\x00"
				"        " "\x09\x00"
				"        " "\x0A\x00"
				"        " "\x0B\x00" // 10
				"        " "\x0C\x00"
				"        " "\x0D\x00"
				"        " "\x0E\x00"
				"        " "\x0F\x00"
				"        " "\x10\x00"
				"        " "\x11\x00"
				"        " "\x12\x00"
				"        " "\x13\x00"
				"        " "\x14\x00"
				"        " "\x15\x00" // 20
				"        " "\x16\x00"
				"        " "\x17\x00"
				"        " "\x18\x00"
				"        " "\x19\x00"
				"        " "\x1A\x00"
				"        " "\x1B\x00"
				"        " "\x1C\x00"
				"        " "\x1D\x00"
				"        " "\x1E\x00"
				"        " "\x1F\x00" // 30
				"        " "\x20\x00"
				"        " "\x21\x00"
				"        " "\x22\x00"
				"        " "\x23\x00"
				"        " "\x24\x00"
				"        " "\x25\x00"
				"        " "\x26\x00"
				"        " "\x27\x00"
				"        " "\x28\x00"
				"        " "\x29\x00" // 40
				"        " "\x2A\x00"
				"        " "\x2B\x00"
				"        " "\x2C\x00"
				"        " "\x2D\x00"
				"        " "\x2E\x00"
				"        " "\x2F\x00"
				"        " "\x30\x00"
				"        " "\x31\x00"
				"        " "\x32\x00"
				"        " "\x33\x00" // 50
				"        " "\x34\x00" // 512 bytes at end of this fat entry
				"\x00") + std::string(512 - (/*2+*/1 + 10*0), '\0') + STRING_WITH_NULLS(
				"This is one.dat") + std::string(512 - 15, '\0') + STRING_WITH_NULLS(
				"This is two.dat") + std::string(512 - 15, '\0')
				+ std::string(512 * (51 - 2), '\0')
			;
		}

		virtual std::string content_65_files()
		{
			return STRING_WITH_NULLS(
				"\x36\x00"
				"ONE     " "\x02\x00"
				"TWO     " "\x03\x00"
				"        " "\x04\x00"
				"        " "\x05\x00"
				"        " "\x06\x00"
				"        " "\x07\x00"
				"        " "\x08\x00"
				"        " "\x09\x00"
				"        " "\x0A\x00"
				"        " "\x0B\x00" // 10
				"        " "\x0C\x00"
				"        " "\x0D\x00"
				"        " "\x0E\x00"
				"        " "\x0F\x00"
				"        " "\x10\x00"
				"        " "\x11\x00"
				"        " "\x12\x00"
				"        " "\x13\x00"
				"        " "\x14\x00"
				"        " "\x15\x00" // 20
				"        " "\x16\x00"
				"        " "\x17\x00"
				"        " "\x18\x00"
				"        " "\x19\x00"
				"        " "\x1A\x00"
				"        " "\x1B\x00"
				"        " "\x1C\x00"
				"        " "\x1D\x00"
				"        " "\x1E\x00"
				"        " "\x1F\x00" // 30
				"        " "\x20\x00"
				"        " "\x21\x00"
				"        " "\x22\x00"
				"        " "\x23\x00"
				"        " "\x24\x00"
				"        " "\x25\x00"
				"        " "\x26\x00"
				"        " "\x27\x00"
				"        " "\x28\x00"
				"        " "\x29\x00" // 40
				"        " "\x2A\x00"
				"        " "\x2B\x00"
				"        " "\x2C\x00"
				"        " "\x2D\x00"
				"        " "\x2E\x00"
				"        " "\x2F\x00"
				"        " "\x30\x00"
				"        " "\x31\x00"
				"        " "\x32\x00"
				"        " "\x33\x00" // 50
				"        " "\x34\x00" // 512 bytes at end of this fat entry
				"        " "\x35\x00"
				"\x00") + std::string(512 - (/*2+*/1 + 10*1), '\0') + STRING_WITH_NULLS(
				"This is one.dat") + std::string(512 - 15, '\0') + STRING_WITH_NULLS(
				"This is two.dat") + std::string(512 - 15, '\0')
				+ std::string(512 * (52 - 2), '\0')
			;
		}

		/// Insert the maximum number of files without expanding the FAT beyond one
		/// chunk.
		void test_noexpand_fat()
		{
			BOOST_TEST_MESSAGE(this->basename << ": Inserting max number of files without expanding the FAT");

			// Insert 48 files, bringing the total to 50
			for (int i = 0; i < 50-2; i++) {
				auto ep = this->pArchive->insert(
					nullptr, "", 0x10, "", Archive::File::Attribute::Default
				);
			}

			this->pArchive->flush();
			BOOST_CHECK_MESSAGE(
				this->is_content_equal(this->content_63_files()),
				"Inserting files without expanding the FAT didn't work"
			);
		}

		/// Insert enough files that the FAT has to expand to take up another chunk,
		/// just to fit the terminating null.
		void test_expand_fat_chunk()
		{
			BOOST_TEST_MESSAGE(this->basename << ": Inserting enough files to expand the FAT by a chunk just for the terminating null");

			// Insert 49 files, bringing the total to 51
			for (int i = 0; i < 51-2; i++) {
				auto ep = this->pArchive->insert(
					nullptr, "", 0x10, "", Archive::File::Attribute::Default
				);
			}

			this->pArchive->flush();
			BOOST_CHECK_MESSAGE(
				this->is_content_equal(this->content_64_files()),
				"Inserting files did not expand the FAT by a chunk, just for the terminating null"
			);
		}

		/// Insert enough files that the FAT has to expand to take up another chunk
		/// and write one file entry into it.
		void test_expand_fat_chunk_with_entry()
		{
			BOOST_TEST_MESSAGE(this->basename << ": Inserting enough files to expand the FAT by a chunk and writing FAT entries into it");

			// Insert 50 files, bringing the total to 52
			for (int i = 0; i < 52-2; i++) {
				auto ep = this->pArchive->insert(
					nullptr, "", 0x10, "", Archive::File::Attribute::Default
				);
			}

			this->pArchive->flush();
			BOOST_CHECK_MESSAGE(
				this->is_content_equal(this->content_65_files()),
				"Inserting files did not expand the FAT by a chunk"
			);
		}

		/// Insert enough files that the FAT has to expand to take up another chunk
		/// and write one file entry into it, then remove a few entries to confirm
		/// the extra chunk is removed again.
		void test_shrink_fat()
		{
			BOOST_TEST_MESSAGE(this->basename << ": Inserting and removing files to enlarge and shrink the FAT");

			// Insert 50 files, bringing the total to 52
			for (int i = 0; i < 52-2; i++) {
				auto ep = this->pArchive->insert(
					nullptr, "", 0x10, "", Archive::File::Attribute::Default
				);
			}

			this->pArchive->flush();
			BOOST_CHECK_MESSAGE(
				this->is_content_equal(this->content_65_files()),
				"Inserting files did not expand the FAT by a chunk"
			);

			// Now remove two files, to see if the FAT shrinks back to only one chunk
			for (int i = 0; i < 2; i++) {
				auto ep = this->getFileAt(this->pArchive->files(), 48);
				this->pArchive->remove(ep);
			}

			this->pArchive->flush();
			BOOST_CHECK_MESSAGE(
				this->is_content_equal(this->content_63_files()),
				"Removing files did not shrink the FAT by a chunk"
			);
		}
};

IMPLEMENT_TESTS(dat_zool);
