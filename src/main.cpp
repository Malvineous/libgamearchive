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
#include "filter-prehistorik.hpp"
#include "filter-skyroads.hpp"
#include "filter-stargunner.hpp"
#include "filter-stellar7.hpp"
#include "filter-xor-blood.hpp"
#include "filter-xor-sagent.hpp"
#include "filter-xor.hpp"
#include "filter-zone66.hpp"
#include "fmt-bnk-harry.hpp"
#include "fmt-bpa-drally.hpp"
#include "fmt-cur-prehistorik.hpp"
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
#include "fmt-mni-czone.hpp"
#include "fmt-pcxlib.hpp"
#include "fmt-pod-tv.hpp"
#include "fmt-res-stellar7.hpp"
#include "fmt-resource-tim-fat.hpp"
#include "fmt-resource-tim.hpp"
#include "fmt-rff-blood.hpp"
#include "fmt-roads-skyroads.hpp"
#include "fmt-vol-cosmo.hpp"
#include "fmt-wad-doom.hpp"

using namespace camoto::gamearchive;

namespace camoto {

template <>
const std::vector<std::shared_ptr<const ArchiveType> > FormatEnumerator<ArchiveType>::formats()
{
	std::vector<std::shared_ptr<const ArchiveType> > list;
	FormatEnumerator<ArchiveType>::addFormat<
		ArchiveType_BNK_Harry,
		ArchiveType_BPA_DRally,
		ArchiveType_DAT_Bash,
		ArchiveType_DAT_GoT,
		ArchiveType_DAT_Highway,
		ArchiveType_DAT_LostVikings,
		ArchiveType_DAT_Mystic,
		ArchiveType_DAT_Sango,
		ArchiveType_DAT_Wacky,
		ArchiveType_DLT_Stargunner,
		ArchiveType_EPF_LionKing,
		ArchiveType_EXE_CCaves,
		ArchiveType_EXE_DDave,
		ArchiveType_GLB_Raptor,
		ArchiveType_GRP_Duke3D,
		ArchiveType_HOG_Descent,
		ArchiveType_LBR_Vinyl,
		ArchiveType_LIB_Mythos,
		ArchiveType_PCXLib,
		ArchiveType_POD_TV,
		ArchiveType_RES_Stellar7,
		ArchiveType_RFF_Blood,
		ArchiveType_Roads_SkyRoads,
		ArchiveType_Resource_TIM_FAT,
		ArchiveType_Resource_TIM,
		ArchiveType_VOL_Cosmo,
		ArchiveType_WAD_Doom,
		// The following formats are difficult to autodetect, so putting them last
		// means they should only be checked if all the more robust formats above
		// have already failed to match.
		ArchiveType_CUR_Prehistorik,
		ArchiveType_GD_Doofus,
		ArchiveType_DAT_Hugo,
		ArchiveType_DAT_Hocus,
		ArchiveType_DA_Levels,
		ArchiveType_MNI_CZone
	>(list);
	return list;
}

template <>
const std::vector<std::shared_ptr<const FilterType> > FormatEnumerator<FilterType>::formats()
{
	std::vector<std::shared_ptr<const FilterType> > list;
	FormatEnumerator<FilterType>::addFormat<
		FilterType_Bash,
		FilterType_DDaveRLE,
		FilterType_DAT_GOT,
		FilterType_EPFS,
		FilterType_GLB_Raptor_FAT,
		FilterType_GLB_Raptor_File,
		FilterType_Prehistorik,
		FilterType_RFF,
		FilterType_SAM_16Sprite,
		FilterType_SAM_8Sprite,
		FilterType_SAM_Map,
		FilterType_SkyRoads,
		FilterType_Stargunner,
		FilterType_Stellar7,
		FilterType_XOR,
		FilterType_Zone66
	>(list);
	return list;
}

namespace gamearchive {

constexpr const char* const ArchiveType::obj_t_name;
constexpr const char* const FilterType::obj_t_name;

} // namespace gamearchive

} // namespace camoto
