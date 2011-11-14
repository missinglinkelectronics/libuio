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

#include <sys/types.h>

#include "config.h"
#include "libuio.h"

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
const char *argp_program_bug_address = "<b.spranger@linutronix.de>";
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
	int i;

	textdomain (PACKAGE);
	argp_parse (&argp, argc, argv, 0, NULL, NULL);

	uio_list = uio_find_devices ();
	if (!uio_list)
	{
		printf ("No UIO devices found\n");
		return 1;
	}

	for (;*uio_list; uio_list++)
	{
		info = *uio_list;
		printf ("Name   : %s\n", uio_get_name (info));
		printf ("Version: %s\n", uio_get_version (info));
		printf ("DevId  : %d:%d\n", uio_get_major (info),
			uio_get_minor (info));
		printf ("DevNode: %s\n", uio_get_devname (info));

		if (want_verbose)
		{
			printf ("Map    :\n");
			for (i = 0; i < uio_get_maxmap (info); i++)
			{
				printf ("%3d addr: 0x%08lx\n", i,
					uio_get_mem_addr (info, i));
				printf ("    size: 0x%08zx\n",
					uio_get_mem_size (info, i));
				printf ("  offset: 0x%08zx\n",
					uio_get_offset (info, i));
			}
		}
		printf ("\n");

		if (want_access)
		{
			printf ("open : ");
			if (!uio_open (info))
				printf ("OK\n");
			else
				printf ("failed\n");

			printf ("close: ");
			if (!uio_close (info))
				printf ("OK\n");
			else
				printf ("failed\n");
		}
	}

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

	fputs(PACKAGE" "VERSION"\n", stream);
	fprintf(stream, _("Written by %s.\n\n"), "Benedikt Spranger");
	fprintf(stream, _("Copyright (C) %s %s\n"), "2011", "Benedikt Spranger");
	fputs(_("\
This program is free software; you may redistribute it under the terms of\n\
the GNU General Public License.  This program has absolutely no warranty.\n"),
	      stream);
}
