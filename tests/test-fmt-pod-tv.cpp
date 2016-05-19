/**
 * @file   test-fmt-pod-tv.cpp
 * @brief  Test code for Terminal Velocity .POD archives.
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

#define POD_DESC \
	"Startup 1.1 Gold" \
	"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
	"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
	"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
	"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"

class test_pod_tv: public test_archive
{
	public:
		test_pod_tv()
		{
			this->type = "pod-tv";
			this->lenMaxFilename = 32;

			Attribute comment;
			comment.type = Attribute::Type::Text;
			comment.textValue = "Startup 1.1 Gold";
			comment.textMaxLength = 80;
			this->attributes.push_back(comment);
		}

		void addTests()
		{
			this->test_archive::addTests();

			// c00: Initial state
			this->isInstance(ArchiveType::Certainty::DefinitelyYes, this->content_12());

			// c01: Control characters in filename
			this->isInstance(ArchiveType::Certainty::DefinitelyNo, STRING_WITH_NULLS(
				"\x02\x00\x00\x00" POD_DESC
				"ONE.DAT\x05\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\xa4\x00\x00\x00"
				"TWO.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\xb3\x00\x00\x00"
				"This is one.dat"
				"This is two.dat"
			));

			// c02: File offset past end of archive
			this->isInstance(ArchiveType::Certainty::DefinitelyNo, STRING_WITH_NULLS(
				"\x02\x00\x00\x00" POD_DESC
				"ONE.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\xa4\x01\x00\x00"
				"TWO.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\xb3\x00\x00\x00"
				"This is one.dat"
				"This is two.dat"
			));

			// c03: File length larger than archive
			this->isInstance(ArchiveType::Certainty::DefinitelyNo, STRING_WITH_NULLS(
				"\x02\x00\x00\x00" POD_DESC
				"ONE.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x01\x00\x00" "\xa4\x00\x00\x00"
				"TWO.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\xb3\x00\x00\x00"
				"This is one.dat"
				"This is two.dat"
			));

			// c04: Control characters in the description field
			this->isInstance(ArchiveType::Certainty::DefinitelyNo, STRING_WITH_NULLS(
				"\x02\x00\x00\x00"
				"Startup 1.1 Gold"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x05"
				"ONE.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\xa4\x00\x00\x00"
				"TWO.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\xb3\x00\x00\x00"
				"This is one.dat"
				"This is two.dat"
			));

			// a01: Shorten comment attribute
			this->changeAttribute(0, "Hello", STRING_WITH_NULLS(
				"\x02\x00\x00\x00"

				"Hello\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"

				"ONE.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\xa4\x00\x00\x00"
				"TWO.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\xb3\x00\x00\x00"
				"This is one.dat"
				"This is two.dat"
			));

			// a02: Lengthen comment attribute
			this->changeAttribute(0, "This is a test", STRING_WITH_NULLS(
				"\x02\x00\x00\x00"

				"This is a test\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"

				"ONE.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\xa4\x00\x00\x00"
				"TWO.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\xb3\x00\x00\x00"
				"This is one.dat"
				"This is two.dat"
			));
		}

		virtual std::string content_12()
		{
			return STRING_WITH_NULLS(
				"\x02\x00\x00\x00" POD_DESC
				"ONE.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\xa4\x00\x00\x00"
				"TWO.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\xb3\x00\x00\x00"
				"This is one.dat"
				"This is two.dat"
			);
		}

		virtual std::string content_1r2()
		{
			return STRING_WITH_NULLS(
				"\x02\x00\x00\x00" POD_DESC
				"THREE.DAT\0\0\0\0\0\0\0"   "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\xa4\x00\x00\x00"
				"TWO.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\xb3\x00\x00\x00"
				"This is one.dat"
				"This is two.dat"
			);
		}

		virtual std::string content_123()
		{
			return STRING_WITH_NULLS(
				"\x03\x00\x00\x00" POD_DESC
				"ONE.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\xcc\x00\x00\x00"
				"TWO.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\xdb\x00\x00\x00"
				"THREE.DAT\0\0\0\0\0\0\0"   "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x11\x00\x00\x00" "\xea\x00\x00\x00"
				"This is one.dat"
				"This is two.dat"
				"This is three.dat"
			);
		}

		virtual std::string content_132()
		{
			return STRING_WITH_NULLS(
				"\x03\x00\x00\x00" POD_DESC
				"ONE.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\xcc\x00\x00\x00"
				"THREE.DAT\0\0\0\0\0\0\0"   "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x11\x00\x00\x00" "\xdb\x00\x00\x00"
				"TWO.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\xec\x00\x00\x00"
				"This is one.dat"
				"This is three.dat"
				"This is two.dat"
			);
		}

		virtual std::string content_1342()
		{
			return STRING_WITH_NULLS(
				"\x04\x00\x00\x00" POD_DESC
				"ONE.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\xf4\x00\x00\x00"
				"THREE.DAT\0\0\0\0\0\0\0"   "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x11\x00\x00\x00" "\x03\x01\x00\x00"
				"FOUR.DAT\0\0\0\0\0\0\0\0"  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x10\x00\x00\x00" "\x14\x01\x00\x00"
				"TWO.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\x24\x01\x00\x00"
				"This is one.dat"
				"This is three.dat"
				"This is four.dat"
				"This is two.dat"
			);
		}

		virtual std::string content_2()
		{
			return STRING_WITH_NULLS(
				"\x01\x00\x00\x00" POD_DESC
				"TWO.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\x7c\x00\x00\x00"
				"This is two.dat"
			);
		}

		virtual std::string content_0()
		{
			return STRING_WITH_NULLS(
				"\x00\x00\x00\x00" POD_DESC
			);
		}

		virtual std::string content_32()
		{
			return STRING_WITH_NULLS(
				"\x02\x00\x00\x00" POD_DESC
				"THREE.DAT\0\0\0\0\0\0\0"   "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x11\x00\x00\x00" "\xa4\x00\x00\x00"
				"TWO.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\xb5\x00\x00\x00"
				"This is three.dat"
				"This is two.dat"
			);
		}

		virtual std::string content_21()
		{
			return STRING_WITH_NULLS(
				"\x02\x00\x00\x00" POD_DESC
				"TWO.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\xa4\x00\x00\x00"
				"ONE.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\xb3\x00\x00\x00"
				"This is two.dat"
				"This is one.dat"
			);
		}

		virtual std::string content_1l2()
		{
			return STRING_WITH_NULLS(
				"\x02\x00\x00\x00" POD_DESC
				"ONE.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x14\x00\x00\x00" "\xa4\x00\x00\x00"
				"TWO.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\xb8\x00\x00\x00"
				"This is one.dat\0\0\0\0\0"
				"This is two.dat"
			);
		}

		virtual std::string content_1s2()
		{
			return STRING_WITH_NULLS(
				"\x02\x00\x00\x00" POD_DESC
				"ONE.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0a\x00\x00\x00" "\xa4\x00\x00\x00"
				"TWO.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\xae\x00\x00\x00"
				"This is on"
				"This is two.dat"
			);
		}

		virtual std::string content_1w2()
		{
			return STRING_WITH_NULLS(
				"\x02\x00\x00\x00" POD_DESC
				"ONE.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x17\x00\x00\x00" "\xa4\x00\x00\x00"
				"TWO.DAT\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\x0f\x00\x00\x00" "\xbb\x00\x00\x00"
				"Now resized to 23 chars"
				"This is two.dat"
			);
		}
};

IMPLEMENT_TESTS(pod_tv);
