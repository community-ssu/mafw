#
# Makefile.am for MAFW library.
#
# Author: Visa Smolander <visa.smolander@nokia.com>
#
# Copyright (C) 2007, 2008, 2009 Nokia. All rights reserved.

ACLOCAL_AMFLAGS			= -I m4
SUBDIRS				= libmafw checkmore
if ENABLE_GTK_DOC
SUBDIRS				+= doc
endif
#if ENABLE_TESTS
#SUBDIRS				+= tests
#endif

xdgmimedir			= $(datadir)/mime/packages
xdgmime_DATA			= mafw.xml
pkgconfigdir			= $(libdir)/pkgconfig
pkgconfig_DATA			= mafw.pc
noinst_DATA			= mafw-uninstalled.pc
EXTRA_DIST			= mafw.pc.in mafw-uninstalled.pc.in mafw.xml

DISTCHECK_CONFIGURE_FLAGS	= --enable-gtk-doc

MAINTAINERCLEANFILES		= aclocal.m4 compile config.guess config.sub \
				  configure depcomp install-sh ltmain.sh \
				  Makefile.in missing config.h.in

# Keep the mime database up-to-date.
install-data-hook:
	$(UPDATEMIME) "$(DESTDIR)$(datadir)/mime";
	$(UPDATEDESKTOP);
uninstall-hook:
	$(UPDATEMIME) "$(DESTDIR)$(datadir)/mime";
	$(UPDATEDESKTOP);
# We are not supposed to delete the whole directory on uninstall.
distuninstallcheck_listfiles	= find . \
				  -name "$(DESTDIR)$(datadir)/mime" -prune \
				  -o -type -f -print

maintainer-clean-local:
	rm -rf build-aux doc/html-upload

if ENABLE_COVERAGE
LCOV_DATA_DIR			= lcov-data
LCOV_DATA_FILE			= lcov.info

distclean-local:
	-rm -rf $(LCOV_DATA_DIR)

lcov-zero-counters:
	$(LCOV) -z -d .

lcov:
	-mkdir -p $(LCOV_DATA_DIR)
	$(LCOV) -c -d . -o $(LCOV_DATA_DIR)/$(LCOV_DATA_FILE)
	genhtml -s $(LCOV_DATA_DIR)/$(LCOV_DATA_FILE) -o $(LCOV_DATA_DIR) --show-details --legend
	@echo
	@echo "Please, have a look on ./$(LCOV_DATA_DIR)/index.html for coverage statistics"
	@echo
endif
