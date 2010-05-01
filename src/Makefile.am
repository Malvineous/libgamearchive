lib_LTLIBRARIES = libgamearchive.la

libgamearchive_la_SOURCES = main.cpp
libgamearchive_la_SOURCES += archive.cpp
libgamearchive_la_SOURCES += subdirarchive.cpp
libgamearchive_la_SOURCES += segmented_stream.cpp
libgamearchive_la_SOURCES += substream.cpp
libgamearchive_la_SOURCES += iostream_helpers.cpp
libgamearchive_la_SOURCES += fatarchive.cpp
libgamearchive_la_SOURCES += fmt-grp-duke3d.cpp
libgamearchive_la_SOURCES += fmt-vol-cosmo.cpp
libgamearchive_la_SOURCES += fmt-pod-tv.cpp
libgamearchive_la_SOURCES += fmt-bnk-harry.cpp
libgamearchive_la_SOURCES += fmt-res-stellar7.cpp
libgamearchive_la_SOURCES += fmt-hog-descent.cpp

EXTRA_libgamearchive_la_SOURCES = debug.hpp
EXTRA_libgamearchive_la_SOURCES += byteorder.h
EXTRA_libgamearchive_la_SOURCES += subdirarchive.hpp
EXTRA_libgamearchive_la_SOURCES += segmented_stream.hpp
EXTRA_libgamearchive_la_SOURCES += substream.hpp
EXTRA_libgamearchive_la_SOURCES += iostream_helpers.hpp
EXTRA_libgamearchive_la_SOURCES += fatarchive.hpp
EXTRA_libgamearchive_la_SOURCES += fmt-grp-duke3d.hpp
EXTRA_libgamearchive_la_SOURCES += fmt-vol-cosmo.hpp
EXTRA_libgamearchive_la_SOURCES += fmt-pod-tv.hpp
EXTRA_libgamearchive_la_SOURCES += fmt-bnk-harry.hpp
EXTRA_libgamearchive_la_SOURCES += fmt-res-stellar7.hpp
EXTRA_libgamearchive_la_SOURCES += fmt-hog-descent.hpp

AM_CPPFLAGS = $(DEBUG_CPPFLAGS) $(BOOST_CPPFLAGS) -I $(top_srcdir)/include
AM_CXXFLAGS = $(DEBUG_CXXFLAGS)

libgamearchive_la_LDFLAGS = $(AM_LDFLAGS) -release @VERSION@ -version-info 0
libgamearchive_la_LIBADD = $(BOOST_SYSTEM_LIBS) $(BOOST_FILESYSTEM_LIBS)
