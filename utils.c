/*
 * JMPXRDS, an FM MPX signal generator with RDS support on
 * top of Jack Audio Connection Kit - General utilities
 *
 * Copyright (C) 2016 Nick Kossifidis <mickflemm@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "utils.h"
#include <stdlib.h>	/* For malloc(), NULL etc */
#include <string.h>	/* For memset() */
#include <sys/mman.h>	/* For shm_open, mmap etc  */
#include <sys/stat.h>	/* For mode constants */
#include <fcntl.h>	/* For O_* constants */
#include <unistd.h>	/* For ftruncate() */

struct shm_mapping*
utils_shm_init(const char* name, int size)
{
	int ret = 0;
	struct shm_mapping* shmem = NULL;

	shmem = malloc(sizeof(struct shm_mapping));
	if(!shmem)
		return NULL;
	memset(shmem, 0, sizeof(struct shm_mapping));
	shmem->name = name;
	shmem->size = size;

	/* Create the shm segment */
	shmem->fd = shm_open(name, O_CREAT | O_EXCL | O_RDWR, 0600);
	if (shmem->fd < 0)
		goto cleanup;

	/* Resize it */
	ret = ftruncate(shmem->fd, size);
	if (ret < 0)
		goto cleanup;

	/* mmap() it */
	shmem->mem = mmap(0, size, PROT_READ | PROT_WRITE,
			  MAP_SHARED, shmem->fd, 0);

 cleanup:
	if(shmem->fd >= 0)
		close(shmem->fd);
	else
		return NULL;

	if (shmem->mem == MAP_FAILED) {
		shm_unlink(name);
		free(shmem);
		shmem = NULL;
	} else
		memset(shmem->mem, 0, shmem->size);

	return shmem;
}

struct shm_mapping*
utils_shm_attach(char* name, int size)
{
	int ret = 0;
	struct shm_mapping* shmem = NULL;

	shmem = malloc(sizeof(struct shm_mapping));
	if(!shmem)
		return NULL;
	memset(shmem, 0, sizeof(struct shm_mapping));
	shmem->name = name;
	shmem->size = size;

	/* Open the shm segment */
	shmem->fd = shm_open(name, O_RDWR, 0600);
	if (shmem->fd < 0)
		goto cleanup;

	/* mmap() it */
	shmem->mem = mmap(0, size, PROT_READ | PROT_WRITE,
			  MAP_SHARED, shmem->fd, 0);

 cleanup:
	if(shmem->fd >= 0)
		close(shmem->fd);
	else
		return NULL;

	if (shmem->mem == MAP_FAILED) {
		shm_unlink(name);
		free(shmem);
		shmem = NULL;
	}

	return shmem;
}

void
utils_shm_destroy(struct shm_mapping* shmem, int unlink)
{
	if(!shmem)
		return;

	munmap(shmem->mem, shmem->size);

	if(unlink)
		shm_unlink(shmem->name);

	free(shmem);
}