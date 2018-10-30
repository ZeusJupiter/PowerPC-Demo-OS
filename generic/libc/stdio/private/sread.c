/*
 *   File name: sread.c
 *
 *  Created on: 2017年7月13日, 下午3:56:22
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */
#include <macros.h>
#include <types.h>
#include <stdio.h>

#include "../private.h"

int __sread(void *cookie, char *buf, int n)
{
	register FILE *fp = cookie;
	register int ret;

	ret = (int) read(fp->_file, buf, n);

	if (ret >= 0)
		fp->_offset += ret;
	else
		fp->_flags &= ~__SOFF;

	return (ret);
}

