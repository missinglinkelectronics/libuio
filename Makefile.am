pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libUIO.pc

lib_LTLIBRARIES = libUIO.la
libUIO_la_SOURCES = libUIO.c libUIO.h
libUIO_la_CFLAGS = -O2 -Wall -Wextra $(LIBUIO_WERROR)
include_HEADERS = libUIO.h

EXTRA_DIST = libUIO.pc.in libUIO-uninstalled.pc.in ChangeLog-from-git

DISTCLEANFILES = *~

MAINTAINERCLEANFILES = aclocal.m4 compile config.guess config.h.in \
	config.sub configure depcomp INSTALL install-sh ltmain.sh m4/* \
	Makefile.in missing

ACLOCAL_AMFLAGS = -I m4

dist-hook:
	$(top_srcdir)/ChangeLog-from-git $(top_srcdir)

doc: libUIO.dox $(libUIO_la_SOURCES)
	doxygen libUIO.dox
