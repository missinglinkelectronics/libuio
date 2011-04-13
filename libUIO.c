/*
 * libUIO - UserspaceIO helper library
 *
 * Copyright (C) 2008 Hans J. Koch
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

/**
 * @mainpage
 *
 * This manual documents the libUIO C API.
 */

/**
 * @defgroup libUIO_helper generic libUIO helper functions
 * @ingroup libUIO
 * @brief helper functions
 * @{
 */

static const char *sysfs = "/sys";

/**
 * read a line from a file
 * @param filename file name
 * @param linebuf line buffer
 */
static int line_from_file(char *filename, char *linebuf)
{
	char *s;
	int i;
	memset(linebuf, 0, UIO_MAX_NAME_SIZE);
	FILE* file = fopen(filename,"r");
	if (!file)
		return -1;
	s = fgets(linebuf,UIO_MAX_NAME_SIZE,file);
	if (!s)
		return -2;
	for (i=0; (*s)&&(i<UIO_MAX_NAME_SIZE); i++) {
		if (*s == '\n') *s = 0;
		s++;
	}
	return 0;
}

/**
 * device attribute filter
 * @param filename file name
 */
static int dev_attr_filter(char *filename)
{
	struct stat filestat;

	if (lstat(filename, &filestat))
		return 0;
	if (S_ISREG(filestat.st_mode))
		return 1;
	return 0;
}

/**
 * get UIO enumeration number from file name
 * @param name file name
 */
static int uio_num_from_filename(const char *name)
{
	char *p;
	char *endp;
	int len;
	int num;

	p = strstr(name, "uio");
	if (!p)
		return -1;

	len = strlen(p);
	if (len < 4)
		return -1;

	p += 3;
	num = strtoul(p, &endp, 10);

	if (*endp != '\0')
		return -1;
	return num;
}

/**
 * get UIO info structure from file name
 * @param name file name
 * @param filter_num UIO enumeration number
 */
static struct uio_info_t* info_from_name(char* name, int filter_num)
{
	struct uio_info_t* info;
	int num = uio_num_from_filename(name);
	if (num < 0)
		return NULL;
	if ((filter_num >= 0) && (num != filter_num))
		return NULL;

	info = malloc(sizeof(struct uio_info_t));
	if (!info)
		return NULL;
	memset(info,0,sizeof(struct uio_info_t));
	info->uio_num = num;
	return info;
}

static dev_t uio_get_dev_num(const char *name)
{
	struct stat buf;
	int ret;

	ret = stat(name, &buf);
	if (ret)
		goto out;

	if ((buf.st_mode & S_IFMT) != S_IFCHR)
	{
		printf("%s is no char device\n", name);
		goto out;
	}

	return buf.st_rdev;
out:
	return makedev(0,0);
}

static dev_t uio_dev_from_uevent(const char *name)
{
	char *pos, line [2048];
	int maj = 0, min = 0;
	FILE *f;

	f = fopen(name, "r");
	if (!f)
		goto out;

	while(fgets(line, sizeof(line), f))
	{
		pos = strchr(line, '\n');
		if (pos == NULL)
			continue;
		pos[0] = '\0';

		if (strncmp(line, "MAJOR=", 6) == 0)
			maj = strtoull(&line[6], NULL, 10);
		else if (strncmp (line, "MINOR=", 6) == 0)
			min = strtoull(&line[6], NULL, 10);
	}

	fclose(f);
out:
	return makedev(maj, min);
}

/** @} */

/**
 * @defgroup libUIO_public
 * @ingroup libUIO
 * @brief public functions
 * @{
 */

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
int uio_get_mem_size(struct uio_info_t* info, int map_num)
{
	int ret;
	char filename[64];
	FILE* file;

	info->maps[map_num].size = UIO_INVALID_SIZE;
	snprintf(filename, sizeof(filename),
		"%s/class/uio/uio%d/maps/map%d/size", sysfs, info->uio_num,
		map_num);
	file = fopen(filename,"r");
	if (!file)
		return -1;

	ret = fscanf(file, "0x%lx", (unsigned long *)&info->maps[map_num].size);
	fclose(file);
	if (ret < 0)
		return -2;
	return 0;
}

/**
 * get memory map physical address of UIO memory bar
 * @param info UIO device info struct
 * @param map_num memory bar number
 */
int uio_get_mem_addr(struct uio_info_t* info, int map_num)
{
	int ret;
	char filename[64];
	FILE* file;

	info->maps[map_num].addr = UIO_INVALID_ADDR;
	snprintf(filename, sizeof(filename),
		"%s/class/uio/uio%d/maps/map%d/addr", sysfs, info->uio_num,
		map_num);
	file = fopen(filename,"r");
	if (!file)
		return -1;
	ret = fscanf(file,"0x%lx",&info->maps[map_num].addr);
	fclose(file);
	if (ret < 0)
		return -2;
	return 0;
}

/**
 * get UIO device event count
 * @param info UIO device info struct
 */
int uio_get_event_count(struct uio_info_t* info)
{
	int ret;
	char filename[64];
	FILE* file;

	info->event_count = 0;
	snprintf(filename, sizeof(filename), "%s/class/uio/uio%d/event", sysfs,
			info->uio_num);
	file = fopen(filename,"r");
	if (!file)
		return -1;
	ret = fscanf(file,"%d",&info->event_count);
	fclose(file);
	if (ret < 0)
		return -2;
	return 0;
}

/**
 * get UIO device name
 * @param info UIO device info struct
 */
int uio_get_name(struct uio_info_t* info)
{
	char filename[64];
	snprintf(filename, sizeof(filename), "%s/class/uio/uio%d/name", sysfs,
			info->uio_num);
	return line_from_file(filename, info->name);
}

/**
 * get UIO driver version
 * @param info UIO device info struct
 */
int uio_get_version(struct uio_info_t* info)
{
	char filename[64];
	snprintf(filename, sizeof(filename), "%s/class/uio/uio%d/version",
			sysfs, info->uio_num);
	return line_from_file(filename, info->version);
}

/**
 * read nearly all UIO Device informations into UIO device struct
 * @param info UIO device info struct
 */
int uio_get_all_info(struct uio_info_t* info)
{
	int i;
	if (!info)
		return -1;
	if ((info->uio_num < 0)||(info->uio_num > UIO_MAX_NUM))
		return -1;
	for (i = 0; i < MAX_UIO_MAPS; i++) {
		uio_get_mem_size(info, i);
		uio_get_mem_addr(info, i);
	}
	uio_get_event_count(info);
	uio_get_name(info);
	uio_get_version(info);
	return 0;
}

/**
 * get UIO device attributes
 * @param info UIO device info struct
 */
int uio_get_device_attributes(struct uio_info_t* info)
{
	struct dirent **namelist;
	struct uio_dev_attr_t *attr, *last = NULL;
	char fullname[96];
	int n;

	info->dev_attrs = NULL;
	snprintf(fullname, sizeof(fullname), "%s/class/uio/uio%d/device", sysfs,
			info->uio_num);
	n = scandir(fullname, &namelist, 0, alphasort);
	if (n < 0)
		return -1;

	while(n--) {
		snprintf(fullname, sizeof(fullname),
			"%s/class/uio/uio%d/device/%s", sysfs, info->uio_num,
			namelist[n]->d_name);
		if (!dev_attr_filter(fullname))
			continue;
		attr = malloc(sizeof(struct uio_dev_attr_t));
		if (!attr)
			return -1;
		strncpy(attr->name, namelist[n]->d_name, UIO_MAX_NAME_SIZE);
		free(namelist[n]);
		if (line_from_file(fullname, attr->value)) {
			free(attr);
			continue;
		}

		if (!info->dev_attrs)
			info->dev_attrs = attr;
		else
			last->next = attr;
		attr->next = NULL;
		last = attr;
	}
	free(namelist);

	return 0;
}

/**
 * free UIO device attributes
 * @param info UIO device info struct
 */
void uio_free_dev_attrs(struct uio_info_t* info)
{
	struct uio_dev_attr_t *p1, *p2;
	p1 = info->dev_attrs;
	while (p1) {
		p2 = p1->next;
		free(p1);
		p1 = p2;
	}
	info->dev_attrs = NULL;
}

/**
 * free UIO device information struct
 * @param info UIO device info struct
 */
void uio_free_info(struct uio_info_t* info)
{
	struct uio_info_t *p1,*p2;
	p1 = info;
	while (p1) {
		uio_free_dev_attrs(p1);
		p2 = p1->next;
		free(p1);
		p1 = p2;
	}
}

/**
 * find a UIO device
 * @param filter_num UIO enumeration number
 */
struct uio_info_t* uio_find_devices(int filter_num)
{
	struct dirent **namelist;
	struct uio_info_t *infolist = NULL, *infp, *last = NULL;
	char sysfsname[64];
	int n;

	snprintf(sysfsname, sizeof(sysfsname), "%s/class/uio", sysfs);
	n = scandir(sysfsname, &namelist, 0, alphasort);
	if (n < 0)
		return NULL;

	while(n--) {
		infp = info_from_name(namelist[n]->d_name, filter_num);
		free(namelist[n]);
		if (!infp)
			continue;
		if (!infolist)
			infolist = infp;
		else
			last->next = infp;
		last = infp;
	}
	free(namelist);

	return infolist;
}

/**
 * find a UIP device by name
 * @param name UIO device name
 */
struct uio_info_t* uio_find_devices_by_name(const char *name)
{
	struct dirent **namelist;
	struct uio_info_t *infolist = NULL, *infp, *last = NULL;
	char sysfsname[64];
	int n, i;

	if (!name)
		return NULL;

	snprintf(sysfsname, sizeof(sysfsname), "%s/class/uio", sysfs);
	n = scandir(sysfsname, &namelist, 0, alphasort);
	if (n < 0)
		return NULL;

	while(n--) {
		infp = info_from_name(namelist[n]->d_name, -1);
		free(namelist[n]);
		if (!infp)
			continue;
		if (uio_get_name(infp))
			continue;
		if (strncmp(infp->name, name, UIO_MAX_NAME_SIZE))
			continue;
		for (i = 0; i < MAX_UIO_MAPS; i++) {
			uio_get_mem_size(infp, i);
			uio_get_mem_addr(infp, i);
		}
		uio_get_event_count(infp);
		if (!infolist)
			infolist = infp;
		else
			last->next = infp;
		last = infp;
	}
	free(namelist);

	return infolist;
}

/**
 * open a UIO device
 * @param name UIO device name
 */
int uio_open_device(struct uio_info_t* info)
{
	char dev_name[16];
	int fd;

	snprintf(dev_name, sizeof(dev_name), "/dev/uio%d", info->uio_num);
	fd = open(dev_name,O_RDWR);

	return fd;
}

/**
 * map a UIO device memory bar to userspace
 * @param name UIO device name
 * @param map_num memory map number
 */
void *uio_mmap(struct uio_info_t* info, int map_num)
{
	char dev_name[16];
	int fd;
	void* map_addr;
	int64_t offset;

	if (info->maps[map_num].size <= 0)
		return MAP_FAILED;

	snprintf(dev_name, sizeof(dev_name), "/dev/uio%d", info->uio_num);

	fd = open(dev_name,O_RDWR);
	if (fd < 0)
		return MAP_FAILED;

	map_addr = mmap(NULL, info->maps[map_num].size,
			       PROT_READ|PROT_WRITE, MAP_SHARED|MAP_LOCKED|MAP_POPULATE, fd,
			       map_num * getpagesize());

	/* fixup map_addr, cause mmap only maps hole pages */
	offset = ((int64_t) info->maps[map_num].addr & (getpagesize() - 1));
	map_addr = map_addr + offset;
#if 0
	printf("errno: %d %s\n", errno, strerror(errno));
#endif
	close(fd);
	return map_addr;
}

/**
 * unmap a UIO device memory bar
 * @param p pointer
 * @param size size
 */
void uio_munmap(void *p, size_t size)
{
	munmap(p, size);
}

/**
 * find a UIO device by device node name
 * @param name device node name
 */
struct uio_info_t* uio_find_devices_by_devname(const char *name)
{
	struct uio_info_t *infop = NULL;
	struct dirent **namelist;
	char uevent[PATH_MAX];
	char sysfsname[64];
	dev_t dev, dev1;
	int num, n, i;

	if (!name)
		return NULL;

	dev = uio_get_dev_num(name);
	if (dev == makedev(0,0))
		return NULL;

	snprintf(sysfsname, sizeof(sysfsname), "%s/class/uio", sysfs);
	n = scandir(sysfsname, &namelist, 0, alphasort);
	if (n < 0)
		return NULL;

	while(n--) {
		snprintf(uevent, sizeof(uevent),
			 "%s/class/uio/%s/uevent", sysfs, namelist[n]->d_name);
		num = uio_num_from_filename(namelist[n]->d_name);
		free(namelist[n]);
		if (infop)
			continue;

		dev1 = uio_dev_from_uevent(uevent);
		if (dev == dev1) {
			infop = calloc(1, sizeof(struct uio_info_t));
			if (!infop)
				continue;
			infop->uio_num = num;

			if (!infop)
				continue;
			if (uio_get_name(infop))
				continue;
			for (i = 0; i < MAX_UIO_MAPS; i++) {
				uio_get_mem_size(infop, i);
				uio_get_mem_addr(infop, i);
			}
			uio_get_event_count(infop);
		}
	}
	free(namelist);

	return infop;
}

/** @} */
