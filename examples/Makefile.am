bin_PROGRAMS = gamearch
noinst_PROGRAMS = hello

gamearch_SOURCES = gamearch.cpp
hello_SOURCES = hello.cpp

AM_CPPFLAGS = $(BOOST_CPPFLAGS) $(libgamecommon_CFLAGS) -I $(top_srcdir)/include
AM_LDFLAGS = $(BOOST_SYSTEM_LIBS) $(BOOST_FILESYSTEM_LIBS) $(BOOST_PROGRAM_OPTIONS_LIBS) $(BOOST_UNIT_TEST_FRAMEWORK_LIBS) $(libgamecommon_LIBS) $(top_builddir)/src/libgamearchive.la
