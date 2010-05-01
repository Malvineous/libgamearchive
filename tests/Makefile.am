check_PROGRAMS = tests

tests_SOURCES = tests.cpp
tests_SOURCES += test-byteorder.cpp
tests_SOURCES += test-streamMove.cpp
tests_SOURCES += test-substream.cpp
tests_SOURCES += test-segmented_stream.cpp
tests_SOURCES += test-fmt-grp-duke3d.cpp
tests_SOURCES += test-fmt-vol-cosmo.cpp
tests_SOURCES += test-fmt-pod-tv.cpp
tests_SOURCES += test-fmt-bnk-harry.cpp
tests_SOURCES += test-fmt-res-stellar7.cpp
tests_SOURCES += test-fmt-hog-descent.cpp

EXTRA_tests_SOURCES = tests.hpp test-archive.hpp

TESTS = tests

AM_CPPFLAGS = $(BOOST_CPPFLAGS) -I $(top_srcdir)/include
AM_LDFLAGS = $(BOOST_SYSTEM_LIBS) $(BOOST_FILESYSTEM_LIBS) $(BOOST_PROGRAM_OPTIONS_LIBS) $(BOOST_UNIT_TEST_FRAMEWORK_LIBS) $(top_builddir)/src/libgamearchive.la
