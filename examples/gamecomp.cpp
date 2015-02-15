/**
 * @file  gamecomp.cpp
 * @brief Command-line interface to the compression/encryption filters in
 *        libgamearchive.
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

#ifdef WIN32
#include <io.h>
#include <fcntl.h>
#endif

#include <boost/algorithm/string.hpp> // for case-insensitive string compare
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/bind.hpp>
#include <camoto/gamearchive.hpp>
#include <camoto/util.hpp>
#include <camoto/stream_file.hpp>

namespace po = boost::program_options;
namespace fs = boost::filesystem;
namespace ga = camoto::gamearchive;
namespace stream = camoto::stream;

#define PROGNAME "gamecomp"

// Return values
#define RET_OK           0  ///< All is good
#define RET_BADARGS      1  ///< Bad arguments (missing/invalid parameters)
#define RET_SHOWSTOPPER  2  ///< I/O error

int main(int iArgC, char *cArgV[])
{
#ifdef __GLIBCXX__
	// Set a better exception handler
	std::set_terminate(__gnu_cxx::__verbose_terminate_handler);
#endif

#ifdef WIN32
	// Change stdout to be binary, so writing \x0A does not get changed to \x0D\x0A
	_setmode(0, _O_BINARY);
	_setmode(1, _O_BINARY);
	_setmode(2, _O_BINARY);
#endif

	// Disable stdin/printf/etc. sync for a speed boost
	std::ios_base::sync_with_stdio(false);

	// Declare the supported options.
	po::options_description poOptions("Options");
	poOptions.add_options()
		("apply,a",
			"apply the filter instead (compress/encrypt the input data) rather than "
			"the default of reversing the filter (to decompress/decrypt)")
		("list,l",
			"list all filters")
		("type,t", po::value<std::string>(),
			"specify the filter type")
	;

	po::options_description poHidden("Hidden parameters");
	poHidden.add_options()
		("help", "produce help message")
	;

	po::options_description poVisible("");
	poVisible.add(poOptions);

	po::options_description poComplete("Parameters");
	poComplete.add(poOptions).add(poHidden);
	po::variables_map mpArgs;

	auto pstdout = stream::open_stdout();
	auto pstdin = stream::open_stdin();
	ga::FilterManager::handler_t pFilterType;
	bool bApply = false;   // default is to reverse the algorithm (decompress)
	try {
		po::parsed_options pa = po::parse_command_line(iArgC, cArgV, poComplete);

		// Parse the global command line options
		for (std::vector<po::option>::iterator i = pa.options.begin(); i != pa.options.end(); i++) {
			if (i->string_key.empty()) {
				std::cerr << "Error: unexpected extra parameter (you can't list "
					"filenames on the command line, you must redirect stdin/out)"
					<< std::endl;
				return RET_BADARGS;
			} else if (i->string_key.compare("help") == 0) {
				std::cout <<
					"Copyright (C) 2010-2015 Adam Nielsen <malvineous@shikadi.net>\n"
					"This program comes with ABSOLUTELY NO WARRANTY.  This is free software,\n"
					"and you are welcome to change and redistribute it under certain conditions;\n"
					"see <http://www.gnu.org/licenses/> for details.\n"
					"\n"
					"Utility to apply and reverse compression and encryption algorithms used by\n"
					"games on their data files.\n"
					"Build date " __DATE__ " " __TIME__ << "\n"
					"\n"
					"Usage: gamecomp -t <type> < infile > outfile\n" << poVisible << "\n"
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
				pFilterType = ga::FilterManager::byCode(i->value[0]);
				if (!pFilterType) {
					std::cerr << PROGNAME ": Unknown filter code given by --type (-t) - "
						"use -l for a list." << std::endl;
					return RET_BADARGS;
				}
			} else if (
				(i->string_key.compare("a") == 0) ||
				(i->string_key.compare("apply") == 0)
			) {
				bApply = true;
			} else if (
				(i->string_key.compare("l") == 0) ||
				(i->string_key.compare("list") == 0)
			) {
				for (const auto& i : ga::FilterManager::formats()) {
					std::cout << i->code() << "\n";
				}
				std::cout << std::flush;
				return RET_OK;
			}
		}

		if (!pFilterType) {
			std::cerr << PROGNAME ": No filter type given (--type/-t).  Use -l for "
				"a list." << std::endl;
			return RET_BADARGS;
		}

		if (bApply) {
			// Apply the filter
			auto out = pFilterType->apply(
				std::shared_ptr<stream::output>(std::move(pstdout)),
				camoto::stream::fn_truncate_filter()
			);

			// Copy filtered data to stdout
			stream::copy(*out, *pstdin);
			out->flush();

		} else {
			// Apply the filter
			auto in = pFilterType->apply(std::move(pstdin));

			// Copy filtered data to stdout
			stream::copy(*pstdout, *in);
			pstdout->flush();
		}

	} catch (const camoto::filter_error& e) {
		pstdout->flush(); // keep as much data as we could process
		std::cerr << PROGNAME ": Filtering failed.  " << e.what() << std::endl;
		return RET_SHOWSTOPPER;
	} catch (const stream::error& e) {
		std::cerr << PROGNAME ": I/O error - " << e.what() << std::endl;
		return RET_SHOWSTOPPER;
	} catch (const po::unknown_option& e) {
		std::cerr << PROGNAME ": " << e.what()
			<< ".  Use --help for help." << std::endl;
		return RET_BADARGS;
	} catch (const po::invalid_command_line_syntax& e) {
		std::cerr << PROGNAME ": " << e.what()
			<< ".  Use --help for help." << std::endl;
		return RET_BADARGS;
	}

	return RET_OK;
}
