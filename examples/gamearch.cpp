/*
 * gamearch.cpp - command-line interface to libgamearchive.
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

#include <boost/algorithm/string.hpp> // for case-insensitive string compare
#include <boost/program_options.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/bind.hpp>
#include <camoto/gamearchive.hpp>
#include <iostream>
#include <fstream>

namespace po = boost::program_options;
namespace ga = camoto::gamearchive;

#define PROGNAME "gamearch"

//#define BUFFER_SIZE  2048  // Buffer size when copying files in/out of arch
//#define BUFFER_SIZE  4096  // Buffer size when copying files in/out of arch

/*** Return values ***/
// All is good
#define RET_OK                 0
// Bad arguments (missing/invalid parameters)
#define RET_BADARGS            1
// Major error (couldn't open archive file, etc.)
#define RET_SHOWSTOPPER        2
// More info needed (-t auto didn't work, specify a type)
#define RET_BE_MORE_SPECIFIC   3
// One or more files failed, probably user error (file not found, etc.)
#define RET_NONCRITICAL_FAILURE 4
// Some files failed, but not in a common way (cut off write, disk full, etc.)
#define RET_UNCOMMON_FAILURE   5

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
std::string sanitisePath(const std::string& strInput)
{
	// TODO: Check strLocalFile, replace backslashes, make any intermediate
	// directories
	return strInput;
}

// Find the given file, or if it starts with an '@', the file at that index.
ga::Archive::EntryPtr findFile(const ga::ArchivePtr& archive, const std::string& filename)
{
	ga::Archive::EntryPtr id = archive->find(filename);
	if (archive->isValid(id)) return id;

	// The file doesn't exist, so see if it's an index.
	if ((filename[0] == '@') && (filename.length() > 1)) {
		char *endptr;
		// strtoul allows arbitrary whitespace at the start, so if ever there is
		// a file called "@5" which gets extracted instead of the fifth file,
		// giving -x "@ 5" should do the trick.
		unsigned long index = strtoul(&(filename.c_str()[1]), &endptr, 10);
		if (*endptr == '\0') {
			// The number was entirely valid (no junk at end)
			ga::Archive::VC_ENTRYPTR files = archive->getFileList();
			if (index < files.size()) return files[index];
			throw std::ios::failure("index too large");
		}
	}
	return ga::Archive::EntryPtr(); // file not found
}

// Insert a file at the given location.  Shared by --insert and --add.
bool insertFile(ga::Archive *pArchive, const std::string& strLocalFile,
	const std::string& strArchFile, const camoto::gamearchive::Archive::EntryPtr& idBeforeThis)
{
	// Open the file
	std::ifstream fsIn(strLocalFile.c_str(), std::ios::binary);
	// Figure out how big it is
	fsIn.seekg(0, std::ios::end);
	boost::iostreams::stream_offset iSize = fsIn.tellg();
	fsIn.seekg(0, std::ios::beg);

	// Create a new entry in the archive large enough to hold the file
	camoto::gamearchive::Archive::EntryPtr id =
		pArchive->insert(idBeforeThis, strArchFile, iSize);

	// Open the new (empty) file in the archive
	boost::shared_ptr<std::iostream> psNew(pArchive->open(id));

	// Copy all the data from the file on disk into the archive file.
	try {
		boost::iostreams::copy(fsIn, *psNew);
	} catch (std::ios_base::failure& e) {
		std::cout << " [failed; " << e.what() << "]";
		//iRet = RET_UNCOMMON_FAILURE; // some files failed, but not in a usual way
		return false;
	}
	return true;
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
	;

	po::options_description poOptions("Options");
	poOptions.add_options()
		("type,t", po::value<std::string>(),
			"specify the archive type (default is autodetect)")
		("script,s",
			"format output suitable for script parsing")
		("force,f",
			"force open even if the archive is not in the given format")
		//("verbose", "display more detail")
	;

	po::options_description poHidden("Hidden parameters");
	poHidden.add_options()
		("archive", "archive file to manipulate")
		("help", "produce help message")
	;

	//po::positional_options_description poPositional;
	// The first argument without an option is the archive name
	//poPositional.add("archive", 1);

	po::options_description poVisible("");//po::options_description::m_default_line_length);
	poVisible.add(poActions).add(poOptions);

	po::options_description poComplete("Parameters");
	poComplete.add(poActions).add(poOptions).add(poHidden);
	po::variables_map mpArgs;

	std::string strFilename;
	std::string strType;

	bool bScript = false; // show output suitable for script parsing?
	bool bForceOpen = false; // open anyway even if archive not in given format?
	try {
		//po::store(po::parse_command_line(iArgC, cArgV, poComplete), mpArgs);
		po::parsed_options pa = po::parse_command_line(iArgC, cArgV, poComplete);
		//po::store(pa, mpArgs);
		//std::cout << "Malvineous' Game Editor." << std::endl;

		// Parse the global command line options
		for (std::vector<po::option>::iterator i = pa.options.begin(); i != pa.options.end(); i++) {
			if (i->string_key.empty()) {
				// If we've already got an archive filename, complain that a second one
				// was given (probably a typo.)
				if (!strFilename.empty()) {
					std::cerr << "Error: unexpected extra parameter (multiple archive "
						"filenames given?!)" << std::endl;
					return 1;
				}
				assert(i->value.size() > 0);  // can't have no values with no name!
				strFilename = i->value[0];
			} else if (i->string_key.compare("help") == 0) {
				std::cout <<
					"Copyright (C) 2010 Adam Nielsen <malvineous@shikadi.net>\n"
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
			}
		}

		if (strFilename.empty()) {
			std::cerr << "Error: no game archive filename given" << std::endl;
			return RET_BADARGS;
		}
		std::cout << "Opening " << strFilename << " as type "
			<< (strType.empty() ? "<autodetect>" : strType) << std::endl;

		boost::shared_ptr<std::fstream> psArchive(new std::fstream());
		psArchive->exceptions(std::ios::badbit | std::ios::failbit);
		try {
			psArchive->open(strFilename.c_str(), std::ios::in | std::ios::out | std::ios::binary);
		} catch (std::ios::failure& e) {
			std::cerr << "Error opening " << strFilename << std::endl;
			#ifdef DEBUG
				std::cerr << "e.what(): " << e.what() << std::endl;
			#endif
			return RET_SHOWSTOPPER;
		}

		// Get the format handler for this file format
		boost::shared_ptr<ga::Manager> pManager(ga::getManager());

		ga::ArchiveTypePtr pArchType;
		if (strType.empty()) {
			// Need to autodetect the file format.
			ga::ArchiveTypePtr pTestType;
			int i = 0;
			while ((pTestType = pManager->getArchiveType(i++))) {
				ga::E_CERTAINTY cert = pTestType->isInstance(psArchive);
				switch (cert) {
					case ga::EC_DEFINITELY_NO:
						// Don't print anything (TODO: Maybe unless verbose?)
						break;
					case ga::EC_UNSURE:
						std::cout << "File could be a " << pTestType->getFriendlyName()
							<< " [" << pTestType->getArchiveCode() << "]" << std::endl;
						// If we haven't found a match already, use this one
						if (!pArchType) pArchType = pTestType;
						break;
					case ga::EC_POSSIBLY_YES:
						std::cout << "File is likely to be a " << pTestType->getFriendlyName()
							<< " [" << pTestType->getArchiveCode() << "]" << std::endl;
						// Take this one as it's better than an uncertain match
						pArchType = pTestType;
						break;
					case ga::EC_DEFINITELY_YES:
						std::cout << "File is definitely a " << pTestType->getFriendlyName()
							<< " [" << pTestType->getArchiveCode() << "]" << std::endl;
						pArchType = pTestType;
						// Don't bother checking any other formats if we got a 100% match
						goto finishTesting;
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
				return 3;
			}
		}

		// See if the format requires any supplemental files
		ga::MP_SUPPLIST suppList = pArchType->getRequiredSupps(strFilename);
		ga::MP_SUPPDATA suppData;
		if (suppList.size() > 0) {
			for (ga::MP_SUPPLIST::iterator i = suppList.begin(); i != suppList.end(); i++) {
				try {
					boost::shared_ptr<std::fstream> suppStream(new std::fstream());
					suppStream->exceptions(std::ios::badbit | std::ios::failbit);
					std::cout << "Opening supplemental file " << i->second << std::endl;
					suppStream->open(i->second.c_str(), std::ios::in | std::ios::out | std::ios::binary);
					ga::SuppItem si;
					si.stream = suppStream;
					si.fnTruncate = boost::bind<void>(truncate, i->second.c_str(), _1);
					suppData[i->first] = si;
				} catch (std::ios::failure e) {
					std::cerr << "Error opening supplemental file " << i->second.c_str() << std::endl;
					#ifdef DEBUG
						std::cerr << "e.what(): " << e.what() << std::endl;
					#endif
					return RET_SHOWSTOPPER;
				}
			}
		}

		// Open the archive file
		boost::shared_ptr<ga::Archive> pArchive(pArchType->open(psArchive, suppData));
		assert(pArchive);
		pArchive->fnTruncate = boost::bind<void>(truncate, strFilename.c_str(), _1);

		int iRet = RET_OK;

		// Run through the actions on the command line
		for (std::vector<po::option>::iterator i = pa.options.begin(); i != pa.options.end(); i++) {
			if (i->string_key.compare("list") == 0) {
				camoto::gamearchive::Archive::VC_ENTRYPTR vcEntries = pArchive->getFileList();
				std::cout << "Found " << vcEntries.size() << " files" << std::endl;

				int j = 0;
				for (camoto::gamearchive::Archive::VC_ENTRYPTR::const_iterator i =
					vcEntries.begin(); i != vcEntries.end(); i++, j++)
				{
					if (bScript) {
						std::cout << "index=" << j << ';' << (*i)->getContent() << std::endl;
					} else {
						std::cout << j << ": " << (*i)->strName;
						int len = (*i)->strName.length();
						// Pad the filename out to 14 chars if it's short enough
						if (len < 14) std::cout << std::string(14 - len, ' ');

						std::cout << "\t[";

						// Display the "MIME" type if there is one
						if (!(*i)->type.empty()) std::cout << (*i)->type << "; ";

						// Display file size
						std::cout << (*i)->iSize << " bytes]\n";
					}
				}

			} else if (i->string_key.compare("extract-all") == 0) {
				camoto::gamearchive::Archive::VC_ENTRYPTR vcEntries = pArchive->getFileList();
				if (!bScript)
					std::cout << "Extracting " << vcEntries.size() << " files" << std::endl;

				for (camoto::gamearchive::Archive::VC_ENTRYPTR::const_iterator i =
					vcEntries.begin(); i != vcEntries.end(); i++)
				{
					// Get the name of the file we're extracting from the archive.
					std::string strArchFile = (*i)->strName;
					if (strArchFile.empty()) {
						// This file has no filename (probably the archive format doesn't
						// support filenames) so we have to make one up.
						std::ostringstream ss;
						ss << "@" << (i - vcEntries.begin());
						strArchFile = ss.str();
					}

					// Tell the user what's going on
					if (bScript) {
						std::cout << "extracted=" << strArchFile << ";status=";
					} else {
						std::cout << " extracting: " << strArchFile << std::flush;
					}

					// Open on disk
					try {
						boost::shared_ptr<std::iostream> pfsIn(pArchive->open(*i));
						std::ofstream fsOut;
						fsOut.exceptions(std::ios::badbit | std::ios::failbit | std::ios::eofbit);
						fsOut.open(strArchFile.c_str(), std::ios::trunc | std::ios::binary);

						// Copy the data from the in-archive stream to the on-disk stream
						boost::iostreams::copy(*pfsIn, fsOut);

						if (bScript) std::cout << "ok";
					} catch (...) {
						if (bScript) {
							std::cout << "fail";
						} else {
							std::cout << " [error]";
						}
						iRet = RET_NONCRITICAL_FAILURE; // one or more files failed
					}
					std::cout << std::endl;
				}

			} else if (i->string_key.compare("extract") == 0) {
				std::string strArchFile, strLocalFile;
				bool bAltDest = split(i->value[0], '=', &strArchFile, &strLocalFile);
				if (!bAltDest) strLocalFile = sanitisePath(strLocalFile);

				std::cout << " extracting: " << strArchFile;
				if (bAltDest) std::cout << " (into " << strLocalFile << ")";
				std::cout << std::flush;

				try {
					// Find the file
					ga::Archive::EntryPtr id = findFile(pArchive, strArchFile);
					if (!id) {
						std::cout << " [failed; file not found]";
						iRet = RET_NONCRITICAL_FAILURE; // one or more files failed
					} else {
						// Found it, open on disk
						boost::shared_ptr<std::iostream> pfsIn(pArchive->open(id));
						std::ofstream fsOut;
						fsOut.exceptions(std::ios::badbit | std::ios::failbit | std::ios::eofbit);
						fsOut.open(strLocalFile.c_str(), std::ios::trunc | std::ios::binary);
						boost::iostreams::copy(*pfsIn, fsOut);
					}
				} catch (std::ios_base::failure& e) {
					std::cout << " [failed; " << e.what() << "]";
					iRet = RET_UNCOMMON_FAILURE; // some files failed, but not in a usual way
				}
				std::cout << std::endl;

			} else if (i->string_key.compare("delete") == 0) {
				std::string& strArchFile = i->value[0];
				std::cout << "   deleting: " << strArchFile << std::flush;

				try {
					ga::Archive::EntryPtr id = findFile(pArchive, strArchFile);
					if (!id) {
						std::cout << " [failed; file not found]";
						iRet = RET_NONCRITICAL_FAILURE; // one or more files failed
					} else {
						pArchive->remove(id);
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
					return 1;
				}

				std::string strArchFile, strLocalFile;
				bool bAltDest = split(strSource, '=', &strArchFile, &strLocalFile);

				std::cout << "  inserting: " << strArchFile << " (before "
					<< strInsertBefore;
				if (bAltDest) std::cout << ", from " << strLocalFile;
				std::cout << ")" << std::flush;

				// Try to find strInsertBefore
				ga::Archive::EntryPtr idBeforeThis = findFile(pArchive, strInsertBefore);
				if (!idBeforeThis) {
					std::cout << " [failed; could not find " << strInsertBefore << "]";
					iRet = RET_NONCRITICAL_FAILURE; // one or more files failed
					continue;
				}

				try {
					insertFile(pArchive.get(), strLocalFile, strArchFile, idBeforeThis);
				} catch (std::ios_base::failure& e) {
					std::cout << " [failed; " << e.what() << "]";
					iRet = RET_UNCOMMON_FAILURE; // some files failed, but not in a usual way
				}

				std::cout << std::endl;

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
					if (bAltDest) std::cout << " (from " << strLocalFile << ")";
					std::cout << std::endl;

					try {
						insertFile(pArchive.get(), strLocalFile, strArchFile, ga::Archive::EntryPtr());
					} catch (std::ios_base::failure& e) {
						std::cout << " [failed; " << e.what() << "]";
						iRet = RET_UNCOMMON_FAILURE; // some files failed, but not in a usual way
					}

				} else if (i->string_key.compare("rename") == 0) {
					if ((!bAltDest) || (boost::iequals(strArchFile, strLocalFile))) {
						std::cout << "ignoring attempt to rename " << strArchFile
							<< " into the same name" << std::endl;
					} else {
						std::cout << "   renaming: " << strArchFile << " to "
							<< strLocalFile << std::flush;

						try {
							ga::Archive::EntryPtr id = findFile(pArchive, strArchFile);
							if (!id) {
								std::cout << " [failed; file not found inside archive]";
								iRet = RET_NONCRITICAL_FAILURE; // one or more files failed
							} else {
								pArchive->rename(id, strLocalFile);
							}
						} catch (std::ios_base::failure& e) {
							std::cout << " [failed; " << e.what() << "]";
							iRet = RET_UNCOMMON_FAILURE; // some files failed, but not in a common way
						}
						std::cout << std::endl;
					}

				} else if (i->string_key.compare("overwrite") == 0) {
					std::cout << "overwriting: " << strArchFile;
					if (bAltDest) std::cout << " (from " << strLocalFile << ")";
					std::cout << std::flush;

					try {
						// Find the file
						ga::Archive::EntryPtr id = findFile(pArchive, strArchFile);
						if (!id) {
							std::cout << " [failed; file not found inside archive]";
							iRet = RET_NONCRITICAL_FAILURE; // one or more files failed
						} else {
							// Found it, open replacement file
							std::ifstream sSrc(strLocalFile.c_str(), std::ios::binary);
							if (sSrc.is_open()) {
								// Figure out how big our incoming file is
								sSrc.seekg(0, std::ios::end);
								boost::iostreams::stream_offset iIncomingSize = sSrc.tellg();
								sSrc.seekg(0, std::ios::beg);
								if (iIncomingSize != id->iSize) {
									pArchive->resize(id, iIncomingSize);
								}
								// Now the file has been resized it's safe to open (if we opened
								// it before the resize it'd be stuck at the old size)
								boost::shared_ptr<std::iostream> psDest(pArchive->open(id));
									boost::iostreams::copy(sSrc, *psDest);
									psDest->flush();
							} else {
								std::cout << " [failed; unable to open replacement file]";
								iRet = RET_NONCRITICAL_FAILURE; // one or more files failed
							}
						}
					} catch (std::ios_base::failure& e) {
						std::cout << " [failed; " << e.what() << "]";
						iRet = RET_UNCOMMON_FAILURE; // some files failed, but not in a common way
					}
					std::cout << std::endl;
				}
				// else it's the archive filename, but we already have that
			}
		} // for (all command line elements)
		pArchive->flush();
	} catch (po::unknown_option& e) {
		std::cerr << PROGNAME ": " << e.what()
			<< ".  Use --help for help." << std::endl;
		return 1;
	} catch (po::invalid_command_line_syntax& e) {
		std::cerr << PROGNAME ": " << e.what()
			<< ".  Use --help for help." << std::endl;
		return 1;
	}

	return 0;
}
