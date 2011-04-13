dnl Every other copy of the package version number gets its value from here
AC_INIT(libuio, 0.2, john.ogness@linutronix.de)
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

dnl Checks for typedefs, structures, and compiler characteristics.

dnl Checks for library functions.

dnl with or without -Werror
AC_ARG_WITH([werror],
            [AS_HELP_STRING([--with-werror],
            [compile with -Werror @<:@default=without@:>@])],
            [LIBUIO_WERROR="-Werror"],
            [LIBUIO_WERROR=""])
AC_SUBST(CIF_WERROR)

dnl last but not least
AC_OUTPUT([Makefile
	libUIO-uninstalled.pc
	libUIO.pc])
