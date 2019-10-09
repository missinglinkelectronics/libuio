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
 * @defgroup libuio_irq libuio irq functions
 * @ingroup libuio_public
 * @brief public irq functions
 * @{
 */

/**
 * enable UIO device interrupt
 * @param info UIO device info struct
 * @returns 0 on success or -1 on failure and errno is set
 */
int uio_enable_irq (struct uio_info_t* info)
{
	unsigned long tmp = 1;

	if (!info || info->fd == -1)
	{
		errno = EINVAL;
		g_warning (_("%s: %s"), __func__, g_strerror (errno));
		return -1;
	}

	return (write (info->fd, &tmp, 4) == 4) ? 0 : -1;
}

/**
 * disable UIO device interrupt
 * @param info UIO device info struct
 * @returns 0 on success or -1 on failure and errno is set
 */
int uio_disable_irq (struct uio_info_t* info)
{
	unsigned long tmp = 0;

	if (!info || info->fd == -1)
	{
		errno = EINVAL;
		g_warning (_("%s: %s"), __func__, g_strerror (errno));
		return -1;
	}

	return (write (info->fd, &tmp, 4) == 4) ? 0 : -1;
}

/**
 * wait for UIO device interrupt
 * @param info UIO device struct
 * @param timeout timeout or NULL to wait forever
 * @returns 0 on success or -1 on failure and errno is set
 */
int uio_irqwait_timeout (struct uio_info_t* info, struct timeval *timeout)
{
	unsigned long dummy;
	int ret;

	if (!info || info->fd == -1)
	{
		errno = EINVAL;
		g_warning (_("%s: %s"), __func__, g_strerror (errno));
		return -1;
	}

	if (timeout)
	{
		fd_set rfds;
		FD_ZERO (&rfds);
		FD_SET (info->fd, &rfds);

		ret = select (info->fd + 1, &rfds, NULL, NULL, timeout);
		if (ret < 1) {
			if (ret == 0)
				errno = ETIMEDOUT;
			return -1;
		}
	}

	ret = read (info->fd, &dummy, 4);

	return (ret < 0) ? ret : 0;
}

/** @} */
