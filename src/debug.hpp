/*
 * debug.hpp - Helper functions to assist with debugging.
 *
 * Copyright (C) 2009 Adam Nielsen <malvineous@shikadi.net>
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

// Compile in debugging output?
//#define DEBUG

#ifndef _CAMOTO_DEBUG_HPP_
#define _CAMOTO_DEBUG_HPP_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define TOSTRING_X(x)  #x
#define TOSTRING(x)    TOSTRING_X(x)

#define CLR_NORM   "\e[22;39m"
#define CLR_GREY   "\e[1;30m"
#define CLR_RED    "\e[1;31m"
#define CLR_GREEN  "\e[1;32m"
#define CLR_YELLOW "\e[1;33m"
#define CLR_MAG    "\e[1;35m"
#define CLR_CYAN   "\e[1;36m"
#define CLR_WHITE  "\e[1;37m"

#ifdef DEBUG

#include <iostream>
//#include <sstream>
//#include <vector>

#define refcount_declclass(x) \
	int g_iRefCount_##x = 0; \
	int g_iRefCountMax_##x = 0;

#define refcount_dump(x) \
	extern int g_iRefCount_##x; \
	extern int g_iRefCountMax_##x; \
	std::cerr << CLR_WHITE #x << CLR_NORM ": " \
		<< ((g_iRefCount_##x) == 0 ? CLR_GREEN : CLR_RED) << g_iRefCount_##x \
		<< CLR_NORM " instances left (" CLR_WHITE << g_iRefCountMax_##x \
		<< CLR_NORM " peak)" << std::endl;

// Enter/exit class (constructor/destructor) quietly
#define refcount_qenterclass(x) \
	g_iRefCount_##x++; \
	if (g_iRefCount_##x > g_iRefCountMax_##x) g_iRefCountMax_##x = g_iRefCount_##x;

#define refcount_qexitclass(x) \
	g_iRefCount_##x--;

// As above but print a message
#define refcount_enterclass(x) \
	refcount_qenterclass(x); \
	std::cerr << "+" CLR_MAG #x " " CLR_GREY << __PRETTY_FUNCTION__ << CLR_NORM << std::endl;

#define refcount_exitclass(x) \
	refcount_qexitclass(x); \
	std::cerr << "-" CLR_MAG #x " " CLR_GREY << __PRETTY_FUNCTION__ << CLR_NORM << std::endl;

#else

// no-ops
#define refcount_declclass(x)
#define refcount_dump(x)
#define refcount_qenterclass(x)
#define refcount_qexitclass(x)
#define refcount_enterclass(x)
#define refcount_exitclass(x)

#endif

#endif // _CAMOTO_DEBUG_HPP_
