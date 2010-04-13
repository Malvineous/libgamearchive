/*
 * fmt-grp-duke3d.cpp - Implementation of Duke3D .GRP file reader/writer.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/GRP_Format
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

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/progress.hpp>
#include <boost/shared_array.hpp>
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <exception>
#include <string.h>

#include "fmt-grp-duke3d.hpp"
#include "iostream_helpers.hpp"
#include "debug.hpp"

#define GRP_FAT_OFFSET        16  // "KenSilverman" header + u32le file count
#define GRP_MAX_FILENAME_LEN  12
#define GRP_FAT_ENTRY_LEN     16  // filename + u32le size

namespace camoto {
namespace gamearchive {

refcount_declclass(GRPType);

GRPType::GRPType()
	throw ()
{
	refcount_qenterclass(GRPType);
}

GRPType::~GRPType()
	throw ()
{
	refcount_qexitclass(GRPType);
}

std::string GRPType::getArchiveCode() const
	throw ()
{
	return "grp-duke3d";
}

std::string GRPType::getFriendlyName() const
	throw ()
{
	return "Duke Nukem 3D Group File";
}

// Get a list of the known file extensions for this format.
std::vector<std::string> GRPType::getFileExtensions() const
	throw ()
{
	std::vector<std::string> vcExtensions;
	vcExtensions.push_back("grp");
	return vcExtensions;
}

std::vector<std::string> GRPType::getGameList() const
	throw ()
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Duke Nukem 3D");
	vcGames.push_back("Redneck Rampage");
	vcGames.push_back("Shadow Warrior");
	return vcGames;
}

E_CERTAINTY GRPType::isInstance(iostream_sptr psArchive) const
	throw (std::ios::failure)
{
	char sig[12];
	psArchive->seekg(0, std::ios::beg);
	psArchive->read(sig, 12);
	if (psArchive->gcount() < 12) return EC_DEFINITELY_NO; // short read
	if (strncmp(sig, "KenSilverman", 12) == 0) return EC_DEFINITELY_YES;
	return EC_DEFINITELY_NO;
}

// Preconditions: isInstance() has returned > EC_DEFINITELY_NO
Archive *GRPType::open(iostream_sptr psArchive) const
	throw (std::ios::failure)
{
	return new GRPArchive(psArchive);
}


refcount_declclass(GRPArchive);

GRPArchive::GRPArchive(iostream_sptr psArchive)
	throw (std::ios::failure) :
		FATArchive(psArchive)
{
	psArchive->seekg(12, std::ios::beg); // skip "KenSilverman" sig

	// We still have to perform sanity checks in case the user forced an archive
	// to open even though it failed the signature check.
	if (psArchive->tellg() != 12) throw std::ios::failure("File too short");

	uint32_t iFileCount = read_u32le(psArchive);
	boost::shared_array<uint8_t> pFATBuf;
	try {
		pFATBuf.reset(new uint8_t[iFileCount * GRP_FAT_ENTRY_LEN]);
		this->vcFAT.reserve(iFileCount);
	} catch (std::bad_alloc) {
		std::cerr << "Unable to allocate enough memory for " << iFileCount
			<< " files." << std::endl;
		throw std::ios::failure("Memory allocation failure (archive corrupted?)");
	}

	// Read in all the FAT in one operation
	psArchive->read((char *)pFATBuf.get(), iFileCount * GRP_FAT_ENTRY_LEN);
	if (psArchive->gcount() != iFileCount * GRP_FAT_ENTRY_LEN) {
		std::cerr << "GRP file only " << psArchive->tellg()
			<< " bytes long (FAT is meant to be first " << (iFileCount + 1) * GRP_FAT_ENTRY_LEN
			<< " bytes.)" << std::endl;
		throw std::ios::failure("File has been truncated, it stops in the middle "
			"of the FAT!");
	}

	int iNextOffset = (iFileCount+1) * GRP_FAT_ENTRY_LEN; // offset of first file (+1 for KenSilverman sig)
	for (int i = 0; i < iFileCount; i++) {
		FATEntry *pEntry = new FATEntry();
		pEntry->iIndex = i;
		pEntry->strName = string_from_buf(&pFATBuf[i*GRP_FAT_ENTRY_LEN], GRP_MAX_FILENAME_LEN);
		pEntry->iOffset = iNextOffset;
		pEntry->iSize = u32le_from_buf(&pFATBuf[i*GRP_FAT_ENTRY_LEN + GRP_MAX_FILENAME_LEN]);
		pEntry->eType = EFT_USEFILENAME;
		pEntry->fAttr = 0;
		pEntry->bValid = true;
		this->vcFAT.push_back(EntryPtr(pEntry));

		// Update the offset for the next file
		iNextOffset += pEntry->iSize;
	}
	refcount_qenterclass(GRPArchive);
}

GRPArchive::~GRPArchive()
	throw ()
{
	refcount_qexitclass(GRPArchive);
}

// Does not invalidate existing EntryPtrs
void GRPArchive::rename(EntryPtr& id, const std::string& strNewName)
	throw (std::ios_base::failure)
{
	// TESTED BY: fmt_grp_duke3d_rename
	assert(this->isValid(id));
	FATEntry *pEntry = FATEntryPtr_from_EntryPtr(id);

	int iLen = strNewName.length();
	if (iLen > GRP_MAX_FILENAME_LEN) throw std::ios_base::failure("name too long");

	// Pad out the filename with NULLs
	char cBuffer[GRP_MAX_FILENAME_LEN];
	const char *p = strNewName.c_str();
	for (int i = 0; i < iLen; i++) cBuffer[i] = p[i];
	for (int i = iLen; i < GRP_MAX_FILENAME_LEN; i++) cBuffer[i] = 0;

	this->psArchive->seekp((pEntry->iIndex + 1) * GRP_FAT_ENTRY_LEN);
	this->psArchive->rdbuf()->sputn(cBuffer, GRP_MAX_FILENAME_LEN);
	pEntry->strName = strNewName;

	return;
}

void GRPArchive::updateFileOffset(const FATEntry *pid)
	throw (std::ios::failure)
{
	// This format doesn't have any offsets that need updating.  As this function
	// is only called when removing a file, the "offsets" will be sorted out
	// when the FAT entry is removed later.
	return;
}

void GRPArchive::updateFileSize(const FATEntry *pid)
	throw (std::ios::failure)
{
	// TESTED BY: fmt_grp_duke3d_insert*
	// TESTED BY: fmt_grp_duke3d_resize*
	this->psArchive->seekp((pid->iIndex + 1) * GRP_FAT_ENTRY_LEN + GRP_MAX_FILENAME_LEN);
	write_u32le(this->psArchive, pid->iSize);
	return;
}

void GRPArchive::insertFATEntry(const FATEntry *idBeforeThis, FATEntry *pNewEntry)
	throw (std::ios::failure)
{
	// TESTED BY: fmt_grp_duke3d_insert*
	if (pNewEntry->strName.length() > GRP_MAX_FILENAME_LEN) {
		throw std::ios::failure("maximum filename length is 12 chars");
	}

	// Because the new entry isn't in the vector yet we need to shift it manually
	pNewEntry->iOffset += GRP_FAT_ENTRY_LEN;

	this->psArchive->seekp((pNewEntry->iIndex + 1) * GRP_FAT_ENTRY_LEN);
	this->psArchive->insert(GRP_FAT_ENTRY_LEN);
	boost::to_upper(pNewEntry->strName);
	this->psArchive->write(pNewEntry->strName.c_str(), pNewEntry->strName.length());

	// Pad out to GRP_MAX_FILENAME_LEN chars, the null string must be at least
	// this long.
	char blank[GRP_FAT_ENTRY_LEN];
	memset(blank, 0, GRP_FAT_ENTRY_LEN);
	this->psArchive->write(blank, GRP_MAX_FILENAME_LEN - pNewEntry->strName.length());
	write_u32le(this->psArchive, pNewEntry->iSize);

	// Update the offsets now there's a new FAT entry taking up space.
	this->shiftFiles(GRP_FAT_OFFSET + this->vcFAT.size() * GRP_FAT_ENTRY_LEN, GRP_FAT_ENTRY_LEN, 0);

	this->updateFileCount(this->vcFAT.size() + 1);
	return;
}

void GRPArchive::removeFATEntry(const FATEntry *pid)
	throw (std::ios::failure)
{
	// TESTED BY: fmt_grp_duke3d_remove*

	// Update the offsets now there's one less FAT entry taking up space.  This
	// must be called before the FAT is altered, because it will write a new
	// offset into the FAT entry we're about to erase (and if we erase it first
	// it'll overwrite something else.)
	this->shiftFiles(GRP_FAT_OFFSET + this->vcFAT.size() * GRP_FAT_ENTRY_LEN, -GRP_FAT_ENTRY_LEN, 0);

	this->psArchive->seekp((pid->iIndex + 1) * GRP_FAT_ENTRY_LEN);
	this->psArchive->remove(GRP_FAT_ENTRY_LEN);

	this->updateFileCount(this->vcFAT.size() - 1);
	return;
}

void GRPArchive::updateFileCount(uint32_t iNewCount)
	throw (std::ios_base::failure)
{
	// TESTED BY: fmt_grp_duke3d_insert*
	// TESTED BY: fmt_grp_duke3d_remove*
	this->psArchive->seekp(12);
	write_u32le(this->psArchive, iNewCount);
	return;
}

} // namespace gamearchive
} // namespace camoto
