SUBDIRS = doc po src

pkgconfigdir = $(datadir)/pkgconfig
pkgconfig_DATA = libuio.pc

DEBIAN_FILES = debian/changelog debian/compat debian/control debian/copyright \
	debian/docs debian/gbp.conf debian/libuio-dev.install \
	debian/libuio2.install debian/libuio2.symbols debian/lsuio.dirs \
	debian/lsuio.install debian/lsuio.manpages debian/rules \
	debian/source/format

EXTRA_DIST = libuio.pc.in libuio-uninstalled.pc.in ChangeLog-from-git \
	doc/lsuio.texi $(DEBIAN_FILES)

DISTCLEANFILES = *~

MAINTAINERCLEANFILES = ABOUT-NLS aclocal.m4 compile config.guess config.h.in \
	config.rpath config.sub configure depcomp INSTALL install-sh \
	ltmain.sh m4/* Makefile.in missing texinfo.tex ChangeLog \
	po/Makefile.in.in po/Makevars.template po/Rules-quot po/*.header \
	po/libuio.pot po/*.sed po/*.sin

ACLOCAL_AMFLAGS = -I m4

# ax_doxygen
@DX_RULES@
MOSTLYCLEANFILES = $(DX_CLEANFILES)

dist-hook:
	$(top_srcdir)/ChangeLog-from-git $(top_srcdir)
