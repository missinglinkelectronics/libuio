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

#ifndef LIBUIO_H
#define LIBUIO_H

#include <stddef.h>
#include <stdint.h>

#include <sys/time.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct uio_info_t;

/* base functions */
struct uio_info_t **uio_find_devices ();
struct uio_info_t *uio_find_by_uio_name (char *uio_name);
struct uio_info_t *uio_find_by_uio_num (int num);
struct uio_info_t *uio_find_by_base_addr (unsigned int base_addr);
void uio_setsysfs_point (const char *sysfs_mpoint);
char *uio_get_name (struct uio_info_t* info);
char *uio_get_version (struct uio_info_t* info);
char *uio_get_devname (struct uio_info_t* info);
int uio_get_major (struct uio_info_t* info);
int uio_get_minor (struct uio_info_t* info);
dev_t uio_get_devid (struct uio_info_t* info);
int uio_get_fd (struct uio_info_t* info);
int uio_open (struct uio_info_t* info);
int uio_open_fix (struct uio_info_t* info, void *ptr);
int uio_open_private (struct uio_info_t* info);
int uio_close (struct uio_info_t* info);

/* attribute functions */
char **uio_list_attr (struct uio_info_t* info);
char *uio_get_attr (struct uio_info_t* info, char *attr);
int uio_set_attr (struct uio_info_t* info, char *attr, char *value);
void *uio_get_bin_attr (struct uio_info_t* info, char *attr, size_t count);
int uio_set_bin_attr (struct uio_info_t* info, char *attr,
		      void *value, size_t count);

/* memory functions */
int uio_get_maxmap (struct uio_info_t* info);
size_t uio_get_mem_size (struct uio_info_t* info, int map);
unsigned long uio_get_mem_addr (struct uio_info_t* info, int map);
void *uio_get_mem_map (struct uio_info_t* info, int map);
char *uio_get_mem_name (struct uio_info_t* info, int map_num);
size_t uio_get_offset (struct uio_info_t* info, int map);
int uio_get_map_index_by_name (struct uio_info_t* info, char *name);
int uio_read8 (struct uio_info_t* info, int map, unsigned long offset,
	       uint8_t *val);
int uio_read16 (struct uio_info_t* info, int map, unsigned long offset,
		uint16_t *val);
int uio_read32 (struct uio_info_t* info, int map, unsigned long offset,
		uint32_t *val);
int uio_read64 (struct uio_info_t* info, int map, unsigned long offset,
		uint64_t *val);
int uio_write8 (struct uio_info_t* info, int map, unsigned long offset,
		uint8_t val);
int uio_write16 (struct uio_info_t* info, int map, unsigned long offset,
		 uint16_t val);
int uio_write32 (struct uio_info_t* info, int map, unsigned long offset,
		 uint32_t val);
int uio_write64 (struct uio_info_t* info, int map, unsigned long offset,
		 uint64_t val);

/* irq functions */
int uio_enable_irq (struct uio_info_t* info);
int uio_disable_irq (struct uio_info_t* info);
int uio_irqwait_timeout (struct uio_info_t* info, struct timeval *timeout);

static inline int uio_irqwait (struct uio_info_t* info)
{
	return uio_irqwait_timeout (info, NULL);
}

#ifdef __cplusplus
}
#endif

#endif /* LIBUIO_H */
