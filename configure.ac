## website: http://www.dwheeler.com/autotools/
# This is David A. Wheeler's template for configure.ac, showing
# some suggested options and setups.

# Process this file with autoconf to produce a configure script.

# Initialize autoconf.
AC_INIT([dian_night], [0.0.1])
# Force autoconf to be at least this version number:
AC_PREREQ([2.68])
# Safety check - list a source file that wouldn't be in other directories:
AC_CONFIG_SRCDIR([src/client.c])
# Put configuration results here, so we can easily #include them:
AC_CONFIG_HEADERS([config.h])
# Put autotools auxiliary files in subdirectories to reduce clutter:
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])

# Enable "automake" to simplify creating makefiles:
AM_INIT_AUTOMAKE([subdir-objects -Wall -Werror])
AC_CONFIG_FILES([Makefile])

# Checks for programs, e.g., AC_PROG_CC
AC_PROG_CC
# Checks for libraries.

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

# Do final output.
AC_OUTPUT
