/**
 * @file  main.cpp
 * @brief Entry points for libgamearchive.
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
	this->vcFilters.push_back(FilterTypePtr(new FilterType_Bash()));
	this->vcFilters.push_back(FilterTypePtr(new FilterType_DDaveRLE()));
	this->vcFilters.push_back(FilterTypePtr(new FilterType_EPFS()));
	this->vcFilters.push_back(FilterTypePtr(new FilterType_GLB_Raptor_FAT()));
	this->vcFilters.push_back(FilterTypePtr(new FilterType_GLB_Raptor_File()));
	this->vcFilters.push_back(FilterTypePtr(new FilterType_DAT_GOT()));
	this->vcFilters.push_back(FilterTypePtr(new FilterType_RFF()));
	this->vcFilters.push_back(FilterTypePtr(new FilterType_SAM_16Sprite()));
	this->vcFilters.push_back(FilterTypePtr(new FilterType_SAM_8Sprite()));
	this->vcFilters.push_back(FilterTypePtr(new FilterType_SAM_Map()));
	this->vcFilters.push_back(FilterTypePtr(new FilterType_SkyRoads()));
	this->vcFilters.push_back(FilterTypePtr(new FilterType_Stargunner()));
	this->vcFilters.push_back(FilterTypePtr(new FilterType_Stellar7()));
	this->vcFilters.push_back(FilterTypePtr(new FilterType_XOR()));
	this->vcFilters.push_back(FilterTypePtr(new FilterType_Zone66()));

	this->vcTypes.push_back(ArchiveTypePtr(new ArchiveType_BNK_Harry()));
	this->vcTypes.push_back(ArchiveTypePtr(new ArchiveType_DAT_Bash()));
	this->vcTypes.push_back(ArchiveTypePtr(new ArchiveType_DAT_GoT()));
	this->vcTypes.push_back(ArchiveTypePtr(new ArchiveType_DAT_Highway()));
	this->vcTypes.push_back(ArchiveTypePtr(new ArchiveType_DAT_LostVikings()));
	this->vcTypes.push_back(ArchiveTypePtr(new ArchiveType_DAT_Mystic()));
	this->vcTypes.push_back(ArchiveTypePtr(new ArchiveType_DAT_Sango()));
	this->vcTypes.push_back(ArchiveTypePtr(new ArchiveType_DAT_Wacky()));
	this->vcTypes.push_back(ArchiveTypePtr(new ArchiveType_DLT_Stargunner()));
	this->vcTypes.push_back(ArchiveTypePtr(new ArchiveType_EPF_LionKing()));
	this->vcTypes.push_back(ArchiveTypePtr(new ArchiveType_EXE_CCaves()));
	this->vcTypes.push_back(ArchiveTypePtr(new ArchiveType_EXE_DDave()));
	this->vcTypes.push_back(ArchiveTypePtr(new ArchiveType_GLB_Raptor()));
	this->vcTypes.push_back(ArchiveTypePtr(new ArchiveType_GRP_Duke3D()));
	this->vcTypes.push_back(ArchiveTypePtr(new ArchiveType_HOG_Descent()));
	this->vcTypes.push_back(ArchiveTypePtr(new ArchiveType_LBR_Vinyl()));
	this->vcTypes.push_back(ArchiveTypePtr(new ArchiveType_LIB_Mythos()));
	this->vcTypes.push_back(ArchiveTypePtr(new ArchiveType_PCXLib()));
	this->vcTypes.push_back(ArchiveTypePtr(new ArchiveType_POD_TV()));
	this->vcTypes.push_back(ArchiveTypePtr(new ArchiveType_RES_Stellar7()));
	this->vcTypes.push_back(ArchiveTypePtr(new ArchiveType_RFF_Blood()));
	this->vcTypes.push_back(ArchiveTypePtr(new ArchiveType_Roads_SkyRoads()));
	this->vcTypes.push_back(ArchiveTypePtr(new ArchiveType_Resource_TIM_FAT()));
	this->vcTypes.push_back(ArchiveTypePtr(new ArchiveType_Resource_TIM()));
	this->vcTypes.push_back(ArchiveTypePtr(new ArchiveType_VOL_Cosmo()));
	this->vcTypes.push_back(ArchiveTypePtr(new ArchiveType_WAD_Doom()));

	// The following formats are difficult to autodetect, so putting them last
	// means they should only be checked if all the more robust formats above
	// have already failed to match.
	this->vcTypes.push_back(ArchiveTypePtr(new ArchiveType_GD_Doofus()));
	this->vcTypes.push_back(ArchiveTypePtr(new ArchiveType_DAT_Hugo()));
	this->vcTypes.push_back(ArchiveTypePtr(new ArchiveType_DAT_Hocus()));
	this->vcTypes.push_back(ArchiveTypePtr(new ArchiveType_DA_Levels()));
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
