#
# Makefile.am for MAFW library.
#
# Author: Visa Smolander <visa.smolander@nokia.com>
#
# Copyright (C) 2007, 2008, 2009 Nokia. All rights reserved.

lib_LTLIBRARIES		= libmafw.la

libmafw_la_CFLAGS	= $(_CFLAGS)
libmafw_la_CPPFLAGS	= $(GLIB_CFLAGS) $(SQLITE_CFLAGS) \
			  -I$(top_srcdir) \
			  -DG_LOG_DOMAIN=\"mafw\" \
			  -DMAFW_PREFIX=\"$(prefix)\"
libmafw_la_LIBADD	= $(SQLITE_LIBS) $(GLIB_LIBS)
libmafw_la_LDFLAGS	= $(_LDFLAGS) -version-info 0:0:0

BUILT_SOURCES		= mafw-marshal.c \
			  mafw-marshal.h

libmafw_la_SOURCES	= $(BUILT_SOURCES) \
			  mafw-playlist.c \
			  mafw-extension.c \
			  mafw-renderer.c \
			  mafw-source.c \
			  mafw-registry.c \
			  mafw-log.c \
			  mafw-filter.c \
			  mafw-metadata.c \
			  mafw-callbas.c \
			  mafw-uri-source.c \
			  mafw-db.c \
			  mafw-metadata-serializer.c

# The generated C source doesn't #include the header which contains
# the function prototypes required by -Wmissing-declarations.
mafw-marshal.c: mafw-marshal.list
	( \
		echo '#include "mafw-marshal.h"'; \
		$(GLIB_GENMARSHAL) --prefix=mafw_marshal --body $^ \
	) > $@

mafw-marshal.h: mafw-marshal.list
	$(GLIB_GENMARSHAL) --prefix=mafw_marshal --header \
		$^ > $@


libmafwincdir		= $(includedir)/mafw-1.0/libmafw
libmafwinc_HEADERS	= mafw-playlist.h \
			  mafw-extension.h \
			  mafw.h \
			  mafw-renderer.h \
			  mafw-source.h \
			  mafw-metadata.h \
			  mafw-callbas.h \
			  mafw-uri-source.h \
			  mafw-registry.h \
			  mafw-log.h \
			  mafw-filter.h \
			  mafw-errors.h \
			  mafw-property.h \
			  mafw-db.h \
			  mafw-metadata-serializer.h

EXTRA_DIST		= mafw-marshal.list
CLEANFILES		= $(BUILT_SOURCES) *.gcno *.gcda
MAINTAINERCLEANFILES	= Makefile.in
