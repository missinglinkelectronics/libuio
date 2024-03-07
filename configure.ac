dnl Every other copy of the package version number gets its value from here
AC_INIT(libuio, 0.3.0, https://github.com/missinglinkelectronics/libuio/issues)
AM_INIT_AUTOMAKE

AM_CONFIG_HEADER(config.h)

AC_SUBST(VERSION)

ISODATE=`date +%Y-%m-%d`
AC_SUBST(ISODATE)

AC_CANONICAL_HOST
AC_CANONICAL_BUILD
AC_LANG([C])

dnl Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_INSTALL
AC_PROG_LIBTOOL

dnl Checks for libraries.

dnl Checks for header files.
AC_CHECK_HEADER(argp.h,,AC_MSG_ERROR(Cannot continue: argp.h not found))

dnl Checks for typedefs, structures, and compiler characteristics.

dnl Checks for library functions.

dnl pkg-conf section
AC_ARG_WITH([glib], AS_HELP_STRING([--without-glib], [Do not use glib-2.0]))

AS_IF([test "x$with_glib" != "xno"], [
   PKG_CHECK_MODULES(PKGCONF, glib-2.0)
   AC_SUBST(PKGCONF_CFLAGS)
   AC_SUBST(PKGCONF_LIBS)
   AC_DEFINE([USE_GLIB], [1], [use glib-2.0])
])

dnl i18n makros.
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.17])

dnl with or without -Werror
AC_ARG_WITH([werror],
            [AS_HELP_STRING([--without-werror],
            [compile with -Werror @<:@default=with@:>@])],
            [LIBUIO_WERROR=""],
            [LIBUIO_WERROR="-Werror"])

AC_SUBST(LIBUIO_WERROR)

dnl last but not least
AC_OUTPUT([Makefile
	libuio.dox
	libuio-uninstalled.pc
	libuio.pc
	po/Makefile.in])
