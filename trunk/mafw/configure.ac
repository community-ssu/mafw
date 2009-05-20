#
# configure.ac for MAFW library
# 
# Author: Visa Smolander <visa.smolander@nokia.com>
#
# Copyright (C) 2007, 2008, 2009 Nokia. All rights reserved.

AC_PREREQ([2.53])
AC_INIT([mafw], [0.1.2009.21-3])

AC_CONFIG_SRCDIR([libmafw/mafw.h])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_AUX_DIR([build-aux])

AM_INIT_AUTOMAKE([foreign])
AM_MAINTAINER_MODE

AC_DISABLE_STATIC

dnl Prevent AC_PROG_CC adding '-g -O2' to CFLAGS.
SAVEDCFLAGS="$CFLAGS"
AC_PROG_CC
if test "x$GCC" = xyes; then
	CFLAGS="$SAVEDCFLAGS"
fi

AC_PROG_LIBTOOL
AC_PROG_INSTALL

# DISABLED_BY_DEFAULT(NAME, DESCRIPTION)
# ---------------------------------
# Creates a new --enable-* option, with default value `no'.
AC_DEFUN([DISABLED_BY_DEFAULT], [dnl
         AC_ARG_ENABLE([$1], AS_HELP_STRING([--enable-$1], [$2]), [],[dnl
         m4_bpatsubst([enable_$1], [[^0-9a-z]], [_])=no])dnl
])# DISABLED_BY_DEFAULT

# ENABLED_BY_DEFAULT(NAME, DESCRIPTION)
# ---------------------------------
# Creates a new --disable-* option, with default value `yes'.
AC_DEFUN([ENABLED_BY_DEFAULT], [dnl
         AC_ARG_ENABLE([$1], AS_HELP_STRING([--disable-$1], [$2]), [],[dnl
         m4_bpatsubst([enable_$1], [[^0-9a-z]], [_])=yes])dnl
])# ENABLED_BY_DEFAULT

dnl Prerequisites.

AM_PATH_GLIB_2_0(2.15.0, [], [], [gobject gmodule])
PKG_CHECK_MODULES(SQLITE,  [sqlite3])

dnl Checkmore prerequisite.

PKG_CHECK_MODULES(CHECK, [check >= 0.9.4])

GLIB_PREFIX="`pkg-config --variable=prefix glib-2.0`"
AC_SUBST(GLIB_PREFIX)

dnl Default compile flags.  (NOTE: CFLAGS is reserved for the user!)

AC_SUBST([_CFLAGS])
AC_SUBST([_LDFLAGS])
_CFLAGS="-Wall -Wmissing-prototypes -Wstrict-prototypes -Wmissing-declarations"
_CFLAGS="$_CFLAGS -ggdb3"

dnl Configure-time options.

dnl Debugging.
DISABLED_BY_DEFAULT([debug], [compile with debug flags and extra output])
if test "x$enable_debug" = xyes; then
	AC_DEFINE([MAFW_DEBUG], [1], [Enable debugging related parts.])
	_CFLAGS="$_CFLAGS -Werror -O0"
else
	AC_DEFINE([G_DEBUG_DISABLE], [1], [Disable g_debug() calls.])
	_CFLAGS="$_CFLAGS -O2"
fi

dnl Tests.
#ENABLED_BY_DEFAULT([tests], [disable unit tests])
AM_CONDITIONAL(ENABLE_TESTS,
               [test "x$enable_tests" = xyes])

dnl Tracing.
DISABLED_BY_DEFAULT([tracing], [enable function instrumentation (tracing)])
if test "x$enable_tracing" = xyes; then
	_CFLAGS="$_CFLAGS -finstrument-functions -rdynamic"
fi

dnl Coverage.
DISABLED_BY_DEFAULT([coverage], [enable coverage data generation (gcov)])
if test "x$enable_coverage" = xyes; then
	AC_PATH_PROG(LCOV, [lcov], [lcov])
	if test "x$LCOV" = x; then
		echo You need to install lcov to get actual reports!
		echo See http://ltp.sf.net/coverage/lcov.php
	fi
	if test "x$SBOX_USE_CCACHE" == xyes; then
		AC_MSG_ERROR([Please set SBOX_USE_CCACHE=no to use coverage.])
	fi
	_CFLAGS="$_CFLAGS -fprofile-arcs -ftest-coverage"
	_LDFLAGS="$_LDFLAGS -g -lgcov"
fi
AM_CONDITIONAL(ENABLE_COVERAGE,
               [test "x$enable_coverage" != xno && test -n "$LCOV"])

dnl Documentation.
GTK_DOC_CHECK([1.0])

dnl shared-mime-info
AC_PATH_PROG(UPDATEMIME,    update-mime-database,    true)
AC_PATH_PROG(UPDATEDESKTOP, update-desktop-database, true)

dnl Output files.
AC_CONFIG_FILES([
        Makefile
        libmafw/Makefile
	checkmore/Makefile
	checkmore/checkmore.pc
	checkmore/checkmore-uninstalled.pc
        tests/Makefile
        doc/Makefile
        doc/version.xml
        mafw.pc
        mafw-uninstalled.pc
])

AC_OUTPUT

