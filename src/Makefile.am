lib_LTLIBRARIES = libgamearchive.la

libgamearchive_la_SOURCES = main.cpp
libgamearchive_la_SOURCES += archive.cpp
libgamearchive_la_SOURCES += fatarchive.cpp
libgamearchive_la_SOURCES += fixedarchive.cpp
libgamearchive_la_SOURCES += fmt-grp-duke3d.cpp
libgamearchive_la_SOURCES += fmt-vol-cosmo.cpp
libgamearchive_la_SOURCES += fmt-pod-tv.cpp
libgamearchive_la_SOURCES += fmt-bnk-harry.cpp
libgamearchive_la_SOURCES += fmt-res-stellar7.cpp
libgamearchive_la_SOURCES += fmt-hog-descent.cpp
libgamearchive_la_SOURCES += fmt-epf-lionking.cpp
libgamearchive_la_SOURCES += fmt-dat-wacky.cpp
libgamearchive_la_SOURCES += fmt-dat-hugo.cpp
libgamearchive_la_SOURCES += fmt-exe-ccaves.cpp
libgamearchive_la_SOURCES += fmt-dat-bash.cpp
libgamearchive_la_SOURCES += fmt-dat-hocus.cpp
libgamearchive_la_SOURCES += fmt-dat-sango.cpp
libgamearchive_la_SOURCES += fmt-lbr-vinyl.cpp

EXTRA_libgamearchive_la_SOURCES = fatarchive.hpp
EXTRA_libgamearchive_la_SOURCES += fixedarchive.hpp
EXTRA_libgamearchive_la_SOURCES += fmt-grp-duke3d.hpp
EXTRA_libgamearchive_la_SOURCES += fmt-vol-cosmo.hpp
EXTRA_libgamearchive_la_SOURCES += fmt-pod-tv.hpp
EXTRA_libgamearchive_la_SOURCES += fmt-bnk-harry.hpp
EXTRA_libgamearchive_la_SOURCES += fmt-res-stellar7.hpp
EXTRA_libgamearchive_la_SOURCES += fmt-hog-descent.hpp
EXTRA_libgamearchive_la_SOURCES += fmt-epf-lionking.hpp
EXTRA_libgamearchive_la_SOURCES += fmt-dat-wacky.hpp
EXTRA_libgamearchive_la_SOURCES += fmt-dat-hugo.hpp
EXTRA_libgamearchive_la_SOURCES += fmt-exe-ccaves.hpp
EXTRA_libgamearchive_la_SOURCES += fmt-dat-bash.hpp
EXTRA_libgamearchive_la_SOURCES += fmt-dat-hocus.hpp
EXTRA_libgamearchive_la_SOURCES += fmt-dat-sango.hpp
EXTRA_libgamearchive_la_SOURCES += fmt-lbr-vinyl.hpp
EXTRA_libgamearchive_la_SOURCES += filter-bash-rle.hpp

AM_CPPFLAGS = $(DEBUG_CPPFLAGS) $(BOOST_CPPFLAGS) -I $(top_srcdir)/include $(libgamecommon_CFLAGS)
AM_CXXFLAGS = $(DEBUG_CXXFLAGS)

libgamearchive_la_LDFLAGS = $(AM_LDFLAGS) -release @VERSION@ -version-info 0
libgamearchive_la_LIBADD = $(BOOST_SYSTEM_LIBS) $(BOOST_FILESYSTEM_LIBS) $(libgamecommon_LIBS)
