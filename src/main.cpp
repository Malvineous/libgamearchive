/**
 * @file  main.cpp
 * @brief Entry points for libgamearchive.
 *
 * Copyright (C) 2010-2013 Adam Nielsen <malvineous@shikadi.net>
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

#include <camoto/gamearchive/manager.hpp>

// Include all the file formats for the Manager to load
#include "filter-bash.hpp"
#include "filter-ddave-rle.hpp"
#include "filter-epfs.hpp"
#include "filter-glb-raptor.hpp"
#include "filter-got-lzss.hpp"
#include "filter-skyroads.hpp"
#include "filter-stargunner.hpp"
#include "filter-stellar7.hpp"
#include "filter-xor-blood.hpp"
#include "filter-xor-sagent.hpp"
#include "filter-xor.hpp"
#include "filter-zone66.hpp"
#include "fmt-bnk-harry.hpp"
#include "fmt-da-levels.hpp"
#include "fmt-dat-bash.hpp"
#include "fmt-dat-got.hpp"
#include "fmt-dat-highway.hpp"
#include "fmt-dat-hocus.hpp"
#include "fmt-dat-hugo.hpp"
#include "fmt-dat-lostvikings.hpp"
#include "fmt-dat-mystic.hpp"
#include "fmt-dat-sango.hpp"
#include "fmt-dat-wacky.hpp"
#include "fmt-dlt-stargunner.hpp"
#include "fmt-epf-lionking.hpp"
#include "fmt-exe-ccaves.hpp"
#include "fmt-exe-ddave.hpp"
#include "fmt-gd-doofus.hpp"
#include "fmt-glb-raptor.hpp"
#include "fmt-grp-duke3d.hpp"
#include "fmt-hog-descent.hpp"
#include "fmt-lbr-vinyl.hpp"
#include "fmt-lib-mythos.hpp"
#include "fmt-pcxlib.hpp"
#include "fmt-pod-tv.hpp"
#include "fmt-res-stellar7.hpp"
#include "fmt-resource-tim-fat.hpp"
#include "fmt-resource-tim.hpp"
#include "fmt-rff-blood.hpp"
#include "fmt-roads-skyroads.hpp"
#include "fmt-vol-cosmo.hpp"
#include "fmt-wad-doom.hpp"

namespace camoto {
namespace gamearchive {

class ActualManager: virtual public Manager
{
	private:
		ArchiveTypeVector vcTypes;   ///< List of available archive types
		FilterTypeVector vcFilters;  ///< List of available filter types

	public:
		ActualManager();
		~ActualManager();

		virtual const ArchiveTypePtr getArchiveType(unsigned int iIndex) const;
		virtual const ArchiveTypePtr getArchiveTypeByCode(
			const std::string& strCode) const;
		virtual const FilterTypePtr getFilterType(unsigned int iIndex) const;
		virtual const FilterTypePtr getFilterTypeByCode(const std::string& strCode)
			const;
};

const ManagerPtr getManager()
{
	return ManagerPtr(new ActualManager());
}

ActualManager::ActualManager()
{
	this->vcFilters.push_back(FilterTypePtr(new BashFilterType()));
	this->vcFilters.push_back(FilterTypePtr(new DDaveRLEFilterType()));
	this->vcFilters.push_back(FilterTypePtr(new EPFSFilterType()));
	this->vcFilters.push_back(FilterTypePtr(new GLBFATFilterType()));
	this->vcFilters.push_back(FilterTypePtr(new GLBFileFilterType()));
	this->vcFilters.push_back(FilterTypePtr(new GOTDatFilterType()));
	this->vcFilters.push_back(FilterTypePtr(new RFFFilterType()));
	this->vcFilters.push_back(FilterTypePtr(new SAM16SpriteFilterType()));
	this->vcFilters.push_back(FilterTypePtr(new SAM8SpriteFilterType()));
	this->vcFilters.push_back(FilterTypePtr(new SAMMapFilterType()));
	this->vcFilters.push_back(FilterTypePtr(new SkyRoadsFilterType()));
	this->vcFilters.push_back(FilterTypePtr(new StargunnerFilterType()));
	this->vcFilters.push_back(FilterTypePtr(new Stellar7FilterType()));
	this->vcFilters.push_back(FilterTypePtr(new XORFilterType()));
	this->vcFilters.push_back(FilterTypePtr(new Zone66FilterType()));

	this->vcTypes.push_back(ArchiveTypePtr(new BNKType()));
	this->vcTypes.push_back(ArchiveTypePtr(new DAT_BashType()));
	this->vcTypes.push_back(ArchiveTypePtr(new DAT_GoTType()));
	this->vcTypes.push_back(ArchiveTypePtr(new DAT_HighwayType()));
	this->vcTypes.push_back(ArchiveTypePtr(new DAT_LostVikingsType()));
	this->vcTypes.push_back(ArchiveTypePtr(new DAT_MysticType()));
	this->vcTypes.push_back(ArchiveTypePtr(new DAT_SangoType()));
	this->vcTypes.push_back(ArchiveTypePtr(new DAT_WackyType()));
	this->vcTypes.push_back(ArchiveTypePtr(new DLTType()));
	this->vcTypes.push_back(ArchiveTypePtr(new EPFType()));
	this->vcTypes.push_back(ArchiveTypePtr(new EXE_CCavesType()));
	this->vcTypes.push_back(ArchiveTypePtr(new EXE_DDaveType()));
	this->vcTypes.push_back(ArchiveTypePtr(new GLBType()));
	this->vcTypes.push_back(ArchiveTypePtr(new GRPType()));
	this->vcTypes.push_back(ArchiveTypePtr(new HOGType()));
	this->vcTypes.push_back(ArchiveTypePtr(new LBRType()));
	this->vcTypes.push_back(ArchiveTypePtr(new LIB_MythosType()));
	this->vcTypes.push_back(ArchiveTypePtr(new PCXLibType()));
	this->vcTypes.push_back(ArchiveTypePtr(new PODType()));
	this->vcTypes.push_back(ArchiveTypePtr(new RESType()));
	this->vcTypes.push_back(ArchiveTypePtr(new RFFType()));
	this->vcTypes.push_back(ArchiveTypePtr(new SkyRoadsRoadsType()));
	this->vcTypes.push_back(ArchiveTypePtr(new TIMResourceFATType()));
	this->vcTypes.push_back(ArchiveTypePtr(new TIMResourceType()));
	this->vcTypes.push_back(ArchiveTypePtr(new VOLType()));
	this->vcTypes.push_back(ArchiveTypePtr(new WADType()));

	// The following formats are difficult to autodetect, so putting them last
	// means they should only be checked if all the more robust formats above
	// have already failed to match.
	this->vcTypes.push_back(ArchiveTypePtr(new GD_DoofusType()));
	this->vcTypes.push_back(ArchiveTypePtr(new DAT_HugoType()));
	this->vcTypes.push_back(ArchiveTypePtr(new DAT_HocusType()));
	this->vcTypes.push_back(ArchiveTypePtr(new DA_LevelsType()));
}

ActualManager::~ActualManager()
{
}

const ArchiveTypePtr ActualManager::getArchiveType(unsigned int iIndex) const
{
	if (iIndex >= this->vcTypes.size()) return ArchiveTypePtr();
	return this->vcTypes[iIndex];
}

const ArchiveTypePtr ActualManager::getArchiveTypeByCode(
	const std::string& strCode) const
{
	for (ArchiveTypeVector::const_iterator i = this->vcTypes.begin();
		i != this->vcTypes.end(); i++
	) {
		if ((*i)->getArchiveCode().compare(strCode) == 0) return *i;
	}
	return ArchiveTypePtr();
}

const FilterTypePtr ActualManager::getFilterType(unsigned int iIndex) const
{
	if (iIndex >= this->vcFilters.size()) return FilterTypePtr();
	return this->vcFilters[iIndex];
}

const FilterTypePtr ActualManager::getFilterTypeByCode(
	const std::string& strCode) const
{
	for (FilterTypeVector::const_iterator i = this->vcFilters.begin();
		i != this->vcFilters.end(); i++
	) {
		if ((*i)->getFilterCode().compare(strCode) == 0) return *i;
	}
	return FilterTypePtr();
}

} // namespace gamearchive
} // namespace camoto
