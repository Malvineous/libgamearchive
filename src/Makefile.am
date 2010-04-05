lib_LTLIBRARIES = libgamearchive.la
libgamearchive_la_SOURCES = main.cpp archive.cpp segmented_stream.cpp \
	substream.cpp iostream_helpers.cpp \
	fatarchive.cpp fmt-grp-duke3d.cpp
libgamearchive_la_CPPFLAGS = $(BOOST_CPPFLAGS) -I $(top_srcdir)/include
libgamearchive_la_LDFLAGS = $(BOOST_SYSTEM_LDFLAGS) $(BOOST_FILESYSTEM_LDFLAGS)
libgamearchive_la_LIBADD = $(BOOST_SYSTEM_LIBS) $(BOOST_FILESYSTEM_LIBS)
