/*
 * test-archive.hpp - generic test code for Archive class descendents.
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
#include <boost/bind.hpp>
#include <camoto/gamearchive.hpp>
#include <iostream>
#include <iomanip>

#include "tests.hpp"

// Local headers that will not be installed
#include <camoto/segmented_stream.hpp>

using namespace camoto;
namespace ga = camoto::gamearchive;

// Defines to allow code reuse
#define COMBINE_CLASSNAME_EXP(c, n)  c ## _ ## n
#define COMBINE_CLASSNAME(c, n)  COMBINE_CLASSNAME_EXP(c, n)

#define TEST_VAR(n)        COMBINE_CLASSNAME(ARCHIVE_CLASS, n)
#define TEST_NAME(n)       TEST_VAR(n)
#define TEST_RESULT(n)     testdata_ ## n

#define FIXTURE_NAME       TEST_VAR(sample)
#define EMPTY_FIXTURE_NAME TEST_VAR(sample_empty)
#define SUITE_NAME         TEST_VAR(suite)
#define EMPTY_SUITE_NAME   TEST_VAR(suite_empty)
#define INITIALSTATE_NAME  TEST_RESULT(initialstate)

// Allow a string constant to be passed around with embedded nulls
#define makeString(x)  std::string((x), sizeof((x)) - 1)

#include "../src/fatarchive.hpp"

#ifndef INSERT_ATTRIBUTE
/// Default attributes for newly inserted files, unless overridden
#define INSERT_ATTRIBUTE ga::EA_NONE
#endif

struct FIXTURE_NAME: public default_sample {

	typedef boost::shared_ptr<std::stringstream> sstr_ptr;

	sstr_ptr baseData;
	void *_do; // unused var, but allows a statement to run in constructor init
	camoto::iostream_sptr baseStream;
	boost::shared_ptr<ga::Archive> pArchive;
	ga::MP_SUPPDATA suppData;
	std::map<ga::E_SUPPTYPE, sstr_ptr> suppBase;

	FIXTURE_NAME() :
		baseData(new std::stringstream),
		_do((*this->baseData) << makeString(INITIALSTATE_NAME)),
		baseStream(this->baseData)
	{
		#ifdef HAS_FAT
		{
			boost::shared_ptr<std::stringstream> suppSS(new std::stringstream);
			suppSS->exceptions(std::ios::badbit | std::ios::failbit | std::ios::eofbit);
			(*suppSS) << makeString(TEST_RESULT(FAT_initialstate));
			camoto::iostream_sptr suppStream(suppSS);
			ga::SuppItem si;
			si.stream = suppStream;
			si.fnTruncate = boost::bind<void>(stringStreamTruncate, suppSS.get(), _1);
			this->suppData[ga::EST_FAT] = si;
			this->suppBase[ga::EST_FAT] = suppSS;
		}
		#endif

		ga::ManagerPtr pManager;
		ga::ArchiveTypePtr pTestType;
		BOOST_REQUIRE_NO_THROW(
			this->baseData->exceptions(std::ios::badbit | std::ios::failbit | std::ios::eofbit);

			pManager = ga::getManager();
			pTestType = pManager->getArchiveTypeByCode(ARCHIVE_TYPE);
		);
		BOOST_REQUIRE_MESSAGE(pTestType, "Could not find archive type " ARCHIVE_TYPE);

		BOOST_REQUIRE_NO_THROW(
			this->pArchive = pTestType->open(this->baseStream, this->suppData);
		);
		BOOST_REQUIRE_MESSAGE(this->pArchive, "Could not create archive class");

		BOOST_REQUIRE_NO_THROW(
			this->pArchive->fnTruncate = boost::bind<void>(
				stringStreamTruncate, this->baseData.get(), _1);
		);
	}

	FIXTURE_NAME(int i) :
		baseData(new std::stringstream),
		baseStream(this->baseData)
	{
		this->baseData->exceptions(std::ios::badbit | std::ios::failbit | std::ios::eofbit);
	}

	boost::test_tools::predicate_result is_equal(const std::string& strExpected)
	{
		// Flush out any changes before we perform the check
		BOOST_CHECK_NO_THROW(
			this->pArchive->flush()
		);

		return this->default_sample::is_equal(strExpected, this->baseData->str());
	}

	boost::test_tools::predicate_result is_supp_equal(ga::E_SUPPTYPE type, const std::string& strExpected)
	{
		// Flush out any changes to the main archive before we perform the check,
		// in case this function was called first.
		BOOST_CHECK_NO_THROW(
			this->pArchive->flush()
		);

		return this->default_sample::is_equal(strExpected, this->suppBase[type]->str());
	}

};

BOOST_FIXTURE_TEST_SUITE(SUITE_NAME, FIXTURE_NAME)

// Define an ISINSTANCE_TEST macro which we use to confirm the initial state
// is a valid instance of this format.  This is defined as a macro so the
// format-specific code can reuse it later to test various invalid formats.
#define ISINSTANCE_TEST(c, d, r) \
	BOOST_AUTO_TEST_CASE(TEST_NAME(isinstance_ ## c)) \
	{ \
		BOOST_TEST_MESSAGE("isInstance check (" ARCHIVE_TYPE "; " #c ")"); \
		\
		boost::shared_ptr<ga::Manager> pManager(ga::getManager()); \
		ga::ArchiveTypePtr pTestType(pManager->getArchiveTypeByCode(ARCHIVE_TYPE)); \
		BOOST_REQUIRE_MESSAGE(pTestType, "Could not find archive type " ARCHIVE_TYPE); \
		\
		boost::shared_ptr<std::stringstream> psstrBase(new std::stringstream); \
		(*psstrBase) << makeString(d); \
		camoto::iostream_sptr psBase(psstrBase); \
		\
		BOOST_CHECK_EQUAL(pTestType->isInstance(psBase), r); \
	}

ISINSTANCE_TEST(c00, INITIALSTATE_NAME, ga::EC_DEFINITELY_YES);


// Define an INVALIDDATA_TEST macro which we use to confirm the reader correctly
// rejects a file with invalid data.  This is defined as a macro so the
// format-specific code can reuse it later to test various invalid formats.
#ifdef HAS_FAT
#	define INVALIDDATA_FATCODE(d) \
	{ \
		boost::shared_ptr<std::stringstream> suppSS(new std::stringstream); \
		suppSS->exceptions(std::ios::badbit | std::ios::failbit | std::ios::eofbit); \
		(*suppSS) << makeString(d); \
		camoto::iostream_sptr suppStream(suppSS); \
		ga::SuppItem si; \
		si.stream = suppStream; \
		si.fnTruncate = boost::bind<void>(stringStreamTruncate, suppSS.get(), _1); \
		suppData[ga::EST_FAT] = si; \
	}
#else
#	define INVALIDDATA_FATCODE(d)
#endif

#define INVALIDDATA_TEST(c, d) \
	INVALIDDATA_TEST_FULL(c, d, 0)

#define INVALIDDATA_TEST_FAT(c, d, f) \
	INVALIDDATA_TEST_FULL(c, d, f)

#define INVALIDDATA_TEST_FULL(c, d, f) \
	/* Run an isInstance test first to make sure the data is accepted */ \
	ISINSTANCE_TEST(invaliddata_ ## c, d, ga::EC_DEFINITELY_YES); \
	\
	BOOST_AUTO_TEST_CASE(TEST_NAME(invaliddata_ ## c)) \
	{ \
		BOOST_TEST_MESSAGE("invalidData check (" ARCHIVE_TYPE "; " #c ")"); \
		\
		boost::shared_ptr<ga::Manager> pManager(ga::getManager()); \
		ga::ArchiveTypePtr pTestType(pManager->getArchiveTypeByCode(ARCHIVE_TYPE)); \
		\
		/* Prepare an invalid archive */ \
		boost::shared_ptr<std::stringstream> psstrBase(new std::stringstream); \
		(*psstrBase) << makeString(d); \
		camoto::iostream_sptr psBase(psstrBase); \
		\
		ga::MP_SUPPDATA suppData; \
		INVALIDDATA_FATCODE(f) \
		\
		BOOST_CHECK_THROW( \
			ga::ArchivePtr pArchive(pTestType->open(psBase, suppData)), \
			std::ios::failure \
		); \
	}

BOOST_AUTO_TEST_CASE(TEST_NAME(open))
{
	BOOST_TEST_MESSAGE("Opening file in archive");

#if !NO_FILENAMES
	// Find the file we're going to open
	ga::Archive::EntryPtr ep = pArchive->find(FILENAME1);

	// Make sure we found it
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(ep),
		"Couldn't find " FILENAME1 " in sample archive");
#else
	// No filenames in this format, do it by order
	const ga::Archive::VC_ENTRYPTR& files = pArchive->getFileList();
	ga::Archive::EntryPtr ep = ga::getFileAt(files, 0);
#endif

	// Open it
	camoto::iostream_sptr pfsIn(pArchive->open(ep));
	std::stringstream out;
	out.exceptions(std::ios::badbit | std::ios::failbit | std::ios::eofbit);

	// Make sure the file opens at the start
	BOOST_REQUIRE_EQUAL(pfsIn->tellg(), 0);

	// Apply any decryption/decompression filter
	applyFilter(pArchive, ep, &pfsIn);

	// Copy it into the stringstream
	boost::iostreams::copy(*pfsIn, out);

	BOOST_CHECK_MESSAGE(
		default_sample::is_equal(makeString(
			"This is one.dat"
		), out.str()),
		"Error opening file or wrong file opened"
	);
}

#if !NO_FILENAMES
BOOST_AUTO_TEST_CASE(TEST_NAME(rename))
{
	BOOST_TEST_MESSAGE("Renaming file inside archive");

	// Find the file we're going to rename
	ga::Archive::EntryPtr ep = pArchive->find(FILENAME1);

	// Make sure we found it
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(ep),
		"Couldn't find " FILENAME1 " in sample archive");

	pArchive->rename(ep, FILENAME3);

	BOOST_CHECK_MESSAGE(
		is_equal(makeString(TEST_RESULT(rename))),
		"Error renaming file"
	);

#ifdef HAS_FAT
	BOOST_CHECK_MESSAGE(
		is_supp_equal(ga::EST_FAT, makeString(TEST_RESULT(FAT_rename))),
		"Error renaming file"
	);
#endif
}

#endif

#if MAX_FILENAME_LEN > 0
BOOST_AUTO_TEST_CASE(TEST_NAME(rename_long))
{
	BOOST_TEST_MESSAGE("Rename file with name too long");

	// Find the file we're going to insert before
	ga::Archive::EntryPtr ep = pArchive->find(FILENAME1);

	// Make sure we found it
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(ep),
		"Couldn't find " FILENAME1 " in sample archive");

	char name[MAX_FILENAME_LEN + 2];
	memset(name, 'A', MAX_FILENAME_LEN + 1);
	name[MAX_FILENAME_LEN + 1] = 0;

	// Make sure renaming fails when the filename is too long
	BOOST_CHECK_THROW(
		pArchive->rename(ep, name),
		std::ios::failure
	);

	BOOST_CHECK_MESSAGE(
		is_equal(makeString(INITIALSTATE_NAME)),
		"Archive corrupted after failed insert"
	);

#ifdef HAS_FAT
	BOOST_CHECK_MESSAGE(
		is_supp_equal(ga::EST_FAT, makeString(TEST_RESULT(FAT_initialstate))),
		"Archive corrupted after failed insert"
	);
#endif

	memset(name, 'A', MAX_FILENAME_LEN);
	name[MAX_FILENAME_LEN - 4] = '.';
	name[MAX_FILENAME_LEN] = 0;

	// Make sure the rename succeeds when the filename is exactly the max length
	BOOST_CHECK_NO_THROW(
		pArchive->rename(ep, name)
	);

}
#endif

#if MAX_FILENAME_LEN > 0
BOOST_AUTO_TEST_CASE(TEST_NAME(insert_long))
{
	BOOST_TEST_MESSAGE("Inserting file with name too long");

	// Find the file we're going to insert before
	ga::Archive::EntryPtr epb = pArchive->find(FILENAME1);

	// Make sure we found it
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(epb),
		"Couldn't find " FILENAME1 " in sample archive");

	char name[MAX_FILENAME_LEN + 2];
	memset(name, 'A', MAX_FILENAME_LEN + 1);
	name[MAX_FILENAME_LEN + 1] = 0;

	BOOST_CHECK_THROW(
		ga::Archive::EntryPtr ep = pArchive->insert(epb, name, 5, FILETYPE_GENERIC, INSERT_ATTRIBUTE),
		std::ios::failure
	);

	BOOST_CHECK_MESSAGE(
		is_equal(makeString(INITIALSTATE_NAME)),
		"Archive corrupted after failed insert"
	);

#ifdef HAS_FAT
	BOOST_CHECK_MESSAGE(
		is_supp_equal(ga::EST_FAT, makeString(TEST_RESULT(FAT_initialstate))),
		"Archive corrupted after failed insert"
	);
#endif

	memset(name, 'A', MAX_FILENAME_LEN);
	name[MAX_FILENAME_LEN - 4] = '.';
	name[MAX_FILENAME_LEN] = 0;

	BOOST_CHECK_NO_THROW(
		ga::Archive::EntryPtr ep = pArchive->insert(ga::Archive::EntryPtr(), name, 5, FILETYPE_GENERIC, INSERT_ATTRIBUTE)
	);

}
#endif

BOOST_AUTO_TEST_CASE(TEST_NAME(insert_end))
{
	BOOST_TEST_MESSAGE("Inserting file into archive");

	// Insert the file
#if !NO_FILENAMES
	ga::Archive::EntryPtr ep = pArchive->insert(ga::Archive::EntryPtr(), FILENAME3, 17, FILETYPE_GENERIC, INSERT_ATTRIBUTE);
#else
	ga::Archive::EntryPtr ep = pArchive->insert(ga::Archive::EntryPtr(), "dummy", 17, FILETYPE_GENERIC, INSERT_ATTRIBUTE);
#endif

	// Make sure it went in ok
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(ep),
		"Couldn't create new file in sample archive");

	// Open it
	iostream_sptr pfsNew(pArchive->open(ep));

	// Apply any encryption/compression filter
	applyFilter(pArchive, ep, &pfsNew);

	pfsNew->write("This is three.dat", 17);
	pfsNew->flush();

	BOOST_CHECK_MESSAGE(
		is_equal(makeString(TEST_RESULT(insert_end))),
		"Error inserting file at end of archive"
	);

#ifdef HAS_FAT
	BOOST_CHECK_MESSAGE(
		is_supp_equal(ga::EST_FAT, makeString(TEST_RESULT(FAT_insert_end))),
		"Error inserting file at end of archive"
	);
#endif
}

BOOST_AUTO_TEST_CASE(TEST_NAME(insert_mid))
{
	BOOST_TEST_MESSAGE("Inserting file into middle of archive");

	// Find the file we're going to insert before
#if !NO_FILENAMES
	ga::Archive::EntryPtr idBefore = pArchive->find(FILENAME2);

	// Make sure we found it
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(idBefore),
		"Couldn't find " FILENAME2 " in sample archive");

	// Insert the file
	ga::Archive::EntryPtr ep = pArchive->insert(idBefore, FILENAME3, 17, FILETYPE_GENERIC, INSERT_ATTRIBUTE);
#else
	// No filenames in this format, do it by order
	const ga::Archive::VC_ENTRYPTR& files = pArchive->getFileList();
	ga::Archive::EntryPtr idBefore = ga::getFileAt(files, 1);

	// Make sure we found it
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(idBefore),
		"Couldn't find second file in sample archive");

	// Insert the file
	ga::Archive::EntryPtr ep = pArchive->insert(idBefore, "dummy", 17, FILETYPE_GENERIC, INSERT_ATTRIBUTE);
#endif

	// Make sure it went in ok
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(ep),
		"Couldn't insert new file in sample archive");

	// Open it
	iostream_sptr pfsNew(pArchive->open(ep));

	// Apply any encryption/compression filter
	applyFilter(pArchive, ep, &pfsNew);

	pfsNew->write("This is three.dat", 17);
	pfsNew->flush();

	BOOST_CHECK_MESSAGE(
		is_equal(makeString(TEST_RESULT(insert_mid))),
		"Error inserting file in middle of archive"
	);

#ifdef HAS_FAT
	BOOST_CHECK_MESSAGE(
		is_supp_equal(ga::EST_FAT, makeString(TEST_RESULT(FAT_insert_mid))),
		"Error inserting file in middle of archive"
	);
#endif
}

BOOST_AUTO_TEST_CASE(TEST_NAME(insert2))
{
	BOOST_TEST_MESSAGE("Inserting multiple files");

#if !NO_FILENAMES
	// Find the file we're going to insert before
	ga::Archive::EntryPtr idBefore = pArchive->find(FILENAME2);

	// Make sure we found it
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(idBefore),
		"Couldn't find " FILENAME2 " in sample archive");

	// Insert the file
	ga::Archive::EntryPtr ep1 = pArchive->insert(idBefore, FILENAME3, 17, FILETYPE_GENERIC, INSERT_ATTRIBUTE);
#else
	// No filenames in this format, do it by order
	const ga::Archive::VC_ENTRYPTR& files = pArchive->getFileList();
	ga::Archive::EntryPtr idBefore = ga::getFileAt(files, 1); // FILENAME2 is the second file at this point

	// Make sure we found it
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(idBefore),
		"Couldn't find second file in sample archive");

	// Insert the file
	ga::Archive::EntryPtr ep1 = pArchive->insert(idBefore, "dummy", 17, FILETYPE_GENERIC, INSERT_ATTRIBUTE);
#endif

	// Make sure it went in ok
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(ep1),
		"Couldn't insert first new file in sample archive");

	// Open it
	iostream_sptr pfsNew1(pArchive->open(ep1));

	// Apply any encryption/compression filter
	applyFilter(pArchive, ep1, &pfsNew1);

	pfsNew1->write("This is three.dat", 17);
	pfsNew1->flush();

#if !NO_FILENAMES
	// Find the file we're going to insert before (since the previous insert
	// invalidated all EntryPtrs.)
	idBefore = pArchive->find(FILENAME2);

	// Make sure we found it
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(idBefore),
		"Couldn't find " FILENAME2 " in sample archive after first insert");

	// Insert the file
	ga::Archive::EntryPtr ep2 = pArchive->insert(idBefore, FILENAME4, 16, FILETYPE_GENERIC, INSERT_ATTRIBUTE);
#else
	// No filenames in this format, do it by order

	idBefore = ga::getFileAt(files, 2); // FILENAME2 is now the third file

	// Make sure we found it
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(idBefore),
		"Couldn't find second file in sample archive after first insert");

	// Insert the file
	ga::Archive::EntryPtr ep2 = pArchive->insert(idBefore, "dummy", 16, FILETYPE_GENERIC, INSERT_ATTRIBUTE);
#endif

	// Make sure it went in ok
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(ep2),
		"Couldn't insert second new file in sample archive");

	iostream_sptr pfsNew2(pArchive->open(ep2));

	// Apply any encryption/compression filter
	applyFilter(pArchive, ep2, &pfsNew2);

	pfsNew2->write("This is four.dat", 16);
	pfsNew2->flush();

	BOOST_CHECK_MESSAGE(
		is_equal(makeString(TEST_RESULT(insert2))),
		"Error inserting two files"
	);

#ifdef HAS_FAT
	BOOST_CHECK_MESSAGE(
		is_supp_equal(ga::EST_FAT, makeString(TEST_RESULT(FAT_insert2))),
		"Error inserting two files"
	);
#endif
}

BOOST_AUTO_TEST_CASE(TEST_NAME(remove))
{
	BOOST_TEST_MESSAGE("Removing file from archive");

#if !NO_FILENAMES
	// Find the file we're going to remove
	ga::Archive::EntryPtr ep = pArchive->find(FILENAME1);

	// Make sure we found it
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(ep),
		"Couldn't find " FILENAME1 " in sample archive");
#else
	// No filenames in this format, do it by order
	const ga::Archive::VC_ENTRYPTR& files = pArchive->getFileList();
	ga::Archive::EntryPtr ep = ga::getFileAt(files, 0);

	// Make sure we found it
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(ep),
		"Couldn't find first file in sample archive");
#endif

	// Remove it
	pArchive->remove(ep);

	BOOST_CHECK_MESSAGE(
		is_equal(makeString(TEST_RESULT(remove))),
		"Error removing file"
	);

#ifdef HAS_FAT
	BOOST_CHECK_MESSAGE(
		is_supp_equal(ga::EST_FAT, makeString(TEST_RESULT(FAT_remove))),
		"Error removing file"
	);
#endif
}

BOOST_AUTO_TEST_CASE(TEST_NAME(remove2))
{
	BOOST_TEST_MESSAGE("Removing multiple files from archive");

#if !NO_FILENAMES
	// Find the files we're going to remove
	ga::Archive::EntryPtr ep1 = pArchive->find(FILENAME1);
	ga::Archive::EntryPtr ep2 = pArchive->find(FILENAME2);

	// Make sure we found it
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(ep1),
		"Couldn't find " FILENAME1 " in sample archive");
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(ep2),
		"Couldn't find " FILENAME2 " in sample archive");
#else
	// No filenames in this format, do it by order
	const ga::Archive::VC_ENTRYPTR& files = pArchive->getFileList();
	ga::Archive::EntryPtr ep1 = ga::getFileAt(files, 0);
	ga::Archive::EntryPtr ep2 = ga::getFileAt(files, 1);

	// Make sure we found it
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(ep1),
		"Couldn't find first file in sample archive");
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(ep2),
		"Couldn't find second file in sample archive");
#endif

	// Remove it
	pArchive->remove(ep1);
	pArchive->remove(ep2);

	BOOST_CHECK_MESSAGE(
		is_equal(makeString(TEST_RESULT(remove2))),
		"Error removing multiple files"
	);

#ifdef HAS_FAT
	BOOST_CHECK_MESSAGE(
		is_supp_equal(ga::EST_FAT, makeString(TEST_RESULT(FAT_remove2))),
		"Error removing multiple files"
	);
#endif
}

BOOST_AUTO_TEST_CASE(TEST_NAME(insert_remove))
{
	BOOST_TEST_MESSAGE("Insert then remove file from archive");

#if !NO_FILENAMES
	// Find the file we're going to insert before
	ga::Archive::EntryPtr idBefore = pArchive->find(FILENAME2);

	// Make sure we found it
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(idBefore),
		"Couldn't find " FILENAME2 " in sample archive");

	// Insert the file
	ga::Archive::EntryPtr ep = pArchive->insert(idBefore, FILENAME3, 17, FILETYPE_GENERIC, INSERT_ATTRIBUTE);
#else
	// No filenames in this format, do it by order
	const ga::Archive::VC_ENTRYPTR& files = pArchive->getFileList();
	ga::Archive::EntryPtr idBefore = ga::getFileAt(files, 1);

	// Make sure we found it
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(idBefore),
		"Couldn't find second file in sample archive");

	// Insert the file
	ga::Archive::EntryPtr ep = pArchive->insert(idBefore, "dummy", 17, FILETYPE_GENERIC, INSERT_ATTRIBUTE);
#endif

	// Make sure it went in ok
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(ep),
		"Couldn't insert new file in sample archive");

	// Open it
	iostream_sptr pfsNew(pArchive->open(ep));

	// Apply any encryption/compression filter
	applyFilter(pArchive, ep, &pfsNew);

	pfsNew->write("This is three.dat", 17);
	pfsNew->flush();

#if !NO_FILENAMES
	// Find the file we're going to remove
	ga::Archive::EntryPtr ep2 = pArchive->find(FILENAME1);

	// Make sure we found it
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(ep2),
		"Couldn't find " FILENAME1 " in sample archive");
#else
	// No filenames in this format, do it by order
	ga::Archive::EntryPtr ep2 = ga::getFileAt(files, 0);

	// Make sure we found it
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(ep2),
		"Couldn't find first file in sample archive");
#endif

	// Remove it
	pArchive->remove(ep2);

	BOOST_CHECK_MESSAGE(
		is_equal(makeString(TEST_RESULT(insert_remove))),
		"Error inserting then removing file"
	);

#ifdef HAS_FAT
	BOOST_CHECK_MESSAGE(
		is_supp_equal(ga::EST_FAT, makeString(TEST_RESULT(FAT_insert_remove))),
		"Error inserting then removing file"
	);
#endif
}

BOOST_AUTO_TEST_CASE(TEST_NAME(remove_insert))
{
	BOOST_TEST_MESSAGE("Remove then insert file from archive");

#if !NO_FILENAMES
	// Find the file we're going to remove
	ga::Archive::EntryPtr ep2 = pArchive->find(FILENAME1);

	// Make sure we found it
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(ep2),
		"Couldn't find " FILENAME1 " in sample archive");
#else
	// No filenames in this format, do it by order
	const ga::Archive::VC_ENTRYPTR& files = pArchive->getFileList();
	ga::Archive::EntryPtr ep2 = ga::getFileAt(files, 0);

	// Make sure we found it
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(ep2),
		"Couldn't find first file in sample archive");
#endif

	// Remove it
	pArchive->remove(ep2);

#if !NO_FILENAMES
	// Find the file we're going to insert before
	ga::Archive::EntryPtr idBefore = pArchive->find(FILENAME2);

	// Make sure we found it
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(idBefore),
		"Couldn't find " FILENAME2 " in sample archive");

	// Insert the file
	ga::Archive::EntryPtr ep = pArchive->insert(idBefore, FILENAME3, 17, FILETYPE_GENERIC, INSERT_ATTRIBUTE);
#else
	// No filenames in this format, do it by order
	ga::Archive::EntryPtr idBefore = ga::getFileAt(files, 0); // FILENAME2 is the first (only) file in the archive at this point

	// Make sure we found it
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(idBefore),
		"Couldn't find second file in sample archive");

	// Insert the file
	ga::Archive::EntryPtr ep = pArchive->insert(idBefore, "dummy", 17, FILETYPE_GENERIC, INSERT_ATTRIBUTE);
#endif

	// Make sure it went in ok
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(ep),
		"Couldn't insert new file in sample archive");

	// Open it
	iostream_sptr pfsNew(pArchive->open(ep));

	// Apply any encryption/compression filter
	applyFilter(pArchive, ep, &pfsNew);

	pfsNew->write("This is three.dat", 17);
	pfsNew->flush();

	BOOST_CHECK_MESSAGE(
		// This test checks against the insert_remove result instead, as the end
		// result should be the same as that test.
		is_equal(makeString(TEST_RESULT(insert_remove))),
		"Error removing then inserting file"
	);

#ifdef HAS_FAT
	BOOST_CHECK_MESSAGE(
		// Again, use insert_remove result instead
		is_supp_equal(ga::EST_FAT, makeString(TEST_RESULT(FAT_insert_remove))),
		"Error removing then inserting file"
	);
#endif
}

BOOST_AUTO_TEST_CASE(TEST_NAME(move))
{
	BOOST_TEST_MESSAGE("Moving file inside archive");

#if !NO_FILENAMES
	// Find the file we're going to move
	ga::Archive::EntryPtr ep1 = pArchive->find(FILENAME1);
	ga::Archive::EntryPtr ep2 = pArchive->find(FILENAME2);

	// Make sure we found it
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(ep1),
		"Couldn't find " FILENAME1 " in sample archive");
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(ep2),
		"Couldn't find " FILENAME2 " in sample archive");
#else
	// No filenames in this format, do it by order
	const ga::Archive::VC_ENTRYPTR& files = pArchive->getFileList();
	ga::Archive::EntryPtr ep1 = ga::getFileAt(files, 0);
	ga::Archive::EntryPtr ep2 = ga::getFileAt(files, 1);

	// Make sure we found it
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(ep1),
		"Couldn't find first file in sample archive");
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(ep2),
		"Couldn't find second file in sample archive");
#endif

	// Swap the file positions
	pArchive->move(ep1, ep2);

	BOOST_CHECK_MESSAGE(
		is_equal(makeString(TEST_RESULT(move))),
		"Error moving file"
	);

#ifdef HAS_FAT
	BOOST_CHECK_MESSAGE(
		is_supp_equal(ga::EST_FAT, makeString(TEST_RESULT(FAT_move))),
		"Error moving file"
	);
#endif
}

BOOST_AUTO_TEST_CASE(TEST_NAME(resize_larger))
{
	BOOST_TEST_MESSAGE("Enlarging a file inside the archive");

#if !NO_FILENAMES
	// Find the file we're going to resize
	ga::Archive::EntryPtr ep = pArchive->find(FILENAME1);

	// Make sure we found it
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(ep),
		"Couldn't find " FILENAME1 " in sample archive");
#else
	// No filenames in this format, do it by order
	const ga::Archive::VC_ENTRYPTR& files = pArchive->getFileList();
	ga::Archive::EntryPtr ep = ga::getFileAt(files, 0);

	// Make sure we found it
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(ep),
		"Couldn't find first file in sample archive");
#endif

	// Swap the file positions
	pArchive->resize(ep, 20, 20);

	BOOST_CHECK_MESSAGE(
		is_equal(makeString(TEST_RESULT(resize_larger))),
		"Error enlarging a file"
	);

#ifdef HAS_FAT
	BOOST_CHECK_MESSAGE(
		is_supp_equal(ga::EST_FAT, makeString(TEST_RESULT(FAT_resize_larger))),
		"Error enlarging a file"
	);
#endif
}

BOOST_AUTO_TEST_CASE(TEST_NAME(resize_smaller))
{
	BOOST_TEST_MESSAGE("Shrink a file inside the archive");

#if !NO_FILENAMES
	// Find the file we're going to resize
	ga::Archive::EntryPtr ep = pArchive->find(FILENAME1);

	// Make sure we found it
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(ep),
		"Couldn't find " FILENAME1 " in sample archive");
#else
	// No filenames in this format, do it by order
	const ga::Archive::VC_ENTRYPTR& files = pArchive->getFileList();
	ga::Archive::EntryPtr ep = ga::getFileAt(files, 0);

	// Make sure we found it
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(ep),
		"Couldn't find first file in sample archive");
#endif

	// Swap the file positions
	pArchive->resize(ep, 10, 10);

	BOOST_CHECK_MESSAGE(
		is_equal(makeString(TEST_RESULT(resize_smaller))),
		"Error shrinking a file"
	);

#ifdef HAS_FAT
	BOOST_CHECK_MESSAGE(
		is_supp_equal(ga::EST_FAT, makeString(TEST_RESULT(FAT_resize_smaller))),
		"Error shrinking a file"
	);
#endif
}

BOOST_AUTO_TEST_CASE(TEST_NAME(resize_write))
{
	BOOST_TEST_MESSAGE("Enlarging a file inside the archive");

#if !NO_FILENAMES
	// Find the file we're going to resize
	ga::Archive::EntryPtr ep = pArchive->find(FILENAME1);

	// Make sure we found it
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(ep),
		"Couldn't find " FILENAME1 " in sample archive");
#else
	// No filenames in this format, do it by order
	const ga::Archive::VC_ENTRYPTR& files = pArchive->getFileList();
	ga::Archive::EntryPtr ep = ga::getFileAt(files, 0);

	// Make sure we found it
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(ep),
		"Couldn't find first file in sample archive");
#endif

	// Swap the file positions
	pArchive->resize(ep, 23, 23);

	iostream_sptr pfsNew(pArchive->open(ep));

	// Apply any encryption/compression filter
	applyFilter(pArchive, ep, &pfsNew);

	pfsNew->write("Now resized to 23 chars", 23);
	pfsNew->flush();

	BOOST_CHECK_MESSAGE(
		is_equal(makeString(TEST_RESULT(resize_write))),
		"Error enlarging a file then writing into new space"
	);

#ifdef HAS_FAT
	BOOST_CHECK_MESSAGE(
		is_supp_equal(ga::EST_FAT, makeString(TEST_RESULT(FAT_resize_write))),
		"Error enlarging a file then writing into new space"
	);
#endif

	// Open the file following it to make sure it was moved out of the way

#if !NO_FILENAMES
	// Find the file we're going to open
	ga::Archive::EntryPtr ep2 = pArchive->find(FILENAME2);

	// Make sure we found it
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(ep2),
		"Couldn't find " FILENAME2 " in sample archive");
#else
	// No filenames in this format, do it by order
	ga::Archive::EntryPtr ep2 = ga::getFileAt(files, 1);

	// Make sure we found it
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(ep2),
		"Couldn't find second file in sample archive");
#endif

	// Open it
	camoto::iostream_sptr pfsIn(pArchive->open(ep2));
	std::stringstream out;
	out.exceptions(std::ios::badbit | std::ios::failbit | std::ios::eofbit);

	// Apply any decryption/decompression filter
	applyFilter(pArchive, ep2, &pfsIn);

	// Copy it into the stringstream
	boost::iostreams::copy(*pfsIn, out);

	BOOST_CHECK_MESSAGE(
		default_sample::is_equal(makeString(
			"This is two.dat"
		), out.str()),
		"Unrelated file was corrupted after file resize operation"
	);

}

// Remove all the files from the archive, then add them back in again.  This
// differs from the insert/remove tests above as it takes the archive to the
// point where it has no files at all.
BOOST_AUTO_TEST_CASE(TEST_NAME(remove_all_re_add))
{
	BOOST_TEST_MESSAGE("Remove all files then add them again");

#if !NO_FILENAMES
	ga::Archive::EntryPtr idOne = pArchive->find(FILENAME1);
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(idOne),
		"Couldn't find " FILENAME1 " in sample archive");
#else
	// No filenames in this format, do it by order
	const ga::Archive::VC_ENTRYPTR& files = pArchive->getFileList();
	ga::Archive::EntryPtr idOne = ga::getFileAt(files, 0);

	// Make sure we found it
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(idOne),
		"Couldn't find first file in sample archive");
#endif

	pArchive->remove(idOne);

#if !NO_FILENAMES
	ga::Archive::EntryPtr idTwo = pArchive->find(FILENAME2);
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(idTwo),
		"Couldn't find " FILENAME2 " in sample archive");
#else
	// No filenames in this format, do it by order
	ga::Archive::EntryPtr idTwo = ga::getFileAt(files, 0); // FILENAME2 is the first (only) file in the archive at this point

	// Make sure we found it
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(idTwo),
		"Couldn't find second file in sample archive");
#endif
	pArchive->remove(idTwo);

	// Make sure there are now no files in the archive
	const ga::Archive::VC_ENTRYPTR& allfiles = pArchive->getFileList();
	BOOST_REQUIRE_EQUAL(allfiles.size(), 0);

#if !NO_FILENAMES
	// Add the files back again
	idOne = pArchive->insert(ga::Archive::EntryPtr(), FILENAME1, 15, FILETYPE_GENERIC, INSERT_ATTRIBUTE);
#else
	// No filenames in this format
	idOne = pArchive->insert(ga::Archive::EntryPtr(), "dummy", 15, FILETYPE_GENERIC, INSERT_ATTRIBUTE);
#endif
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(idOne),
		"Couldn't insert new file after removing all files");

	iostream_sptr pfsNew(pArchive->open(idOne));

	// Apply any encryption/compression filter
	applyFilter(pArchive, idOne, &pfsNew);

	pfsNew->write("This is one.dat", 15);
	pfsNew->flush();

#if !NO_FILENAMES
	idTwo = pArchive->insert(ga::Archive::EntryPtr(), FILENAME2, 15, FILETYPE_GENERIC, INSERT_ATTRIBUTE);
#else
	// No filenames in this format
	idTwo = pArchive->insert(ga::Archive::EntryPtr(), "dummy", 15, FILETYPE_GENERIC, INSERT_ATTRIBUTE);
#endif
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(idTwo),
		"Couldn't insert second new file after removing all files");
	pfsNew = pArchive->open(idTwo);

	// Apply any encryption/compression filter
	applyFilter(pArchive, idTwo, &pfsNew);

	pfsNew->write("This is two.dat", 15);
	pfsNew->flush();

	BOOST_CHECK_MESSAGE(
		is_equal(makeString(TEST_RESULT(initialstate))),
		"Error removing all files then reinserting them again"
	);

#ifdef HAS_FAT
	BOOST_CHECK_MESSAGE(
		is_supp_equal(ga::EST_FAT, makeString(TEST_RESULT(FAT_initialstate))),
		"Error removing all files then reinserting them again"
	);
#endif
}

// The function shifting files can get confused if a zero-length file is
// inserted, incorrectly moving it because of the zero size.
BOOST_AUTO_TEST_CASE(TEST_NAME(insert_zero_then_resize))
{
	BOOST_TEST_MESSAGE("Inserting empty file into archive, then resize it");

	// Insert the file
#if !NO_FILENAMES
	ga::Archive::EntryPtr ep = pArchive->insert(ga::Archive::EntryPtr(), FILENAME3, 0, FILETYPE_GENERIC, INSERT_ATTRIBUTE);
#else
	ga::Archive::EntryPtr ep = pArchive->insert(ga::Archive::EntryPtr(), "dummy", 0, FILETYPE_GENERIC, INSERT_ATTRIBUTE);
#endif

	// Make sure it went in ok
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(ep),
		"Couldn't create new file in sample archive");

	// Open it
	camoto::iostream_sptr pfsNew(pArchive->open(ep));

	// Apply any encryption/compression filter
	applyFilter(pArchive, ep, &pfsNew);

	pArchive->resize(ep, 17, 17);
	pfsNew->seekp(0, std::ios::beg);
	pfsNew->write("This is three.dat", 17);
	pfsNew->flush();

	BOOST_CHECK_MESSAGE(
		is_equal(makeString(TEST_RESULT(insert_end))),
		"Error resizing newly inserted empty file"
	);

#ifdef HAS_FAT
	BOOST_CHECK_MESSAGE(
		is_supp_equal(ga::EST_FAT, makeString(TEST_RESULT(FAT_insert_end))),
		"Error resizing newly inserted empty file"
	);
#endif
}

//
// Metadata tests
//

#ifdef testdata_get_metadata_description
BOOST_AUTO_TEST_CASE(TEST_NAME(get_metadata_description))
{
	BOOST_TEST_MESSAGE("Get 'description' metadata field");

	// Make sure this format reports having a 'description' metadata field
	camoto::Metadata::MetadataTypes items = pArchive->getMetadataList();
	bool bFound = false;
	for (camoto::Metadata::MetadataTypes::iterator i = items.begin(); i != items.end(); i++) {
		if (*i == camoto::Metadata::Description) {
			bFound = true;
			break;
		}
	}
	BOOST_REQUIRE_EQUAL(bFound, true);

	// Change the field's value
	std::string value = pArchive->getMetadata(camoto::Metadata::Description);

	// Make sure we didn't read in extra data (e.g. 400MB with a broken length)
	BOOST_REQUIRE_EQUAL(value.length(), sizeof(TEST_RESULT(get_metadata_description)) - 1);

	// Put it in a stringstream to allow use of the standard checking mechanism
	std::ostringstream out;
	out << value;

	BOOST_CHECK_MESSAGE(
		default_sample::is_equal(makeString(
			TEST_RESULT(get_metadata_description)
		), out.str()),
		"Error getting 'description' metadata field"
	);

}

BOOST_AUTO_TEST_CASE(TEST_NAME(set_metadata_description_larger))
{
	BOOST_TEST_MESSAGE("Set 'description' metadata field to larger value");

	// We assume the format supports this metadata type, as this is checked in
	// get_metadata_description above.

	// Change the field's value
	pArchive->setMetadata(camoto::Metadata::Description, TEST_RESULT(set_metadata_description_target_larger));

	BOOST_CHECK_MESSAGE(
		is_equal(makeString(TEST_RESULT(set_metadata_description_larger))),
		"Error setting 'description' metadata field"
	);

#ifdef HAS_FAT
	BOOST_CHECK_MESSAGE(
		is_supp_equal(ga::EST_FAT, makeString(TEST_RESULT(FAT_set_metadata_description_larger))),
		"Error setting 'description' metadata field"
	);
#endif
}

BOOST_AUTO_TEST_CASE(TEST_NAME(set_metadata_description_smaller))
{
	BOOST_TEST_MESSAGE("Set 'description' metadata field to smaller value");

	// We assume the format supports this metadata type, as this is checked in
	// get_metadata_description above.

	// Change the field's value
	pArchive->setMetadata(camoto::Metadata::Description, TEST_RESULT(set_metadata_description_target_smaller));

	BOOST_CHECK_MESSAGE(
		is_equal(makeString(TEST_RESULT(set_metadata_description_smaller))),
		"Error setting 'description' metadata field"
	);

#ifdef HAS_FAT
	BOOST_CHECK_MESSAGE(
		is_supp_equal(ga::EST_FAT, makeString(TEST_RESULT(FAT_set_metadata_description_smaller))),
		"Error setting 'description' metadata field"
	);
#endif
}
#endif // testdata_get_metadata_description

#ifdef testdata_get_metadata_version
BOOST_AUTO_TEST_CASE(TEST_NAME(get_metadata_version))
{
	BOOST_TEST_MESSAGE("Get 'version' metadata field");

	// Make sure this format reports having a 'description' metadata field
	camoto::Metadata::MetadataTypes items = pArchive->getMetadataList();
	bool bFound = false;
	for (camoto::Metadata::MetadataTypes::iterator i = items.begin(); i != items.end(); i++) {
		if (*i == camoto::Metadata::Version) {
			bFound = true;
			break;
		}
	}
	BOOST_REQUIRE_EQUAL(bFound, true);

	// Change the field's value
	std::string value = pArchive->getMetadata(camoto::Metadata::Version);

	// Make sure we didn't read in extra data (e.g. 400MB with a broken length)
	BOOST_REQUIRE_EQUAL(value.length(), sizeof(TEST_RESULT(get_metadata_version)) - 1);

	// Put it in a stringstream to allow use of the standard checking mechanism
	std::ostringstream out;
	out << value;

	BOOST_CHECK_MESSAGE(
		default_sample::is_equal(makeString(
			TEST_RESULT(get_metadata_version)
		), out.str()),
		"Error getting 'version' metadata field"
	);

}
#endif

BOOST_AUTO_TEST_SUITE_END()

#include "test-archive_new.hpp"
