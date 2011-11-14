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
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/mman.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "libuio_internal.h"

/**
 * @mainpage
 *
 * This manual documents the libuio C API.
 */

/**
 * @defgroup libuio_public public available libuio functions
 * @ingroup libuio
 * @brief public functions
 */

/**
 * @defgroup libuio_base libuio base functions
 * @ingroup libuio_public
 * @brief public base functions
 * @{
 */

static const char *sysfs = "/sys";

/**
 * Set sysfs mount point
 * @param sysfs_mpoint path to sysfs mount point
 */
void uio_setsysfs_point (const char *sysfs_mpoint)
{
	sysfs = sysfs_mpoint;
}

/**
 * get UIO device name
 * @param info UIO device info struct
 * @returns UIO device name or NULL on failure
 */
char *uio_get_name(struct uio_info_t* info)
{
	if (!info)
		return NULL;

	return info->name;
}

/**
 * get UIO device node name
 * @param info UIO device info struct
 * @returns UIO device node name or NULL on failure
 */
char *uio_get_devname(struct uio_info_t* info)
{
	if (!info)
		return NULL;

	return info->devname;
}

/**
 * get UIO driver version
 * @param info UIO device info struct
 * @returns UIO device version or NULL on failure
 */
char *uio_get_version(struct uio_info_t* info)
{
	if (!info)
		return NULL;

	return info->version;
}

/**
 * get UIO device major number
 * @param info UIO device info struct
 * @returns UIO device node major or 0 on failure
 */
int uio_get_major(struct uio_info_t* info)
{
	if (!info)
		return 0;

	return major (info->devid);
}

/**
 * get UIO device minor number
 * @param info UIO device info struct
 * @returns UIO device node minor or 0 on failure
 */
int uio_get_minor(struct uio_info_t* info)
{
	if (!info)
		return 0;

	return minor (info->devid);
}

/**
 * get UIO device id
 * @param info UIO device info struct
 * @returns UIO device id or 0 on failure
 */
dev_t uio_get_devid(struct uio_info_t* info)
{
	if (!info)
		return 0;

	return info->devid;
}

/**
 * free UIO device information struct
 * @param info UIO device info struct
 */
void uio_free_info(struct uio_info_t* info)
{
	if (info)
	{
		if (info->name)
			free (info->name);
		if (info->version)
			free (info->version);
		if (info->maps)
			free (info->maps);
		if (info->devname)
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
 * find UIO devices by UIO enumeration number
 * @param uio_num UIO enumeration number
 * @returns device info or NULL on failure
 */
struct uio_info_t *uio_find_by_uio_num (int uio_num)
{
	struct uio_info_t *info;
	char sysfsname [PATH_MAX];
	char name [PATH_MAX];

	snprintf (sysfsname, sizeof (sysfsname), "%s/class/uio", sysfs);
	snprintf (name, sizeof (name), "uio%d", uio_num);

	info = create_uio_info (sysfsname, name);
	if (errno)
	{
		uio_free_info (info);
		info = NULL;
	}

	return info;
}

/**
 * open a UIO device
 * @param info UIO device info stuct
 * @returns 0 on success or -1 on failure and errno is set
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

/**
 * close a UIO device
 * @param info UIO device info struct
 * @returns 0 on success or -1 on failure and errno is set
 */
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
