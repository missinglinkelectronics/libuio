pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libuio.pc

bin_PROGRAMS = lsuio
lsuio_SOURCES = lsuio.c lsuio.1
lsuio_CFLAGS = -W -Wall
lsuio_LDADD = libuio.la

lib_LTLIBRARIES = libuio.la
libuio_la_SOURCES = base.c helper.c irq.c mem.c libuio.h libuio_internal.h
libuio_la_CFLAGS = -O2 -Wall -Wextra $(LIBUIO_WERROR)
include_HEADERS = libuio.h

# 1) If the library source code has changed at all since the last update, then
#    increment revision ("c:r:a" becomes "c:r+1:a").
# 2) If any interfaces have been added, removed, or changed since the last
#    update, increment current, and set revision to 0.
# 3) If any interfaces have been added since the last public release,
#    then increment age.
# 4) If any interfaces have been removed or changed since the last public
#    release, then set age to 0. 

libuio_la_LDFLAGS = -version-info 1:0:0

EXTRA_DIST = libuio.pc.in libuio-uninstalled.pc.in ChangeLog-from-git \
	lsuio.texinfo

info_TEXINFOS = lsuio.texinfo
lsuio_TEXINFOS = lgpl21.texinfo

man_MANS = lsuio.1

DISTCLEANFILES = *~

MAINTAINERCLEANFILES = aclocal.m4 compile config.guess config.h.in \
	config.sub configure depcomp INSTALL install-sh ltmain.sh m4/* \
	Makefile.in missing texinfo.tex ChangeLog

ACLOCAL_AMFLAGS = -I m4

dist-hook:
	$(top_srcdir)/ChangeLog-from-git $(top_srcdir)

doc: libuio.dox $(libuio_la_SOURCES)
	doxygen libuio.dox
