bin_PROGRAMS = lsuio readuio

lsuio_SOURCES = lsuio.c
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
