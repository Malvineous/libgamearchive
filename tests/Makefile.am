check_PROGRAMS = tests

tests_SOURCES = tests.cpp test-streamMove.cpp test-substream.cpp test-segmented_stream.cpp \
	test-fmt-grp-duke3d.cpp test-fmt-vol-cosmo.cpp test-fmt-pod-tv.cpp

EXTRA_tests_SOURCES = tests.hpp test-archive.hpp

TESTS = tests

AM_CPPFLAGS = $(BOOST_CPPFLAGS) -I $(top_srcdir)/include
AM_LDFLAGS = $(BOOST_SYSTEM_LIBS) $(BOOST_FILESYSTEM_LIBS) $(BOOST_PROGRAM_OPTIONS_LIBS) $(BOOST_UNIT_TEST_FRAMEWORK_LIBS) $(top_builddir)/src/libgamearchive.la
