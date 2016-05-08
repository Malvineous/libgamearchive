Camoto: Classic-game Modding Tools
==================================
Copyright 2010-2016 Adam Nielsen <<malvineous@shikadi.net>>  
http://www.shikadi.net/camoto/  
Linux/OSX: [![Build Status](https://travis-ci.org/Malvineous/libgamearchive.svg?branch=master)](https://travis-ci.org/Malvineous/libgamearchive)

Camoto is a collection of utilities for editing (modding) "classic" PC
games - those running under MS-DOS from the 1980s and 1990s.

This is **libgamearchive**, one component of the Camoto suite.  libgamearchive
is a library that provides access to different "archive" files used by games
to store all their data files.  Just like a .zip file, many games squash all
their data into one big file, and this library provides access to it.  It
currently supports full editing, so files can be extracted, replaced, added
and removed from supported archive formats.

File formats from the following games have been implemented:

  * Alien Breed Tower Assault (.epf)
  * Arcade Pool (.epf)
  * Asterix & Obelix (.epf)
  * BlackThorne (.dat)
  * Blood (.rff)
  * Cosmo's Cosmic Adventures (.vol, .stn)
  * Crystal Caves (.exe)
  * Dangerous Dave (.exe)
  * Dark Ages (file05.da[123])
  * Death Rally (.bpa)
  * Descent (.hog)
  * Doofus (.g-d)
  * Doom (.wad)
  * Duke Nukem II (.cmp)
  * Duke Nukem 3D (.grp, .rts)
  * Galactix (.glb)
  * God of Thunder (.dat) [no decompression yet]
  * Halloween Harry (.bnk, harry.-0)
  * Highway Hunter (.dat)
  * Heretic (.wad)
  * Hexen (.wad)
  * Hocus Pocus (.dat)
  * Hugo II: Whodunit? and Hugo III: Jungle of Doom! (.dat)
  * Incredible Machine, The (resource.*)
  * In Search of Dr. Riptide (.dat)
  * Jungle Book, The (.epf)
  * Lion King, The (.epf)
  * Lost Vikings (.dat)
  * Major Stryker (.ms[123])
  * Monster Bash (.dat)
  * Mystic Towers (.dat)
  * Overdrive (.epf)
  * Prehistorik (.cur, .vga)
  * Project X (.epf)
  * Raptor (.glb)
  * Rise of the Triad (.wad, .rts)
  * Redneck Rampage (.grp, .rts)
  * Sango Fighter (.dat, .pcm, and many others)
  * Sensible Golf (.epf)
  * Sherlock Holmes, The Lost Files of (.lib)
  * SkyRoads (.lzs)
  * Shadow Warrior (.grp, .rts)
  * Spirou (.epf)
  * Stargunner (.dlt)
  * Stellar 7 (.res)
  * Terminal Velocity (.pod)
  * Tin Tin in Tibet (.epf)
  * Universe (.epf)
  * Vinyl Goddess From Mars (.lbr)
  * Wacky Wheels (.dat)
  * WarCraft: Orcs & Humans (.dat)
  * Word Rescue (.1)
  * Zool (.dat) [no decompression yet]

Many more formats are planned.  The library also implements the following
compression/encryption algorithms used by these games:

  * Alien Breed Tower Assault (decompress only)
  * Arcade Pool (decompress only)
  * Blood
  * Dangerous Dave
  * God of Thunder
  * Lion King, The (decompress only)
  * Monster Bash (decompress only)
  * Overdrive (decompress only)
  * Prehistorik
  * Project X (decompress only)
  * Raptor
  * Secret Agent
  * Sensible Golf (decompress only)
  * SkyRoads
  * Smurfs, The (decompress only)
  * Spirou (decompress only)
  * Stargunner (decompress only, game works with uncompressed files)
  * Stellar 7 (decompress only)
  * Tin Tin in Tibet (decompress only)
  * Universe (decompress only)
  * Zone 66

The library is compiled and installed in the usual way:

    ./autogen.sh          # Only if compiling from git
    ./configure && make
    make check            # Optional, compile and run tests
    sudo make install
    sudo ldconfig

You will need the following prerequisites already installed:

  * [libgamecommon](https://github.com/Malvineous/libgamecommon) >= 2.0
  * Boost >= 1.59 (Boost >= 1.46 will work if not using `make check`)
  * xmlto (optional for tarball releases, required for git version and if
    manpages are to be changed)

This distribution includes an example program `gamearch` which serves as both
a command-line interface to the library as well as an example of how to use
the library.  This program is installed as part of the `make install` process.
See `man gamearch` for full details.

A second example `gamecomp` is also included which provides command line
access to the compression algorithms implemented in the library.  Although
`gamearch` will automatically decompress files on extraction, `gamecomp` can be
used to decompress files that are not contained within an archive (such as
the Zone 66 data files.)

All supported file formats are fully documented on the
[ModdingWiki](http://www.shikadi.net/moddingwiki/Category:Archive_formats).

If you'd like to contribute (please do!) there is a tutorial about [how to add a
new file format to the library](http://www.shikadi.net/camoto/help/archive/new).

This library is released under the GPLv3 license.
