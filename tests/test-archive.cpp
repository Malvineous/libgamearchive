/**
 * @file   test-archive.cpp
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

#include <iomanip>
#include <functional>
#include <camoto/util.hpp>
#include <camoto/gamearchive/archive-fat.hpp> // Archive_FAT::FATEntry
#include <camoto/gamearchive/fixedarchive.hpp> // FixedArchive::FixedEntry
#include "test-archive.hpp"

using namespace camoto;
using namespace camoto::gamearchive;

BOOST_AUTO_TEST_CASE(archive_attribute_operators)
{
	BOOST_TEST_MESSAGE("Confirm Attribute operators calculate as expected");

	Archive::File::Attribute a;

	a = Archive::File::Attribute::Default;
	BOOST_REQUIRE_EQUAL((unsigned int)a, 0);

	a |= Archive::File::Attribute::Compressed;
	BOOST_REQUIRE_EQUAL((unsigned int)a, 4);

	a |= Archive::File::Attribute::Hidden;
	BOOST_REQUIRE_EQUAL((unsigned int)a, 6);

	a &= ~Archive::File::Attribute::Compressed;
	BOOST_REQUIRE_EQUAL((unsigned int)a, 2);
}

test_archive::test_archive()
	:	numIsInstanceTests(0),
		numInvalidContentTests(1),
		numChangeAttributeTests(1)
{
	this->create = true;
	this->newIsInstance = true;
	this->staticFiles = false;
	this->virtualFiles = false;
	this->foldersOnly = false;

	this->filename[0] = "ONE.DAT";
	this->filename[1] = "TWO.DAT";
	this->filename[2] = "THREE.DAT";
	this->filename[3] = "FOUR.DAT";
	this->filename_shortext = "TEST.A";
	this->lenMaxFilename = 12;
	this->lenFilesizeFixed = -1;
	this->insertAttr = Archive::File::Attribute::Default;
	this->insertType = FILETYPE_GENERIC;

	this->content[0] = "This is one.dat";
	this->content[1] = "This is two.dat";
	this->content[2] = "This is three.dat";
	this->content[3] = "This is four.dat";
	this->content0_overwritten = "Now resized to 23 chars";

	this->content0_largeSize = 20;
	this->content0_smallSize = 10;
	this->content0_largeSize_unfiltered = this->content0_largeSize;
	this->content0_smallSize_unfiltered = this->content0_smallSize;
}

void test_archive::addTests()
{
	// Tests on existing archives (in the initial state)
	ADD_ARCH_TEST(false, &test_archive::test_isinstance_others);
	if (!this->virtualFiles) {
		ADD_ARCH_TEST(false, &test_archive::test_open);
	}
	if (this->lenMaxFilename >= 0) {
		// Only perform the rename test if the archive has filenames
		ADD_ARCH_TEST(false, &test_archive::test_rename);
		ADD_ARCH_TEST(false, &test_archive::test_shortext);
	}
	if (this->lenMaxFilename > 0) {
		// Only perform these tests if the archive has a filename length limit
		ADD_ARCH_TEST(false, &test_archive::test_rename_long);
		ADD_ARCH_TEST(false, &test_archive::test_insert_long);
	}
	if (!this->staticFiles) {
		ADD_ARCH_TEST(false, &test_archive::test_insert_mid);
		ADD_ARCH_TEST(false, &test_archive::test_insert_end);
		ADD_ARCH_TEST(false, &test_archive::test_insert2);
		ADD_ARCH_TEST(false, &test_archive::test_remove);
		ADD_ARCH_TEST(false, &test_archive::test_remove2);
		ADD_ARCH_TEST(false, &test_archive::test_remove_open);
		ADD_ARCH_TEST(false, &test_archive::test_insert_remove);
		ADD_ARCH_TEST(false, &test_archive::test_remove_insert);
		ADD_ARCH_TEST(false, &test_archive::test_move);
		if (this->lenFilesizeFixed < 0) {
			// Only perform these tests if the archive's files can be resized
			ADD_ARCH_TEST(false, &test_archive::test_resize_larger);
			ADD_ARCH_TEST(false, &test_archive::test_resize_smaller);
			ADD_ARCH_TEST(false, &test_archive::test_resize_write);
			ADD_ARCH_TEST(false, &test_archive::test_resize_after_close);
			ADD_ARCH_TEST(false, &test_archive::test_insert_zero_then_resize);
			ADD_ARCH_TEST(false, &test_archive::test_resize_over64k);
		}
		ADD_ARCH_TEST(false, &test_archive::test_remove_all_re_add);
	}

	// Only perform the attribute tests if supported by the archive format
	if (!this->attributes.empty()) {
		ADD_ARCH_TEST(false, &test_archive::test_attributes);
	}

	// Tests on new archives (in an empty state)
	if (this->create) {
		if (this->newIsInstance) {
			ADD_ARCH_TEST(true, &test_archive::test_new_isinstance);
		}
		ADD_ARCH_TEST(true, &test_archive::test_new_to_initialstate);
		if (this->lenFilesizeFixed < 0) {
			// Only perform these tests if the archive's files can be resized
			ADD_ARCH_TEST(true, &test_archive::test_new_manipulate_zero_length_files);
		}
	}
	return;
}

void test_archive::addBoundTest(bool empty, std::function<void()> fnTest,
	boost::unit_test::const_string file, std::size_t line,
	boost::unit_test::const_string name)
{
	this->ts->add(
		boost::unit_test::make_test_case(
			std::bind(&test_archive::runTest, this, empty, fnTest),
			createString(name << '[' << this->basename << ']'),
			file, line
		)
	);
	return;
}

void test_archive::runTest(bool empty, std::function<void()> fnTest)
{
	this->pArchive.reset();
	this->prepareTest(empty);
	BOOST_REQUIRE_MESSAGE(
		this->pArchive.unique(),
		"Archive has multiple references (" << this->pArchive.use_count()
			<< ", expected 1) before use - this shouldn't happen!"
	);
	fnTest();
	if (this->pArchive) {
		BOOST_REQUIRE_MESSAGE(
			this->pArchive.unique(),
			"Archive left with " << this->pArchive.use_count()
				<< " references after test (should be only 1)"
		);
	}
	return;
}

void test_archive::prepareTest(bool empty)
{
	auto pArchType = ArchiveManager::byCode(this->type);
	BOOST_REQUIRE_MESSAGE(pArchType, "Could not find archive type " + this->type);

	// Make this->suppData valid
	this->resetSuppData(empty);
	this->populateSuppData();

	this->base = std::make_unique<stream::string>();

	if (empty) {
		BOOST_TEST_CHECKPOINT("About to create new empty instance of "
			+ this->basename);
		// This should really use BOOST_REQUIRE_NO_THROW but the message is more
		// informative without it.
		//BOOST_REQUIRE_NO_THROW(
			this->pArchive = pArchType->create(
				stream_wrap(this->base), this->suppData);
		//);
	} else {
		*base << this->initialstate();
		BOOST_TEST_CHECKPOINT("About to open " + this->basename
			+ " initialstate as an archive");
		// This should really use BOOST_REQUIRE_NO_THROW but the message is more
		// informative without it.
		//BOOST_REQUIRE_NO_THROW(
		this->pArchive = pArchType->open(
			stream_wrap(this->base), this->suppData);
		//);
	}
	BOOST_REQUIRE_MESSAGE(this->pArchive, "Could not create archive class");

	if (this->lenMaxFilename < 0) {
		// This format has no filenames, so set them to obviously incorrect values.
		this->filename[0] = "dummy";
		this->filename[1] = "dummy";
		this->filename[2] = "dummy";
		this->filename[3] = "dummy";
	}
	return;
}

Archive::FileHandle test_archive::findFile(unsigned int index,
	const std::string& altname)
{
	BOOST_TEST_CHECKPOINT("Searching for file #" << index);
	Archive::FileHandle ep;
	if (this->lenMaxFilename >= 0) {
		// Find the file we're going to open by name
		std::string filename = altname;
		if (filename.empty()) {
			switch (index) {
				case 0: filename = this->filename[0]; break;
				case 1: filename = this->filename[1]; break;
				case 2: filename = this->filename[2]; break;
				case 3: filename = this->filename[3]; break;
				default:
					BOOST_REQUIRE_MESSAGE(index < 4, "findFile() index out of range");
					break;
			}
		}
		ep = this->pArchive->find(filename);

		// Make sure we found it
		BOOST_REQUIRE_MESSAGE(this->pArchive->isValid(ep),
			"Couldn't find " << filename << " in sample archive");

	} else {
		// No filenames in this format, do it by order
		auto& files = this->pArchive->files();
		ep = getFileAt(files, index);

		// Make sure we found it
		BOOST_REQUIRE_MESSAGE(this->pArchive->isValid(ep),
			"Couldn't find file at index " << index << " in sample archive");
	}
	return ep;
}

Archive::FileHandle test_archive::getFileAt(
	const Archive::FileVector& files, unsigned int index)
{
	for (const auto& i : files) {
		auto pEntry = dynamic_cast<const Archive_FAT::FATEntry *>(&*i);
		if (pEntry) {
			if (pEntry->iIndex == index) return i;
		} else {
			auto pEntry2 = dynamic_cast<const FixedArchive::FixedEntry *>(&*i);
			if (pEntry2) {
				if (pEntry2->index == index) return i;
			} else {
				assert(false);
			}
		}
	}
	return nullptr;
}

void test_archive::resetSuppData(bool empty)
{
	this->suppBase.clear();
	for (auto& i : this->suppResult) {
		auto& item = i.first;
		if (!i.second) {
			std::cout << "Warning: " << this->basename << " sets empty "
				<< suppToString(item) << " suppitem, ignoring.\n";
			continue;
		}
		auto suppSS = std::make_shared<stream::string>();
		if (!empty) {
			// Populate the suppitem with its initial state
			*suppSS << i.second->initialstate();
		}
		this->suppBase[item] = suppSS;
	}
	return;
}

void test_archive::populateSuppData()
{
	this->suppData.clear();
	for (auto& i : this->suppBase) {
		auto& item = i.first;
		auto& suppSS = i.second;
		// Wrap this in a substream to get a unique pointer, with an independent
		// seek position.
		this->suppData[item] = stream_wrap(suppSS);
	}
	return;
}

void test_archive::setAttributes()
{
	int i = 0;
	for (auto& a : this->attributes) {
		switch (a.type) {
			case Attribute::Type::Integer:
				this->pArchive->attribute(i, a.integerValue);
				break;
			case Attribute::Type::Enum:
				this->pArchive->attribute(i, a.enumValue);
				break;
			case Attribute::Type::Filename:
				this->pArchive->attribute(i, a.filenameValue);
				break;
			case Attribute::Type::Text:
				this->pArchive->attribute(i, a.textValue);
				break;
			case Attribute::Type::Image:
				this->pArchive->attribute(i, a.imageIndex);
				break;
		}
		i++;
	}
	return;
}

void test_archive::checkData(std::function<std::string(test_archive&)> fnExpected,
	const std::string& msg)
{
	// Flush out any changes before we perform the check
	if (this->pArchive) this->pArchive->flush();

	// Check main data
	BOOST_CHECK_MESSAGE(
		this->is_content_equal(fnExpected(*this)),
		msg
	);

	// Check all available suppitems
	for (auto& suppItem : this->suppResult) {
		BOOST_CHECK_MESSAGE(
			this->is_supp_equal(suppItem.first, fnExpected(*suppItem.second)),
			"[SuppItem::" << camoto::suppToString(suppItem.first) << "] " << msg
		);
	}
	return;
}

void test_archive::isInstance(ArchiveType::Certainty result,
	const std::string& content)
{
	this->ts->add(
		boost::unit_test::make_test_case(
			std::bind(&test_archive::test_isInstance, this, result, content,
				this->numIsInstanceTests),
			createString("test_archive[" << this->basename << "]::isinstance_c"
				<< std::setfill('0') << std::setw(2) << this->numIsInstanceTests),
			__FILE__, __LINE__
		)
	);
	this->numIsInstanceTests++;
	return;
}

void test_archive::test_isInstance(ArchiveType::Certainty result,
	const std::string& content, unsigned int testNumber)
{
	BOOST_TEST_MESSAGE(this->basename << ": isInstance_c"
		<< std::setfill('0') << std::setw(2) << testNumber);

	auto pTestType = ArchiveManager::byCode(this->type);
	BOOST_REQUIRE_MESSAGE(pTestType,
		createString("Could not find archive type " << this->type));

	stream::string ss;
	ss << content;

	BOOST_CHECK_EQUAL(pTestType->isInstance(ss), result);
	return;
}

void test_archive::invalidContent(const std::string& content)
{
	this->ts->add(
		boost::unit_test::make_test_case(
			std::bind(&test_archive::test_invalidContent, this, content,
				this->numInvalidContentTests),
			createString("test_archive[" << this->basename << "]::invalidcontent_i"
				<< std::setfill('0') << std::setw(2) << this->numInvalidContentTests),
			__FILE__, __LINE__
		)
	);
	this->numInvalidContentTests++;
	return;
}

void test_archive::test_invalidContent(const std::string& content,
	unsigned int testNumber)
{
	BOOST_TEST_MESSAGE(this->basename << ": invalidContent_i"
		<< std::setfill('0') << std::setw(2) << testNumber);

	auto pTestType = ArchiveManager::byCode(this->type);
	BOOST_REQUIRE_MESSAGE(pTestType,
		createString("Could not find archive type " << this->type));

	auto ss = std::make_unique<stream::string>();
	*ss << content;

	// Make sure isInstance reports this is valid
	BOOST_CHECK_EQUAL(pTestType->isInstance(*ss), ArchiveType::DefinitelyYes);

	// Make this->suppData valid again, reusing previous data
	this->populateSuppData();

	// But that we get an error when trying to open the file
	BOOST_CHECK_THROW(
		auto pArchive = pTestType->open(std::move(ss), this->suppData),
		stream::error
	);

	return;
}

void test_archive::changeAttribute(unsigned int attributeIndex,
	const std::string& newValue, const std::string& content)
{
	this->ts->add(
		boost::unit_test::make_test_case(
			std::bind(
				(void(test_archive::*)(unsigned int, const std::string&,
					const std::string&, unsigned int))&test_archive::test_changeAttribute,
				this, attributeIndex,
				newValue, content, this->numChangeAttributeTests),
			createString("test_archive[" << this->basename << "]::changeAttribute_a"
				<< std::setfill('0') << std::setw(2) << this->numChangeAttributeTests),
			__FILE__, __LINE__
		)
	);
	this->numChangeAttributeTests++;
	return;
}

void test_archive::changeAttribute(unsigned int attributeIndex,
	unsigned int newValue, const std::string& content)
{
	this->ts->add(
		boost::unit_test::make_test_case(
			std::bind(
				(void(test_archive::*)(unsigned int, int,
					const std::string&, unsigned int))&test_archive::test_changeAttribute,
				this, attributeIndex,
				newValue, content, this->numChangeAttributeTests),
			createString("test_archive[" << this->basename << "]::changeAttribute_a"
				<< std::setfill('0') << std::setw(2) << this->numChangeAttributeTests),
			__FILE__, __LINE__
		)
	);
	this->numChangeAttributeTests++;
	return;
}

void test_archive::test_changeAttribute(unsigned int attributeIndex,
	const std::string& newValue, const std::string& content,
	unsigned int testNumber)
{
	BOOST_TEST_MESSAGE(this->basename << ": changeAttribute_a"
		<< std::setfill('0') << std::setw(2) << testNumber);

	this->prepareTest(false);
	this->pArchive->attribute(attributeIndex, newValue);
	this->pArchive->flush();

	// Can't use checkData() here as we don't have parameter for target suppData
	BOOST_CHECK_MESSAGE(
		this->is_content_equal(content),
		"Error setting string attribute"
	);
	return;
}

void test_archive::test_changeAttribute(unsigned int attributeIndex,
	int newValue, const std::string& content,
	unsigned int testNumber)
{
	BOOST_TEST_MESSAGE(this->basename << ": changeAttribute_a"
		<< std::setfill('0') << std::setw(2) << testNumber);

	this->prepareTest(false);
	this->pArchive->attribute(attributeIndex, newValue);
	this->pArchive->flush();

	// Can't use checkData() here as we don't have parameter for target suppData
	BOOST_CHECK_MESSAGE(
		this->is_content_equal(content),
		"Error setting int attribute"
	);
	return;
}

boost::test_tools::predicate_result test_archive::is_content_equal(
	const std::string& exp)
{
	return this->is_equal(exp, this->base->data);
}

boost::test_tools::predicate_result test_archive::is_supp_equal(
	camoto::SuppItem type, const std::string& strExpected)
{
	// Use the supp's test-class' own comparison function, as this will use its
	// preferred outputWidth value, which might be different to the main file's.
	return this->suppResult[type]->is_equal(strExpected,
		this->suppBase[type]->data);
}

void test_archive::test_isinstance_others()
{
	// Check all file formats except this one to avoid any false positives
	BOOST_TEST_MESSAGE(this->basename << ": isInstance check against other formats");

	stream::string content;
	content << this->initialstate();

	for (const auto& pTestType : ArchiveManager::formats()) {
		// Don't check our own type, that's done by the other isinstance_* tests
		std::string otherType = pTestType->code();
		if (otherType.compare(this->type) == 0) continue;

		// Skip any formats known to produce false detections unavoidably
		if (
			std::find(
				this->skipInstDetect.begin(), this->skipInstDetect.end(), otherType
			) != this->skipInstDetect.end()) continue;

		BOOST_TEST_CHECKPOINT("Checking " << this->type
			<< " content against isInstance() for " << otherType);

		// Put this outside the BOOST_CHECK_MESSAGE macro so if an exception is
		// thrown we can see the above BOOST_TEST_CHECKPOINT message telling us which
		// handler is to blame.
		auto isInstanceResult = pTestType->isInstance(content);

		BOOST_CHECK_MESSAGE(isInstanceResult < ArchiveType::DefinitelyYes,
			"isInstance() for " << otherType << " incorrectly recognises content for "
			<< this->type);
	}
	return;
}

void test_archive::test_open()
{
	BOOST_TEST_MESSAGE(this->basename << ": Opening file in archive");

	auto ep = this->findFile(0);

	if (this->foldersOnly) {
		BOOST_REQUIRE_MESSAGE(ep->fAttr & Archive::File::Attribute::Folder,
			"Folder-only archive contains files!");

		this->pArchive = this->pArchive->openFolder(ep);
		ep = this->findFile(0);
	}

	// Open it
	auto pfsIn = this->pArchive->open(ep, true);

	stream::string out;

	// Make sure the file opens at the start
	BOOST_REQUIRE_EQUAL(pfsIn->tellg(), 0);

	// Copy it into the stringstream
	stream::copy(out, *pfsIn);

	BOOST_CHECK_MESSAGE(
		this->is_equal(this->content[0], out.data),
		"Error opening file or wrong file opened"
	);

	// No changes, so no flush
}

void test_archive::test_rename()
{
	BOOST_TEST_MESSAGE(this->basename << ": Renaming file inside archive");

	BOOST_REQUIRE_MESSAGE(this->lenMaxFilename >= 0,
		"Tried to run test_archive::test_rename() on a format with no filenames!");

	Archive::FileHandle ep = this->findFile(0);

	this->pArchive->rename(ep, this->filename[2]);

	this->checkData(&test_archive::rename,
		"Error renaming file");
}

void test_archive::test_rename_long()
{
	BOOST_TEST_MESSAGE(this->basename << ": Rename file with name too long");

	BOOST_REQUIRE_MESSAGE(this->lenMaxFilename >= 0,
		"Tried to run test_archive::test_rename_long() on a format with no filenames!");
	BOOST_REQUIRE_MESSAGE(this->lenMaxFilename > 0,
		"Tried to run test_archive::test_rename_long() on a format with unlimited-length filenames!");

	Archive::FileHandle ep = this->findFile(0);

	assert(this->lenMaxFilename < 256);
	char name[256 + 2];
	memset(name, 'A', this->lenMaxFilename + 1);
	name[this->lenMaxFilename + 1] = 0;

	// Make sure renaming fails when the filename is too long
	BOOST_CHECK_THROW(
		this->pArchive->rename(ep, name),
		stream::error
	);

	this->checkData(&test_archive::initialstate,
		"Archive corrupted after failed rename"
	);

	memset(name, 'A', this->lenMaxFilename);
	name[this->lenMaxFilename - 4] = '.';
	name[this->lenMaxFilename] = 0;

	// Make sure the rename succeeds when the filename is exactly the max length
	BOOST_CHECK_NO_THROW(
		this->pArchive->rename(ep, name)
	);

	// Flush to avoid warning when destroying after modifying without flushing
	this->pArchive->flush();
}

void test_archive::test_insert_long()
{
	BOOST_TEST_MESSAGE(this->basename << ": Inserting file with name too long");

	BOOST_REQUIRE_MESSAGE(this->lenMaxFilename >= 0,
		"Tried to run test_archive::test_insert_long() on a format with no filenames!");
	BOOST_REQUIRE_MESSAGE(this->lenMaxFilename > 0,
		"Tried to run test_archive::test_insert_long() on a format with unlimited-length filenames!");

	Archive::FileHandle epb = this->findFile(0);

	assert(this->lenMaxFilename < 256);
	char name[256 + 2];
	memset(name, 'A', this->lenMaxFilename + 1);
	name[this->lenMaxFilename + 1] = 0;

	BOOST_CHECK_THROW(
		Archive::FileHandle ep = this->pArchive->insert(epb, name,
			this->content[0].length(), this->insertType, this->insertAttr),
		stream::error
	);

	this->checkData(&test_archive::initialstate,
		"Archive corrupted after failed insert"
	);

	memset(name, 'A', this->lenMaxFilename);
	name[this->lenMaxFilename - 4] = '.';
	name[this->lenMaxFilename] = 0;

	BOOST_CHECK_NO_THROW(
		Archive::FileHandle ep = this->pArchive->insert(epb, name,
			this->content[0].length(), this->insertType, this->insertAttr)
	);

	// Flush to avoid warning when destroying after modifying without flushing
	this->pArchive->flush();
}

void test_archive::test_insert_end()
{
	BOOST_TEST_MESSAGE(this->basename << ": Inserting file at end of archive");

	if (this->foldersOnly) {
		// Create a folder for the new file
		auto folder = this->pArchive->insert(nullptr,
			this->filename[2], 0, {}, Archive::File::Attribute::Folder);

		BOOST_REQUIRE_MESSAGE(this->pArchive->isValid(folder),
			"Couldn't create new folder in sample archive");

		this->pArchive = this->pArchive->openFolder(folder);
	}

	// Insert the file
	auto ep = this->pArchive->insert(nullptr,
		this->filename[2], this->content[2].length(), this->insertType,
		this->insertAttr);

	// Make sure it went in ok
	BOOST_REQUIRE_MESSAGE(this->pArchive->isValid(ep),
		"Couldn't create new file in sample archive");

	// Open it
	auto pfsNew = this->pArchive->open(ep, true);

	// Set the size of the file we want to write
	pfsNew->truncate(this->content[2].length());
	pfsNew->seekp(0, stream::start);
	pfsNew->write(this->content[2]);
	pfsNew->flush();

	this->checkData(&test_archive::insert_end,
		"Error inserting file at end of archive"
	);
}

void test_archive::test_insert_mid()
{
	BOOST_TEST_MESSAGE(this->basename << ": Inserting file into middle of archive");

	Archive::FileHandle epBefore = this->findFile(1);

	if (this->foldersOnly) {
		// Create a folder for the new file
		auto folder = this->pArchive->insert(epBefore,
			this->filename[2], 0, {}, Archive::File::Attribute::Folder);

		BOOST_REQUIRE_MESSAGE(this->pArchive->isValid(folder),
			"Couldn't create new folder in sample archive");

		this->pArchive = this->pArchive->openFolder(folder);
		epBefore = nullptr; // insert file at end of new folder
	}

	// Insert the file
	Archive::FileHandle ep = this->pArchive->insert(epBefore, this->filename[2],
		this->content[2].length(), this->insertType, this->insertAttr);

	// Make sure it went in ok
	BOOST_REQUIRE_MESSAGE(this->pArchive->isValid(ep),
		"Couldn't insert new file in sample archive");

	// Open it
	auto pfsNew = this->pArchive->open(ep, true);

	pfsNew->write(this->content[2]);
	pfsNew->flush();

	this->checkData(&test_archive::insert_mid,
		"Error inserting file in middle of archive"
	);
}

void test_archive::test_insert2()
{
	BOOST_TEST_MESSAGE(this->basename << ": Inserting multiple files");

	Archive::FileHandle epBefore = this->findFile(1);

	auto origArchive = this->pArchive;
	if (this->foldersOnly) {
		// Create a folder for the new file
		auto folder = this->pArchive->insert(epBefore,
			this->filename[2], 0, {}, Archive::File::Attribute::Folder);

		BOOST_REQUIRE_MESSAGE(this->pArchive->isValid(folder),
			"Couldn't create new folder in sample archive");

		this->pArchive = this->pArchive->openFolder(folder);
		epBefore = nullptr; // insert file at end of new folder
	}

	// Insert the file
	Archive::FileHandle ep1 = this->pArchive->insert(epBefore, this->filename[2],
		this->content[2].length(), this->insertType, this->insertAttr);

	// Make sure it went in ok
	BOOST_REQUIRE_MESSAGE(this->pArchive->isValid(ep1),
		"Couldn't insert first new file in sample archive");

	// Open it
	auto pfsNew1 = this->pArchive->open(ep1, true);

	pfsNew1->write(this->content[2]);
	pfsNew1->flush();

	// Go back to root dir
	this->pArchive = origArchive;

	epBefore = this->findFile(2, this->filename[1]); // FILENAME2 is now the third fil

	if (this->foldersOnly) {
		// Create a folder for the new file
		auto folder = this->pArchive->insert(epBefore,
			this->filename[3], 0, {}, Archive::File::Attribute::Folder);

		BOOST_REQUIRE_MESSAGE(this->pArchive->isValid(folder),
			"Couldn't create new folder in sample archive");

		this->pArchive = this->pArchive->openFolder(folder);
		epBefore = nullptr; // insert file at end of new folder
	}

	// Insert the file
	Archive::FileHandle ep2 = this->pArchive->insert(epBefore, this->filename[3],
		this->content[3].length(), this->insertType, this->insertAttr);

	// Make sure it went in ok
	BOOST_REQUIRE_MESSAGE(this->pArchive->isValid(ep2),
		"Couldn't insert second new file in sample archive");

	auto pfsNew2 = this->pArchive->open(ep2, true);

	pfsNew2->write(this->content[3]);
	pfsNew2->flush();

	this->checkData(&test_archive::insert2,
		"Error inserting two files"
	);
}

void test_archive::test_remove()
{
	BOOST_TEST_MESSAGE(this->basename << ": Removing file from archive");

	Archive::FileHandle ep = this->findFile(0);

	// Remove it
	this->pArchive->remove(ep);

	this->checkData(&test_archive::remove,
		"Error removing file"
	);
}

void test_archive::test_remove2()
{
	BOOST_TEST_MESSAGE(this->basename << ": Removing multiple files from archive");

	Archive::FileHandle ep1 = this->findFile(0);
	Archive::FileHandle ep2 = this->findFile(1);

	// Remove it
	this->pArchive->remove(ep1);
	this->pArchive->remove(ep2);

	this->checkData(&test_archive::remove2,
		"Error removing multiple files"
	);
}

void test_archive::test_remove_open()
{
	BOOST_TEST_MESSAGE(this->basename << ": Attemping to remove an open file");

	auto ep1 = this->findFile(0);
	auto ep2 = ep1;

	auto origArchive = this->pArchive;
	if (this->foldersOnly) {
		// Create a folder for the new file
		auto folder = this->pArchive->openFolder(ep1);

		this->pArchive = this->pArchive->openFolder(ep1);
		ep2 = this->findFile(0);
	}
	auto content1 = this->pArchive->open(ep2, false);

	// Go back to root dir
	this->pArchive = origArchive;

	// Removing an open file should be allowed
	this->pArchive->remove(ep1);

	this->checkData(&test_archive::remove,
		"Error removing open file"
	);

	// But now it should no longer be possible to use the file
	BOOST_CHECK_THROW(
		content1->seekg(0, stream::start),
		stream::error
	);
}

void test_archive::test_insert_remove()
{
	BOOST_TEST_MESSAGE(this->basename << ": Insert then remove file from archive");

	Archive::FileHandle epBefore = this->findFile(1);

	auto origArchive = this->pArchive;
	if (this->foldersOnly) {
		// Create a folder for the new file
		auto folder = this->pArchive->insert(epBefore,
			this->filename[2], 0, {}, Archive::File::Attribute::Folder);

		BOOST_REQUIRE_MESSAGE(this->pArchive->isValid(folder),
			"Couldn't create new folder in sample archive");

		this->pArchive = this->pArchive->openFolder(folder);
		epBefore = nullptr;
	}

	// Insert the file
	Archive::FileHandle ep = this->pArchive->insert(epBefore, this->filename[2],
		this->content[2].length(), this->insertType, this->insertAttr);

	// Make sure it went in ok
	BOOST_REQUIRE_MESSAGE(this->pArchive->isValid(ep),
		"Couldn't insert new file in sample archive");

	// Open it
	auto pfsNew = this->pArchive->open(ep, true);

	pfsNew->write(this->content[2]);
	pfsNew->flush();

	this->pArchive = origArchive;
	Archive::FileHandle ep2 = this->findFile(0);

	// Remove it
	this->pArchive->remove(ep2);

	this->checkData(&test_archive::insert_remove,
		"Error inserting then removing file"
	);
}

void test_archive::test_remove_insert()
{
	BOOST_TEST_MESSAGE(this->basename << ": Remove then insert file from archive");

	Archive::FileHandle ep2 = this->findFile(0);

	// Remove it
	this->pArchive->remove(ep2);

	Archive::FileHandle epBefore = this->findFile(0, this->filename[1]);

	if (this->foldersOnly) {
		// Create a folder for the new file
		auto folder = this->pArchive->insert(epBefore,
			this->filename[2], 0, {}, Archive::File::Attribute::Folder);

		BOOST_REQUIRE_MESSAGE(this->pArchive->isValid(folder),
			"Couldn't create new folder in sample archive");

		this->pArchive = this->pArchive->openFolder(folder);
		epBefore = nullptr;
	}

	// Insert the file
	auto ep = this->pArchive->insert(epBefore, this->filename[2],
		this->content[2].length(), this->insertType, this->insertAttr);

	// Make sure it went in ok
	BOOST_REQUIRE_MESSAGE(this->pArchive->isValid(ep),
		"Couldn't insert new file in sample archive");

	// Open it
	auto pfsNew = this->pArchive->open(ep, true);

	pfsNew->write(this->content[2]);
	pfsNew->flush();

	// This test checks against the insert_remove result instead, as the end
	// result should be the same as that test.
	this->checkData(&test_archive::insert_remove,
		"Error removing then inserting file"
	);
}

void test_archive::test_move()
{
	BOOST_TEST_MESSAGE(this->basename << ": Moving file inside archive");

	Archive::FileHandle ep1 = this->findFile(0);
	Archive::FileHandle ep2 = this->findFile(1);

	// Swap the file positions
	this->pArchive->move(ep1, ep2);

	this->checkData(&test_archive::move,
		"Error moving file"
	);
}

void test_archive::test_resize_larger()
{
	BOOST_TEST_MESSAGE(this->basename << ": Enlarging a file inside the archive");

	Archive::FileHandle ep = this->findFile(0);

	this->pArchive->resize(ep, this->content0_largeSize,
		this->content0_largeSize_unfiltered);

	this->checkData(&test_archive::resize_larger,
		"Error enlarging a file"
	);
}

void test_archive::test_resize_smaller()
{
	BOOST_TEST_MESSAGE(this->basename << ": Shrink a file inside the archive");

	// Find the file we're going to resize
	Archive::FileHandle ep = this->findFile(0);

	if (this->foldersOnly) {
		this->pArchive = this->pArchive->openFolder(ep);
		ep = this->findFile(0);
	}

	this->pArchive->resize(ep, this->content0_smallSize,
		this->content0_smallSize_unfiltered);

	this->checkData(&test_archive::resize_smaller,
		"Error shrinking a file"
	);
}

void test_archive::test_resize_write()
{
	BOOST_TEST_MESSAGE(this->basename << ": Enlarging a file inside the archive");

	// Find the file we're going to resize
	auto ep = this->findFile(0);

	auto origArchive = this->pArchive;
	if (this->foldersOnly) {
		this->pArchive = this->pArchive->openFolder(ep);
		ep = this->findFile(0);
	}

	// We can't call Archive::resize() because that resizes the storage space,
	// but if there are filters involved the storage space might be quite a
	// different size to the data we want to write, so we have to open the
	// stream and use truncate() instead.
	//this->pArchive->resize(ep, sizeof(CONTENT1_OVERWRITTEN) - 1, sizeof(CONTENT1_OVERWRITTEN) - 1);

	auto pfsNew = this->pArchive->open(ep, true);

	// Make sure it's the right size
	BOOST_REQUIRE_EQUAL(pfsNew->size(), this->content[0].length());

	pfsNew->truncate(this->content0_overwritten.length());

	// Make sure it's the right size
	BOOST_REQUIRE_EQUAL(pfsNew->size(), this->content0_overwritten.length());

	pfsNew->seekp(0, stream::start);
	pfsNew->write(this->content0_overwritten);
	pfsNew->flush();

	// Make sure it's the right size
	BOOST_REQUIRE_EQUAL(pfsNew->size(), this->content0_overwritten.length());

	this->checkData(&test_archive::resize_write,
		"Error enlarging a file then writing into new space"
	);

	if (this->foldersOnly) this->pArchive = origArchive;

	// Open the file following it to make sure it was moved out of the way
	auto ep2 = this->findFile(1);

	if (this->foldersOnly) {
		this->pArchive = this->pArchive->openFolder(ep2);
		ep2 = this->findFile(0);
	}

	// Open it
	auto pfsIn = this->pArchive->open(ep2, true);

	// Make sure it's the right size
	BOOST_REQUIRE_EQUAL(pfsIn->size(), this->content[1].length());

	// Copy it into the stringstream
	stream::string out;
	stream::copy(out, *pfsIn);

	BOOST_CHECK_MESSAGE(
		this->is_equal(this->content[1], out.data),
		"Unrelated file was corrupted after file resize operation"
	);
}

void test_archive::test_resize_after_close()
{
	BOOST_TEST_MESSAGE(this->basename << ": Write to a file after closing the archive");

	// Find the file we're going to resize
	Archive::FileHandle ep = this->findFile(0);

	if (this->foldersOnly) {
		this->pArchive = this->pArchive->openFolder(ep);
		ep = this->findFile(0);
	}

	auto pfsNew = this->pArchive->open(ep, true);

	// Close our reference to the archive to make sure we can still write to the
	// stream afterwards.
	this->pArchive = nullptr;

	// Make sure it's the right size
	BOOST_REQUIRE_EQUAL(pfsNew->size(), this->content[0].length());

	pfsNew->truncate(this->content0_overwritten.length());

	// Make sure it's the right size
	BOOST_REQUIRE_EQUAL(pfsNew->size(), this->content0_overwritten.length());

	pfsNew->seekp(0, stream::start);
	pfsNew->write(this->content0_overwritten);
	pfsNew->flush();

	// Make sure it's the right size
	BOOST_REQUIRE_EQUAL(pfsNew->size(), this->content0_overwritten.length());

	this->checkData(&test_archive::resize_write,
		"Error writing to a file after closing the archive"
	);
}

// Remove all the files from the archive, then add them back in again.  This
// differs from the insert/remove tests above as it takes the archive to the
// point where it has no files at all.
void test_archive::test_remove_all_re_add()
{
	BOOST_TEST_MESSAGE(this->basename << ": Remove all files then add them again");

	Archive::FileHandle epOne = this->findFile(0);
	this->pArchive->remove(epOne);

	Archive::FileHandle epTwo = this->findFile(0, this->filename[1]);
	this->pArchive->remove(epTwo);

	// Make sure there are now no files in the archive
	auto& allfiles = this->pArchive->files();
	BOOST_REQUIRE_EQUAL(allfiles.size(), 0);

	auto origArchive = this->pArchive;
	if (this->foldersOnly) {
		// Create a folder for the new file
		auto folder = this->pArchive->insert(nullptr,
			this->filename[0], 0, {}, Archive::File::Attribute::Folder);

		BOOST_REQUIRE_MESSAGE(this->pArchive->isValid(folder),
			"Couldn't create new folder in sample archive");

		this->pArchive = this->pArchive->openFolder(folder);
	}

	// Add the files back again
	epOne = this->pArchive->insert(nullptr, this->filename[0],
		this->content[0].length(), this->insertType, this->insertAttr);

	BOOST_REQUIRE_MESSAGE(this->pArchive->isValid(epOne),
		"Couldn't insert new file after removing all files");

	auto pfsNew = this->pArchive->open(epOne, true);

	pfsNew->write(this->content[0]);
	pfsNew->flush();

	if (this->foldersOnly) {
		this->pArchive = origArchive;
		// Create a folder for the new file
		auto folder = this->pArchive->insert(nullptr,
			this->filename[1], 0, {}, Archive::File::Attribute::Folder);

		BOOST_REQUIRE_MESSAGE(this->pArchive->isValid(folder),
			"Couldn't create new folder in sample archive");

		this->pArchive = this->pArchive->openFolder(folder);
	}

	epTwo = this->pArchive->insert(nullptr, this->filename[1],
		this->content[1].length(), this->insertType, this->insertAttr);

	BOOST_REQUIRE_MESSAGE(this->pArchive->isValid(epTwo),
		"Couldn't insert second new file after removing all files");

	pfsNew = this->pArchive->open(epTwo, true);

	pfsNew->write(this->content[1]);
	pfsNew->flush();

	this->checkData(&test_archive::initialstate,
		"Error removing all files then reinserting them again"
	);
}

// The function shifting files can get confused if a zero-length file is
// inserted, incorrectly moving it because of the zero size.
void test_archive::test_insert_zero_then_resize()
{
	BOOST_TEST_MESSAGE(this->basename << ": Inserting empty file into archive, then resize it");

	if (this->foldersOnly) {
		// Create a folder for the new file
		auto folder = this->pArchive->insert(nullptr,
			this->filename[2], 0, {}, Archive::File::Attribute::Folder);

		BOOST_REQUIRE_MESSAGE(this->pArchive->isValid(folder),
			"Couldn't create new folder in sample archive");

		this->pArchive = this->pArchive->openFolder(folder);
	}

	// Insert the file
	auto ep = this->pArchive->insert(nullptr,
		this->filename[2], 0, this->insertType, this->insertAttr);

	// Make sure it went in ok
	BOOST_REQUIRE_MESSAGE(this->pArchive->isValid(ep),
		"Couldn't create new file in sample archive");

	// Open it
	auto pfsNew = this->pArchive->open(ep, true);

	this->pArchive->resize(ep, this->content[2].length(), this->content[2].length());
	pfsNew->seekp(0, stream::start);
	pfsNew->write(this->content[2]);
	pfsNew->flush();

	this->checkData(&test_archive::insert_end,
		"Error resizing newly inserted empty file"
	);
}

void test_archive::test_resize_over64k()
{
	BOOST_TEST_MESSAGE(this->basename << ": Enlarging a file to over the 64k limit");

	Archive::FileHandle ep = this->findFile(0);
	if (this->foldersOnly) {
		this->pArchive = this->pArchive->openFolder(ep);
		ep = this->findFile(0);
	}

	// Do a potentially illegal resize
	try {
		pArchive->resize(ep, 65537, 65537);

		// Flush to avoid warning when destroying after modifying without flushing
		this->pArchive->flush();
	} catch (const stream::error&) {
		this->checkData(&test_archive::initialstate,
			"Archive corrupted after failed file resize to over 64k"
		);
	}
}

void test_archive::test_shortext()
{
	BOOST_TEST_MESSAGE(this->basename << ": Rename a file with a short extension");

	Archive::FileHandle ep = this->findFile(0);
	this->pArchive->rename(ep, this->filename_shortext);
	this->pArchive->flush();
	this->pArchive.reset();

	// Reopen the archive
	auto pTestType = ArchiveManager::byCode(this->type);
	BOOST_REQUIRE_MESSAGE(pTestType,
		createString("Could not find archive type " << this->type));

	// Make this->suppData valid again, reusing previous data
	this->populateSuppData();

	auto base2 = stream_wrap(this->base);
	this->pArchive = pTestType->open(std::move(base2), this->suppData);

	// See if we can find the file again
	ep = this->pArchive->find(this->filename_shortext);

	// Make sure we found it
	BOOST_REQUIRE_MESSAGE(this->pArchive->isValid(ep),
		createString("Couldn't find file after rename to "
			<< this->filename_shortext));

	this->pArchive->rename(ep, this->filename[0]);

	this->checkData(&test_archive::initialstate,
		"Failed to rename file with short extension back to long"
	);
}

void test_archive::test_attributes()
{
	BOOST_TEST_MESSAGE(this->basename << ": Test attributes");

	auto& attrAllArchive = this->pArchive->attributes();
	int i = 0;
	for (auto& attrExpected : this->attributes) {
		BOOST_REQUIRE_MESSAGE(i < attrAllArchive.size(),
			"Cannot find attribute #" << i);
		auto& attrArchive = attrAllArchive[i];

		BOOST_REQUIRE_EQUAL((int)attrExpected.type, (int)attrArchive.type);

		switch (attrExpected.type) {
			case Attribute::Type::Integer:
				BOOST_REQUIRE_EQUAL(attrExpected.integerValue, attrArchive.integerValue);
				break;
			case Attribute::Type::Enum:
				BOOST_REQUIRE_EQUAL(attrExpected.enumValue, attrArchive.enumValue);
				break;
			case Attribute::Type::Filename:
				BOOST_REQUIRE_MESSAGE(
					this->is_equal(attrExpected.filenameValue, attrArchive.filenameValue),
					"Error getting filename attribute"
				);
				break;
			case Attribute::Type::Text:
				BOOST_REQUIRE_MESSAGE(
					this->is_equal(attrExpected.textValue, attrArchive.textValue),
					"Error getting text attribute"
				);
				break;
			case Attribute::Type::Image:
				BOOST_REQUIRE_EQUAL(attrExpected.imageIndex, attrArchive.imageIndex);
		}
		i++;
	}
}

//
// Tests on empty archives
//

// Make sure a newly created archive is confirmed as a valid instance of that
// archive format.
void test_archive::test_new_isinstance()
{
	BOOST_TEST_MESSAGE(this->basename << ": Checking new archive is valid instance of itself");

	this->pArchive->flush();

	auto pTestType = ArchiveManager::byCode(this->type);
	BOOST_REQUIRE_MESSAGE(pTestType,
		createString("Could not find archive type " << this->type));

	BOOST_REQUIRE_MESSAGE(pTestType->isInstance(*this->base),
		"Newly created archive was not recognised as a valid instance");

	BOOST_TEST_CHECKPOINT("New archive reported valid, trying to open");

	// Make this->suppData valid again, reusing previous data
	this->populateSuppData();

	auto base2 = stream_wrap(this->base);

	// This should really use BOOST_REQUIRE_NO_THROW but the message is more
	// informative without it.
	//BOOST_REQUIRE_NO_THROW(
		auto pArchive = pTestType->open(std::move(base2), this->suppData);
	//);

	// Make sure there are now no files in the archive
	auto& files = pArchive->files();
	BOOST_REQUIRE_EQUAL(files.size(), 0);
}

void test_archive::test_new_to_initialstate()
{
	BOOST_TEST_MESSAGE(this->basename << ": Creating archive from scratch");

	// Need to set this first as (in the case of Blood RFF) it affects what type
	// of files we are allowed to insert.
	this->setAttributes();

	auto& files2 = this->pArchive->files();
	BOOST_REQUIRE_EQUAL(files2.size(), 0);

	auto origArchive = this->pArchive;
	if (this->foldersOnly) {
		// Create a folder for the new file
		auto folder = this->pArchive->insert(nullptr,
			this->filename[0], 0, {}, Archive::File::Attribute::Folder);

		BOOST_REQUIRE_MESSAGE(this->pArchive->isValid(folder),
			"Couldn't create new folder in sample archive");

		this->pArchive = this->pArchive->openFolder(folder);
	}

	// Add the files to the new archive
	auto epOne = this->pArchive->insert(nullptr,
		this->filename[0], this->content[0].length(), this->insertType,
		this->insertAttr);

	BOOST_REQUIRE_MESSAGE(this->pArchive->isValid(epOne),
		"Couldn't insert new file in empty archive");

	auto pfsNew = this->pArchive->open(epOne, true);
	pfsNew->write(this->content[0]);
	pfsNew->flush();

	if (this->foldersOnly) {
		this->pArchive = origArchive;
		// Create a folder for the new file
		auto folder = this->pArchive->insert(nullptr,
			this->filename[1], 0, {}, Archive::File::Attribute::Folder);

		BOOST_REQUIRE_MESSAGE(this->pArchive->isValid(folder),
			"Couldn't create new folder in sample archive");

		this->pArchive = this->pArchive->openFolder(folder);
	}

	auto epTwo = pArchive->insert(nullptr,
		this->filename[1], this->content[1].length(), this->insertType,
		this->insertAttr);

	BOOST_REQUIRE_MESSAGE(this->pArchive->isValid(epTwo),
		"Couldn't insert second new file in empty archive");
	pfsNew = pArchive->open(epTwo, true);
	pfsNew->write(this->content[1]);
	pfsNew->flush();

	this->pArchive = origArchive;

	this->checkData(&test_archive::initialstate,
		"Error inserting files in new/empty archive"
	);

	// Make sure there are now the correct number of files in the archive.
	// This is done after the above check so that if something goes wrong,
	// we can see what the file content looks like.
	auto& files = this->pArchive->files();
	BOOST_REQUIRE_EQUAL(files.size(), 2);
}

// The function shifting files can get confused if a zero-length file is
// inserted, incorrectly moving it because of the zero size.
void test_archive::test_new_manipulate_zero_length_files()
{
	BOOST_TEST_MESSAGE(this->basename << ": Inserting empty files into archive, then resizing them");

	this->setAttributes();

	auto origArchive = this->pArchive;
	Archive::FileHandle folder3;
	if (this->foldersOnly) {
		// Create a folder for the new file
		folder3 = this->pArchive->insert(nullptr,
			this->filename[2], 0, {}, Archive::File::Attribute::Folder);

		BOOST_REQUIRE_MESSAGE(this->pArchive->isValid(folder3),
			"Couldn't create new folder in sample archive");

		this->pArchive = this->pArchive->openFolder(folder3);
	}

	// Insert the file
	auto ep3 = this->pArchive->insert(nullptr,
		this->filename[2], 0, this->insertType, this->insertAttr);

	// Make sure it went in ok
	BOOST_REQUIRE_MESSAGE(this->pArchive->isValid(ep3),
		"Couldn't create new file in archive");

	// Open it
	auto file3 = this->pArchive->open(ep3, true);

	auto ep3_before = ep3;
	if (this->foldersOnly) {
		this->pArchive = origArchive;
		// Create a folder for the new file
		auto folder = this->pArchive->insert(folder3,
			this->filename[0], 0, {}, Archive::File::Attribute::Folder);

		BOOST_REQUIRE_MESSAGE(this->pArchive->isValid(folder),
			"Couldn't create new folder in sample archive");

		this->pArchive = this->pArchive->openFolder(folder);
		ep3_before.reset();// = nullptr;
	}

	auto ep1 = this->pArchive->insert(ep3_before, this->filename[0], 0,
		this->insertType, this->insertAttr);

	// Make sure it went in ok
	BOOST_REQUIRE_MESSAGE(this->pArchive->isValid(ep1),
		"Couldn't create new file in archive");

	// Open it
	auto file1 = this->pArchive->open(ep1, true);

	if (this->foldersOnly) {
		this->pArchive = origArchive;
		// Create a folder for the new file
		auto folder = this->pArchive->insert(folder3,
			this->filename[1], 0, {}, Archive::File::Attribute::Folder);

		BOOST_REQUIRE_MESSAGE(this->pArchive->isValid(folder),
			"Couldn't create new folder in sample archive");

		this->pArchive = this->pArchive->openFolder(folder);
	}

	auto ep2 = this->pArchive->insert(ep3_before, this->filename[1], 0,
		this->insertType, this->insertAttr);

	// Make sure it went in ok
	BOOST_REQUIRE_MESSAGE(this->pArchive->isValid(ep2),
		"Couldn't create new file in archive");

	// Open it
	auto file2 = this->pArchive->open(ep2, true);

	// Get offsets of each file for later testing
	auto fat1 =
		std::dynamic_pointer_cast<const Archive_FAT::FATEntry>(ep1);
	auto fat3 =
		std::dynamic_pointer_cast<const Archive_FAT::FATEntry>(ep3);

	int off1 = fat1->iOffset;
	int off3 = fat3->iOffset;

	// This will resize the second file.  Since all three files are zero-length,
	// they currently all share the same offset.  This should result in file1
	// keeping its original offset (same as file2) and file3's offset being
	// increased.
	file2->truncate(this->content[1].length());
	file2->seekp(0, stream::start);
	file2->write(this->content[1]);
	file2->flush();

	// Make sure the first file hasn't moved
	BOOST_REQUIRE_EQUAL(fat1->iOffset, off1);

	// Make sure the third file has moved.  In theory this could fail if an
	// archive format comes along that can do this correctly without moving the
	// file, but if that ever happens this test can be adjusted then.
	BOOST_REQUIRE_GT(fat3->iOffset, off3);

	file1->truncate(this->content[0].length());
	file1->seekp(0, stream::start);
	file1->write(this->content[0]);
	file1->flush();

	// Make sure the first file hasn't moved
	BOOST_REQUIRE_EQUAL(fat1->iOffset, off1);

	// Make sure the third file has moved again.  Same caveat as above.
	BOOST_REQUIRE_GT(fat3->iOffset, off3);

	file3->truncate(this->content[2].length());
	file3->seekp(0, stream::start);
	file3->write(this->content[2]);
	//file3->write(CONTENT3, sizeof(CONTENT3) - 1);
	file3->flush();

	this->checkData(&test_archive::insert_end,
		"Error manipulating zero-length files"
	);
}
