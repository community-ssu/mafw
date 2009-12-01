#
# Makefile.am for MAFW library.
#
# Author: Visa Smolander <visa.smolander@nokia.com>
#
# Copyright (C) 2007, 2008, 2009 Nokia. All rights reserved.

compile_these			= \
				  test-registry \
				  test-mafw-init \
				  test-mafw-log \
				  test-renderer \
				  test-mafw-filter \
				  test-mafw-extension \
				  test-metadata \
				  test-serialization \
				  test-playlist \
				  test-db \
				  test-defaults \
				  stress-miwmd

check_PROGRAMS			= $(compile_these)
noinst_PROGRAMS			= $(compile_these)
TESTS_ENVIRONMENT		= top_srcdir=$(top_srcdir)
TESTS				= \
				  test-registry \
				  test-mafw-init \
				  test-mafw-log \
				  test-renderer \
				  test-mafw-filter \
				  test-mafw-extension \
				  test-metadata \
				  test-serialization \
				  test-playlist \
				  test-db \
				  test-defaults

EXTRA_DIST			= test.suppressions

# libtool won't create a .so file if you only list it in noinst_LTLIBRARIES...
lib_LTLIBRARIES			= test-extension.la

AM_CFLAGS			= $(_CFLAGS)
AM_CPPFLAGS			= $(GLIB_CFLAGS) $(CHECK_CFLAGS) \
				  -I$(top_srcdir) -I$(top_srcdir)/checkmore
AM_LDFLAGS			= $(_LDFLAGS)
LDADD				= $(top_builddir)/libmafw/libmafw.la \
				  $(top_builddir)/checkmore/libcheckmore.la \
				  $(GLIB_LIBS) $(CHECK_LIBS)

test_extension_la_SOURCES	= test-extension.c nopsource.c
# Trick automake into prefixing nopsource.o, otherwise we are libfooled.
test_extension_la_CFLAGS	= $(AM_CFLAGS)
test_extension_la_LIBADD	= $(GLIB_LIBS) \
				  $(top_builddir)/libmafw/libmafw.la
test_extension_la_LDFLAGS	= $(AM_LDFLAGS) -module -avoid-version
test_registry_SOURCES		= noprenderer.c nopsource.c test-registry.c
test_renderer_SOURCES		= nopsource.c test-renderer.c

# It's enough to list these .h files in noinst_HEADERS to include them in the
# distribution.  For those worrying about dependencies, my experience is:
# i)  if you --disable-dependency-tracking, it won't even help if they are
#     listed in *_SOURCES.
# ii) if you --enable-dependency-tracking, automake will have gcc extract
#     dependencies, thus again making listing them in *_SOURCES irrelevant.
noinst_HEADERS			= nopsource.h noprenderer.h

CLEANFILES			= $(noinst_PROGRAMS) \
				  *.gcda *.gcno core* test.log *.db
MAINTAINERCLEANFILES		= Makefile.in

clean-local:
	-rm -rf mime;

# Runs valgrind on tests.
vg: $(TESTS)
	for test in $^; do \
		G_DEBUG="always-malloc" \
		libtool --mode=execute valgrind -q \
			--leak-check=full --suppressions=test.suppressions \
			./$$test 2> vglog.$$test; \
	done
	-rm -f vgcore.*;

# Prevent test-extension.so (actually, anything) from getting installed.
install:;
