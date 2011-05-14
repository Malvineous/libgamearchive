/**
 * @file   fmt-exe-ccaves.cpp
 * @brief  FixedArchive implementation for Crystal Caves .exe file.
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

#include "fmt-exe-ccaves.hpp"

namespace camoto {
namespace gamearchive {

#define MAPSIZE   (41*36)
#define LEVELSIZE (41*24)
#define SMLEVEL   (41*23)
FixedArchiveFile ccaves_file_list[] = {
	{0x8CE0,                            MAPSIZE,   "e1map.ccl", FILTER_NONE},
	{0x8CE0 + MAPSIZE + LEVELSIZE * 0,  LEVELSIZE, "e1l01.ccl", FILTER_NONE},
	{0x8CE0 + MAPSIZE + LEVELSIZE * 1,  LEVELSIZE, "e1l02.ccl", FILTER_NONE},
	{0x8CE0 + MAPSIZE + LEVELSIZE * 2,  LEVELSIZE, "e1l03.ccl", FILTER_NONE},
	{0x8CE0 + MAPSIZE + LEVELSIZE * 3,  LEVELSIZE, "e1l04.ccl", FILTER_NONE},
	{0x8CE0 + MAPSIZE + LEVELSIZE * 4,  LEVELSIZE, "e1l05.ccl", FILTER_NONE},
	{0x8CE0 + MAPSIZE + LEVELSIZE * 5,  LEVELSIZE, "e1l06.ccl", FILTER_NONE},
	{0x8CE0 + MAPSIZE + LEVELSIZE * 6,  SMLEVEL,   "e1l07.ccl", FILTER_NONE},
	{0x8CE0 + MAPSIZE + LEVELSIZE * 6 + SMLEVEL, SMLEVEL, "e1l08.ccl", FILTER_NONE},
	{0x8CE0 + MAPSIZE + LEVELSIZE * 6 + SMLEVEL*2, LEVELSIZE, "e1l09.ccl", FILTER_NONE},
	{0x8CE0 + MAPSIZE + LEVELSIZE * 7 + SMLEVEL*2, LEVELSIZE, "e1l10.ccl", FILTER_NONE},
	{0x8CE0 + MAPSIZE + LEVELSIZE * 8 + SMLEVEL*2, LEVELSIZE, "e1l11.ccl", FILTER_NONE},
	{0x8CE0 + MAPSIZE + LEVELSIZE * 9 + SMLEVEL*2, LEVELSIZE, "e1l12.ccl", FILTER_NONE},
	{0x8CE0 + MAPSIZE + LEVELSIZE * 10 + SMLEVEL*2, LEVELSIZE, "e1l13.ccl", FILTER_NONE},
	{0x8CE0 + MAPSIZE + LEVELSIZE * 11 + SMLEVEL*2, SMLEVEL, "e1l14.ccl", FILTER_NONE},
	{0x8CE0 + MAPSIZE + LEVELSIZE * 11 + SMLEVEL*3, LEVELSIZE, "e1l15.ccl", FILTER_NONE},
	{0x8CE0 + MAPSIZE + LEVELSIZE * 12 + SMLEVEL*3, LEVELSIZE, "e1l16.ccl", FILTER_NONE},
};

EXE_CCavesType::EXE_CCavesType()
	throw ()
{
}

EXE_CCavesType::~EXE_CCavesType()
	throw ()
{
}

std::string EXE_CCavesType::getArchiveCode() const
	throw ()
{
	return "exe-ccaves";
}

std::string EXE_CCavesType::getFriendlyName() const
	throw ()
{
	return "Crystal Caves Executable";
}

// Get a list of the known file extensions for this format.
std::vector<std::string> EXE_CCavesType::getFileExtensions() const
	throw ()
{
	std::vector<std::string> vcExtensions;
	vcExtensions.push_back("exe");
	return vcExtensions;
}

std::vector<std::string> EXE_CCavesType::getGameList() const
	throw ()
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Crystal Caves");
	return vcGames;
}

E_CERTAINTY EXE_CCavesType::isInstance(iostream_sptr psArchive) const
	throw (std::ios::failure)
{
	psArchive->seekg(0, std::ios::end);
	io::stream_offset lenArchive = psArchive->tellg();

	if (lenArchive == 191984) {
		// TESTED BY: TODO fixed_exe_ccaves_isinstance_c00
		psArchive->seekg(0x1E00);
		char buffer[8];
		psArchive->read(buffer, 8);
		// Unfortunately no version strings, so check some data I
		// selected at random...
		if (strncmp(buffer, "\x55\x89\xE5\x8B\x46\x06\xBA\xA0", 8) != 0)
			return EC_DEFINITELY_NO;

		return EC_DEFINITELY_YES;
	}

	// TESTED BY: TODO (generic)
	return EC_DEFINITELY_NO;
}

ArchivePtr EXE_CCavesType::newArchive(iostream_sptr psArchive, SuppData& suppData) const
	throw (std::ios::failure)
{
	// This isn't a true archive so we can't create new versions of it.
	throw std::ios::failure("Can't create a new archive in this format.");
}

ArchivePtr EXE_CCavesType::open(iostream_sptr psArchive, SuppData& suppData) const
	throw (std::ios::failure)
{
	return ArchivePtr(new EXE_CCavesArchive(psArchive));
}

SuppFilenames EXE_CCavesType::getRequiredSupps(const std::string& filenameArchive) const
	throw ()
{
	// No supplemental types/empty list
	return SuppFilenames();
}


EXE_CCavesArchive::EXE_CCavesArchive(iostream_sptr psArchive)
	throw (std::ios::failure) :
		FixedArchive(psArchive, ccaves_file_list, sizeof(ccaves_file_list) / sizeof(FixedArchiveFile))
{
}

EXE_CCavesArchive::~EXE_CCavesArchive()
	throw ()
{
}

} // namespace gamearchive
} // namespace camoto
