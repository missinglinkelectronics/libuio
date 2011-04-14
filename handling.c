/*
 * libUIO - UserspaceIO helper library
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

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#include <errno.h>
#include <string.h>

#include "libUIO.h"

struct uio_info_t* create_uio_info (char *dir, char* name);

/**
 * @mainpage
 *
 * This manual documents the libUIO C API.
 */

/**
 * @defgroup libUIO_public public available libUIO functions
 * @ingroup libUIO
 * @brief public functions
 * @{
 */

static const char *sysfs = "/sys";

/**
 * Set sysfs mount point
 * @param sysfs_mpoint path to sysfs mount point
 */
void uio_setsysfs_point(const char *sysfs_mpoint)
{
	sysfs = sysfs_mpoint;
}

/**
 * get memory map size of UIO memory bar
 * @param info UIO device info struct
 * @param map_num memory bar number
 */
size_t uio_get_mem_size (struct uio_info_t* info, int map)
{
	if (!info || map >= info->maxmap)
		return 0;

	return info->maps [map].size;
}

/**
 * get memory map offset of UIO memory bar
 * @param info UIO device info struct
 * @param map_num memory bar number
 */
size_t uio_get_offset (struct uio_info_t* info, int map)
{
	if (!info || map >= info->maxmap)
		return 0;

	return info->maps [map].offset;
}

/**
 * get memory map physical address of UIO memory bar
 * @param info UIO device info struct
 * @param map_num memory bar number
 */
unsigned long uio_get_mem_addr (struct uio_info_t* info, int map)
{
	if (!info || map >= info->maxmap)
		return 0;

	return info->maps [map].addr;
}

#if 0
/**
 * get UIO device event count
 * @param info UIO device info struct
 */
int uio_get_event_count(struct uio_info_t* info)
{
	return 0;
}
#endif

/**
 * get UIO device name
 * @param info UIO device info struct
 */
char *uio_get_name(struct uio_info_t* info)
{
	if (!info)
		return NULL;

	return info->name;
}

/**
 * get UIO driver version
 * @param info UIO device info struct
 */
char *uio_get_version(struct uio_info_t* info)
{
	if (!info)
		return NULL;

	return info->version;
}

/**
 * free UIO device information struct
 * @param info UIO device info struct
 */
void uio_free_info(struct uio_info_t* info)
{
	if (info)
	{
		free (info->name);
		free (info->version);
		free (info->maps);
		free (info->devname);
		free (info);
	}
}

/**
 * find UIO devices
 * @returns device list or NULL on failure
 */
struct uio_info_t **uio_find_devices ()
{
	struct dirent **namelist;
	struct uio_info_t **info;
	char sysfsname [PATH_MAX];
	int i, t = 0, nr;

	snprintf (sysfsname, sizeof (sysfsname), "%s/class/uio", sysfs);
	nr = scandir (sysfsname, &namelist, 0, alphasort);
	if (nr < 0)
	{
		perror ("scandir");
		return NULL;
	}

	info = calloc (nr, sizeof (struct uio_info_t *));
	if (!info)
	{
		errno = ENOMEM;
		perror ("calloc");
		goto out;
	}

	for (i = 0; i < nr; i++)
	{
		if (!strcmp (namelist [i]->d_name, ".") ||
		    !strcmp (namelist [i]->d_name, ".."))
			continue;

		info [t++] = create_uio_info (sysfsname, namelist [i]->d_name);
	}

out:
	for (i = 0; i < nr; i++)
		free (namelist [i]);
	free (namelist);

	return info;
}

/**
 * open a UIO device
 * @param name UIO device name
 */
int uio_open (struct uio_info_t* info)
{
	int fd, i;

	if (!info)
	{
		errno = EINVAL;
		perror ("uio_open");
		return -1;
	}

	fd = open (info->devname, O_RDWR);
	if (fd < 0)
	{
		perror ("open");
		return -1;
	}

	for (i = 0; i < info->maxmap; i++)
		info->maps [i].map = mmap (NULL, info->maps [i].size,
					   PROT_READ | PROT_WRITE,
					   MAP_SHARED, fd, i);
	info->fd = fd;

	return 0;
}

int uio_close (struct uio_info_t* info)
{
	int i;

	if (!info)
	{
		errno = EINVAL;
		perror ("uio_close");
		return -1;
	}

	for (i = 0; i < info->maxmap; i++)
	{
		if (info->maps [i].map != MAP_FAILED)
			munmap (info->maps [i].map, info->maps [i].size);
		info->maps [i].map = MAP_FAILED;
	}

	close (info->fd);

	return 0;
}

/** @} */
