/*
 * libuio - UserspaceIO helper library
 *
 * Copyright (C) 2011 Benedikt Spranger
 * based on libUIO by Hans J. Koch
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA  02110-1301 USA
 */

#include <dirent.h>
#include <errno.h>
#include <error.h>
#include <fcntl.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "libuio_internal.h"

/**
 * @defgroup libuio_helper generic libuio helper functions
 * @ingroup libuio
 * @brief helper functions
 * @{
 */

/**
 * read a line from a file
 * @param filename file name
 * @returns first line or NULL on failure
 */
char *first_line_from_file (char *filename)
{
	char c, *out;
	int fd, len;

	fd = open (filename, O_RDONLY);
	if (fd < 0)
	{
		perror ("open");
		return NULL;
	}

	for (len = 0; ((read (fd, &c, 1) == 1) && (c != '\n')); len++);
	lseek (fd, 0, SEEK_SET);

	out = malloc (len + 1);
	if (!out)
	{
		errno = ENOMEM;
		perror ("malloc");
		goto out;
	}

	len = read (fd, out, len);
	if (len < 0)
	{
		perror ("read");
		free (out);
		out = NULL;
	}

	out [len] = 0;
out:
	close (fd);

	return out;
}

/**
 * read device id from file
 * @param filename file name
 * @returns device id or 0 on failure
 */
dev_t devid_from_file (char *filename)
{
	FILE *fhan;
	int major, minor;
	dev_t devid = 0;

	fhan = fopen (filename, "r");
	if (!fhan)
	{
		perror ("fopen");
		goto out;
	}

	if (fscanf (fhan, "%d:%d", &major, &minor) == 2)
		devid = makedev (major, minor);

out:
	if (fhan)
		fclose (fhan);
	return devid;
}

/**
 * read a line from a file
 * @param dir map directory
 * @param maxmap available maps
 * @returns maps or NULL on failure
 */
static struct uio_map_t *scan_maps (char *dir, int *maxmap)
{
	struct uio_map_t *map = NULL;
	struct dirent **namelist;
	char *tmp, name [PATH_MAX];
	int nr, i, t = 0;

	*maxmap = 0;
	nr = scandir (dir, &namelist, 0, alphasort);
	if (nr < 0)
	{
		perror ("scandir");
		return NULL;
	}

	map = calloc (nr, sizeof (struct uio_map_t));
	if (!map)
	{
		errno = ENOMEM;
		perror ("calloc");
		goto out;
	}

	for (i = 0; i < nr; i++)
	{
		if (!strcmp (namelist [i]->d_name, ".") ||
		    !strcmp (namelist [i]->d_name, ".."))
		{
			free (namelist [i]);
			continue;
		}

		snprintf (name, sizeof (name), "%s/%s/addr",
			  dir, namelist [i]->d_name);
		tmp = first_line_from_file (name);
		map [t].addr = strtoul (tmp, NULL, 0);
		free (tmp);

		snprintf (name, sizeof (name), "%s/%s/size",
			  dir, namelist [i]->d_name);
		tmp = first_line_from_file (name);
		map [t].size = strtoul (tmp, NULL, 0);
		free (tmp);
		map [t].offset = map [t].addr & (getpagesize () - 1);
		map [t].map = MAP_FAILED;

		*maxmap = ++t;

		free (namelist [i]);
	}
out:
	free (namelist);
	return map;
}

/**
 * search device node name by major/minor
 * @param dir start in directory dir
 * @param devid major/minor
 * @param devname first matching device node name
 * @returns -1 on error, 0 on not found and 1 on success
 */
static int search_major_minor (const char *dir, dev_t devid, char **devname)
{
	struct dirent **namelist;
	struct stat stat;
	char name [PATH_MAX];
	int i, nr, ret = 0;

	if (!devname)
	{
		errno = EINVAL;
		perror ("search_major_minor");
		return -1;
	}

	nr = scandir (dir, &namelist, 0, alphasort);
	if (nr < 0)
	{
		perror ("scandir");
		return nr;
	}

	for (i = 0; i < nr; i++)
	{
		if (!strcmp (namelist [i]->d_name, ".") ||
		    !strcmp (namelist [i]->d_name, ".."))
			continue;

		snprintf (name, sizeof (name), "%s/%s",
			  dir, namelist [i]->d_name);

		ret = lstat (name, &stat);
		if (ret < 0)
		{
			perror ("lstat");
			goto out;
		}

		if (S_ISDIR (stat.st_mode))
		{
			ret = search_major_minor (name, devid, devname);
			if (ret != 0)
				goto out;
		}

		if (S_ISCHR (stat.st_mode))
		{
			if (stat.st_rdev != devid)
				continue;

			*devname = strdup (name);
			if (!*devname)
			{
				perror ("strdup");
				ret = -1;
			}
			else
				ret = 1;
			goto out;
		}
	}
out:
	for (i = 0; i < nr; i++)
		free (namelist [i]);

	free (namelist);

	return ret;
}

/**
 * create UIO device info struct
 * @param dir sysfs directory
 * @param name uio device entry
 * @returns UIO device info struct or NULL on failure
 */
struct uio_info_t *create_uio_info (char *dir, char *name)
{
	struct uio_info_t *info;
	char filename [PATH_MAX];

	info = calloc (1, sizeof (struct uio_info_t));
	if (!info)
		return NULL;

	snprintf (filename, PATH_MAX, "%s/%s/name", dir, name);
	info->name = first_line_from_file (filename);

	snprintf (filename, PATH_MAX, "%s/%s/version", dir, name);
	info->version = first_line_from_file (filename);

	snprintf (filename, PATH_MAX, "%s/%s/dev", dir, name);
	info->devid = devid_from_file (filename);

	search_major_minor ("/dev", info->devid, &info->devname);

	snprintf (filename, PATH_MAX, "%s/%s/maps", dir, name);
	info->maps = scan_maps (filename, &info->maxmap);

	info->fd = -1;

	return info;
}

/** @} */
