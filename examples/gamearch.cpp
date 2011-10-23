/**
 * @file   gamearch.cpp
 * @brief  Command-line interface to libgamearchive.
 *
 * Copyright (C) 2010-2011 Adam Nielsen <malvineous@shikadi.net>
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

#define BOOST_FILESYSTEM_VERSION 3
#include <boost/algorithm/string.hpp> // for case-insensitive string compare
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/bind.hpp>
#include <camoto/stream_file.hpp>
#include <camoto/util.hpp>
#include <camoto/gamearchive.hpp>

namespace po = boost::program_options;
namespace fs = boost::filesystem;
namespace ga = camoto::gamearchive;
namespace stream = camoto::stream;

#define PROGNAME "gamearch"

/*** Return values ***/
/// All is good
#define RET_OK                 0
/// Bad arguments (missing/invalid parameters)
#define RET_BADARGS            1
/// Major error (couldn't open archive file, etc.)
#define RET_SHOWSTOPPER        2
/// More info needed (-t auto didn't work, specify a type)
#define RET_BE_MORE_SPECIFIC   3
/// One or more files failed, probably user error (file not found, etc.)
#define RET_NONCRITICAL_FAILURE 4
/// Some files failed, but not in a common way (cut off write, disk full, etc.)
#define RET_UNCOMMON_FAILURE   5

/// Return value that will be used
int iRet = RET_OK;

/// Use any decompression filters? (unset with -u option)
bool bUseFilters = true;

/// Handle to the libgamearchive entry interface
ga::ManagerPtr pManager;

// Split a string in two at a delimiter, e.g. "one=two" becomes "one" and "two"
// and true is returned.  If there is no delimiter both output strings will be
// the same as the input string and false will be returned.
//
// If delim == '=' then:
// in        -> ret, out1, out2
// "one=two" -> true, "one", "two"
// "one=two=three" -> true, "one=two", "three"
// "four" -> false, "four", "four"
// If delim == '@' then:
// "one@two" -> true, "one", "two"
// "@1=myfile@@4"
// "test.txt@here.txt"
// "@2=test.txt"
// "e1m1.mid=mysong.mid:@4"
// "e1m1.mid=mysong.mid:e1m2.mid"
bool split(const std::string& in, char delim, std::string *out1, std::string *out2)
{
	std::string::size_type iEqualPos = in.find_last_of(delim);
	*out1 = in.substr(0, iEqualPos);
	// Does the destination have a different filename?
	bool bAltDest = iEqualPos != std::string::npos;
	*out2 = bAltDest ? in.substr(iEqualPos + 1) : *out1;
	return bAltDest;
}

// strInput is a filename that has come out of an archive, and we want to
// create the file on the local filesystem.  Escape any potentially hostile
// characters (possibly included slashes which might put files in different
// directories - TODO: unless -d or something has been specified)
void sanitisePath(std::string& strInput)
{
	// TODO: Check strLocalFile, replace backslashes, make any intermediate
	// directories
	for (std::string::iterator i = strInput.begin(); i != strInput.end(); i++) {
		switch (*i) {
			case '/':
#ifdef __WIN32
			case '\\':
			case ':':
#endif
				*i = '_';
				break;
		}
	}
	return;
}

/// Callback function to set expanded/native file size.
void setNativeSize(camoto::gamearchive::ArchivePtr arch,
	camoto::gamearchive::Archive::EntryPtr id, stream::len newSize)
	throw (std::ios::failure)
{
	arch->resize(id, newSize, id->iPrefilteredSize);
	return;
}

/// Apply the correct filter to the stream.
/**
 * If the given entry pointer has a filter attached, apply it to the given
 * stream pointer.
 *
 * @note This function will always apply the filter, don't call it if the user
 *   has given the -u option to bypass filtering.
 *
 * @param ppStream
 *   Pointer to the stream.  On return may point to a different stream.
 *
 * @param id
 *   EntryPtr for the stream.
 */
void applyFilter(camoto::stream::inout_sptr *ppStream, ga::ArchivePtr arch,
	ga::Archive::EntryPtr id)
	throw (stream::error)
{
	if (!id->filter.empty()) {
		// The file needs to be filtered first
		ga::FilterTypePtr pFilterType(::pManager->getFilterTypeByCode(id->filter));
		if (!pFilterType) {
			throw stream::error(createString(
				"could not find filter \"" << id->filter << "\""
			));
		}
		stream::fn_truncate fn_resize = boost::bind<void>(setNativeSize, arch, id, _1);
		*ppStream = pFilterType->apply(*ppStream, fn_resize);
	}
	return;
}

// Insert a file at the given location.  Shared by --insert and --add.
bool insertFile(ga::ArchivePtr pArchive, const std::string& strLocalFile,
	const std::string& strArchFile, const ga::Archive::EntryPtr idBeforeThis,
	const std::string& type, int attr, stream::len lenPrefiltered)
{
	// Open the file
	stream::file_sptr fsIn(new stream::file());
	fsIn->open(strLocalFile);
	stream::len lenSource = fsIn->size();

	fsIn->seekg(0, stream::start);

	// Make sure either filters are active, or we've got a nonzero prefilter
	// length (but it's ok to have a zero prefilter length if the file is empty)
	assert(bUseFilters || (lenSource == 0) || (lenPrefiltered != 0));

	// Create a new entry in the archive large enough to hold the file
	ga::Archive::EntryPtr id = pArchive->insert(idBeforeThis, strArchFile,
		lenSource, type, attr);

	// Open the new (empty) file in the archive
	camoto::stream::inout_sptr psNew(pArchive->open(id));
	if (bUseFilters) applyFilter(&psNew, pArchive, id);

	// Copy all the data from the file on disk into the archive file.
	try {
		stream::copy(psNew, fsIn);
		psNew->flush();
	} catch (std::ios_base::failure& e) {
		std::cout << " [failed; " << e.what() << "]";
		return false;
	}

	if (!bUseFilters) {
		// Since filters were skipped we will pretend we applied the filter and
		// we got more source data than we really did, so the next check works.
		lenSource = lenPrefiltered;
	}

	// If the data that went in was a different length to what we expected it
	// must have been compressed so update the file size (keeping the original
	// size as the 'uncompressed length' field.)
	boost::iostreams::stream_offset lenActual = psNew->tellp();
	if (lenActual != lenSource) {
		pArchive->resize(id, lenActual, lenSource);
	}

	return true;
}

/// List the files in the archive and any subfolders.
/**
 * This function is recursive and will call itself to list files in any
 * subfolders found.
 */
void listFiles(const std::string& idPrefix, const std::string& path, ga::ArchivePtr pArchive, bool bScript)
{
	ga::Archive::VC_ENTRYPTR vcEntries = pArchive->getFileList();

	std::string prefix = idPrefix;
	if (!idPrefix.empty()) prefix.append(".");

	int j = 0;
	for (ga::Archive::VC_ENTRYPTR::const_iterator i =
		vcEntries.begin(); i != vcEntries.end(); i++, j++)
	{
		int len = path.length() + (*i)->strName.length();
		if ((*i)->fAttr & ga::EA_FOLDER) {
			// This is a folder, not a file
			if (bScript) {
				std::cout << "index=" << prefix << j << ";path=" << path
					<< ';' << (*i)->getContent() << std::endl;
			} else {
				std::cout << "@" << prefix << j << "\t" << path << (*i)->strName << '/';
				len++; // because of trailing slash we just added
				if (len < 25) std::cout << std::string(25 - len, ' ');
				std::cout << "[dir";
				if ((*i)->fAttr & ga::EA_HIDDEN) std::cout << "; hidden";
				if ((*i)->fAttr & ga::EA_COMPRESSED) std::cout << "; compressed";
				if ((*i)->fAttr & ga::EA_ENCRYPTED) std::cout << "; encrypted";
				std::cout << "]\n";
			}
			ga::ArchivePtr subArch = pArchive->openFolder(*i);
			std::ostringstream ss(prefix);
			ss << j;
			std::string newPath(path);
			newPath.append((*i)->strName);
			newPath.append("/");
			listFiles(ss.str(), newPath, subArch, bScript);
		} else {
			if (bScript) {
				std::cout << "index=" << prefix << j << ";path=" << path
					<< ';' << (*i)->getContent() << std::endl;
			} else {
				std::cout << "@" << prefix << j << "\t" << path << (*i)->strName;
				// Pad the filename out to 25 chars if it's short enough
				if (len < 25) std::cout << std::string(25 - len, ' ');

				std::cout << "[";

				// Display the "MIME" type if there is one
				if (!(*i)->type.empty()) std::cout << (*i)->type << "; ";

				/// Display any attributes
				if ((*i)->fAttr & ga::EA_EMPTY) std::cout << "empty slot; ";
				if ((*i)->fAttr & ga::EA_HIDDEN) std::cout << "hidden; ";
				if ((*i)->fAttr & ga::EA_COMPRESSED) std::cout << "compressed; ";
				if ((*i)->fAttr & ga::EA_ENCRYPTED) std::cout << "encrypted; ";

				// Display file size
				std::cout << (*i)->iSize << " bytes]\n";
			}
		}
	}
	return;
}

/// Extract all the files in the archive.
/**
 * Calls itself recursively to extract any subfolders as well.
 */
void extractAll(ga::ArchivePtr pArchive, bool bScript)
{
	ga::Archive::VC_ENTRYPTR vcEntries = pArchive->getFileList();
	for (ga::Archive::VC_ENTRYPTR::const_iterator i =
		vcEntries.begin(); i != vcEntries.end(); i++)
	{
		// Get the name of the file we're extracting from the archive.
		std::string strLocalFile = (*i)->strName;
		sanitisePath(strLocalFile);
		if (strLocalFile.empty()) {
			// This file has no filename (probably the archive format doesn't
			// support filenames) so we have to make one up.
			std::ostringstream ss;
			ss << "@" << (i - vcEntries.begin());
			strLocalFile = ss.str();
		}

		if ((*i)->fAttr & ga::EA_FOLDER) {
			// Tell the user what's going on
			if (bScript) {
				std::cout << "mkdir=" << strLocalFile;
			} else {
				std::cout << "      mkdir: " << strLocalFile << '/' << std::flush;
			}
			fs::path old;
			try {
				// If the folder exists, add .1 .2 .3 etc. onto the end until an
				// unused name is found.  This allows extracting folders with the
				// same name, without their files ending up lumped together in
				// the same real on-disk folder.
				if (fs::exists(strLocalFile)) {
					std::ostringstream ss;
					int j = 1;
					do {
						ss.str(std::string()); // empty the stringstream
						ss << strLocalFile << '.' << j;
						j++;
					} while (fs::exists(ss.str()));
					strLocalFile = ss.str();
					if (!bScript) {
						std::cout << " (as " << strLocalFile << ")";
					}
				}

				fs::create_directory(strLocalFile);
				if (bScript) std::cout << ";created=" << strLocalFile;
				old = fs::current_path();
				fs::current_path(strLocalFile);
				if (bScript) std::cout << ";status=ok";

				std::cout << std::endl;
			} catch (const fs::filesystem_error& e) {
				if (bScript) {
					std::cout << ";status=fail";
				} else {
					std::cout << " [failed; skipping folder]";
				}
				::iRet = RET_NONCRITICAL_FAILURE; // one or more files failed
				std::cout << std::endl;
				continue;
			}
			ga::ArchivePtr subArch = pArchive->openFolder(*i);
			extractAll(subArch, bScript);
			fs::current_path(old);
		} else {
			// Tell the user what's going on
			if (bScript) {
				std::cout << "extracting=" << strLocalFile;
			} else {
				std::cout << " extracting: " << strLocalFile;
			}

			// Open on disk
			try {
				camoto::stream::inout_sptr pfsIn(pArchive->open(*i));
				if (bUseFilters) applyFilter(&pfsIn, pArchive, *i);
				stream::output_file_sptr fsOut(new stream::output_file());

				// If the file exists, add .1 .2 .3 etc. onto the end until an
				// unused name is found.  This allows extracting files with the
				// same name, without them getting overwritten.
				if (fs::exists(strLocalFile)) {
					std::ostringstream ss;
					int j = 1;
					do {
						ss.str(std::string()); // empty the stringstream
						ss << strLocalFile << '.' << j;
						j++;
					} while (fs::exists(ss.str()));
					strLocalFile = ss.str();
					if (!bScript) {
						std::cout << " (into " << strLocalFile << ")";
					}
				}
				std::cout << std::flush;

				if (bScript) std::cout << ";wrote=" << strLocalFile;
				fsOut->create(strLocalFile);

				// Copy the data from the in-archive stream to the on-disk stream
				stream::copy(fsOut, pfsIn);

				if (bScript) std::cout << ";status=ok";
			} catch (...) {
				if (bScript) {
					std::cout << ";status=fail";
				} else {
					std::cout << " [error]";
				}
				::iRet = RET_NONCRITICAL_FAILURE; // one or more files failed
			}
			std::cout << std::endl;
		}
	}
	return;
}

int main(int iArgC, char *cArgV[])
{
	// Set a better exception handler
	std::set_terminate( __gnu_cxx::__verbose_terminate_handler );

	// Disable stdin/printf/etc. sync for a speed boost
	std::ios_base::sync_with_stdio(false);

	// Declare the supported options.
	po::options_description poActions("Actions");
	poActions.add_options()
		("list,l",
			"list files in the archive")

		("extract-all,X",
			"extract all files in the archive")

		("extract,x", po::value<std::string>(),
			"extract a specific file")

		("add,a", po::value<std::string>(),
			"add a file at the end of the archive")

		("insert,i", po::value<std::string>(),
			"add a file at a specific point in the archive")

		("overwrite,o", po::value<std::string>(),
			"replace a file in the archive with new data")

		("rename,r", po::value<std::string>(),
			"rename a file inside the archive")

		("delete,d", po::value<std::string>(),
			"remove a file from the archive")

		("uncompressed-size,z", po::value<int>(),
			"[with -u only] specify the uncompressed size to use with -i")
	;

	po::options_description poOptions("Options");
	poOptions.add_options()
		("type,t", po::value<std::string>(),
			"specify the archive type (default is autodetect)")
		("list-types",
			"list available formats that can be passed to --type")
		("filetype,y", po::value<std::string>(),
			"specify the file type when inserting (default is generic file)")
		("attribute,b", po::value<std::string>(),
			"specify the file attributes when inserting (optional)")
		("unfiltered,u",
			"do not filter files (no encrypt/decrypt/compress/decompress)")
		("script,s",
			"format output suitable for script parsing")
		("force,f",
			"force open even if the archive is not in the given format")
	;

	po::options_description poHidden("Hidden parameters");
	poHidden.add_options()
		("archive", "archive file to manipulate")
		("help", "produce help message")
	;

	po::options_description poVisible("");
	poVisible.add(poActions).add(poOptions);

	po::options_description poComplete("Parameters");
	poComplete.add(poActions).add(poOptions).add(poHidden);
	po::variables_map mpArgs;

	std::string strFilename;
	std::string strType;

	::pManager = ga::getManager();

	bool bScript = false; // show output suitable for script parsing?
	bool bForceOpen = false; // open anyway even if archive not in given format?
	try {
		po::parsed_options pa = po::parse_command_line(iArgC, cArgV, poComplete);

		// Parse the global command line options
		for (std::vector<po::option>::iterator i = pa.options.begin(); i != pa.options.end(); i++) {
			if (i->string_key.empty()) {
				// If we've already got an archive filename, complain that a second one
				// was given (probably a typo.)
				if (!strFilename.empty()) {
					std::cerr << "Error: unexpected extra parameter (multiple archive "
						"filenames given?!)" << std::endl;
					return RET_BADARGS;
				}
				assert(i->value.size() > 0);  // can't have no values with no name!
				strFilename = i->value[0];
			} else if (i->string_key.compare("help") == 0) {
				std::cout <<
					"Copyright (C) 2010-2011 Adam Nielsen <malvineous@shikadi.net>\n"
					"This program comes with ABSOLUTELY NO WARRANTY.  This is free software,\n"
					"and you are welcome to change and redistribute it under certain conditions;\n"
					"see <http://www.gnu.org/licenses/> for details.\n"
					"\n"
					"Utility to manipulate archive files used by games to store data files.\n"
					"Build date " __DATE__ " " __TIME__ << "\n"
					"\n"
					"Usage: gamearch <archive> <action> [action...]\n" << poVisible << "\n"
					<< std::endl;
				return RET_OK;
			} else if (
				(i->string_key.compare("list-types") == 0)
			) {
				ga::ArchiveTypePtr nextType;
				int i = 0;
				while ((nextType = pManager->getArchiveType(i++))) {
					std::string code = nextType->getArchiveCode();
					std::cout << code;
					int len = code.length();
					if (len < 20) std::cout << std::string(20-code.length(), ' ');
					std::cout << ' ' << nextType->getFriendlyName() << '\n';
				}
				return RET_OK;
			} else if (
				(i->string_key.compare("t") == 0) ||
				(i->string_key.compare("type") == 0)
			) {
				if (i->value.size() == 0) {
					std::cerr << PROGNAME ": --type (-t) requires a parameter."
						<< std::endl;
					return RET_BADARGS;
				}
				strType = i->value[0];
			} else if (
				(i->string_key.compare("s") == 0) ||
				(i->string_key.compare("script") == 0)
			) {
				bScript = true;
			} else if (
				(i->string_key.compare("f") == 0) ||
				(i->string_key.compare("force") == 0)
			) {
				bForceOpen = true;
			} else if (
				(i->string_key.compare("u") == 0) ||
				(i->string_key.compare("unfiltered") == 0)
			) {
				bUseFilters = false;
			}
		}

		if (strFilename.empty()) {
			std::cerr << "Error: no game archive filename given" << std::endl;
			return RET_BADARGS;
		}
		std::cout << "Opening " << strFilename << " as type "
			<< (strType.empty() ? "<autodetect>" : strType) << std::endl;

		stream::file_sptr psArchive(new stream::file());
		try {
			psArchive->open(strFilename);
		} catch (const stream::open_error& e) {
			std::cerr << "Error opening supplemental file " << strFilename
				<< ": " << e.what() << std::endl;
			return RET_SHOWSTOPPER;
		}

		// Get the format handler for this file format
		ga::ArchiveTypePtr pArchType;
		if (strType.empty()) {
			// Need to autodetect the file format.
			ga::ArchiveTypePtr pTestType;
			int i = 0;
			while ((pTestType = pManager->getArchiveType(i++))) {
				ga::ArchiveType::Certainty cert = pTestType->isInstance(psArchive);
				switch (cert) {
					case ga::ArchiveType::DefinitelyNo:
						// Don't print anything (TODO: Maybe unless verbose?)
						break;
					case ga::ArchiveType::Unsure:
						std::cout << "File could be a " << pTestType->getFriendlyName()
							<< " [" << pTestType->getArchiveCode() << "]" << std::endl;
						// If we haven't found a match already, use this one
						if (!pArchType) pArchType = pTestType;
						break;
					case ga::ArchiveType::PossiblyYes:
						std::cout << "File is likely to be a " << pTestType->getFriendlyName()
							<< " [" << pTestType->getArchiveCode() << "]" << std::endl;
						// Take this one as it's better than an uncertain match
						pArchType = pTestType;
						break;
					case ga::ArchiveType::DefinitelyYes:
						std::cout << "File is definitely a " << pTestType->getFriendlyName()
							<< " [" << pTestType->getArchiveCode() << "]" << std::endl;
						pArchType = pTestType;
						// Don't bother checking any other formats if we got a 100% match
						goto finishTesting;
				}
				if (cert != ga::ArchiveType::DefinitelyNo) {
					// We got a possible match, see if it requires any suppdata
					camoto::SuppFilenames suppList = pTestType->getRequiredSupps(strFilename);
					if (suppList.size() > 0) {
						// It has suppdata, see if it's present
						std::cout << "  * This format requires supplemental files..." << std::endl;
						bool bSuppOK = true;
						for (camoto::SuppFilenames::iterator i = suppList.begin(); i != suppList.end(); i++) {
							try {
								stream::file_sptr suppStream(new stream::file());
								suppStream->open(i->second);
							} catch (const stream::open_error& e) {
								bSuppOK = false;
								std::cout << "  * Could not find/open " << i->second
									<< ", archive is probably not "
									<< pTestType->getArchiveCode() << std::endl;
								break;
							}
						}
						if (bSuppOK) {
							// All supp files opened ok
							std::cout << "  * All supp files present, archive is likely "
								<< pTestType->getArchiveCode() << std::endl;
							// Set this as the most likely format
							pArchType = pTestType;
						}
					}
				}
			}
finishTesting:
			if (!pArchType) {
				std::cerr << "Unable to automatically determine the file type.  Use "
					"the --type option to manually specify the file format." << std::endl;
				return RET_BE_MORE_SPECIFIC;
			}
		} else {
			ga::ArchiveTypePtr pTestType(pManager->getArchiveTypeByCode(strType));
			if (!pTestType) {
				std::cerr << "Unknown file type given to -t/--type: " << strType
					<< std::endl;
				return RET_BADARGS;
			}
			pArchType = pTestType;
		}

		assert(pArchType != NULL);

		// Check to see if the file is actually in this format
		if (!pArchType->isInstance(psArchive)) {
			if (bForceOpen) {
				std::cerr << "Warning: " << strFilename << " is not a "
					<< pArchType->getFriendlyName() << ", open forced." << std::endl;
			} else {
				std::cerr << "Invalid format: " << strFilename << " is not a "
					<< pArchType->getFriendlyName() << "\n"
					<< "Use the -f option to try anyway." << std::endl;
				return RET_BE_MORE_SPECIFIC;
			}
		}

		// See if the format requires any supplemental files
		camoto::SuppFilenames suppList = pArchType->getRequiredSupps(strFilename);
		camoto::SuppData suppData;
		if (suppList.size() > 0) {
			for (camoto::SuppFilenames::iterator i = suppList.begin(); i != suppList.end(); i++) {
				try {
					stream::file_sptr suppStream(new stream::file());
					std::cout << "Opening supplemental file " << i->second << std::endl;
					suppStream->open(i->second.c_str());
					suppData[i->first] = suppStream;
				} catch (const stream::open_error& e) {
					std::cerr << "Error opening supplemental file " << i->second.c_str()
						<< ": " << e.what() << std::endl;
					return RET_SHOWSTOPPER;
				}
			}
		}

		// Open the archive file
		ga::ArchivePtr pArchive;
		try {
			pArchive = pArchType->open(psArchive, suppData);
			assert(pArchive);
		} catch (stream::error& e) {
			std::cerr << "Error opening archive file: " << e.what() << std::endl;
			return RET_SHOWSTOPPER;
		}

		// File type of inserted files defaults to empty, which means 'generic file'
		std::string strLastFiletype;

		// Last attribute value set with -b
		int iLastAttr = 0;

		// Last value set with -z
		stream::len lenPrefiltered = 0;

		// Run through the actions on the command line
		for (std::vector<po::option>::iterator i = pa.options.begin(); i != pa.options.end(); i++) {
			if (i->string_key.compare("list") == 0) {
				listFiles(std::string(), std::string(), pArchive, bScript);

			} else if (i->string_key.compare("extract-all") == 0) {
				extractAll(pArchive, bScript);

			} else if (i->string_key.compare("extract") == 0) {
				std::string strArchFile, strLocalFile;
				bool bAltDest = split(i->value[0], '=', &strArchFile, &strLocalFile);
				if (!bAltDest) sanitisePath(strLocalFile);

				std::cout << " extracting: " << strArchFile;
				if (strArchFile.compare(strLocalFile)) std::cout << " (into " << strLocalFile << ")";
				std::cout << std::flush;

				try {
					// Find the file
					ga::ArchivePtr destArch = pArchive;
					ga::Archive::EntryPtr id = findFile(destArch, strArchFile);
					if (!id) {
						std::cout << " [failed; file not found]";
						iRet = RET_NONCRITICAL_FAILURE; // one or more files failed
					} else {
						// Found it, open on disk
						camoto::stream::inout_sptr pfsIn(destArch->open(id));
						if (bUseFilters) applyFilter(&pfsIn, destArch, id);
						stream::output_file_sptr fsOut(new stream::output_file());
						try {
							fsOut->create(strLocalFile);
							try {
								stream::copy(fsOut, pfsIn);
							} catch (const stream::error& e) {
								std::cout << " [failed; read/write error: " << e.what() << "]";
								iRet = RET_UNCOMMON_FAILURE; // some files failed, but not in a usual way
							}
						} catch (const stream::error& e) {
							std::cout << " [failed; unable to create output file]";
							iRet = RET_UNCOMMON_FAILURE; // some files failed, but not in a usual way
						}
					}
				} catch (const stream::error& e) {
					std::cout << " [failed; " << e.what() << "]";
					iRet = RET_UNCOMMON_FAILURE; // some files failed, but not in a usual way
				}
				std::cout << std::endl;

			} else if (i->string_key.compare("delete") == 0) {
				std::string& strArchFile = i->value[0];
				std::cout << "   deleting: " << strArchFile << std::flush;

				try {
					ga::ArchivePtr destArch = pArchive;
					ga::Archive::EntryPtr id = findFile(destArch, strArchFile);
					if (!id) {
						std::cout << " [failed; file not found]";
						iRet = RET_NONCRITICAL_FAILURE; // one or more files failed
					} else {
						destArch->remove(id);
					}
				} catch (std::ios_base::failure& e) {
					std::cout << " [failed; " << e.what() << "]";
					iRet = RET_UNCOMMON_FAILURE; // some files failed, but not in a usual way
				}
				std::cout << std::endl;

			} else if (i->string_key.compare("insert") == 0) {
				std::string strSource, strInsertBefore;
				if (!split(i->value[0], ':', &strSource, &strInsertBefore)) {
					std::cerr << PROGNAME ": -i/--insert requires a file to insert "
						"before (parameter should end with \":beforeme.xyz\")\n"
						"Or use --add instead." << std::endl;
					return RET_BADARGS;
				}

				std::string strArchFile, strLocalFile;
				bool bAltDest = split(strSource, '=', &strArchFile, &strLocalFile);

				std::cout << "  inserting: " << strArchFile;
				if (!strLastFiletype.empty()) std::cout << " as type " << strLastFiletype;
				std::cout << " (before "
					<< strInsertBefore;
				if (bAltDest) std::cout << ", from " << strLocalFile;
				std::cout << ")";
				if (lenPrefiltered != 0) std::cout << ", with uncompressed size " << lenPrefiltered;
				std::cout << std::flush;

				// Try to find strInsertBefore
				ga::ArchivePtr destArch = pArchive;
				ga::Archive::EntryPtr idBeforeThis = findFile(destArch, strInsertBefore);
				if (!idBeforeThis) {
					std::cout << " [failed; could not find " << strInsertBefore << "]";
					iRet = RET_NONCRITICAL_FAILURE; // one or more files failed
					continue;
				}

				try {
					insertFile(destArch, strLocalFile, strArchFile, idBeforeThis,
						strLastFiletype, iLastAttr, lenPrefiltered);
				} catch (const stream::error& e) {
					std::cout << " [failed; " << e.what() << "]";
					iRet = RET_UNCOMMON_FAILURE; // some files failed, but not in a usual way
				}

				std::cout << std::endl;

			// Remember --filetype/-y
			} else if (i->string_key.compare("filetype") == 0) {
			//} else if (i->string_key.compare("y") == 0) {
				strLastFiletype = i->value[0];

			// Remember --attributes/-b
			} else if (i->string_key.compare("attribute") == 0) {
			//} else if (i->string_key.compare("b") == 0) {
				std::string nextAttr = i->value[0];
				bool disable = (nextAttr[0] == '-');
				if (disable) nextAttr = nextAttr.substr(1);

				int next;
				if      (nextAttr.compare("empty")      == 0) next = ga::EA_EMPTY;
				else if (nextAttr.compare("hidden")     == 0) next = ga::EA_HIDDEN;
				else if (nextAttr.compare("compressed") == 0) next = ga::EA_COMPRESSED;
				else if (nextAttr.compare("encrypted")  == 0) next = ga::EA_ENCRYPTED;
				else {
					std::cerr << "Unknown attribute " << nextAttr
						<< ", valid values are: empty hidden compressed encrypted"
						<< std::endl;
					iRet = RET_UNCOMMON_FAILURE;
					next = 0;
				}
				if (next != 0) {
					int allowed = pArchive->getSupportedAttributes();
					if (allowed & next) {
						if (disable) iLastAttr &= ~next;
						else iLastAttr |= next;
					} else {
						std::cerr << "Warning: Attribute unsupported by archive format, "
							"ignoring: " << nextAttr << std::endl;
					}
				}

			// Remember --uncompressed-size/-z
			} else if (i->string_key.compare("uncompressed-size") == 0) {
			//} else if (i->string_key.compare("z") == 0) {
				if (bUseFilters) {
					std::cerr << PROGNAME ": -z/--uncompressed-size only needs to be "
						"specified when it can't be determined automatically (i.e. when "
						"-u/--unfiltered is in use.)" << std::endl;
					return RET_BADARGS;
				}
				lenPrefiltered = strtoul(i->value[0].c_str(), NULL, 0);

			// Ignore --type/-t
			} else if (i->string_key.compare("type") == 0) {
			} else if (i->string_key.compare("t") == 0) {
			// Ignore --script/-s
			} else if (i->string_key.compare("script") == 0) {
			} else if (i->string_key.compare("s") == 0) {
			// Ignore --force/-f
			} else if (i->string_key.compare("force") == 0) {
			} else if (i->string_key.compare("f") == 0) {

			} else if ((!i->string_key.empty()) && (i->value.size() > 0)) {
				// None of the above (single param) options matched, so it's probably
				// an option with up to two filenames (with an equal-sign as a
				// separator.)  It could also be the --type option, which we'll ignore.
				std::string& strParam = i->value[0];
				std::string strArchFile, strLocalFile;
				bool bAltDest = split(strParam, '=', &strArchFile, &strLocalFile);

				if (i->string_key.compare("add") == 0) {
					std::cout << "     adding: " << strArchFile;
					if (!strLastFiletype.empty()) std::cout << " as type " << strLastFiletype;
					if (bAltDest) std::cout << " (from " << strLocalFile << ")";
					if (lenPrefiltered != 0) std::cout << ", with uncompressed size set to " << lenPrefiltered;
					std::cout << std::endl;

					try {
						insertFile(pArchive, strLocalFile, strArchFile,
							ga::Archive::EntryPtr(), strLastFiletype, iLastAttr,
							lenPrefiltered);
					} catch (const stream::error& e) {
						std::cout << " [failed; " << e.what() << "]";
						iRet = RET_UNCOMMON_FAILURE; // some files failed, but not in a usual way
					}

				} else if (i->string_key.compare("rename") == 0) {
					if ((!bAltDest) || (boost::equals(strArchFile, strLocalFile))) {
						std::cout << "ignoring attempt to rename " << strArchFile
							<< " into the same name" << std::endl;
					} else {
						std::cout << "   renaming: " << strArchFile << " to "
							<< strLocalFile << std::flush;

						try {
							ga::ArchivePtr destArch = pArchive;
							ga::Archive::EntryPtr id = findFile(destArch, strArchFile);
							if (!id) {
								std::cout << " [failed; file not found inside archive]";
								iRet = RET_NONCRITICAL_FAILURE; // one or more files failed
							} else {
								destArch->rename(id, strLocalFile);
							}
						} catch (const stream::error& e) {
							std::cout << " [failed; " << e.what() << "]";
							iRet = RET_UNCOMMON_FAILURE; // some files failed, but not in a common way
						}
						std::cout << std::endl;
					}

				} else if (i->string_key.compare("overwrite") == 0) {
					std::cout << "overwriting: " << strArchFile;
					if (bAltDest) std::cout << " (from " << strLocalFile << ")";
					if (lenPrefiltered != 0) std::cout << ", with uncompressed size set to " << lenPrefiltered;
					std::cout << std::flush;

					try {
						// Find the file
						ga::ArchivePtr destArch = pArchive;
						ga::Archive::EntryPtr id = findFile(destArch, strArchFile);
						if (!id) {
							std::cout << " [failed; file not found inside archive]";
							iRet = RET_NONCRITICAL_FAILURE; // one or more files failed
						} else {
							// Found it, open replacement file
							stream::input_file_sptr sSrc(new stream::input_file());
							sSrc->open(strLocalFile);
							stream::len lenSource = sSrc->size();
							if (lenSource != id->iSize) {
								pArchive->resize(id, lenSource, lenSource);
							}
							// Now the file has been resized it's safe to open (if we opened
							// it before the resize it'd be stuck at the old size)
							camoto::stream::inout_sptr psDest(destArch->open(id));
							if (bUseFilters) applyFilter(&psDest, destArch, id);
							stream::copy(psDest, sSrc);
							psDest->flush();

							if (!bUseFilters) {
								// Since filters were skipped we will pretend we applied the
								// filter and we got more source data than we really did, so
								// the next check works.
								lenSource = lenPrefiltered;
							}

							// If the data that went in was a different length to what we
							// expected it must have been compressed so update the file
							// size (keeping the original size as the 'uncompressed length'
							// field.)
							stream::len lenActual = psDest->tellp();
							if (lenActual != lenSource) {
								pArchive->resize(id, lenActual, lenSource);
							}
						}
					} catch (const stream::open_error& e) {
						std::cout << " [failed; unable to open replacement file]";
						iRet = RET_NONCRITICAL_FAILURE; // one or more files failed
					} catch (const stream::error& e) {
						std::cout << " [failed; " << e.what() << "]";
						iRet = RET_UNCOMMON_FAILURE; // some files failed, but not in a common way
					}
					std::cout << std::endl;
				}
				// else it's the archive filename, but we already have that
			}
		} // for (all command line elements)
		pArchive->flush();
	} catch (const po::unknown_option& e) {
		std::cerr << PROGNAME ": " << e.what()
			<< ".  Use --help for help." << std::endl;
		return RET_BADARGS;
	} catch (const po::invalid_command_line_syntax& e) {
		std::cerr << PROGNAME ": " << e.what()
			<< ".  Use --help for help." << std::endl;
		return RET_BADARGS;
	}

	return iRet;
}
