SUBDIRS = po

pkgconfigdir = $(datadir)/pkgconfig
pkgconfig_DATA = libuio.pc

bin_PROGRAMS = lsuio readuio

lsuio_SOURCES = lsuio.c lsuio.1
lsuio_CFLAGS = -W -Wall @PKGCONF_CFLAGS@
lsuio_LDADD = libuio.la @PKGCONF_LIBS@

readuio_SOURCES = readuio.c
readuio_CFLAGS = -W -Wall @PKGCONF_CFLAGS@
readuio_LDADD = libuio.la @PKGCONF_LIBS@

lib_LTLIBRARIES = libuio.la
libuio_la_SOURCES = base.c helper.c irq.c mem.c attr.c \
	libuio.h libuio_internal.h
libuio_la_CFLAGS = -O2 -Wall -Wextra $(LIBUIO_WERROR) @PKGCONF_CFLAGS@ \
	-DG_LOG_DOMAIN=\"libuio\"
libuio_la_LIBADD = @PKGCONF_LIBS@
include_HEADERS = libuio.h

# 1) If the library source code has changed at all since the last update, then
#    increment revision ("c:r:a" becomes "c:r+1:a").
# 2) If any interfaces have been added, removed, or changed since the last
#    update, increment current, and set revision to 0.
# 3) If any interfaces have been added since the last public release,
#    then increment age.
# 4) If any interfaces have been removed or changed since the last public
#    release, then set age to 0. 

libuio_la_LDFLAGS = -version-info 4:0:2

EXTRA_DIST = libuio.pc.in libuio-uninstalled.pc.in ChangeLog-from-git \
	lsuio.texi

info_TEXINFOS = lsuio.texi
lsuio_TEXINFOS = lgpl21.texi

man_MANS = lsuio.1

DISTCLEANFILES = *~

MAINTAINERCLEANFILES = ABOUT-NLS aclocal.m4 compile config.guess config.h.in \
	config.rpath config.sub configure depcomp INSTALL install-sh \
	ltmain.sh m4/* Makefile.in missing texinfo.tex ChangeLog \
	po/Makefile.in.in po/Makevars.template po/Rules-quot po/*.header \
	po/libuio.pot po/*.sed po/*.sin

ACLOCAL_AMFLAGS = -I m4

dist-hook:
	$(top_srcdir)/ChangeLog-from-git $(top_srcdir)

doc: libuio.dox $(libuio_la_SOURCES)
	doxygen libuio.dox
