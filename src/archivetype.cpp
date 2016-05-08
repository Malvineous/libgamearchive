/**
 * @file  archivetype.cpp
 * @brief Utility functions for ArchiveType.
 *
 * Copyright (C) 2010-2016 Adam Nielsen <malvineous@shikadi.net>
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

#include <iostream>
#include <camoto/gamearchive/archivetype.hpp>

using namespace camoto;
using namespace camoto::gamearchive;

CAMOTO_GAMEARCHIVE_API std::ostream& camoto::gamearchive::operator<< (
	std::ostream& s, const ArchiveType::Certainty& r)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic error "-Wswitch-enum"
	s << "ArchiveType::Certainty::";
	switch (r) {
		case ArchiveType::Certainty::DefinitelyNo: s << "DefinitelyNo"; break;
		case ArchiveType::Certainty::Unsure: s << "Unsure"; break;
		case ArchiveType::Certainty::PossiblyYes: s << "PossiblyYes"; break;
		case ArchiveType::Certainty::DefinitelyYes: s << "DefinitelyYes"; break;
		default: s << "???"; break;
	}
#pragma GCC diagnostic pop
	return s;
}
