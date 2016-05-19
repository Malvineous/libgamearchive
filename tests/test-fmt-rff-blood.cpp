/**
 * @file   test-fmt-rff-blood.cpp
 * @brief  Test code for unencrypted Blood .RFF archives.
 *
 * Copyright (C) 2010-2015 Adam Nielsen <malvineous@shikadi.net>
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

// TODO: tests with 1) <8 char filename, 2) 8 char filename, 3) <3 char file ext

class test_rff_blood: public test_archive
{
	public:
		test_rff_blood()
		{
			this->type = "rff-blood";
			this->lenMaxFilename = 12;

			Attribute ver;
			ver.type = Attribute::Type::Enum;
			ver.enumValue = 0; // version 0x200
			this->attributes.push_back(ver);
		}

		void addTests()
		{
			this->test_archive::addTests();

			ADD_ARCH_TEST(false, &test_rff_blood::test_insert_long_base);
			ADD_ARCH_TEST(false, &test_rff_blood::test_insert_long_nodot);
			ADD_ARCH_TEST(false, &test_rff_blood::test_insert_long_ext);

			// c00: Initial state
			this->isInstance(ArchiveType::Certainty::DefinitelyYes, this->content_12());

			// c01: Bad signature
			this->isInstance(ArchiveType::Certainty::DefinitelyNo, STRING_WITH_NULLS(
				"FFR\x1a" "\x00\x02\x00\x00" "\x3e\x00\x00\x00" "\x02\x00\x00\x00"
				"\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"This is one.dat"
				"This is two.dat"

				"\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"\x20\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"\x00" "DATONE\0\0\0\0\0" "\x00\x00\x00\x00"

				"\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"\x2f\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"\x00" "DATTWO\0\0\0\0\0" "\x00\x00\x00\x00"
			));

			// c02: File too short
			this->isInstance(ArchiveType::Certainty::DefinitelyNo, STRING_WITH_NULLS(
				"RFF"
			));

			// i01: Excessively large number of files
			this->invalidContent(STRING_WITH_NULLS(
				"RFF\x1a" "\x00\x02\x00\x00" "\x3e\x00\x00\x00" "\x02\x00\x00\xf0"
				"\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"This is one.dat"
				"This is two.dat"

				"\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"\x20\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"\x00" "DATONE\0\0\0\0\0" "\x00\x00\x00\x00"

				"\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"\x2f\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"\x00" "DATTWO\0\0\0\0\0" "\x00\x00\x00\x00"
				));
		}

		virtual std::string content_12()
		{
			return STRING_WITH_NULLS(
				"RFF\x1a" "\x00\x02\x00\x00" "\x3e\x00\x00\x00" "\x02\x00\x00\x00"
				"\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"This is one.dat"
				"This is two.dat"

				"\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"\x20\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"\x00" "DATONE\0\0\0\0\0" "\x00\x00\x00\x00"

				"\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"\x2f\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"\x00" "DATTWO\0\0\0\0\0" "\x00\x00\x00\x00"
			);
		}

		virtual std::string content_1r2()
		{
			return STRING_WITH_NULLS(
				"RFF\x1a" "\x00\x02\x00\x00" "\x3e\x00\x00\x00" "\x02\x00\x00\x00"
				"\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"This is one.dat"
				"This is two.dat"

				"\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"\x20\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"\x00" "DATTHREE\0\0\0" "\x00\x00\x00\x00"

				"\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"\x2f\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"\x00" "DATTWO\0\0\0\0\0" "\x00\x00\x00\x00"
			);
		}

		virtual std::string content_123()
		{
			return STRING_WITH_NULLS(
				"RFF\x1a" "\x00\x02\x00\x00" "\x4f\x00\x00\x00" "\x03\x00\x00\x00"
				"\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"This is one.dat"
				"This is two.dat"
				"This is three.dat"

				"\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"\x20\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"\x00" "DATONE\0\0\0\0\0" "\x00\x00\x00\x00"

				"\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"\x2f\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"\x00" "DATTWO\0\0\0\0\0" "\x00\x00\x00\x00"

				"\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"\x3e\x00\x00\x00" "\x11\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"\x00" "DATTHREE\0\0\0"   "\x00\x00\x00\x00"
			);
		}

		virtual std::string content_132()
		{
			return STRING_WITH_NULLS(
				"RFF\x1a" "\x00\x02\x00\x00" "\x4f\x00\x00\x00" "\x03\x00\x00\x00"
				"\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"This is one.dat"
				"This is three.dat"
				"This is two.dat"

				"\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"\x20\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"\x00" "DATONE\0\0\0\0\0" "\x00\x00\x00\x00"

				"\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"\x2f\x00\x00\x00" "\x11\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"\x00" "DATTHREE\0\0\0"   "\x00\x00\x00\x00"

				"\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"\x40\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"\x00" "DATTWO\0\0\0\0\0" "\x00\x00\x00\x00"
			);
		}

		virtual std::string content_1342()
		{
			return STRING_WITH_NULLS(
				"RFF\x1a" "\x00\x02\x00\x00" "\x5f\x00\x00\x00" "\x04\x00\x00\x00"
				"\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"This is one.dat"
				"This is three.dat"
				"This is four.dat"
				"This is two.dat"

				"\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"\x20\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"\x00" "DATONE\0\0\0\0\0" "\x00\x00\x00\x00"

				"\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"\x2f\x00\x00\x00" "\x11\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"\x00" "DATTHREE\0\0\0"   "\x00\x00\x00\x00"

				"\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"\x40\x00\x00\x00" "\x10\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"\x00" "DATFOUR\0\0\0\0"  "\x00\x00\x00\x00"

				"\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"\x50\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"\x00" "DATTWO\0\0\0\0\0" "\x00\x00\x00\x00"
			);
		}

		virtual std::string content_2()
		{
			return STRING_WITH_NULLS(
				"RFF\x1a" "\x00\x02\x00\x00" "\x2f\x00\x00\x00" "\x01\x00\x00\x00"
				"\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"This is two.dat"

				"\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"\x20\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"\x00" "DATTWO\0\0\0\0\0" "\x00\x00\x00\x00"
			);
		}

		virtual std::string content_0()
		{
			return STRING_WITH_NULLS(
				"RFF\x1a" "\x00\x02\x00\x00" "\x20\x00\x00\x00" "\x00\x00\x00\x00"
				"\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
			);
		}

		virtual std::string content_32()
		{
			return STRING_WITH_NULLS(
				"RFF\x1a" "\x00\x02\x00\x00" "\x40\x00\x00\x00" "\x02\x00\x00\x00"
				"\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"This is three.dat"
				"This is two.dat"

				"\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"\x20\x00\x00\x00" "\x11\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"\x00" "DATTHREE\0\0\0"   "\x00\x00\x00\x00"

				"\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"\x31\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"\x00" "DATTWO\0\0\0\0\0" "\x00\x00\x00\x00"
			);
		}

		virtual std::string content_21()
		{
			return STRING_WITH_NULLS(
				"RFF\x1a" "\x00\x02\x00\x00" "\x3e\x00\x00\x00" "\x02\x00\x00\x00"
				"\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"This is two.dat"
				"This is one.dat"

				"\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"\x20\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"\x00" "DATTWO\0\0\0\0\0" "\x00\x00\x00\x00"

				"\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"\x2f\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"\x00" "DATONE\0\0\0\0\0" "\x00\x00\x00\x00"
			);
		}

		virtual std::string content_1l2()
		{
			return STRING_WITH_NULLS(
				"RFF\x1a" "\x00\x02\x00\x00" "\x43\x00\x00\x00" "\x02\x00\x00\x00"
				"\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"This is one.dat\0\0\0\0\0"
				"This is two.dat"

				"\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"\x20\x00\x00\x00" "\x14\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"\x00" "DATONE\0\0\0\0\0" "\x00\x00\x00\x00"

				"\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"\x34\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"\x00" "DATTWO\0\0\0\0\0" "\x00\x00\x00\x00"
			);
		}

		virtual std::string content_1s2()
		{
			return STRING_WITH_NULLS(
				"RFF\x1a" "\x00\x02\x00\x00" "\x39\x00\x00\x00" "\x02\x00\x00\x00"
				"\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"This is on"
				"This is two.dat"

				"\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"\x20\x00\x00\x00" "\x0a\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"\x00" "DATONE\0\0\0\0\0" "\x00\x00\x00\x00"

				"\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"\x2a\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"\x00" "DATTWO\0\0\0\0\0" "\x00\x00\x00\x00"
			);
		}

		virtual std::string content_1w2()
		{
			return STRING_WITH_NULLS(
				"RFF\x1a" "\x00\x02\x00\x00" "\x46\x00\x00\x00" "\x02\x00\x00\x00"
				"\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"Now resized to 23 chars"
				"This is two.dat"

				"\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"\x20\x00\x00\x00" "\x17\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"\x00" "DATONE\0\0\0\0\0" "\x00\x00\x00\x00"

				"\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"\x37\x00\x00\x00" "\x0f\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00"
				"\x00" "DATTWO\0\0\0\0\0" "\x00\x00\x00\x00"
			);
		}

		void test_insert_long_base()
		{
			BOOST_TEST_MESSAGE(this->basename << ": Inserting file with basename too long");

			Archive::FileHandle epb = this->findFile(0);

			const char *name = "123456789.A";

			BOOST_CHECK_THROW(
				Archive::FileHandle ep = this->pArchive->insert(
					epb, name, 5, FILETYPE_GENERIC, Archive::File::Attribute::Default
				),
				stream::error
			);

			BOOST_CHECK_MESSAGE(
				this->is_content_equal(this->content_12()),
				"Archive corrupted after failed insert"
			);
		}

		void test_insert_long_nodot()
		{
			BOOST_TEST_MESSAGE(this->basename << ": Inserting file with no dot and name too long");

			Archive::FileHandle epb = this->findFile(0);

			const char *name = "123456789";

			BOOST_CHECK_THROW(
				Archive::FileHandle ep = this->pArchive->insert(
					epb, name, 5, FILETYPE_GENERIC, Archive::File::Attribute::Default
				),
				stream::error
			);

			BOOST_CHECK_MESSAGE(
				this->is_content_equal(this->content_12()),
				"Archive corrupted after failed insert"
			);
		}

		void test_insert_long_ext()
		{
			BOOST_TEST_MESSAGE(this->basename << ": Inserting file with extension too long");

			Archive::FileHandle epb = this->findFile(0);

			const char *name = "12345.ABCD";

			BOOST_CHECK_THROW(
				Archive::FileHandle ep = this->pArchive->insert(
					epb, name, 5, FILETYPE_GENERIC, Archive::File::Attribute::Default
				),
				stream::error
			);

			BOOST_CHECK_MESSAGE(
				this->is_content_equal(this->content_12()),
				"Archive corrupted after failed insert"
			);
		}
};

IMPLEMENT_TESTS(rff_blood);
