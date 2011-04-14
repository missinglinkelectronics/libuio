/*
 * libUIO - UserspaceIO helper library
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

#ifndef _LIBUIO_H_
#define _LIBUIO_H_

struct uio_map_t {
	unsigned long addr;
	size_t size;
	size_t offset;
	void *map;
};

struct uio_info_t {
	char *name;
	char *version;
	struct uio_map_t *maps;
	char *devname;
	dev_t devid;
	int maxmap;
	int fd;
};

struct uio_info_t **uio_find_devices ();

void uio_setsysfs_point (const char *sysfs_mpoint);
size_t uio_get_mem_size (struct uio_info_t* info, int map);
unsigned long uio_get_mem_addr (struct uio_info_t* info, int map);
size_t uio_get_offset (struct uio_info_t* info, int map);
int uio_open (struct uio_info_t* info);
int uio_close (struct uio_info_t* info);

#endif
