pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libUIO.pc

lib_LTLIBRARIES = libUIO.la
libUIO_la_SOURCES = libUIO.c libUIO.h
libUIO_la_CFLAGS = -O2 -Wall -Wextra $(LIBUIO_WERROR)
include_HEADERS = libUIO.h

# 1) If the library source code has changed at all since the last update, then
#    increment revision ("c:r:a" becomes "c:r+1:a").
# 2) If any interfaces have been added, removed, or changed since the last
#    update, increment current, and set revision to 0.
# 3) If any interfaces have been added since the last public release,
#    then increment age.
# 4) If any interfaces have been removed or changed since the last public
#    release, then set age to 0. 

libUIO_la_LDFLAGS = -version-info 0:0:0

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
