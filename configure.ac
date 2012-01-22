#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.60)

# package author
AC_INIT(7colors, 0.10, slitvinov@gmail.com)
AC_CONFIG_SRCDIR([src/7colors.c])
AM_INIT_AUTOMAKE(7colors, 0.10)

AC_PROG_RANLIB

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
PKG_CHECK_MODULES([GTK], gtk+-2.0)

AC_OUTPUT(Makefile src/Makefile)
AC_OUTPUT
