/**
 * @file   fmt-resource-tim.cpp
 * @brief  File reader/writer for The Incredible Machine resource files.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/TIM_Resource_Format
 *
 * Copyright (C) 2010-2012 Adam Nielsen <malvineous@shikadi.net>
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

#include <boost/algorithm/string.hpp>
#include <camoto/iostream_helpers.hpp>
#include <camoto/util.hpp>

#include "fmt-resource-tim.hpp"

#define TIM_FIRST_FILE_OFFSET     0
#define TIM_MAX_FILENAME_LEN      12
#define TIM_FILENAME_FIELD_LEN    13

// Embedded FAT (no offset, has sig)
#define TIM_EFAT_FILENAME_OFFSET  0
#define TIM_EFAT_FILESIZE_OFFSET  13
#define TIM_EFAT_ENTRY_LEN        17  // filename + u32le size

// FAT file (no sig, has offset)
#define TIM_FAT_FILEOFFSET_OFFSET 4
#define TIM_FAT_ENTRY_LEN         8   // 2x unknown + offset

namespace camoto {
namespace gamearchive {

TIMResourceType::TIMResourceType()
	throw ()
{
}

TIMResourceType::~TIMResourceType()
	throw ()
{
}

std::string TIMResourceType::getArchiveCode() const
	throw ()
{
	return "resource-tim";
}

std::string TIMResourceType::getFriendlyName() const
	throw ()
{
	return "The Incredible Machine Resource File";
}

std::vector<std::string> TIMResourceType::getFileExtensions() const
	throw ()
{
	std::vector<std::string> vcExtensions;
	vcExtensions.push_back("001");
	vcExtensions.push_back("002");
	vcExtensions.push_back("003");
	vcExtensions.push_back("004");
	return vcExtensions;
}

std::vector<std::string> TIMResourceType::getGameList() const
	throw ()
{
	std::vector<std::string> vcGames;
	vcGames.push_back("The Incredible Machine");
	return vcGames;
}

ArchiveType::Certainty TIMResourceType::isInstance(stream::input_sptr psArchive) const
	throw (stream::error)
{
	try {
		stream::len lenArchive = psArchive->size();
		// TESTED BY: fmt_resource_tim_new_isinstance
		if (lenArchive == 0) return DefinitelyYes; // empty archive

		// TESTED BY: fmt_resource_tim_isinstance_c01
		if (lenArchive < TIM_EFAT_ENTRY_LEN) return DefinitelyNo; // too short

		stream::pos step = 0;
		uint32_t fileSize;
		while (step < lenArchive) {
			psArchive->seekg(step + TIM_FILENAME_FIELD_LEN, stream::start);
			psArchive >> u32le(fileSize);
			step += TIM_FILENAME_FIELD_LEN + 4 + fileSize;
		}

		// TESTED BY: fmt_resource_tim_isinstance_c02
		if (step != lenArchive) return DefinitelyNo;
	} catch (const stream::incomplete_read&) {
		// TESTED BY: fmt_resource_tim_isinstance_c03
		return DefinitelyNo;
	}

	// If we've made it this far, this is almost certainly in the correct format.
	// TESTED BY: fmt_resource_tim_isinstance_c00
	return DefinitelyYes;
}

ArchivePtr TIMResourceType::open(stream::inout_sptr psArchive, SuppData& suppData) const
	throw (stream::error)
{
	assert(suppData.find(SuppItem::FAT) != suppData.end());
	return ArchivePtr(new TIMResourceArchive(psArchive, suppData[SuppItem::FAT]));
}

SuppFilenames TIMResourceType::getRequiredSupps(stream::input_sptr data,
	const std::string& filenameArchive) const
	throw ()
{
	// No supplemental types/empty list
	SuppFilenames supps;
	supps[SuppItem::FAT] = "fat/" + filenameArchive;
	return supps;
}


TIMResourceArchive::TIMResourceArchive(stream::inout_sptr psArchive, stream::inout_sptr psFAT)
	throw (stream::error) :
		FATArchive(psArchive, TIM_FIRST_FILE_OFFSET, TIM_MAX_FILENAME_LEN),
		psFAT(new stream::seg())
{
	this->psFAT->open(psFAT);

	stream::len lenArchive = this->psArchive->size();
	this->psArchive->seekg(0, stream::start);

	stream::pos pos = 0;
	unsigned int index = 0;
	while (pos < lenArchive) {
		FATEntry *fatEntry = new FATEntry();
		EntryPtr ep(fatEntry);
		this->psArchive
			>> nullPadded(fatEntry->strName, TIM_FILENAME_FIELD_LEN)
			>> u32le(fatEntry->iSize)
		;
		fatEntry->iOffset = pos;
		fatEntry->iIndex = index++;
		fatEntry->lenHeader = TIM_EFAT_ENTRY_LEN;
		fatEntry->type = FILETYPE_GENERIC;
		fatEntry->fAttr = 0;
		fatEntry->bValid = true;
		fatEntry->iPrefilteredSize = fatEntry->iSize;
		this->vcFAT.push_back(ep);

		this->psArchive->seekg(fatEntry->iSize, stream::cur);
		pos += TIM_EFAT_ENTRY_LEN + fatEntry->iSize;
	}
}

TIMResourceArchive::~TIMResourceArchive()
	throw ()
{
}

void TIMResourceArchive::flush()
	throw (stream::error)
{
	this->psFAT->flush();
	this->FATArchive::flush();
	return;
}

void TIMResourceArchive::updateFileName(const FATEntry *pid, const std::string& strNewName)
	throw (stream::error)
{
	// TESTED BY: fmt_resource_tim_rename
	assert(strNewName.length() <= TIM_MAX_FILENAME_LEN);

	this->psArchive->seekp(pid->iOffset + TIM_EFAT_FILENAME_OFFSET, stream::start);
	this->psArchive << nullPadded(strNewName, TIM_FILENAME_FIELD_LEN);

	return;
}

void TIMResourceArchive::updateFileOffset(const FATEntry *pid, stream::delta offDelta)
	throw (stream::error)
{
	// TESTED BY: fmt_resource_tim_insert*
	// TESTED BY: fmt_resource_tim_resize*

	// Only the external FAT file has offsets, not the embedded FAT
	this->psFAT->seekp(pid->iIndex * TIM_FAT_ENTRY_LEN + TIM_FAT_FILEOFFSET_OFFSET, stream::start);
	this->psFAT << u32le(pid->iOffset);
	return;
}

void TIMResourceArchive::updateFileSize(const FATEntry *pid, stream::delta sizeDelta)
	throw (stream::error)
{
	// TESTED BY: fmt_resource_tim_insert*
	// TESTED BY: fmt_resource_tim_resize*

	// Update embedded FAT
	this->psArchive->seekp(pid->iOffset + TIM_EFAT_FILESIZE_OFFSET, stream::start);
	this->psArchive << u32le(pid->iSize);

	return;
}

FATArchive::FATEntry *TIMResourceArchive::preInsertFile(const FATEntry *idBeforeThis, FATEntry *pNewEntry)
	throw (stream::error)
{
	// TESTED BY: fmt_resource_tim_insert*
	int len = pNewEntry->strName.length();
	assert(len <= TIM_MAX_FILENAME_LEN);

	// Set the format-specific variables
	pNewEntry->lenHeader = TIM_EFAT_ENTRY_LEN;

	boost::to_upper(pNewEntry->strName);

	// Write out the new embedded FAT entry
	this->psArchive->seekp(pNewEntry->iOffset, stream::start);
	this->psArchive->insert(TIM_EFAT_ENTRY_LEN);

	// Write the header
	this->psArchive
		<< nullPadded(pNewEntry->strName, TIM_FILENAME_FIELD_LEN)
		<< u32le(pNewEntry->iSize)
	;

	// Since we've inserted some data for the embedded header, we need to update
	// the other file offsets accordingly.  This call updates the offset of the
	// files, then calls updateFileOffset() on them, using the *new* offset, so
	// we need to do this after the insert() call above to make sure the extra
	// data has been inserted.  Then when updateFileOffset() writes data out it
	// will go into the correct spot.
	this->shiftFiles(NULL, pNewEntry->iOffset, pNewEntry->lenHeader, 0);

	// Write out same info again but into the external FAT
	this->psFAT->seekp(pNewEntry->iIndex * TIM_FAT_ENTRY_LEN, stream::start);
	this->psFAT->insert(TIM_FAT_ENTRY_LEN);
	this->psFAT
		<< u16le(0)
		<< u16le(0)
		<< u32le(pNewEntry->iOffset)
	;

	return pNewEntry;
}

void TIMResourceArchive::preRemoveFile(const FATEntry *pid)
	throw (stream::error)
{
	// TESTED BY: fmt_resource_tim_remove*

	// Remove the FAT entry
	this->psFAT->seekp(pid->iIndex * TIM_FAT_ENTRY_LEN, stream::start);
	this->psFAT->remove(TIM_FAT_ENTRY_LEN);

	return;
}

} // namespace gamearchive
} // namespace camoto
