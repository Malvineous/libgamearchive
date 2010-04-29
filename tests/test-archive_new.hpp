/*
 * test-archive_new.hpp - generic test code for creating new archives.
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

struct EMPTY_FIXTURE_NAME: public FIXTURE_NAME {
	EMPTY_FIXTURE_NAME() :
		FIXTURE_NAME(0)
	{
		#ifdef HAS_FAT
		{
			boost::shared_ptr<std::stringstream> suppSS(new std::stringstream);
			suppSS->exceptions(std::ios::badbit | std::ios::failbit | std::ios::eofbit);
			camoto::iostream_sptr suppStream(suppSS);
			ga::SuppItem si;
			si.stream = suppStream;
			si.fnTruncate = boost::bind<void>(stringStreamTruncate, suppSS.get(), _1);
			this->suppData[ga::EST_FAT] = si;
			this->suppBase[ga::EST_FAT] = suppSS;
		}
		#endif

		boost::shared_ptr<ga::Manager> pManager(ga::getManager());
		ga::Manager::arch_sptr pTestType(pManager->getArchiveTypeByCode(ARCHIVE_TYPE));

		BOOST_TEST_CHECKPOINT("About to call newArchive()");

		// This should really use BOOST_REQUIRE_NO_THROW but the message is more
		// informative without it.
		//BOOST_REQUIRE_NO_THROW(
			this->pArchive = pTestType->newArchive(this->baseStream, this->suppData);
		//);
		BOOST_REQUIRE_MESSAGE(this->pArchive, "Could not create new archive");

		this->pArchive->fnTruncate = boost::bind<void>(
			stringStreamTruncate, this->baseData.get(), _1);

		BOOST_TEST_CHECKPOINT("New archive created successfully");
	}
};

BOOST_FIXTURE_TEST_SUITE(EMPTY_SUITE_NAME, EMPTY_FIXTURE_NAME)

// Make sure a newly created archive is confirmed as a valid instance of that
// archive format.
BOOST_AUTO_TEST_CASE(TEST_NAME(new_isinstance))
{
	BOOST_TEST_MESSAGE("Checking new archive is valid instance of itself");

	pArchive->flush();

	boost::shared_ptr<ga::Manager> pManager(ga::getManager());
	ga::Manager::arch_sptr pTestType(pManager->getArchiveTypeByCode(ARCHIVE_TYPE));

	BOOST_REQUIRE_MESSAGE(pTestType->isInstance(baseStream),
		"Newly created archive was not recognised as a valid instance");

	BOOST_TEST_CHECKPOINT("New archive reported valid, trying to open");

	// This should really use BOOST_REQUIRE_NO_THROW but the message is more
	// informative without it.
	//BOOST_REQUIRE_NO_THROW(
		boost::shared_ptr<ga::Archive> pArchive(pTestType->open(baseStream, suppData));
	//);

	pArchive->fnTruncate = boost::bind<void>(
		stringStreamTruncate, this->baseData.get(), _1);

	// Make sure there are now no files in the archive
	const ga::Archive::VC_ENTRYPTR& files = pArchive->getFileList();
	BOOST_REQUIRE_EQUAL(files.size(), 0);
}

BOOST_AUTO_TEST_CASE(TEST_NAME(new_to_initialstate))
{
	BOOST_TEST_MESSAGE("Creating archive from scratch");

	const ga::Archive::VC_ENTRYPTR& files2 = pArchive->getFileList();
	BOOST_REQUIRE_EQUAL(files2.size(), 0);

	// Add the files to the new archive
	ga::Archive::EntryPtr idOne = pArchive->insert(ga::Archive::EntryPtr(), FILENAME1, 15);
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(idOne),
		"Couldn't insert new file in empty archive");
	boost::shared_ptr<std::iostream> pfsNew(pArchive->open(idOne));
	pfsNew->write("This is one.dat", 15);
	pfsNew->flush();

	ga::Archive::EntryPtr idTwo = pArchive->insert(ga::Archive::EntryPtr(), FILENAME2, 15);
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(idTwo),
		"Couldn't insert second new file in empty archive");
	pfsNew = pArchive->open(idTwo);
	pfsNew->write("This is two.dat", 15);
	pfsNew->flush();

	// Make sure there are now the correct number of files in the archive
	const ga::Archive::VC_ENTRYPTR& files = pArchive->getFileList();
	BOOST_REQUIRE_EQUAL(files.size(), 2);

#ifdef testdata_get_metadata_description
	// If this format has metadata, set it to the same value used when comparing
	// against the initialstate, so that this new archive will hopefully match
	// the initialstate itself.
	pArchive->setMetadata(ga::EM_DESCRIPTION, TEST_RESULT(get_metadata_description));
#endif

	BOOST_CHECK_MESSAGE(
		is_equal(makeString(TEST_RESULT(initialstate))),
		"Error inserting files in new/empty archive"
	);

#ifdef HAS_FAT
	BOOST_CHECK_MESSAGE(
		is_supp_equal(ga::EST_FAT, makeString(TEST_RESULT(FAT_initialstate))),
		"Error inserting files in new/empty archive"
	);
#endif
}

BOOST_AUTO_TEST_SUITE_END()
