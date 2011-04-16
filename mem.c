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

#include "libUIO_internal.h"

/**
 * @defgroup libUIO_mem libUIO memory functions
 * @ingroup libUIO_public
 * @brief public memory functions
 * @{
 */

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

/**
 * get memory map pointer
 * @param info UIO device info struct
 * @param map_num memory bar number
 */
void *uio_get_mem_map (struct uio_info_t* info, int map)
{
	if (!info || map >= info->maxmap || info->maps [map].map == MAP_FAILED)
		return NULL;

	return info->maps [map].map;
}

/**
 * get UIO device map count
 * @param info UIO device info struct
 */
int uio_get_maxmap(struct uio_info_t* info)
{
	if (!info)
		return 0;

	return info->maxmap;
}

/** @} */
