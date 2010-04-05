#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT([libgamearchive], [0.1], [camoto@lists.shikadi.net])
AM_INIT_AUTOMAKE([libgamearchive], [0.1])
AC_CONFIG_SRCDIR([src/main.cpp])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

# Overarching version of all libraries (used for installation dir)
AC_SUBST(camoto_release, camoto-1.0)

# Checks for programs.
AC_PROG_CXX

# Checks for libraries.
AC_PROG_LIBTOOL

BOOST_REQUIRE([1.37])
BOOST_FILESYSTEM
BOOST_PROGRAM_OPTIONS
BOOST_TEST

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_OUTPUT(Makefile src/Makefile include/Makefile include/camoto/Makefile examples/Makefile tests/Makefile doc/Makefile $PACKAGE-$VERSION.pc)
