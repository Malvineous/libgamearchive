bin_PROGRAMS = gamearch
bin_PROGRAMS += gamecomp
noinst_PROGRAMS = hello

gamearch_SOURCES = gamearch.cpp
gamecomp_SOURCES = gamecomp.cpp
hello_SOURCES = hello.cpp

AM_CPPFLAGS = $(BOOST_CPPFLAGS) -I $(top_srcdir)/include $(libgamecommon_CFLAGS)
AM_LDFLAGS = $(BOOST_SYSTEM_LIBS) $(BOOST_FILESYSTEM_LIBS) $(BOOST_PROGRAM_OPTIONS_LIBS) $(BOOST_UNIT_TEST_FRAMEWORK_LIBS) $(libgamecommon_LIBS) $(top_builddir)/src/libgamearchive.la
