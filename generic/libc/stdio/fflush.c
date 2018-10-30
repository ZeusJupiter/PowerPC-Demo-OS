/*
 *   File name: fflush.c
 *
 *  Created on: 2017年6月26日, 上午3:31:31
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */
#include <macros.h>
#include <types.h>
#include <stdio.h>
#include <errno.h>

#include "private.h"

int fflush(register FILE *fp)
{
	if (fp == NULL)
		return (_fwalk(__sflush));
	if ((fp->_flags & (__SWR | __SRW)) == 0) {
		errno = EBADF;
		return (EOF);
	}
	return (__sflush(fp));
}

