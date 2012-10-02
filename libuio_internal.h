/*
 * libuio - UserspaceIO helper library
 *
 * Copyright (C) 2011 Benedikt Spranger
 * based on libUIO by Hans J. Koch
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA  02110-1301 USA
 */

#ifndef LIBUIO_INTERNAL_H
#define LIBUIO_INTERNAL_H

#include "libuio.h"

#ifdef USE_GLIB
#include <glib.h>
#else
#define g_strerror	strerror
#define g_warning(...)	fprintf(stderr, __VA_ARGS__)
#endif

#if ENABLE_NLS
# include <libintl.h>
# define _(Text) gettext (Text)
#else
# define textdomain(Domain)
# define _(Text) Text
#endif
#define N_(Text) Text

struct uio_map_t {
	unsigned long addr;
	size_t size;
	size_t offset;
	char *name;
	void *map;
};

struct uio_info_t {
	char *path;
	char *name;
	char *version;
	struct uio_map_t *maps;
	char *devname;
	dev_t devid;
	int maxmap;
	int fd;
};

struct uio_info_t* create_uio_info (char *dir, char* name);
char *first_line_from_file (char *filename);

#endif /* LIBUIO_INTERNAL_H */
