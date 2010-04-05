AM_CFLAGS = -g

INCLUDES = -I$(top_srcdir)/include

bin_PROGRAMS = tests
tests_SOURCES = tests.cpp test-streamMove.cpp test-substream.cpp test-segmented_stream.cpp test-fmt-grp-duke3d.cpp
tests_CPPFLAGS = $(BOOST_CPPFLAGS)
tests_LDFLAGS = $(BOOST_SYSTEM_LDFLAGS) $(BOOST_FILESYSTEM_LDFLAGS) $(BOOST_PROGRAM_OPTIONS_LDFLAGS) $(BOOST_UNIT_TEST_FRAMEWORK_LDFLAGS)
tests_LDADD = $(BOOST_SYSTEM_LIBS) $(BOOST_FILESYSTEM_LIBS) $(BOOST_PROGRAM_OPTIONS_LIBS) $(BOOST_UNIT_TEST_FRAMEWORK_LIBS) -L../src/ -lgamearchive
