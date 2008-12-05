/*
 * libUIO - UserspaceIO helper library
 *
 * Copyright (C) 2008 Hans J. Koch
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

#define UIO_MAX_NAME_SIZE	64
#define UIO_MAX_NUM		255

#define UIO_INVALID_SIZE	-1
#define UIO_INVALID_ADDR	(~0)

#define MAX_UIO_MAPS	5

struct uio_map_t {
	unsigned long addr;
	size_t size;
};

struct uio_dev_attr_t {
	char name[UIO_MAX_NAME_SIZE];
	char value[UIO_MAX_NAME_SIZE];
	struct uio_dev_attr_t *next;
};

struct uio_info_t {
	int uio_num;
	struct uio_map_t maps[MAX_UIO_MAPS];
	int event_count;
	char name[ UIO_MAX_NAME_SIZE ];
	char version[ UIO_MAX_NAME_SIZE ];
	struct uio_dev_attr_t *dev_attrs;
	struct uio_info_t* next;
};

int uio_get_mem_size(struct uio_info_t* info, int map_num);
int uio_get_mem_addr(struct uio_info_t* info, int map_num);
int uio_get_event_count(struct uio_info_t* info);
int uio_get_name(struct uio_info_t* info);
int uio_get_version(struct uio_info_t* info);
int uio_get_all_info(struct uio_info_t* info);
int uio_get_device_attributes(struct uio_info_t* info);
int uio_open_device(struct uio_info_t* info);
void uio_free_dev_attrs(struct uio_info_t* info);
void uio_free_info(struct uio_info_t* info);
struct uio_info_t* uio_find_devices(int filter_num);
struct uio_info_t* uio_find_devices_by_devname(const char *name);
struct uio_info_t* uio_find_devices_by_name(const char *name);

void *uio_mmap(struct uio_info_t* info, int map_num);
void uio_munmap(void *p, size_t size);

#endif
