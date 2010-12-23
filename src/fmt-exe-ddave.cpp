/**
 * @file   fmt-exe-ddave.cpp
 * @brief  FixedArchive implementation for Dangerous Dave .exe file.
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

#include "fmt-exe-ddave.hpp"

namespace camoto {
namespace gamearchive {

FixedArchiveFile ddave_file_list[] = {
	{0x0b4ff, 0x0c620 - 0x0b4ff, "first.bin",   FILTER_NONE},
	{0x0c620, 0x120f0 - 0x0c620, "cgadave.dav", "rle-ddave"},
	{0x120f0, 0x1c4e0 - 0x120f0, "vgadave.dav", "rle-ddave"},
	{0x1c4e0, 0x1d780 - 0x1c4e0, "sounds.spk",  FILTER_NONE},
	{0x1d780, 0x1ea40 - 0x1d780, "menucga.gfx", FILTER_NONE},
	{0x1ea40, 0x20ec0 - 0x1ea40, "menuega.gfx", FILTER_NONE},
	{0x20ec0, 0x256c0 - 0x20ec0, "menuvga.gfx", FILTER_NONE},
	{0x26b0a, 768,               "vga.pal",     FILTER_NONE},
#define SIZE_LEVEL  (256 + 100*10 + 24)
#define LEVEL_OFFSET(x)  (0x26e0a + SIZE_LEVEL * (x))
	{LEVEL_OFFSET(0), SIZE_LEVEL, "level01.dav", FILTER_NONE},
	{LEVEL_OFFSET(1), SIZE_LEVEL, "level02.dav", FILTER_NONE},
	{LEVEL_OFFSET(2), SIZE_LEVEL, "level03.dav", FILTER_NONE},
	{LEVEL_OFFSET(3), SIZE_LEVEL, "level04.dav", FILTER_NONE},
	{LEVEL_OFFSET(4), SIZE_LEVEL, "level05.dav", FILTER_NONE},
	{LEVEL_OFFSET(5), SIZE_LEVEL, "level06.dav", FILTER_NONE},
	{LEVEL_OFFSET(6), SIZE_LEVEL, "level07.dav", FILTER_NONE},
	{LEVEL_OFFSET(7), SIZE_LEVEL, "level08.dav", FILTER_NONE},
	{LEVEL_OFFSET(8), SIZE_LEVEL, "level09.dav", FILTER_NONE},
	{LEVEL_OFFSET(9), SIZE_LEVEL, "level10.dav", FILTER_NONE},
};

EXE_DDaveType::EXE_DDaveType()
	throw ()
{
}

EXE_DDaveType::~EXE_DDaveType()
	throw ()
{
}

std::string EXE_DDaveType::getArchiveCode() const
	throw ()
{
	return "exe-ddave";
}

std::string EXE_DDaveType::getFriendlyName() const
	throw ()
{
	return "Dangerous Dave Executable";
}

// Get a list of the known file extensions for this format.
std::vector<std::string> EXE_DDaveType::getFileExtensions() const
	throw ()
{
	std::vector<std::string> vcExtensions;
	vcExtensions.push_back("exe");
	return vcExtensions;
}

std::vector<std::string> EXE_DDaveType::getGameList() const
	throw ()
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Dangerous Dave");
	return vcGames;
}

E_CERTAINTY EXE_DDaveType::isInstance(iostream_sptr psArchive) const
	throw (std::ios::failure)
{
	psArchive->seekg(0, std::ios::end);
	io::stream_offset lenArchive = psArchive->tellg();

	if (lenArchive == 172848) {
		// TESTED BY: TODO fixed_exe_ddave_isinstance_c00
		psArchive->seekg(0x26A80);
		char buffer[25];
		psArchive->read(buffer, 25);
		// No version strings, so check some data unlikely to be modded
		if (strncmp(buffer, "Trouble loading tileset!$", 25) != 0)
			return EC_DEFINITELY_NO;

		return EC_DEFINITELY_YES;
	}

	// TESTED BY: TODO (generic)
	return EC_DEFINITELY_NO;
}

ArchivePtr EXE_DDaveType::newArchive(iostream_sptr psArchive, MP_SUPPDATA& suppData) const
	throw (std::ios::failure)
{
	// This isn't a true archive so we can't create new versions of it.
	throw std::ios::failure("Can't create a new archive in this format.");
}

ArchivePtr EXE_DDaveType::open(iostream_sptr psArchive, MP_SUPPDATA& suppData) const
	throw (std::ios::failure)
{
	return ArchivePtr(new EXE_DDaveArchive(psArchive));
}

MP_SUPPLIST EXE_DDaveType::getRequiredSupps(const std::string& filenameArchive) const
	throw ()
{
	// No supplemental types/empty list
	return MP_SUPPLIST();
}


EXE_DDaveArchive::EXE_DDaveArchive(iostream_sptr psArchive)
	throw (std::ios::failure) :
		FixedArchive(psArchive, ddave_file_list, sizeof(ddave_file_list) / sizeof(FixedArchiveFile))
{
}

EXE_DDaveArchive::~EXE_DDaveArchive()
	throw ()
{
}

} // namespace gamearchive
} // namespace camoto
