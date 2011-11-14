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
#include <stdint.h>
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
 * @defgroup libuio_mem libuio memory functions
 * @ingroup libuio_public
 * @brief public memory functions
 * @{
 */

/**
 * get memory map size of UIO memory bar
 * @param info UIO device info struct
 * @param map_num memory bar number
 * @return size of UIO memory bar or 0 on failure
 */
size_t uio_get_mem_size (struct uio_info_t* info, int map_num)
{
	if (!info || map_num >= info->maxmap)
		return 0;

	return info->maps [map_num].size;
}

/**
 * get memory map offset of UIO memory bar
 * @param info UIO device info struct
 * @param map_num memory bar number
 * @return size of UIO memory bar
 */
size_t uio_get_offset (struct uio_info_t* info, int map_num)
{
	if (!info || map_num >= info->maxmap)
		return 0;

	return info->maps [map_num].offset;
}

/**
 * get memory map physical address of UIO memory bar
 * @param info UIO device info struct
 * @param map_num memory bar number
 * @return physical address of UIO memory bar or 0 on failure
 */
unsigned long uio_get_mem_addr (struct uio_info_t* info, int map_num)
{
	if (!info || map_num >= info->maxmap)
		return 0;

	return info->maps [map_num].addr;
}

/**
 * get memory map pointer
 * @param info UIO device info struct
 * @param map_num memory bar number
 * @return UIO memory bar maped pointer or NULL on failure
 */
void *uio_get_mem_map (struct uio_info_t* info, int map_num)
{
	if (!info ||
	    map_num >= info->maxmap ||
	    info->maps [map_num].map == MAP_FAILED)
		return NULL;

	return info->maps [map_num].map;
}

/**
 * get UIO device map count
 * @param info UIO device info struct
 * @return number of UIO memory bars
 */
int uio_get_maxmap(struct uio_info_t* info)
{
	if (!info)
		return 0;

	return info->maxmap;
}

/**
 * read 8 bit from UIO device map
 * @param info UIO device info struct
 * @param map_num memory bar number
 * @param offset register offset
 * @param val register value
 * @return 0 on success or -1 on failure
 */
int uio_read8 (struct uio_info_t* info, int map_num, unsigned long offset,
	       uint8_t *val)
{
	void *ptr;

	if (!info || !val)
		return -1;

	ptr = info->maps [map_num].map + info->maps [map_num].offset + offset;

	*val = *(volatile uint8_t *) ptr;

	return 0;
}

/**
 * read 16 bit from UIO device map
 * @param info UIO device info struct
 * @param map_num memory bar number
 * @param offset register offset
 * @param val register value
 * @return 0 on success or -1 on failure
 */
int uio_read16 (struct uio_info_t* info, int map_num, unsigned long offset,
	       uint16_t *val)
{
	void *ptr;

	if (!info || !val)
		return -1;

	ptr = info->maps [map_num].map + info->maps [map_num].offset + offset;

	*val = *(volatile uint16_t *) ptr;

	return 0;
}

/**
 * read 32 bit from UIO device map
 * @param info UIO device info struct
 * @param map_num memory bar number
 * @param offset register offset
 * @param val register value
 * @return 0 on success or -1 on failure
 */
int uio_read32 (struct uio_info_t* info, int map_num, unsigned long offset,
	       uint32_t *val)
{
	void *ptr;

	if (!info || !val)
		return -1;

	ptr = info->maps [map_num].map + info->maps [map_num].offset + offset;

	*val = *(volatile uint32_t *) ptr;

	return 0;
}

/**
 * write 8 bit from UIO device map
 * @param info UIO device info struct
 * @param map_num memory bar number
 * @param offset register offset
 * @param val register value
 * @return 0 on success or -1 on failure
 */
int uio_write8 (struct uio_info_t* info, int map_num, unsigned long offset,
	       uint8_t val)
{
	void *ptr;

	if (!info || !val)
		return -1;

	ptr = info->maps [map_num].map + info->maps [map_num].offset + offset;

	*(volatile uint8_t *) ptr = val;

	return 0;
}

/**
 * write 16 bit from UIO device map
 * @param info UIO device info struct
 * @param map_num memory bar number
 * @param offset register offset
 * @param val register value
 * @return 0 on success or -1 on failure
 */
int uio_write16 (struct uio_info_t* info, int map_num, unsigned long offset,
	       uint16_t val)
{
	void *ptr;

	if (!info || !val)
		return -1;

	ptr = info->maps [map_num].map + info->maps [map_num].offset + offset;

	*(volatile uint16_t *) ptr = val;

	return 0;
}

/**
 * write 32 bit from UIO device map
 * @param info UIO device info struct
 * @param map_num memory bar number
 * @param offset register offset
 * @param val register value
 * @return 0 on success or -1 on failure
 */
int uio_write32 (struct uio_info_t* info, int map_num, unsigned long offset,
	       uint32_t val)
{
	void *ptr;

	if (!info || !val)
		return -1;

	ptr = info->maps [map_num].map + info->maps [map_num].offset + offset;

	*(volatile uint32_t *) ptr = val;

	return 0;
}

/** @} */
