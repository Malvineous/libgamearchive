bin_PROGRAMS = gamearch
bin_PROGRAMS += gamecomp
noinst_PROGRAMS = hello

gamearch_SOURCES = gamearch.cpp
gamecomp_SOURCES = gamecomp.cpp
hello_SOURCES = hello.cpp

WARNINGS = -Wall -Wextra -Wno-unused-parameter

AM_CPPFLAGS  = -I $(top_srcdir)/include
AM_CPPFLAGS += $(BOOST_CPPFLAGS)
AM_CPPFLAGS += $(libgamecommon_CPPFLAGS)
AM_CPPFLAGS += $(WARNINGS)

AM_CXXFLAGS  = $(DEBUG_CXXFLAGS)
AM_CXXFLAGS += $(libgamecommon_CFLAGS)

AM_LDFLAGS  = $(top_builddir)/src/libgamearchive.la
AM_LDFLAGS += $(BOOST_LDFLAGS)
AM_LDFLAGS += $(BOOST_SYSTEM_LIB)
AM_LDFLAGS += $(BOOST_PROGRAM_OPTIONS_LIB)
AM_LDFLAGS += $(libgamecommon_LIBS)
