/**
 * @file  fmt-mni-czone.cpp
 * @brief FixedArchive implementation for Duke Nukem II czone*.mni files.
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
#include "fmt-mni-czone.hpp"

namespace camoto {
namespace gamearchive {

ArchiveType_MNI_CZone::ArchiveType_MNI_CZone()
{
}

ArchiveType_MNI_CZone::~ArchiveType_MNI_CZone()
{
}

std::string ArchiveType_MNI_CZone::code() const
{
	return "mni-czone";
}

std::string ArchiveType_MNI_CZone::friendlyName() const
{
	return "Duke Nukem II CZone";
}

std::vector<std::string> ArchiveType_MNI_CZone::fileExtensions() const
{
	return {"mni"};
}

std::vector<std::string> ArchiveType_MNI_CZone::games() const
{
	return {"Duke Nukem II"};
}

ArchiveType::Certainty ArchiveType_MNI_CZone::isInstance(
	stream::input& content) const
{
	stream::pos len = content.size();

	// Standard tileset
	// TESTED BY: TODO
	if (len == 42000) return PossiblyYes;

	// TESTED BY: TODO
	return DefinitelyNo;
}

std::shared_ptr<Archive> ArchiveType_MNI_CZone::create(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	// This isn't a true archive so we can't create new versions of it.
	throw stream::error("Can't create a new archive in this format.");
}

std::shared_ptr<Archive> ArchiveType_MNI_CZone::open(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	return make_FixedArchive(std::move(content), {
		{0,             3600, "attrdata.mni", "tileattr/nukem2", nullptr},
		{3600,         32000, "solid.mni",    "tileset/ega-apogee", nullptr},
		{3600 + 32000,  6400, "masked.mni",   "tileset/ega-apogee", nullptr},
	});
}

SuppFilenames ArchiveType_MNI_CZone::getRequiredSupps(stream::input& content,
	const std::string& filename) const
{
	return {};
}


} // namespace gamearchive
} // namespace camoto
