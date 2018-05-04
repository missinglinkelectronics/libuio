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
#define g_print	printf
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

static error_t parse_opt (int key, char *arg, struct argp_state *state);
static void show_version (FILE *stream, struct argp_state *state);

/* argp option keys */
enum {DUMMY_KEY = 129};

/* Option flags and variables.  These are initialized in parse_opt.  */

int want_verbose;		/* --verbose */
int want_access;		/* --access */

static struct argp_option options [] =
{
	{"access", 'a', NULL, 0, N_("open and close all devices"), 0},
	{"verbose", 'v', NULL, 0, N_("Print more information"), 0},
	{NULL, 0, NULL, 0, NULL, 0}
};

/* The argp functions examine these global variables.  */
const char *argp_program_bug_address = "https://github.com/linutronix/libuio/issues";
void (*argp_program_version_hook) (FILE *, struct argp_state *) = show_version;

static struct argp argp =
{
	options, parse_opt, NULL,
	N_("list uio devices."),
	NULL, NULL, NULL
};

int main (int argc, char **argv)
{
	struct uio_info_t **uio_list, *info;
	char **attr, **tmp;
	int i;

	textdomain (PACKAGE);
	argp_parse (&argp, argc, argv, 0, NULL, NULL);

	uio_list = uio_find_devices ();
	if (!uio_list)
	{
		g_print (_("No UIO devices found\n"));
		return 1;
	}

	for (;*uio_list; uio_list++)
	{
		info = *uio_list;
		g_print (_("Name   : %s\n"), uio_get_name (info));
		g_print (_("Version: %s\n"), uio_get_version (info));
		g_print (_("DevId  : %d:%d\n"), uio_get_major (info),
			uio_get_minor (info));
		g_print (_("DevNode: %s\n"), uio_get_devname (info));

		if (want_verbose)
		{
			g_print (_("Map    :\n"));
			for (i = 0; i < uio_get_maxmap (info); i++)
			{
				g_print (_("%3d addr: 0x%08lx\n"), i,
					uio_get_mem_addr (info, i));
				g_print (_("    size: 0x%08zx\n"),
					uio_get_mem_size (info, i));
				g_print (_("  offset: 0x%08zx\n"),
					uio_get_offset (info, i));
			}
		}
		g_print (_("\n"));

		g_print (_("Attr.  :\n"));
		attr = uio_list_attr (info);
		for (tmp = attr; tmp && *tmp; tmp++)
		{
			g_print (_("  %s\n"), *tmp);
			if (want_verbose)
				if (!strcmp (*tmp, "name") ||
				    !strcmp (*tmp, "version"))
					g_print (_("    %s\n"),
						uio_get_attr (info, *tmp));
			free (*tmp);
		}
		free (attr);

		if (want_access)
		{
			g_print (_("open : "));
			if (!uio_open (info))
				g_print (_("OK\n"));
			else
				g_print (_("failed\n"));

			g_print (_("close: "));
			if (!uio_close (info))
				g_print (_("OK\n"));
			else
				g_print (_("failed\n"));
		}
	}

	free (uio_list);

	return 0;
}

/* Parse a single option.  */
static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
	switch (key)
	{
	case ARGP_KEY_INIT:
		/* Set up default values.  */
		want_verbose = 0;
		want_access = 0;
		break;

	case 'a':			/* --access */
		want_access = 1;
		break;

	case 'v':			/* --verbose */
		want_verbose = 1;
		break;

	case ARGP_KEY_ARG:		/* [FILE]... */
		/* TODO: Do something with ARG, or remove this case and make
         main give argp_parse a non-NULL fifth argument.  */
		break;

	default:
		return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

/* Show the version number and copyright information.  */
static void show_version (FILE *stream, struct argp_state *state)
{
	(void) state;

	fputs (PACKAGE" "VERSION"\n", stream);
	fprintf (stream, _("Written by %s.\n\n"), "Benedikt Spranger");
	fprintf (stream, _("Copyright (C) %s %s\n"), "2011", "Benedikt Spranger");
	fputs(_("\
This program is free software; you may redistribute it under the terms of\n\
the GNU General Public License.  This program has absolutely no warranty.\n"),
	      stream);
}
