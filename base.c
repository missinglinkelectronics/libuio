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

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

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
#include <sys/sysmacros.h>
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

static int uio_unmap (struct uio_map_t *uio_map)
{
	int ret;
	ret = munmap (uio_map->map, uio_map->size);
	if (ret)
		g_warning (_("munmap: %s\n"), g_strerror (errno));
	else
	        uio_map->map = MAP_FAILED;

	return ret;
}

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
 * get UIO device file descriptor
 * @param info UIO device info struct
 * @returns UIO device file descriptor -1 on failure
 */
int uio_get_fd(struct uio_info_t* info)
{
	if (!info)
		return -1;

	return info->fd;
}

/**
 * free UIO device information struct
 * @param info UIO device info struct
 */
void uio_free_info(struct uio_info_t* info)
{
	if (info)
	{
		if (info->path)
			free (info->path);
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
		g_warning (_("scandir: %s\n"), g_strerror (errno));
		return NULL;
	}

	info = calloc (nr, sizeof (struct uio_info_t *));
	if (!info)
	{
		errno = ENOMEM;
		g_warning (_("calloc: %s\n"), g_strerror (errno));
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
 * find UIO devices by UIO name
 * @param uio_name UIO name
 * @returns device info or NULL on failure
 */
struct uio_info_t *uio_find_by_uio_name (char *uio_name)
{
	struct uio_info_t *info = NULL, **list, **uio_list;
	char *name;

	if (!uio_name)
		return NULL;

	uio_list = uio_find_devices ();
	if (!uio_list)
		return NULL;

	for (list = uio_list; *list; list++)
	{
		struct uio_info_t *candidate = *list;

		name = uio_get_name (candidate);

		if (!strcmp (name, uio_name)) {
			info = candidate;
			break;
		}
	}
	free (uio_list);

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
 * find a UIO device by base address in memory map
 * @param base address of a memory map member
 * @returns device info or NULL on failure
 */
struct uio_info_t *uio_find_by_base_addr (unsigned int base_addr)
{
	struct uio_info_t *info = NULL, **list, **uio_list;
	int mapc, mapnum, found = 0;

	uio_list = uio_find_devices();
	if (!uio_list)
		return NULL;

	for (list = uio_list; *list; list++)
	{
		struct uio_info_t *candidate = *list;

		/* get number of maps and go through each checking the base address */
		mapnum = uio_get_maxmap(candidate);

		for (mapc = 0; mapc < mapnum; mapc++)
		{
			if (base_addr == uio_get_mem_addr(candidate, mapc))
			{
				info = candidate;
				found = 1;
				break;
			}
		}

		if (found)
			break;
	}

	free (uio_list);

	return info;
}

/**
 * open a UIO device (try to map to given address)
 * @param info UIO device info stuct
 * @param ptr try to map at ptr
 * @returns 0 on success or -1 on failure and errno is set
 */
int uio_open_fix (struct uio_info_t* info, void *ptr)
{
	int fd, i;

	if (!info)
	{
		errno = EINVAL;
		g_warning (_("uio_open: %s\n"), g_strerror (errno));
		return -1;
	}

	fd = open (info->devname, O_RDWR);
	if (fd < 0)
	{
		g_warning (_("open: %s\n"), g_strerror (errno));
		return -1;
	}

	for (i = 0; i < info->maxmap; i++)
	{
		info->maps [i].map = mmap (ptr, info->maps [i].size,
					   PROT_READ | PROT_WRITE,
					   MAP_SHARED, fd, i * getpagesize());
		if (info->maps[i].map == MAP_FAILED) {
			while (--i >= 0)
				uio_unmap (info->maps [i].map);
			g_warning (_("mmap: %s\n"), g_strerror (errno));
			return -1;
		}
		if (ptr)
			ptr += info->maps [i].size;
	}
	info->fd = fd;

	return 0;
}

/**
 * open a UIO device
 * @param info UIO device info stuct
 * @returns 0 on success or -1 on failure and errno is set
 */
int uio_open (struct uio_info_t* info)
{
	return uio_open_fix (info, NULL);
}

/**
 * open a UIO device (COW)
 * @param info UIO device info stuct
 * @returns 0 on success or -1 on failure and errno is set
 */
int uio_open_private (struct uio_info_t* info)
{
	int fd, i;

	if (!info)
	{
		errno = EINVAL;
		g_warning (_("uio_open: %s\n"), g_strerror (errno));
		return -1;
	}

	fd = open (info->devname, O_RDWR);
	if (fd < 0)
	{
		g_warning (_("open: %s\n"), g_strerror (errno));
		return -1;
	}

	for (i = 0; i < info->maxmap; i++) {
		info->maps [i].map = mmap (NULL, info->maps [i].size,
					   PROT_READ | PROT_WRITE,
					   MAP_PRIVATE, fd, i * getpagesize());

		if (info->maps[i].map == MAP_FAILED) {
			while (--i >= 0)
				uio_unmap(info->maps [i].map);
			g_warning (_("mmap: %s\n"), g_strerror (errno));
			return -1;
		}
	}

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
		g_warning (_("uio_close: %s\n"), g_strerror (errno));
		return -1;
	}

	for (i = 0; i < info->maxmap; i++)
		if (info->maps [i].map != MAP_FAILED)
			uio_unmap(info->maps [i].map);

	close (info->fd);

	return 0;
}

/** @} */
