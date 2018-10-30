/*
 *   File name: sclose.c
 *
 *  Created on: 2017年7月13日, 下午4:00:16
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */
#include <macros.h>
#include <types.h>
#include <stdio.h>

#include <unistd.h>

#include "../private.h"

int __sclose(void *cookie)
{
	int fd = ((FILE *) cookie)->_file;

	if ((fd <= 2) && (fd >= 0))
	{
		return (0);
	}

	return (close(fd));
}

