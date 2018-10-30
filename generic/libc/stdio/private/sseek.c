/*
 *   File name: sseek.c
 *
 *  Created on: 2017年7月13日, 下午3:59:06
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

fpos_t __sseek(void *cookie, fpos_t offset, int whence)
{
	register FILE *fp = cookie;
	register off_t ret;

	ret = lseek(fp->_file, (off_t) offset, whence);
	if (ret == -1L)
		fp->_flags &= ~__SOFF;
	else
	{
		fp->_flags |= __SOFF;
		fp->_offset = ret;
	}
	return (ret);
}
