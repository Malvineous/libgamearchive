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
		ga::ArchiveTypePtr pTestType(pManager->getArchiveTypeByCode(ARCHIVE_TYPE));

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
	ga::ArchiveTypePtr pTestType(pManager->getArchiveTypeByCode(ARCHIVE_TYPE));

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

#ifdef testdata_get_metadata_version
	// Need to set this first as (in the case of Blood RFF) it affects what type
	// of files we are allowed to insert.
	pArchive->setMetadata(camoto::Metadata::Version, TEST_RESULT(get_metadata_version));
#endif

	const ga::Archive::VC_ENTRYPTR& files2 = pArchive->getFileList();
	BOOST_REQUIRE_EQUAL(files2.size(), 0);

	// Add the files to the new archive
#if !NO_FILENAMES
	ga::Archive::EntryPtr idOne = pArchive->insert(ga::Archive::EntryPtr(), FILENAME1, 15, FILETYPE_GENERIC, INSERT_ATTRIBUTE);
#else
	ga::Archive::EntryPtr idOne = pArchive->insert(ga::Archive::EntryPtr(), "dummy", 15, FILETYPE_GENERIC, INSERT_ATTRIBUTE);
#endif
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(idOne),
		"Couldn't insert new file in empty archive");
	boost::shared_ptr<std::iostream> pfsNew(pArchive->open(idOne));
	// Apply any encryption/compression filter
	applyFilter(&pfsNew, idOne);
	pfsNew->write("This is one.dat", 15);
	pfsNew->flush();

#if !NO_FILENAMES
	ga::Archive::EntryPtr idTwo = pArchive->insert(ga::Archive::EntryPtr(), FILENAME2, 15, FILETYPE_GENERIC, INSERT_ATTRIBUTE);
#else
	ga::Archive::EntryPtr idTwo = pArchive->insert(ga::Archive::EntryPtr(), "dummy", 15, FILETYPE_GENERIC, INSERT_ATTRIBUTE);
#endif
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(idTwo),
		"Couldn't insert second new file in empty archive");
	pfsNew = pArchive->open(idTwo);
	// Apply any encryption/compression filter
	applyFilter(&pfsNew, idTwo);
	pfsNew->write("This is two.dat", 15);
	pfsNew->flush();

#ifdef testdata_get_metadata_description
	// If this format has metadata, set it to the same value used when comparing
	// against the initialstate, so that this new archive will hopefully match
	// the initialstate itself.
	pArchive->setMetadata(camoto::Metadata::Description, TEST_RESULT(get_metadata_description));
#endif

	BOOST_CHECK_MESSAGE(
		is_equal(makeString(TEST_RESULT(initialstate))),
		"Error inserting files in new/empty archive"
	);

	// Make sure there are now the correct number of files in the archive
	const ga::Archive::VC_ENTRYPTR& files = pArchive->getFileList();
	BOOST_REQUIRE_EQUAL(files.size(), 2);

#ifdef HAS_FAT
	BOOST_CHECK_MESSAGE(
		is_supp_equal(ga::EST_FAT, makeString(TEST_RESULT(FAT_initialstate))),
		"Error inserting files in new/empty archive"
	);
#endif
}

// The function shifting files can get confused if a zero-length file is
// inserted, incorrectly moving it because of the zero size.
BOOST_AUTO_TEST_CASE(TEST_NAME(manipulate_zero_length_files))
{
	BOOST_TEST_MESSAGE("Inserting empty files into archive, then resizing them");

#ifdef testdata_get_metadata_description
	// If this format has metadata, set it to the same value used when comparing
	// against the initialstate, so that this new archive will hopefully match
	// the initialstate itself.
	pArchive->setMetadata(camoto::Metadata::Description, TEST_RESULT(get_metadata_description));
#endif

#ifdef testdata_get_metadata_version
	pArchive->setMetadata(camoto::Metadata::Version, TEST_RESULT(get_metadata_version));
#endif

	// Insert the file
#if !NO_FILENAMES
	ga::Archive::EntryPtr ep3 = pArchive->insert(ga::Archive::EntryPtr(), FILENAME3, 0, FILETYPE_GENERIC, INSERT_ATTRIBUTE);
#else
	ga::Archive::EntryPtr ep3 = pArchive->insert(ga::Archive::EntryPtr(), "dummy", 0, FILETYPE_GENERIC, INSERT_ATTRIBUTE);
#endif
	// Make sure it went in ok
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(ep3),
		"Couldn't create new file in archive");
	// Open it
	camoto::iostream_sptr file3(pArchive->open(ep3));
	// Apply any encryption/compression filter
	applyFilter(&file3, ep3);

#if !NO_FILENAMES
	ga::Archive::EntryPtr ep1 = pArchive->insert(ep3, FILENAME1, 0, FILETYPE_GENERIC, INSERT_ATTRIBUTE);
#else
	ga::Archive::EntryPtr ep1 = pArchive->insert(ep3, "dummy", 0, FILETYPE_GENERIC, INSERT_ATTRIBUTE);
#endif
	// Make sure it went in ok
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(ep1),
		"Couldn't create new file in archive");
	// Open it
	camoto::iostream_sptr file1(pArchive->open(ep1));
	// Apply any encryption/compression filter
	applyFilter(&file1, ep1);

#if !NO_FILENAMES
	ga::Archive::EntryPtr ep2 = pArchive->insert(ep3, FILENAME2, 0, FILETYPE_GENERIC, INSERT_ATTRIBUTE);
#else
	ga::Archive::EntryPtr ep2 = pArchive->insert(ep3, "dummy", 0, FILETYPE_GENERIC, INSERT_ATTRIBUTE);
#endif
	// Make sure it went in ok
	BOOST_REQUIRE_MESSAGE(pArchive->isValid(ep2),
		"Couldn't create new file in archive");
	// Open it
	camoto::iostream_sptr file2(pArchive->open(ep2));

	// Apply any encryption/compression filter
	applyFilter(&file2, ep2);

	// Get offsets of each file for later testing
	ga::FATArchive::FATEntryPtr fat1 =
		boost::dynamic_pointer_cast<ga::FATArchive::FATEntry>(ep1);
	ga::FATArchive::FATEntryPtr fat3 =
		boost::dynamic_pointer_cast<ga::FATArchive::FATEntry>(ep3);

	int off1 = fat1->iOffset;
	int off3 = fat3->iOffset;

	// This will resize the second file.  Since all three files are zero-length,
	// they currently all share the same offset.  This should result in file1
	// keeping its original offset (same as file2) and file3's offset being
	// increased.
	pArchive->resize(ep2, 15);
	file2->write("This is two.dat", 15);
	file2->flush();

	// Make sure the first file hasn't moved
	BOOST_REQUIRE_EQUAL(fat1->iOffset, off1);

	// Make sure the third file has moved.  In theory this could fail if an
	// archive format comes along that can do this correctly without moving the
	// file, but if that ever happens this test can be adjusted then.
	BOOST_REQUIRE_GT(fat3->iOffset, off3);

	pArchive->resize(ep1, 15);
	file1->write("This is one.dat", 15);
	file1->flush();

	// Make sure the first file hasn't moved
	BOOST_REQUIRE_EQUAL(fat1->iOffset, off1);

	// Make sure the third file has moved again.  Same caveat as above.
	BOOST_REQUIRE_GT(fat3->iOffset, off3);

	pArchive->resize(ep3, 17);
	file3->write("This is three.dat", 17);
	file3->flush();

	BOOST_CHECK_MESSAGE(
		is_equal(makeString(TEST_RESULT(insert_end))),
		"Error manipulating zero-length files"
	);

#ifdef HAS_FAT
	BOOST_CHECK_MESSAGE(
		is_supp_equal(ga::EST_FAT, makeString(TEST_RESULT(FAT_insert_end))),
		"Error manipulating zero-length files"
	);
#endif
}

BOOST_AUTO_TEST_SUITE_END()
