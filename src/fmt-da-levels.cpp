/**
 * @file  fmt-da-levels.cpp
 * @brief FixedArchive implementation for Dark Ages level file.
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
#include "fmt-da-levels.hpp"

namespace camoto {
namespace gamearchive {

FixedArchiveFile da_file_list[] = {
	{1152 * 0, 1152, "l01.dal",   FILTER_NONE, RESIZE_NONE},
	{1152 * 1, 1152, "l02.dal",   FILTER_NONE, RESIZE_NONE},
	{1152 * 2, 1152, "l03.dal",   FILTER_NONE, RESIZE_NONE},
	{1152 * 3, 1152, "l04.dal",   FILTER_NONE, RESIZE_NONE},
	{1152 * 4, 1152, "l05.dal",   FILTER_NONE, RESIZE_NONE},
	{1152 * 5, 1152, "l06.dal",   FILTER_NONE, RESIZE_NONE},
	{1152 * 6, 1152, "l07.dal",   FILTER_NONE, RESIZE_NONE},
	{1152 * 7, 1152, "l08.dal",   FILTER_NONE, RESIZE_NONE},
	{1152 * 8, 1152, "l09.dal",   FILTER_NONE, RESIZE_NONE},
	{1152 * 9, 1152, "l10.dal",   FILTER_NONE, RESIZE_NONE},
};

ArchiveType_DA_Levels::ArchiveType_DA_Levels()
{
}

ArchiveType_DA_Levels::~ArchiveType_DA_Levels()
{
}

std::string ArchiveType_DA_Levels::code() const
{
	return "da-levels";
}

std::string ArchiveType_DA_Levels::friendlyName() const
{
	return "Dark Ages levels";
}

std::vector<std::string> ArchiveType_DA_Levels::fileExtensions() const
{
	std::vector<std::string> vcExtensions;
	vcExtensions.push_back("da1");
	vcExtensions.push_back("da2");
	vcExtensions.push_back("da3");
	return vcExtensions;
}

std::vector<std::string> ArchiveType_DA_Levels::games() const
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Dark Ages");
	return vcGames;
}

ArchiveType::Certainty ArchiveType_DA_Levels::isInstance(
	stream::input& content) const
{
	stream::pos lenArchive = content.size();
	if (lenArchive == 1152 * 10) {
		return PossiblyYes;
	}
	return DefinitelyNo;
}

std::unique_ptr<Archive> ArchiveType_DA_Levels::create(
	std::shared_ptr<stream::inout> content, SuppData& suppData) const
{
	// This isn't a true archive so we can't create new versions of it.
	throw stream::error("Can't create a new archive in this format.");
}

std::unique_ptr<Archive> ArchiveType_DA_Levels::open(
	std::shared_ptr<stream::inout> content, SuppData& suppData) const
{
	std::vector<FixedArchiveFile> files;
	files.reserve(sizeof(da_file_list) / sizeof(FixedArchiveFile));
	for (unsigned int i = 0; i < sizeof(da_file_list) / sizeof(FixedArchiveFile); i++) {
		files.push_back(da_file_list[i]);
	}
	return createFixedArchive(content, files);
}

SuppFilenames ArchiveType_DA_Levels::getRequiredSupps(stream::input& content,
	const std::string& filenameArchive) const
{
	// No supplemental types/empty list
	return SuppFilenames();
}


} // namespace gamearchive
} // namespace camoto
