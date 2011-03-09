/*
 * main.cpp - Entry points for libgamearchive.
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

#include <string>
#include <camoto/gamearchive.hpp>
#include <camoto/debug.hpp>

// Include all the file formats for the Manager to load
#include "fmt-grp-duke3d.hpp"
#include "fmt-vol-cosmo.hpp"
#include "fmt-pod-tv.hpp"
#include "fmt-bnk-harry.hpp"
#include "fmt-res-stellar7.hpp"
#include "fmt-hog-descent.hpp"
#include "fmt-epf-lionking.hpp"
#include "fmt-rff-blood.hpp"
#include "fmt-dat-wacky.hpp"
#include "fmt-dat-hugo.hpp"
#include "fmt-exe-ccaves.hpp"
#include "fmt-exe-ddave.hpp"
#include "fmt-dat-bash.hpp"
#include "fmt-dat-hocus.hpp"
#include "fmt-dat-sango.hpp"
#include "fmt-lbr-vinyl.hpp"
#include "fmt-roads-skyroads.hpp"
#include "fmt-dat-got.hpp"

#include "filter-bash.hpp"
#include "filter-epfs.hpp"
#include "filter-zone66.hpp"
#include "filter-stellar7.hpp"
#include "filter-xor.hpp"
#include "filter-xor-blood.hpp"

namespace camoto {
namespace gamearchive {

ManagerPtr getManager()
	throw ()
{
	return ManagerPtr(new Manager());
}

Manager::Manager()
	throw ()
{
	this->vcTypes.push_back(ArchiveTypePtr(new GRPType()));
	this->vcTypes.push_back(ArchiveTypePtr(new VOLType()));
	this->vcTypes.push_back(ArchiveTypePtr(new PODType()));
	this->vcTypes.push_back(ArchiveTypePtr(new BNKType()));
	this->vcTypes.push_back(ArchiveTypePtr(new RESType()));
	this->vcTypes.push_back(ArchiveTypePtr(new HOGType()));
	this->vcTypes.push_back(ArchiveTypePtr(new EPFType()));
	this->vcTypes.push_back(ArchiveTypePtr(new RFFType()));
	this->vcTypes.push_back(ArchiveTypePtr(new DAT_WackyType()));
	this->vcTypes.push_back(ArchiveTypePtr(new EXE_CCavesType()));
	this->vcTypes.push_back(ArchiveTypePtr(new EXE_DDaveType()));
	this->vcTypes.push_back(ArchiveTypePtr(new DAT_BashType()));
	this->vcTypes.push_back(ArchiveTypePtr(new DAT_SangoType()));
	this->vcTypes.push_back(ArchiveTypePtr(new LBRType()));
	this->vcTypes.push_back(ArchiveTypePtr(new SkyRoadsRoadsType()));
	this->vcTypes.push_back(ArchiveTypePtr(new DAT_GoTType()));

	// The following formats are difficult to autodetect, so putting them last
	// means they should only be checked if all the more robust formats above
	// have already failed to match.
	this->vcTypes.push_back(ArchiveTypePtr(new DAT_HugoType()));
	this->vcTypes.push_back(ArchiveTypePtr(new DAT_HocusType()));

	this->vcFilters.push_back(FilterTypePtr(new BashFilterType()));
	this->vcFilters.push_back(FilterTypePtr(new EPFSFilterType()));
	this->vcFilters.push_back(FilterTypePtr(new Zone66FilterType()));
	this->vcFilters.push_back(FilterTypePtr(new Stellar7FilterType()));
	this->vcFilters.push_back(FilterTypePtr(new XORFilterType()));
	this->vcFilters.push_back(FilterTypePtr(new RFFFilterType()));
}

Manager::~Manager()
	throw ()
{
}

ArchiveTypePtr Manager::getArchiveType(int iIndex)
	throw ()
{
	if (iIndex >= this->vcTypes.size()) return ArchiveTypePtr();
	return this->vcTypes[iIndex];
}

ArchiveTypePtr Manager::getArchiveTypeByCode(const std::string& strCode)
	throw ()
{
	for (VC_ARCHIVETYPE::const_iterator i = this->vcTypes.begin();
		i != this->vcTypes.end(); i++
	) {
		if ((*i)->getArchiveCode().compare(strCode) == 0) return *i;
	}
	return ArchiveTypePtr();
}

FilterTypePtr Manager::getFilterType(int iIndex)
	throw ()
{
	if (iIndex >= this->vcFilters.size()) return FilterTypePtr();
	return this->vcFilters[iIndex];
}

FilterTypePtr Manager::getFilterTypeByCode(const std::string& strCode)
	throw ()
{
	for (VC_FILTERTYPE::const_iterator i = this->vcFilters.begin();
		i != this->vcFilters.end(); i++
	) {
		if ((*i)->getFilterCode().compare(strCode) == 0) return *i;
	}
	return FilterTypePtr();
}

} // namespace gamearchive
} // namespace camoto
