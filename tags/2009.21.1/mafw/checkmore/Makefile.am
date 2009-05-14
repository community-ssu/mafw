#
# Makefile.am for MAFW library.
#
# Author: Visa Smolander <visa.smolander@nokia.com>
#
# Copyright (C) 2007, 2008, 2009 Nokia. All rights reserved.

lib_LTLIBRARIES			= libcheckmore.la

libcheckmore_la_SOURCES		= checkmore.c
libcheckmore_la_CPPFLAGS	= $(GLIB_CFLAGS) $(CHECK_CFLAGS) \
				  -I$(top_srcdir)
libcheckmore_la_LDFLAGS		= $(_LDFLAGS)
libcheckmore_la_LIBADD		= $(GLIB_LIBS) -lcheck_pic

headersdir			= $(includedir)
headers_HEADERS			= checkmore.h

pkgconfigdir			= $(libdir)/pkgconfig
pkgconfig_DATA			= checkmore.pc
noinst_DATA			= checkmore-uninstalled.pc
EXTRA_DIST			= checkmore.pc.in checkmore-uninstalled.pc.in
MAINTAINERCLEANFILES		= Makefile.in
