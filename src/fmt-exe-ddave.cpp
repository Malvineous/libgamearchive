/**
 * @file  fmt-exe-ddave.cpp
 * @brief FixedArchive implementation for Dangerous Dave .exe file.
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

#include <camoto/iostream_helpers.hpp>
#include <camoto/gamearchive/fixedarchive.hpp>
#include "fmt-exe-ddave.hpp"

namespace camoto {
namespace gamearchive {

/// Update the decompressed-size field for RLE-compressed files
stream::len ddaveResize(std::shared_ptr<stream::inout> arch, unsigned int index,
	stream::len newStoredSize, stream::len newRealSize);

FixedArchiveFile ddave_file_list[] = {
	{0x0b4ff, 0x0c620 - 0x0b4ff, "first.bin",   FILTER_NONE, RESIZE_NONE},
	{0x0c620+4, 0x120f0 - 0x0c620 -4, "cgadave.dav", "rle-ddave", ddaveResize},  // +4/-4 to ignore initial uint32le decompressed size
	{0x120f0+4, 0x1c4e0 - 0x120f0 -4, "vgadave.dav", "rle-ddave", ddaveResize},  // +4/-4 to ignore initial uint32le decompressed size
	{0x1c4e0, 0x1d780 - 0x1c4e0, "sounds.spk",  FILTER_NONE, RESIZE_NONE},
	{0x1d780, 0x1ea40 - 0x1d780, "menucga.gfx", FILTER_NONE, RESIZE_NONE},
	{0x1ea40, 0x20ec0 - 0x1ea40, "menuega.gfx", FILTER_NONE, RESIZE_NONE},
	{0x20ec0, 0x256c0 - 0x20ec0, "menuvga.gfx", FILTER_NONE, RESIZE_NONE},
	{0x26b0a, 768,               "vga.pal",     FILTER_NONE, RESIZE_NONE},
#define SIZE_LEVEL  (256 + 100*10 + 24)
#define LEVEL_OFFSET(x)  (0x26e0a + SIZE_LEVEL * (x))
	{LEVEL_OFFSET(0), SIZE_LEVEL, "level01.dav", FILTER_NONE, RESIZE_NONE},
	{LEVEL_OFFSET(1), SIZE_LEVEL, "level02.dav", FILTER_NONE, RESIZE_NONE},
	{LEVEL_OFFSET(2), SIZE_LEVEL, "level03.dav", FILTER_NONE, RESIZE_NONE},
	{LEVEL_OFFSET(3), SIZE_LEVEL, "level04.dav", FILTER_NONE, RESIZE_NONE},
	{LEVEL_OFFSET(4), SIZE_LEVEL, "level05.dav", FILTER_NONE, RESIZE_NONE},
	{LEVEL_OFFSET(5), SIZE_LEVEL, "level06.dav", FILTER_NONE, RESIZE_NONE},
	{LEVEL_OFFSET(6), SIZE_LEVEL, "level07.dav", FILTER_NONE, RESIZE_NONE},
	{LEVEL_OFFSET(7), SIZE_LEVEL, "level08.dav", FILTER_NONE, RESIZE_NONE},
	{LEVEL_OFFSET(8), SIZE_LEVEL, "level09.dav", FILTER_NONE, RESIZE_NONE},
	{LEVEL_OFFSET(9), SIZE_LEVEL, "level10.dav", FILTER_NONE, RESIZE_NONE},
};

stream::len ddaveResize(std::shared_ptr<stream::inout> arch, unsigned int index,
	stream::len newStoredSize, stream::len newRealSize)
{
	if ((newStoredSize == (stream::len)-1) && (newRealSize == (stream::len)-1)) {
		// Not resizing but querying expanded size
		uint32_t len;
		arch->seekp(ddave_file_list[index].offset - 4, stream::start);
		*arch >> u32le(len);
		return len;
	}
	if (newStoredSize > ddave_file_list[index].size) {
		throw stream::error("There is not enough space in the Dangerous Dave .exe "
			"file to fit this data.");
	}
	arch->seekp(ddave_file_list[index].offset - 4, stream::start);
	*arch << u32le(newRealSize);
	// The stored size is ignored because it's small enough to fit in the existing
	// slot, and with the decompressed size at the start the game will ignore the
	// extra data.
	return newRealSize;
}

ArchiveType_EXE_DDave::ArchiveType_EXE_DDave()
{
}

ArchiveType_EXE_DDave::~ArchiveType_EXE_DDave()
{
}

std::string ArchiveType_EXE_DDave::code() const
{
	return "exe-ddave";
}

std::string ArchiveType_EXE_DDave::friendlyName() const
{
	return "Dangerous Dave Executable";
}

std::vector<std::string> ArchiveType_EXE_DDave::fileExtensions() const
{
	std::vector<std::string> vcExtensions;
	vcExtensions.push_back("exe");
	return vcExtensions;
}

std::vector<std::string> ArchiveType_EXE_DDave::games() const
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Dangerous Dave");
	return vcGames;
}

ArchiveType::Certainty ArchiveType_EXE_DDave::isInstance(
	stream::input& content) const
{
	stream::pos lenArchive = content.size();

	if (lenArchive == 172848) {
		// TESTED BY: TODO fixed_exe_ddave_isinstance_c00
		content.seekg(0x26A80, stream::start);
		char buffer[25];
		content.read(buffer, 25);
		// No version strings, so check some data unlikely to be modded
		if (strncmp(buffer, "Trouble loading tileset!$", 25) != 0)
			return DefinitelyNo;

		return DefinitelyYes;
	}

	// TESTED BY: TODO (generic)
	return DefinitelyNo;
}

std::unique_ptr<Archive> ArchiveType_EXE_DDave::create(
	std::shared_ptr<stream::inout> content, SuppData& suppData) const
{
	// This isn't a true archive so we can't create new versions of it.
	throw stream::error("Can't create a new archive in this format.");
}

std::unique_ptr<Archive> ArchiveType_EXE_DDave::open(
	std::shared_ptr<stream::inout> content, SuppData& suppData) const
{
	std::vector<FixedArchiveFile> files;
	files.reserve(sizeof(ddave_file_list) / sizeof(FixedArchiveFile));
	for (unsigned int i = 0; i < sizeof(ddave_file_list) / sizeof(FixedArchiveFile); i++) {
		files.push_back(ddave_file_list[i]);
	}
	return createFixedArchive(content, files);
}

SuppFilenames ArchiveType_EXE_DDave::getRequiredSupps(stream::input& content,
	const std::string& filenameArchive) const
{
	// No supplemental types/empty list
	return SuppFilenames();
}


} // namespace gamearchive
} // namespace camoto
