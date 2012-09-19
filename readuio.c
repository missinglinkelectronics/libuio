/*
 * lsuio - libuio example application.
 *
 * Copyright (C) 2011 Benedikt Spranger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 */

#include <argp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>

#include "config.h"
#include "libuio.h"

#ifdef USE_GLIB
#include <glib.h>
#else
#define g_print printf
#endif

#define EXIT_FAILURE 1

#if ENABLE_NLS
# include <libintl.h>
# define _(Text) gettext (Text)
#else
# define textdomain(Domain)
# define _(Text) Text
#endif
#define N_(Text) Text

void usage (char *name);

int main (int argc, char **argv)
{
	struct uio_info_t *uio;
	unsigned long offset;
	uint32_t val;
	int i, ret;

	if (argc < 3)
	{
		usage (argv [0]);
		return -1;
	}

	textdomain (PACKAGE);

	uio = uio_find_by_uio_name (argv [1]);
	if (!uio)
	{
		printf (_("could not find UIO device >%s<.\n"), argv [1]);
		return -1;
	}

	ret = uio_open (uio);
	if (ret)
	{
		printf (_("could not open UIO device >%s<: %s\n"),
			argv [1], strerror (errno));
		return -1;
	}

	for (i = 2; i < argc; i++)
	{
		offset = strtoul (argv [i], NULL, 0);
		ret = uio_read32 (uio, 0, offset, &val);
		if (ret)
		{
			printf (_("could not read at offset %ld: %s\n"),
				offset, strerror (errno));
		}
		else
			printf ("%ld: %d\n", offset, val);
	}

	uio_close (uio);

	return 0;
}

void usage (char *name)
{
	printf (_("usage: %s <uio name> <offset> [<offset>...]\n"), name);
}
