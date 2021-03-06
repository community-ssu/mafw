#
# configure.ac for MAFW uPnP source library
# 
# Author: Visa Smolander <visa.smolander@nokia.com>
#
# Copyright (C) 2008, 2009 Nokia. All rights reserved.

AC_PREREQ([2.53])
AC_INIT([mafw-upnp-source], [0.2.2010.07-2])

AC_CONFIG_SRCDIR([upnp-source/mafw-upnp-source.h])
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

PKG_CHECK_MODULES(DEPS,
	[
		gobject-2.0  >= 2.12
		gthread-2.0
		mafw 	     >= 0.1
		gupnp-1.0    >= 0.13
		gupnp-av-1.0 >= 0.5
	]
)

plugindir=`$PKG_CONFIG --variable=plugindir mafw`
AC_SUBST(plugindir)

dnl Check for glib-genmarshal.

GLIB_GENMARSHAL=`pkg-config --variable=glib_genmarshal glib-2.0`
AC_SUBST(GLIB_GENMARSHAL)

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
	_CFLAGS="$_CFLAGS -O0 -Werror"
else
	AC_DEFINE([G_DEBUG_DISABLE], [1], [Disable g_debug() calls.])
	_CFLAGS="$_CFLAGS -O2"
fi
AS_IF([test "x$enable_debug" = xyes],
	[AC_DEFINE([MAFW_DEBUG], [1], [Enable extra debug messages])
	 CFLAGS="$CFLAGS -Werror -O0 -ggdb3"],
	[AC_DEFINE([G_DEBUG_DISABLE], [1], [Disable g_debug calls])
	 CFLAGS="$CFLAGS -O2"])


dnl Tests.
DISABLED_BY_DEFAULT([tests], [disable unit tests])
if test "x$enable_tests" = xyes; then
	PKG_CHECK_MODULES(CHECKMORE, [checkmore, check >= 0.9.4])
	if test -z "$CHECKMORE_LIBS"; then
		AC_MSG_WARN([checkmore is needed for unit tests!])
	fi
fi
AM_CONDITIONAL(ENABLE_TESTS,
               [test "x$enable_tests" = xyes && test -n "$CHECKMORE_LIBS"])

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

dnl Samples

AC_ARG_ENABLE([samples],
        AS_HELP_STRING([--enable-samples],
                       [Enable sample apps (default = no)]),
                      ,
                     [enable_samples=no])
AM_CONDITIONAL(ENABLE_SAMPLES, [test "x$enable_samples" = xyes])
if test x$enable_samples = xyes; then
	PKG_CHECK_MODULES(MAFW_SHARED, [mafw-shared])
fi

dnl Output files.

AC_CONFIG_FILES([
        Makefile
        upnp-source/Makefile
        tests/Makefile
	debian/mafw-upnp-source.install
        mafw-upnp-source-uninstalled.pc
])

AC_OUTPUT
