lib_LTLIBRARIES = libgamearchive.la

libgamearchive_la_SOURCES = main.cpp archive.cpp subdirarchive.cpp \
	segmented_stream.cpp substream.cpp iostream_helpers.cpp \
	fatarchive.cpp fmt-grp-duke3d.cpp fmt-vol-cosmo.cpp fmt-pod-tv.cpp \
	fmt-bnk-harry.cpp fmt-res-stellar7.cpp fmt-hog-descent.cpp

EXTRA_libgamearchive_la_SOURCES = debug.hpp byteorder.h subdirarchive.hpp \
	segmented_stream.hpp substream.hpp iostream_helpers.hpp \
	fatarchive.hpp fmt-grp-duke3d.hpp fmt-vol-cosmo.hpp fmt-pod-tv.hpp \
	fmt-bnk-harry.hpp fmt-res-stellar7.hpp fmt-hog-descent.hpp

AM_CPPFLAGS = $(DEBUG_CPPFLAGS) $(BOOST_CPPFLAGS) -I $(top_srcdir)/include
AM_CXXFLAGS = $(DEBUG_CXXFLAGS)

libgamearchive_la_LDFLAGS = $(AM_LDFLAGS) -release @VERSION@ -version-info 0
libgamearchive_la_LIBADD = $(BOOST_SYSTEM_LIBS) $(BOOST_FILESYSTEM_LIBS)
