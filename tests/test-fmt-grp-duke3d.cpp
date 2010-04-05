/*
 * test-fmt-grp-duke3d.cpp - test code for GRPArchive class.
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

#include <boost/test/unit_test.hpp>

#include <boost/algorithm/string.hpp> // for case-insensitive string compare
#include <boost/iostreams/copy.hpp>
#include <camoto/gamearchive.hpp>
#include <iostream>
#include <iomanip>

#include "tests.hpp"

// Local headers that will not be installed
#include "../src/segmented_stream.hpp"

namespace ga = camoto::gamearchive;

// Allow a string constant to be passed around with embedded nulls
#define makeString(x)  std::string((x), sizeof((x)) - 1)

struct fmt_grp_duke3d_sample: public default_sample {

	boost::shared_ptr<std::stringstream> psstrBase;
	void *_do; // unused var, but allows a statement to run in constructor init
	camoto::iostream_sptr psBase;
	boost::shared_ptr<ga::Archive> pArchive;

	fmt_grp_duke3d_sample() :
		psstrBase(new std::stringstream),
		_do((*this->psstrBase) << makeString(
			"KenSilverman"      "\x02\x00\x00\x00"
			"ONE.DAT\0\0\0\0\0" "\x0f\x00\x00\x00"
			"TWO.DAT\0\0\0\0\0" "\x0f\x00\x00\x00"
			"This is one.dat"
			"This is two.dat"
		)),
		psBase(this->psstrBase)
	{
		this->psstrBase->exceptions(std::ios::badbit | std::ios::failbit | std::ios::eofbit);

		boost::shared_ptr<ga::Manager> pManager(ga::getManager());
		ga::Manager::arch_sptr pTestType(pManager->getArchiveTypeByCode("grp-duke3d"));
		this->pArchive = boost::shared_ptr<ga::Archive>(pTestType->open(psBase));
		BOOST_REQUIRE_MESSAGE(pArchive, "Could not create archive class");
	}

	fmt_grp_duke3d_sample(const char *data) :
		psstrBase(new std::stringstream),
		_do((*this->psstrBase) << data),
		psBase(this->psstrBase)
	{
		boost::shared_ptr<ga::Manager> pManager(ga::getManager());
		ga::Manager::arch_sptr pTestType(pManager->getArchiveTypeByCode("grp-duke3d"));
		this->pArchive = boost::shared_ptr<ga::Archive>(pTestType->open(psBase));
		BOOST_REQUIRE_MESSAGE(pArchive, "Could not create archive class");
	}

	boost::test_tools::predicate_result is_equal(const std::string& strExpected)
	{
		// Flush out any changes before we perform the check
		BOOST_CHECK_NO_THROW(
			this->pArchive->flush()
		);

		return this->default_sample::is_equal(strExpected, psstrBase->str());
	}

};
BOOST_FIXTURE_TEST_SUITE(fmt_grp_duke3d_suite, fmt_grp_duke3d_sample)

// Make sure a corrupted file doesn't segfault
BOOST_AUTO_TEST_CASE(fmt_grp_duke3d_open_invalid)
{
	BOOST_TEST_MESSAGE("Opening invalid archive");

	// Find the GRP file handler
	boost::shared_ptr<ga::Manager> pManager(ga::getManager());
	ga::Manager::arch_sptr pTestType(pManager->getArchiveTypeByCode("grp-duke3d"));

	// Prepare an invalid GRP file
	boost::shared_ptr<std::stringstream> psstrBase(new std::stringstream);
	(*psstrBase) << makeString(
		"KenSilverman"      "\xff\xff\xff\xf0"
		"ONE.DAT\0\0\0\0\0" "\x0f\x00\x00\x00"
		"This is one.dat"
	);
	camoto::iostream_sptr psBase(psstrBase);

	// Try to open the invalid file.  This will result in an attempt to allocate
	// ~16GB of memory.  This is an error condition (the file is corrupt/invalid)
	// but it may succeed on a system with a lot of RAM!
	BOOST_CHECK_THROW(
		boost::shared_ptr<ga::Archive> pArchive(pTestType->open(psBase)),
		std::ios::failure
	);
}

BOOST_AUTO_TEST_CASE(fmt_grp_duke3d_open)
{
	BOOST_TEST_MESSAGE("Opening file in archive");

	// Find the file we're going to open
	ga::Archive::EntryPtr ep = pArchive->find("ONE.DAT");

	// Make sure we found it
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(ep),
		"Couldn't find ONE.DAT in sample archive");

	// Open it
	camoto::iostream_sptr pfsIn(pArchive->open(ep));
	std::stringstream out;
	out.exceptions(std::ios::badbit | std::ios::failbit | std::ios::eofbit);

	// Make sure the file opens at the start
	BOOST_REQUIRE_EQUAL(pfsIn->tellg(), 0);

	// Copy it into the stringstream
	boost::iostreams::copy(*pfsIn, out);

	BOOST_CHECK_MESSAGE(
		default_sample::is_equal(makeString(
			"This is one.dat"
		), out.str()),
		"Error opening file or wrong file opened"
	);
}

BOOST_AUTO_TEST_CASE(fmt_grp_duke3d_rename)
{
	BOOST_TEST_MESSAGE("Renaming file inside archive");

	// Find the file we're going to rename
	ga::Archive::EntryPtr ep = pArchive->find("ONE.DAT");

	// Make sure we found it
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(ep),
		"Couldn't find ONE.DAT in sample archive");

	// Swap the file positions
	pArchive->rename(ep, "HELLO.BIN");

	BOOST_CHECK_MESSAGE(
		is_equal(makeString(
			"KenSilverman"      "\x02\x00\x00\x00"
			"HELLO.BIN\0\0\0"   "\x0f\x00\x00\x00"
			"TWO.DAT\0\0\0\0\0" "\x0f\x00\x00\x00"
			"This is one.dat"
			"This is two.dat"
		)),
		"Error renaming file"
	);
}

BOOST_AUTO_TEST_CASE(fmt_grp_duke3d_insert_long)
{
	BOOST_TEST_MESSAGE("Inserting file with name too long");

	BOOST_CHECK_THROW(
		ga::Archive::EntryPtr ep = pArchive->insert(ga::Archive::EntryPtr(), "123456789.DAT", 5),
		std::ios::failure
	);

	BOOST_CHECK_NO_THROW(
		ga::Archive::EntryPtr ep = pArchive->insert(ga::Archive::EntryPtr(), "12345678.DAT", 5)
	);

}

BOOST_AUTO_TEST_CASE(fmt_grp_duke3d_insert_end)
{
	BOOST_TEST_MESSAGE("Inserting file into archive");

	// Insert the file
	ga::Archive::EntryPtr ep = pArchive->insert(ga::Archive::EntryPtr(), "THREE.DAT", 17);

	// Make sure it went in ok
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(ep),
		"Couldn't create new file in sample archive");

	// Open it
	boost::shared_ptr<std::iostream> pfsNew(pArchive->open(ep));
	pfsNew->write("This is three.dat", 17);
	pfsNew->flush();

	BOOST_CHECK_MESSAGE(
		is_equal(makeString(
			"KenSilverman"      "\x03\x00\x00\x00"
			"ONE.DAT\0\0\0\0\0" "\x0f\x00\x00\x00"
			"TWO.DAT\0\0\0\0\0" "\x0f\x00\x00\x00"
			"THREE.DAT\0\0\0"   "\x11\x00\x00\x00"
			"This is one.dat"
			"This is two.dat"
			"This is three.dat"
		)),
		"Error inserting file at end of archive"
	);
}

BOOST_AUTO_TEST_CASE(fmt_grp_duke3d_insert_mid)
{
	BOOST_TEST_MESSAGE("Inserting file into middle of archive");

	// Find the file we're going to insert before
	ga::Archive::EntryPtr idBefore = pArchive->find("TWO.DAT");

	// Make sure we found it
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(idBefore),
		"Couldn't find TWO.DAT in sample archive");

	// Insert the file
	ga::Archive::EntryPtr ep = pArchive->insert(idBefore, "THREE.DAT", 17);

	// Make sure it went in ok
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(ep),
		"Couldn't insert new file in sample archive");

	// Open it
	boost::shared_ptr<std::iostream> pfsNew(pArchive->open(ep));
	pfsNew->write("This is three.dat", 17);
	pfsNew->flush();

	BOOST_CHECK_MESSAGE(
		is_equal(makeString(
			"KenSilverman"      "\x03\x00\x00\x00"
			"ONE.DAT\0\0\0\0\0" "\x0f\x00\x00\x00"
			"THREE.DAT\0\0\0"   "\x11\x00\x00\x00"
			"TWO.DAT\0\0\0\0\0" "\x0f\x00\x00\x00"
			"This is one.dat"
			"This is three.dat"
			"This is two.dat"
		)),
		"Error inserting file in middle of archive"
	);
}

BOOST_AUTO_TEST_CASE(fmt_grp_duke3d_insert2)
{
	BOOST_TEST_MESSAGE("Inserting multiple files");

	// Find the file we're going to insert before
	ga::Archive::EntryPtr idBefore = pArchive->find("TWO.DAT");

	// Make sure we found it
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(idBefore),
		"Couldn't find TWO.DAT in sample archive");

	// Insert the file
	ga::Archive::EntryPtr ep1 = pArchive->insert(idBefore, "THREE.DAT", 17);

	// Make sure it went in ok
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(ep1),
		"Couldn't insert first new file in sample archive");

	// Open it
	boost::shared_ptr<std::iostream> pfsNew1(pArchive->open(ep1));
	pfsNew1->write("This is three.dat", 17);
	pfsNew1->flush();

	// Find the file we're going to insert before (since the previous insert
	// invalidated all EntryPtrs.)
	idBefore = pArchive->find("TWO.DAT");

	// Make sure we found it
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(idBefore),
		"Couldn't find TWO.DAT in sample archive after first insert");

	// Insert the file
	ga::Archive::EntryPtr ep2 = pArchive->insert(idBefore, "FOUR.DAT", 16);

	// Make sure it went in ok
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(ep2),
		"Couldn't insert second new file in sample archive");

	boost::shared_ptr<std::iostream> pfsNew2(pArchive->open(ep2));
	pfsNew2->write("This is four.dat", 16);
	pfsNew2->flush();

	BOOST_CHECK_MESSAGE(
		is_equal(makeString(
			"KenSilverman"      "\x04\x00\x00\x00"
			"ONE.DAT\0\0\0\0\0" "\x0f\x00\x00\x00"
			"THREE.DAT\0\0\0"   "\x11\x00\x00\x00"
			"FOUR.DAT\0\0\0\0"  "\x10\x00\x00\x00"
			"TWO.DAT\0\0\0\0\0" "\x0f\x00\x00\x00"
			"This is one.dat"
			"This is three.dat"
			"This is four.dat"
			"This is two.dat"
		)),
		"Error inserting two files"
	);
}

BOOST_AUTO_TEST_CASE(fmt_grp_duke3d_remove)
{
	BOOST_TEST_MESSAGE("Removing file from archive");

	// Find the file we're going to remove
	ga::Archive::EntryPtr ep = pArchive->find("ONE.DAT");

	// Make sure we found it
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(ep),
		"Couldn't find ONE.DAT in sample archive");

	// Remove it
	pArchive->remove(ep);

	BOOST_CHECK_MESSAGE(
		is_equal(makeString(
			"KenSilverman"      "\x01\x00\x00\x00"
			"TWO.DAT\0\0\0\0\0" "\x0f\x00\x00\x00"
			"This is two.dat"
		)),
		"Error removing file"
	);
}

BOOST_AUTO_TEST_CASE(fmt_grp_duke3d_remove2)
{
	BOOST_TEST_MESSAGE("Removing multiple files from archive");

	// Find the files we're going to remove
	ga::Archive::EntryPtr ep1 = pArchive->find("ONE.DAT");
	ga::Archive::EntryPtr ep2 = pArchive->find("TWO.DAT");

	// Make sure we found it
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(ep1),
		"Couldn't find ONE.DAT in sample archive");
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(ep2),
		"Couldn't find TWO.DAT in sample archive");

	// Remove it
	pArchive->remove(ep1);
	pArchive->remove(ep2);

	BOOST_CHECK_MESSAGE(
		is_equal(makeString(
			"KenSilverman"      "\x00\x00\x00\x00"
		)),
		"Error removing multiple files"
	);
}

BOOST_AUTO_TEST_CASE(fmt_grp_duke3d_insert_remove)
{
	BOOST_TEST_MESSAGE("Insert then remove file from archive");

	// Find the file we're going to insert before
	ga::Archive::EntryPtr idBefore = pArchive->find("TWO.DAT");

	// Make sure we found it
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(idBefore),
		"Couldn't find TWO.DAT in sample archive");

	// Insert the file
	ga::Archive::EntryPtr ep = pArchive->insert(idBefore, "THREE.DAT", 17);

	// Make sure it went in ok
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(ep),
		"Couldn't insert new file in sample archive");

	// Open it
	boost::shared_ptr<std::iostream> pfsNew(pArchive->open(ep));
	pfsNew->write("This is three.dat", 17);
	pfsNew->flush();

	// Find the file we're going to remove
	ga::Archive::EntryPtr ep2 = pArchive->find("ONE.DAT");

	// Make sure we found it
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(ep2),
		"Couldn't find ONE.DAT in sample archive");

	// Remove it
	pArchive->remove(ep2);

	BOOST_CHECK_MESSAGE(
		is_equal(makeString(
			"KenSilverman"      "\x02\x00\x00\x00"
			"THREE.DAT\0\0\0"   "\x11\x00\x00\x00"
			"TWO.DAT\0\0\0\0\0" "\x0f\x00\x00\x00"
			"This is three.dat"
			"This is two.dat"
		)),
		"Error inserting then removing file"
	);
}

BOOST_AUTO_TEST_CASE(fmt_grp_duke3d_remove_insert)
{
	BOOST_TEST_MESSAGE("Remove then insert file from archive");

	// Find the file we're going to remove
	ga::Archive::EntryPtr ep2 = pArchive->find("ONE.DAT");

	// Make sure we found it
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(ep2),
		"Couldn't find ONE.DAT in sample archive");

	// Remove it
	pArchive->remove(ep2);

	// Find the file we're going to insert before
	ga::Archive::EntryPtr idBefore = pArchive->find("TWO.DAT");

	// Make sure we found it
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(idBefore),
		"Couldn't find TWO.DAT in sample archive");

	// Insert the file
	ga::Archive::EntryPtr ep = pArchive->insert(idBefore, "THREE.DAT", 17);

	// Make sure it went in ok
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(ep),
		"Couldn't insert new file in sample archive");

	// Open it
	boost::shared_ptr<std::iostream> pfsNew(pArchive->open(ep));
	pfsNew->write("This is three.dat", 17);
	pfsNew->flush();

	BOOST_CHECK_MESSAGE(
		is_equal(makeString(
			"KenSilverman"      "\x02\x00\x00\x00"
			"THREE.DAT\0\0\0"   "\x11\x00\x00\x00"
			"TWO.DAT\0\0\0\0\0" "\x0f\x00\x00\x00"
			"This is three.dat"
			"This is two.dat"
		)),
		"Error removing then inserting file"
	);
}

BOOST_AUTO_TEST_CASE(fmt_grp_duke3d_move)
{
	BOOST_TEST_MESSAGE("Moving file inside archive");

	// Find the file we're going to move
	ga::Archive::EntryPtr ep1 = pArchive->find("ONE.DAT");
	ga::Archive::EntryPtr ep2 = pArchive->find("TWO.DAT");

	// Make sure we found it
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(ep1),
		"Couldn't find ONE.DAT in sample archive");
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(ep2),
		"Couldn't find TWO.DAT in sample archive");

	// Swap the file positions
	pArchive->move(ep1, ep2);

	BOOST_CHECK_MESSAGE(
		is_equal(makeString(
			"KenSilverman"      "\x02\x00\x00\x00"
			"TWO.DAT\0\0\0\0\0" "\x0f\x00\x00\x00"
			"ONE.DAT\0\0\0\0\0" "\x0f\x00\x00\x00"
			"This is two.dat"
			"This is one.dat"
		)),
		"Error moving file"
	);
}

BOOST_AUTO_TEST_CASE(fmt_grp_duke3d_resize_larger)
{
	BOOST_TEST_MESSAGE("Enlarging a file inside the archive");

	// Find the file we're going to resize
	ga::Archive::EntryPtr ep = pArchive->find("ONE.DAT");

	// Make sure we found it
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(ep),
		"Couldn't find ONE.DAT in sample archive");

	// Swap the file positions
	pArchive->resize(ep, 20);

	BOOST_CHECK_MESSAGE(
		is_equal(makeString(
			"KenSilverman"      "\x02\x00\x00\x00"
			"ONE.DAT\0\0\0\0\0" "\x14\x00\x00\x00"
			"TWO.DAT\0\0\0\0\0" "\x0f\x00\x00\x00"
			"This is one.dat\0\0\0\0\0"
			"This is two.dat"
		)),
		"Error enlarging a file"
	);
}

BOOST_AUTO_TEST_CASE(fmt_grp_duke3d_resize_smaller)
{
	BOOST_TEST_MESSAGE("Shrink a file inside the archive");

	// Find the file we're going to resize
	ga::Archive::EntryPtr ep = pArchive->find("ONE.DAT");

	// Make sure we found it
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(ep),
		"Couldn't find ONE.DAT in sample archive");

	// Swap the file positions
	pArchive->resize(ep, 10);

	BOOST_CHECK_MESSAGE(
		is_equal(makeString(
			"KenSilverman"      "\x02\x00\x00\x00"
			"ONE.DAT\0\0\0\0\0" "\x0a\x00\x00\x00"
			"TWO.DAT\0\0\0\0\0" "\x0f\x00\x00\x00"
			"This is on"
			"This is two.dat"
		)),
		"Error shrinking a file"
	);
}

BOOST_AUTO_TEST_CASE(fmt_grp_duke3d_resize_write)
{
	BOOST_TEST_MESSAGE("Enlarging a file inside the archive");

	// Find the file we're going to resize
	ga::Archive::EntryPtr ep = pArchive->find("ONE.DAT");

	// Make sure we found it
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(ep),
		"Couldn't find ONE.DAT in sample archive");

	// Swap the file positions
	pArchive->resize(ep, 23);

	boost::shared_ptr<std::iostream> pfsNew(pArchive->open(ep));
	pfsNew->write("Now resized to 23 chars", 23);
	pfsNew->flush();

	BOOST_CHECK_MESSAGE(
		is_equal(makeString(
			"KenSilverman"      "\x02\x00\x00\x00"
			"ONE.DAT\0\0\0\0\0" "\x17\x00\x00\x00"
			"TWO.DAT\0\0\0\0\0" "\x0f\x00\x00\x00"
			"Now resized to 23 chars"
			"This is two.dat"
		)),
		"Error enlarging a file"
	);

	// Open the file following it to make sure it was moved out of the way

	// Find the file we're going to open
	ga::Archive::EntryPtr ep2 = pArchive->find("TWO.DAT");

	// Make sure we found it
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(ep2),
		"Couldn't find TWO.DAT in sample archive");

	// Open it
	camoto::iostream_sptr pfsIn(pArchive->open(ep2));
	std::stringstream out;
	out.exceptions(std::ios::badbit | std::ios::failbit | std::ios::eofbit);

	// Copy it into the stringstream
	boost::iostreams::copy(*pfsIn, out);

	BOOST_CHECK_MESSAGE(
		default_sample::is_equal(makeString(
			"This is two.dat"
		), out.str()),
		"Unrelated file was corrupted after file resize operation"
	);

}

BOOST_AUTO_TEST_SUITE_END()
