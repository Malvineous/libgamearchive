/**
 * @file  fmt-lbr-vinyl.cpp
 * @brief Implementation of Vinyl Goddess From Mars .LBR file reader/writer.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/LBR_Format
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

#include <string>
#include <sstream>

#include <camoto/iostream_helpers.hpp>
#include <camoto/util.hpp> // std::make_unique
#include "fmt-lbr-vinyl.hpp"

#define LBR_FILECOUNT_OFFSET    0
#define LBR_HEADER_LEN          2  // u16le file count
#define LBR_FAT_OFFSET          LBR_HEADER_LEN
#define LBR_FAT_ENTRY_LEN       6  // u16le hash + u32le size
#define LBR_FIRST_FILE_OFFSET   LBR_FAT_OFFSET  // empty archive only

#define LBR_FATENTRY_OFFSET(e) (LBR_HEADER_LEN + e->iIndex * LBR_FAT_ENTRY_LEN)

#define LBR_HASH_OFFSET(e)    LBR_FATENTRY_OFFSET(e)
#define LBR_FILEOFFSET_OFFSET(e) (LBR_FATENTRY_OFFSET(e) + 2)

namespace camoto {
namespace gamearchive {

const char *filenames[] = {
"1000P.CMP",
"100P.CMP",
"250P.CMP",
"500P.CMP",
"50P.CMP",
"APPLE.CMP",
"APPLE.SND",
"BAMBOOP.CMP",
"BAPPLE0.OMP",
"BETA.BIN",
"BGRENSHT.CMP",
"BLOOK.CMP",
"BLUEBALL.CMP",
"BLUEKEY.CMP",
"BLUE.PAL",
"BLUE.TLS",
"BOTTLE.CMP",
"BOUNCE.CMP",
"BRAIN.CMP",
"BREATH.CMP",
"BRIDGE.CMP",
"BSHOT.CMP",
"BUTFLY.CMP",
"CANNON.CMP",
"CASPLAT1.CMP",
"CASPLAT2.CMP",
"CASPLAT3.CMP",
"CASPLAT4.CMP",
"CASTLE.PAL",
"CASTLE.TLS",
"COVERUP.MUS",
"CREDITS.PAL",
"CREDITS.SCR",
"CRUSH.MUS",
"CSTARS.CMP",
"DATA.DAT",
"DARKBAR2.GRA",
"DEATH.CMP",
"DEMO_1.DTA",
"DEMO_2.DTA",
"DEMO_3.DTA",
"DIFFBUTN.CMP",
"DIFFMENU.CMP",
"DOTS1.CMP",
"DUNGEON.PAL",
"DUNGEON.TLS",
"DUNPLAT1.CMP",
"DUSTCLUD.CMP",
"ECHOT1.CMP",
"EGYPPLAT.CMP",
"EGYPT.PAL",
"EGYPT.TLS",
"ENDBOSSW.CMP",
"ENDING.SCN",
"ENTER2.SND",
"EPISODE.PAL",
"EPISODE.SCR",
"EVILEYE.MUS",
"EXIT.CMP",
"EXPL1.SND",
"FEVER.MUS",
"FIRE231.CMP",
"FRUIT.SND",
"GAME1.PAL",
"GAMEOPT.GRA",
"GATEKEY.CMP",
"GOLDKEY.CMP",
"GRAVE.PAL",
"GRAVE.TLS",
"GREYKEY.CMP",
"GRID.DTA",
"HARDHEAD.CMP",
"HEALJUG.CMP",
"HEALPOT.CMP",
"HEALPOTD.CMP",
"HEALPOT.SND",
"HELLO.T",
"HORUS.MUS",
"HURT.SND",
"HUTS.PAL",
"HUTS.TLS",
"INBET.PAL",
"INBETW.SCR",
"INOUTP00.CMP",
"INSURED.MUS",
"INTRO.MUS",
"JFIREB.CMP",
"JILL.CMP",
"JILLEXPB.CMP",
"JILLEXP.CMP",
"JILLFIRE.CMP",
"JILL.SPR",
"JUNGLE2.FON",
"JUNGLE.FON",
"KNIFE.CMP",
"LAND.SND",
"LC_CAPS.RAW",
"LC_NUMS.RAW",
"LEVEL1-1.M",
"LEVEL1-2.M",
"LEVEL1-3.M",
"LEVEL1-4.M",
"LEVEL1-5.M",
"LEVEL1-6.M",
"LEVEL1-7.M",
"LEVEL1-8.M",
"LEVEL1-9.M",
"LEVEL2-1.M",
"LEVEL2-2.M",
"LEVEL2-3.M",
"LEVEL2-4.M",
"LEVEL2-5.M",
"LEVEL2-6.M",
"LEVEL2-7.M",
"LEVEL2-8.M",
"LEVEL2-9.M",
"LEVEL3-1.M",
"LEVEL3-2.M",
"LEVEL3-3.M",
"LEVEL3-4.M",
"LEVEL3-5.M",
"LEVEL3-6.M",
"LEVEL3-7.M",
"LEVEL3-8.M",
"LEVEL3-9.M",
"LGRENSHT.CMP",
"LITSCROL.CMP",
"MAINFONT.GRA",
"MANEATPL.CMP",
"MENU2.RAW",
"MENUCH.GRA",
"MENUCLIK.SND",
"MENU.RAW",
"MENUYSNO.GRA",
"MIDLEVEL.CMP",
"MIDPOST.SND",
"MMREST.GRA",
"MONDIE.SND",
"MOUNT.TLS",
"MPLAT211.CMP",
"MPLAT212.CMP",
"MPLAT221.CMP",
"MPLAT311.CMP",
"MPLAT331.CMP",
"MPLAT332.CMP",
"MUSHSHOT.CMP",
"MYSTIC.MUS",
"NEWBEH.CMP",
"OLDBEH.CMP",
"ORDER.RES",
"OSIRIS.MUS",
"OUTGATE.CMP",
"OVERHEAD.PAL",
"OVERHEAD.TLS",
"OVERHED1.MAP",
"OVERHED2.MAP",
"OVERHED3.MAP",
"PAN2.SND",
"PRESENT.GRA",
"PRESENT.PAL",
"PROWLER.MUS",
"PURPLE.PAL",
"PURPLE.TLS",
"PUZZ6.MUS",
"RABBIT.CMP",
"RABBITD.CMP",
"REDKEY.CMP",
"RETROJIL.MUS",
"RING.CMP",
"RUFEYE.CMP",
"RUFEYES.CMP",
"RUFEYSE.CMP",
"SAVEBOXG.GRA",
"SAVEBOXO.GRA",
"SCORE.CMP",
"SCROLLG.CMP",
"SCROLLO.CMP",
"SGREENE.CMP",
"SHOTEXPL.CMP",
"SHOTTEST.CMP",
"SHWRREM.GRA",
"SIXPS.GRA",
"SIXPS.PAL",
"SKELBONE.CMP",
"SKELETON.CMP",
"SKELETON.SND",
"SKELFLY.CMP",
"SMALLEX.CMP",
"SMALNUM.CMP",
"SPARE.SCR",
"SPIKEBA.CMP",
"SPLADY.CMP",
"SPLAT211.CMP",
"SPLAT223.CMP",
"SPLAT231.CMP",
"SPRING.SND",
"SPROIN.CMP",
"SQUARE.TLS",
"STAR.CMP",
"STARDUST.MUS",
"STHORNSH.CMP",
"STICKEYE.CMP",
"STIKHORN.CMP",
"STLSPIKE.CMP",
"STORY.PAL",
"STORY.SCR",
"STRIKE.MUS",
"STRYFNT1.GRA",
"SVINYL.SPR",
"TAFA.MUS",
"T.CMP",
"TEST0004.CMP",
"THROW.SND",
"TITLE.PAL",
"TITLE.SCR",
"TORNADO.CMP",
"TRAMPLE.MUS",
"TREEMPLA.CMP",
"TREES.PAL",
"TREES.TLS",
"TWILIGHT.MUS",
"UGH.CMP",
"UNLOGIC1.GRA",
"UNLOGIC1.PAL",
"UNLOGIC.UNM",
"VINE.CMP",
"VINYLDIE.SND",
"VINYL.GRA",
"VINYL.PAL",
"VINYL.SPR",
"VSMALLE.CMP",
"WEAPBLNK.OMP",
"WEAPBLUE.OMP",
"WEAPBOTL.OMP",
"WEAPFIRE.OMP",
"WEAPFSKF.OMP",
"WEAPSLKF.OMP",
"WEAPSTAR.OMP",
"WFIREB.CMP",
"WOODSPIK.CMP",
"XHUTS.PAL",
"YELLOW.PAL",
"YELLOW.TLS",
"YES.CMP",

// These names were guessed by looking at others
"ENDG1.PAL",
"ENDG1.SCR",
"ENDG2.PAL",
"ENDG2.SCR",
"ENDG3.PAL",
"ENDG3.SCR",
"MOUNT.PAL",
"JUNGLE3.FON",

// These names were brute-forced from the hashes against a dictionary, so they
// could be wrong (each hash matches about 56 billion different filenames...)
"BEGIN.PAL",    // Also ARCHIL.PAL.   Before Bl, so probably correct.
"P.PAL",        // Also SANGGIL.PAL.  Between O-P, maybe correct.
"HDICFONT.GRA", // probably wrong
"KOEWA.SND",    // almost certainly wrong, also JADEJM.SND
"PALET1.PAL",
"QTYFONT.GRA",
"SHWFFONT.GRA",
"ROLPC.TIM",    // brute forced, but correct because...
"ROLPC.MUS",    // ...there's a matching song name too

// These names were guessed from the music filenames but with a different
// extension for the instruments.
"COVERUP.TIM",
"CRUSH.TIM",
"EVILEYE.TIM",
"FEVER.TIM",
"HORUS.TIM",
"INSURED.TIM",
"INTRO.TIM",
"MYSTIC.TIM",
"OSIRIS.TIM",
"PROWLER.TIM",
"PUZZ6.TIM",
"RETROJIL.TIM",
"STARDUST.TIM",
"STRIKE.TIM",
"TAFA.TIM",
"TRAMPLE.TIM",
"TWILIGHT.TIM",

// These were guessed by lemm
"BAPPLE1.OMP",
"BAPPLE2.OMP",
"BAPPLE3.OMP",
"BAPPLE4.OMP",

// These were guessed by wiivn
"SWOOSH.SND",
"TEXTBOX.GRA",
"TEXTBOX2.GRA",

// Files used by test code
"ONE.DAT",
"TWO.DAT",
"THREE.DAT",
"FOUR.DAT",

};

/// Hash function to convert filenames into LBR hashes
int calcHash(const std::string& data)
{
	int hash = 0;
	for (std::string::const_iterator i = data.begin(); i != data.end(); i++) {
		hash ^= *i << 8;
		for (int j = 0; j < 8; j++) {
			hash <<= 1;
			if (hash & 0x10000) hash ^= 0x1021;
		}
	}
	return hash & 0xffff;
}

ArchiveType_LBR_Vinyl::ArchiveType_LBR_Vinyl()
{
}

ArchiveType_LBR_Vinyl::~ArchiveType_LBR_Vinyl()
{
}

std::string ArchiveType_LBR_Vinyl::code() const
{
	return "lbr-vinyl";
}

std::string ArchiveType_LBR_Vinyl::friendlyName() const
{
	return "Vinyl Goddess From Mars Library";
}

std::vector<std::string> ArchiveType_LBR_Vinyl::fileExtensions() const
{
	return {
		"lbr",
	};
}

std::vector<std::string> ArchiveType_LBR_Vinyl::games() const
{
	return {
		"Vinyl Goddess From Mars",
	};
}

ArchiveType::Certainty ArchiveType_LBR_Vinyl::isInstance(
	stream::input& content) const
{
	stream::pos lenArchive = content.size();

	// TESTED BY: fmt_lbr_vinyl_isinstance_c01
	if (lenArchive < LBR_HEADER_LEN) return DefinitelyNo; // too short

	content.seekg(0, stream::start);

	uint32_t numFiles;
	content >> u16le(numFiles);

	// Since the last file goes from its offset to EOF, it's invalid to have
	// data after the FAT if there are zero files in the archive (because that
	// data would belong to the first file, which doesn't exist.)
	// TESTED BY: fmt_lbr_vinyl_isinstance_c05
	if ((numFiles == 0) && (lenArchive != 2)) return DefinitelyNo;

	stream::pos offContent = LBR_HEADER_LEN + LBR_FAT_ENTRY_LEN * numFiles;

	// Abort if the FAT is truncated.
	// TESTED BY: fmt_lbr_vinyl_isinstance_c03
	if (offContent > lenArchive) return DefinitelyNo;

	uint16_t hash;
	uint32_t offset;
	while (numFiles--) {
		content
			>> u16le(hash)
			>> u32le(offset)
		;
		// Make sure the offset is within the archive file.
		// TESTED BY: fmt_lbr_vinyl_isinstance_c02
		if (offset > lenArchive) return DefinitelyNo;

		// Make sure the offset is after the FAT.
		// TESTED BY: fmt_lbr_vinyl_isinstance_c04
		if (offset < offContent) return DefinitelyNo;
	}

	// TESTED BY: fmt_lbr_vinyl_isinstance_c00
	return DefinitelyYes;
}

std::shared_ptr<Archive> ArchiveType_LBR_Vinyl::create(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	content->seekp(0, stream::start);
	content->write("\x00\x00", 2);
	return std::make_shared<Archive_LBR_Vinyl>(std::move(content));
}

std::shared_ptr<Archive> ArchiveType_LBR_Vinyl::open(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	return std::make_shared<Archive_LBR_Vinyl>(std::move(content));
}

SuppFilenames ArchiveType_LBR_Vinyl::getRequiredSupps(stream::input& content,
	const std::string& filename) const
{
	// No supplemental types/empty list
	return SuppFilenames();
}


Archive_LBR_Vinyl::Archive_LBR_Vinyl(std::unique_ptr<stream::inout> content)
	:	Archive_FAT(std::move(content), LBR_FIRST_FILE_OFFSET, 0 /* no max filename len */)
{
	stream::pos lenArchive = this->content->size();

	if (lenArchive < LBR_HEADER_LEN) throw stream::error("file too short");

	this->content->seekg(0, stream::start);

	uint32_t numFiles;
	*this->content >> u16le(numFiles);

	if (numFiles > 0) {

		// Pre-calculate all the hashes
		std::map<int, const char *> fn;
		for (unsigned int f = 0; f < sizeof(filenames) / sizeof(char *); f++) {
			fn[calcHash(filenames[f])] = filenames[f];
		}

		uint32_t offNext, offCur;
		uint16_t hashNext = 0, hashCur; // TODO: store in new LBREntry class
		*this->content
			>> u16le(hashCur)
			>> u32le(offCur)
		;
		for (unsigned int i = 0; i < numFiles; i++) {
			// Read the data in from the FAT entry in the file
			if (i == numFiles - 1) {
				// Last entry has no 'next' one, so fake it as if next entry is EOF
				offNext = lenArchive;
			} else {
				*this->content
					>> u16le(hashNext)
					>> u32le(offNext)
				;
			}

			auto f = this->createNewFATEntry();

			f->iIndex = i;
			f->lenHeader = 0;
			f->iOffset = offCur;
			f->storedSize = offNext - offCur;
			f->realSize = f->storedSize;
			f->type = FILETYPE_GENERIC;
			f->fAttr = File::Attribute::Default;
			f->bValid = true;
			std::map<int, const char *>::iterator fnit = fn.find(hashCur);
			if (fnit != fn.end()) {
				f->strName = fnit->second;
			} else {
				// No match, use the hash as the filename
				std::stringstream ss;
				ss << std::hex << hashCur;
				f->strName = ss.str();
			}

			this->vcFAT.push_back(std::move(f));
			offCur = offNext;
			hashCur = hashNext;
		}
	}
}

Archive_LBR_Vinyl::~Archive_LBR_Vinyl()
{
}

void Archive_LBR_Vinyl::updateFileName(const FATEntry *pid, const std::string& strNewName)
{
	// TESTED BY: fmt_lbr_vinyl_rename
	this->content->seekp(LBR_HASH_OFFSET(pid), stream::start);
	*this->content << u16le(calcHash(strNewName));
	return;
}

void Archive_LBR_Vinyl::updateFileOffset(const FATEntry *pid, stream::delta offDelta)
{
	this->content->seekp(LBR_FILEOFFSET_OFFSET(pid), stream::start);
	*this->content << u32le(pid->iOffset);
	return;
}

void Archive_LBR_Vinyl::preInsertFile(const FATEntry *idBeforeThis, FATEntry *pNewEntry)
{
	// TESTED BY: fmt_lbr_vinyl_insert*

	// Set the format-specific variables
	pNewEntry->lenHeader = 0;

	// Because the new entry isn't in the vector yet we need to shift it manually
	pNewEntry->iOffset += LBR_FAT_ENTRY_LEN;

	this->content->seekp(LBR_FATENTRY_OFFSET(pNewEntry), stream::start);
	this->content->insert(LBR_FAT_ENTRY_LEN);

	*this->content
		<< u16le(calcHash(pNewEntry->strName))
		<< u32le(pNewEntry->iOffset)
	;

	// Update the offsets now there's a new FAT entry taking up space.
	this->shiftFiles(
		NULL,
		LBR_FAT_OFFSET + this->vcFAT.size() * LBR_FAT_ENTRY_LEN,
		LBR_FAT_ENTRY_LEN,
		0
	);

	this->updateFileCount(this->vcFAT.size() + 1);
	return;
}

void Archive_LBR_Vinyl::preRemoveFile(const FATEntry *pid)
{
	// TESTED BY: fmt_lbr_vinyl_remove*

	// Update the offsets now there's one less FAT entry taking up space.  This
	// must be called before the FAT is altered, because it will write a new
	// offset into the FAT entry we're about to erase (and if we erase it first
	// it'll overwrite something else.)
	this->shiftFiles(
		NULL,
		LBR_FAT_OFFSET + this->vcFAT.size() * LBR_FAT_ENTRY_LEN,
		-LBR_FAT_ENTRY_LEN,
		0
	);

	this->content->seekp(LBR_FATENTRY_OFFSET(pid), stream::start);
	this->content->remove(LBR_FAT_ENTRY_LEN);

	this->updateFileCount(this->vcFAT.size() - 1);
	return;
}

void Archive_LBR_Vinyl::updateFileCount(uint32_t iNewCount)
{
	// TESTED BY: fmt_lbr_vinyl_insert*
	// TESTED BY: fmt_lbr_vinyl_remove*
	this->content->seekp(LBR_FILECOUNT_OFFSET, stream::start);
	*this->content << u16le(iNewCount);
	return;
}

} // namespace gamearchive
} // namespace camoto
