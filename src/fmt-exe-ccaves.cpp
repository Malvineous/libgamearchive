/**
 * @file  fmt-exe-ccaves.cpp
 * @brief FixedArchive implementation for Crystal Caves .exe file.
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

#include <camoto/gamearchive/fixedarchive.hpp>
#include "fmt-exe-ccaves.hpp"

namespace camoto {
namespace gamearchive {

ArchiveType_EXE_CCaves::ArchiveType_EXE_CCaves()
{
}

ArchiveType_EXE_CCaves::~ArchiveType_EXE_CCaves()
{
}

std::string ArchiveType_EXE_CCaves::code() const
{
	return "exe-ccaves";
}

std::string ArchiveType_EXE_CCaves::friendlyName() const
{
	return "Crystal Caves Executable";
}

std::vector<std::string> ArchiveType_EXE_CCaves::fileExtensions() const
{
	return {
		"exe",
	};
}

std::vector<std::string> ArchiveType_EXE_CCaves::games() const
{
	return {
		"Crystal Caves",
	};
}

ArchiveType::Certainty ArchiveType_EXE_CCaves::isInstance(
	stream::input& content) const
{
	stream::pos lenArchive = content.size();

	if (lenArchive == 191984) {
		// TESTED BY: TODO fixed_exe_ccaves_isinstance_c00
		content.seekg(0x1E00, stream::start);
		char buffer[8];
		content.read(buffer, 8);
		// Unfortunately no version strings, so check some data I
		// selected at random...
		if (strncmp(buffer, "\x55\x89\xE5\x8B\x46\x06\xBA\xA0", 8) != 0)
			return DefinitelyNo;

		return DefinitelyYes;
	}

	// TESTED BY: TODO (generic)
	return DefinitelyNo;
}

std::shared_ptr<Archive> ArchiveType_EXE_CCaves::create(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	// This isn't a true archive so we can't create new versions of it.
	throw stream::error("Can't create a new archive in this format.");
}

std::shared_ptr<Archive> ArchiveType_EXE_CCaves::open(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
/// Offset of the first byte of map data
#define MAPDATA_START 0x8CE0

// Map sizes
#define SZ_INT    (41*5)
#define SZ_FIN    (41*6)
#define SZ_MAP    (41*25)
#define SZ_NORMAL (41*24)
#define SZ_SMALL  (41*23)

#define SZ_L01  SZ_NORMAL
#define SZ_L02  SZ_NORMAL
#define SZ_L03  SZ_NORMAL
#define SZ_L04  SZ_NORMAL
#define SZ_L05  SZ_NORMAL
#define SZ_L06  SZ_NORMAL
#define SZ_L07  SZ_SMALL
#define SZ_L08  SZ_SMALL
#define SZ_L09  SZ_NORMAL
#define SZ_L10  SZ_NORMAL
#define SZ_L11  SZ_NORMAL
#define SZ_L12  SZ_NORMAL
#define SZ_L13  SZ_NORMAL
#define SZ_L14  SZ_SMALL
#define SZ_L15  SZ_NORMAL
#define SZ_L16  SZ_NORMAL

// Map offsets
#define OFF_INT  MAPDATA_START
#define OFF_FIN  OFF_INT + SZ_INT
#define OFF_MAP  OFF_FIN + SZ_FIN
#define OFF_L01  OFF_MAP + SZ_MAP
#define OFF_L02  OFF_L01 + SZ_L01
#define OFF_L03  OFF_L02 + SZ_L02
#define OFF_L04  OFF_L03 + SZ_L03
#define OFF_L05  OFF_L04 + SZ_L04
#define OFF_L06  OFF_L05 + SZ_L05
#define OFF_L07  OFF_L06 + SZ_L06
#define OFF_L08  OFF_L07 + SZ_L07
#define OFF_L09  OFF_L08 + SZ_L08
#define OFF_L10  OFF_L09 + SZ_L09
#define OFF_L11  OFF_L10 + SZ_L10
#define OFF_L12  OFF_L11 + SZ_L11
#define OFF_L13  OFF_L12 + SZ_L12
#define OFF_L14  OFF_L13 + SZ_L13
#define OFF_L15  OFF_L14 + SZ_L14
#define OFF_L16  OFF_L15 + SZ_L15
	return make_FixedArchive(std::move(content), {
		{OFF_INT, SZ_INT, "e1int.ccl", FILTER_NONE, RESIZE_NONE},
		{OFF_FIN, SZ_FIN, "e1fin.ccl", FILTER_NONE, RESIZE_NONE},
		{OFF_MAP, SZ_MAP, "e1map.ccl", FILTER_NONE, RESIZE_NONE},
		{OFF_L01, SZ_L01, "e1l01.ccl", FILTER_NONE, RESIZE_NONE},
		{OFF_L02, SZ_L02, "e1l02.ccl", FILTER_NONE, RESIZE_NONE},
		{OFF_L03, SZ_L03, "e1l03.ccl", FILTER_NONE, RESIZE_NONE},
		{OFF_L04, SZ_L04, "e1l04.ccl", FILTER_NONE, RESIZE_NONE},
		{OFF_L05, SZ_L05, "e1l05.ccl", FILTER_NONE, RESIZE_NONE},
		{OFF_L06, SZ_L06, "e1l06.ccl", FILTER_NONE, RESIZE_NONE},
		{OFF_L07, SZ_L07, "e1l07.ccl", FILTER_NONE, RESIZE_NONE},
		{OFF_L08, SZ_L08, "e1l08.ccl", FILTER_NONE, RESIZE_NONE},
		{OFF_L09, SZ_L09, "e1l09.ccl", FILTER_NONE, RESIZE_NONE},
		{OFF_L10, SZ_L10, "e1l10.ccl", FILTER_NONE, RESIZE_NONE},
		{OFF_L11, SZ_L11, "e1l11.ccl", FILTER_NONE, RESIZE_NONE},
		{OFF_L12, SZ_L12, "e1l12.ccl", FILTER_NONE, RESIZE_NONE},
		{OFF_L13, SZ_L13, "e1l13.ccl", FILTER_NONE, RESIZE_NONE},
		{OFF_L14, SZ_L14, "e1l14.ccl", FILTER_NONE, RESIZE_NONE},
		{OFF_L15, SZ_L15, "e1l15.ccl", FILTER_NONE, RESIZE_NONE},
		{OFF_L16, SZ_L16, "e1l16.ccl", FILTER_NONE, RESIZE_NONE},
	});
}

SuppFilenames ArchiveType_EXE_CCaves::getRequiredSupps(stream::input& content,
	const std::string& filename) const
{
	return {};
}


} // namespace gamearchive
} // namespace camoto
