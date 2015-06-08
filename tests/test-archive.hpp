/**
 * @file   test-archive.hpp
 * @brief  Generic test code for Archive class descendents.
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

#ifndef _CAMOTO_GAMEARCHIVE_TEST_ARCHIVE_HPP_
#define _CAMOTO_GAMEARCHIVE_TEST_ARCHIVE_HPP_

#include <map>
#include <functional>
#include <boost/test/unit_test.hpp>
#include <camoto/stream_string.hpp>
#include <camoto/gamearchive.hpp>
#include "tests.hpp"

// This header will only be used by test implementations.
using namespace camoto;
using namespace camoto::gamearchive;

/// Exception thrown if test_archivemetadata::metadata_*() is called for
/// unsupported fields.
class test_metadata_not_supported {};

class test_archive: public test_main
{
	public:
		/// Constructor sets some default values.
		test_archive();

		/// Add all the standard tests.
		/**
		 * This can be overridden by descendent classes to add more tests for
		 * particular file formats.  If this is done, remember to call this
		 * function from the overridden one or the standard tests won't get run.
		 */
		virtual void addTests();

		/// Reset pArchive back to a known state.
		/**
		 * @param empty
		 *   true resets to an empty archive (via ArchiveType::create()) while
		 *   false resets to initialstate() and calls ArchiveType::open().
		 */
		virtual void prepareTest(bool empty);

		Archive::FileHandle findFile(unsigned int index,
			const std::string& altname = std::string());

		/// Find a file by index, by looking into various internal structures.
		/**
		 * Searches for files based on the order/index field as that's the order in the
		 * archive, which could be different to the order in the vector.
		 * Works with Archive_FAT::FATEntry and FixedArchive::FixedEntry.
		 */
		virtual Archive::FileHandle getFileAt(const Archive::FileVector& files,
			unsigned int index);

		virtual void test_isinstance_others();
		void test_open();
		void test_rename();
		void test_rename_long();
		void test_insert_long();
		void test_insert_mid();
		void test_insert_end();
		void test_insert2();
		void test_remove();
		void test_remove2();
		void test_remove_open();
		void test_insert_remove();
		void test_remove_insert();
		void test_move();
		void test_resize_larger();
		void test_resize_smaller();
		void test_resize_write();
		void test_resize_after_close();
		void test_remove_all_re_add();
		void test_insert_zero_then_resize();
		void test_resize_over64k();
		void test_shortext();

		virtual void test_new_isinstance();
		virtual void test_new_to_initialstate();
		void test_new_manipulate_zero_length_files();

		void test_metadata_get_desc();
		void test_metadata_set_desc_larger();
		void test_metadata_set_desc_smaller();
		void test_metadata_get_ver();

	protected:
		/// Initial state.
		/**
		 * This is the base state loaded into a format handler and then
		 * modified to produce the states checked by the functions below.
		 *
		 * It should contain two files: ONE.DAT followed by TWO.DAT.
		 */
		virtual std::string initialstate() = 0;

		/// Result of renaming ONE.DAT to THREE.DAT.
		virtual std::string rename() = 0;

		/// Result of inserting THREE.DAT at the end of the archive.
		virtual std::string insert_end() = 0;

		/// Result of inserting THREE.DAT in between ONE.DAT and TWO.DAT.
		virtual std::string insert_mid() = 0;

		/// Result of inserting THREE.DAT followed by FOUR.DAT, after ONE.DAT.
		virtual std::string insert2() = 0;

		/// Result of removing ONE.DAT.
		virtual std::string remove() = 0;

		/// Result of removing ONE.DAT and TWO.DAT, leaving no files.
		virtual std::string remove2() = 0;

		/// Result of inserting THREE.DAT after ONE.DAT, then removing ONE.DAT.
		virtual std::string insert_remove() = 0;

		/// Result of ONE.DAT and TWO.DAT swapping positions.
		virtual std::string move() = 0;

		/// Result of ONE.DAT being enlarged to 20 bytes.
		virtual std::string resize_larger() = 0;

		/// Result of ONE.DAT being shrunk to 10 bytes.
		virtual std::string resize_smaller() = 0;

		/// Result of ONE.DAT being enlarged to 23 bytes and data written to EOF.
		virtual std::string resize_write() = 0;

		/// Result of setting the description to descLarger.
		virtual std::string metadata_set_desc_larger();

		/// Result of setting the description to descSmaller.
		virtual std::string metadata_set_desc_smaller();

		/// Add a test to the suite.  Used by ADD_ARCH_TEST().
		void addBoundTest(bool empty, std::function<void()> fnTest,
			boost::unit_test::const_string name);

		/// Reset the archive to the initial state and run the given test.
		/**
		 * @param empty
		 *   true resets to an empty archive (via ArchiveType::create()) while
		 *   false resets to initialstate() and calls ArchiveType::open().
		 *
		 * @param fnTest
		 *   Function to call once archive is back to initial state.
		 */
		void runTest(bool empty, std::function<void()> fnTest);

		/// Populate suppBase with default content.
		/**
		 * This may be called mid-test if the suppBase content should be reset to
		 * the initial state.
		 */
		void resetSuppData(bool emptyArchive);

		/// Populate suppData with data loaded from suppBase.
		/**
		 * This may be called mid-test if new suppData structures are needed to
		 * create a new Archive instance, since the original ones have been
		 * std::move()'d to the original Archive instance and won't be available to
		 * create a new Archive with.
		 *
		 * This repopulates suppData from the existing suppBase content, so it is
		 * possible to access modified data this way.  If you don't want suppData
		 * that may have been modified by a previous Archive instance, call
		 * resetSuppData() first to return everything to the initialstate.
		 */
		void populateSuppData();

		/// Add an isInstance check to run later.
		/**
		 * @param result
		 *   Expected result when opening the content.
		 *
		 * @param content
		 *   Content to pass as an archive to ArchiveType::isInstance().
		 */
		void isInstance(ArchiveType::Certainty result, const std::string& content);

		/// Perform an isInstance check now.
		virtual void test_isInstance(ArchiveType::Certainty result,
			const std::string& content, unsigned int testNumber);

		/// Add an invalidContent check to run later.
		/**
		 * These checks make sure files that are in the correct format
		 * don't cause segfaults or infinite loops if the data is corrupted.
		 *
		 * @param content
		 *   Content to pass as an archive to ArchiveType::isInstance() where
		 *   it will be reported as a valid instance, then passed to
		 *   ArchiveType::open(), where an exception should be thrown.
		 */
		void invalidContent(const std::string& content);

		/// Perform an invalidContent check now.
		void test_invalidContent(const std::string& content,
			unsigned int testNumber);

		/// Add an changeMetadata check to run later.
		/**
		 * These checks make sure metadata alterations work correctly.
		 *
		 * @param item
		 *   Metadata item to change.
		 *
		 * @param newValue
		 *   New content for metadata item.
		 *
		 * @param content
		 *   Expected result after taking the initialstate() and changing the
		 *   given metadata item as specified.
		 */
		void changeMetadata(camoto::Metadata::MetadataType item,
			const std::string& newValue, const std::string& content);

		/// Perform a changeMetadata check now.
		void test_changeMetadata(camoto::Metadata::MetadataType item,
			const std::string& newValue, const std::string& content,
			unsigned int testNumber);

		/// Does the archive content match the parameter?
		boost::test_tools::predicate_result is_content_equal(const std::string& exp);

		/// Does the given supplementary item content match the parameter?
		boost::test_tools::predicate_result is_supp_equal(
			camoto::SuppItem type, const std::string& strExpected);

	protected:
		/// Underlying data stream containing archive file content.
		std::shared_ptr<stream::string> base;

		/// Pointer to the active archive instance.
		std::shared_ptr<Archive> pArchive;

		/// Pointers to the underlying storage used for suppitems.
		std::map<SuppItem, std::shared_ptr<stream::string>> suppBase;

		/// Supplementary data for the archive, populated by streams sitting on
		/// top of suppBase.
		camoto::SuppData suppData;

	private:
		/// Number of isInstance tests, used to number them sequentially.
		unsigned int numIsInstanceTests;

		/// Number of invalidData tests, used to number them sequentially.
		unsigned int numInvalidContentTests;

		/// Number of changeMetadata tests, used to number them sequentially.
		unsigned int numChangeMetadataTests;

	public:
		/// File type code for this format.
		std::string type;

		/// Can new instances of this format be created? (default is true)
		bool create;

		/// Should we call isInstance() on new archives? (default is true)
		/**
		 * This should only be set to false for those archives where new instances
		 * will fail isInstance tests for a legitimate reason, e.g. empty new
		 * archives are zero bytes long.
		 */
		bool newIsInstance;

		/// Is the file structure static?
		/**
		 * If true, files can be modified, but they can't be resized, relocated,
		 * added or removed.  Defaults to false.
		 */
		bool staticFiles;

		/// Are the files virtual?
		/**
		 * If true, files cannot be opened as they are placeholders for other data
		 * (such as images that must be opened with Tileset::openImage()).
		 * Defaults to false.
		 */
		bool virtualFiles;

		/// Any formats here identify us as an instance of that type, and it
		/// cannot be avoided.
		/**
		 * If "otherformat" is listed here then we will not pass our initialstate
		 * to otherformat's isInstance function.  This is kind of backwards but is
		 * is the way the test functions are designed.
		 */
		std::vector<std::string> skipInstDetect;

		/// Name of files in the archive, in order.
		/**
		 * Defaults to ONE.DAT, TWO.DAT, THREE.DAT and FOUR.DAT.
		 */
		std::string filename[4];

		/// Filename with an extension less than three chars.
		std::string filename_shortext;

		/// Maximum length of a filename, not including any terminating null.
		/**
		 * Set to 0 if there is no (or a very high) maximum, and the filename
		 * length tests will not be run for this format.
		 *
		 * Set to -1 if the format does not support filenames, and all tests will
		 * access files by index/order instead of by filename.
		 */
		int lenMaxFilename;

		/// Length of files if they are all fixed at the same size.
		/**
		 * Set to -1 if filesizes aren't fixed and can be resized arbitrarily.
		 */
		int lenFilesizeFixed;

		/// Attributes to set when inserting files.  Defaults to File::Attribute::Default.
		/**
		 * This can be set to File::Attribute::Compressed if newly inserted files should be
		 * flagged as compressed, and passed through filters to recover the
		 * original data for the test to pass.
		 *
		 * See test-dat-bash-compressed.cpp for an example.
		 */
		Archive::File::Attribute insertAttr;

		/// File type of newly inserted files.  Defaults to FILETYPE_GENERIC.
		std::string insertType;

		/// Content of each file in the archive.
		std::string content[4];

		/// Content of first file after being overwritten.
		std::string content0_overwritten;

		/// Length to enlarge file 1 to.
		unsigned int content0_largeSize;

		/// Length of enlarged file 1 before compression (used in arch header only.)
		unsigned int content0_largeSize_unfiltered;

		/// Length to shrink file 1 to.
		unsigned int content0_smallSize;

		/// Length of shrunk file 1 before compression (used in arch header only.)
		unsigned int content0_smallSize_unfiltered;

		/// Does this format support metadata?
		std::map<camoto::Metadata::MetadataType, bool> hasMetadata;

		/// Default value for metadata 'description' field.
		std::string metadataDesc;

		/// Metadata text to set for 'description' when the field should be
		/// shorter than the initial state.
		/**
		 * After setting the description to this value, the archive file should
		 * match the value returned by metadata_set_desc_smaller().
		 */
		std::string metadataDescSmaller;

		/// Metadata text to set for 'description' when the field should be
		/// longer than the initial state.
		/**
		 * After setting the description to this value, the archive file should
		 * match the value returned by metadata_set_desc_larger().
		 */
		std::string metadataDescLarger;

		/// Default value for metadata 'version' field.
		std::string metadataVer;

		/// Link between supplementary items and the class containing the expected
		/// content for each test case.
		std::map<camoto::SuppItem, std::unique_ptr<test_archive>> suppResult;
};

/// Add a test_archive member function to the test suite
#define ADD_ARCH_TEST(empty, fn) {	  \
	std::function<void()> fnTest = std::bind(fn, this); \
	this->test_archive::addBoundTest(empty, fnTest, BOOST_TEST_STRINGIZE(fn)); \
}

#endif // _CAMOTO_GAMEARCHIVE_TEST_ARCHIVE_HPP_
