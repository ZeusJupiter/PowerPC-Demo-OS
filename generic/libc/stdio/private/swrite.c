/*
 *   File name: swrite.c
 *
 *  Created on: 2017年7月13日, 下午3:58:11
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

int __swrite(void *cookie, char const *buf, int n)
{
	register FILE *fp = cookie;

	if (fp->_flags & __SAPP)
		(void) lseek(fp->_file, (off_t) 0, SEEK_END);
	fp->_flags &= ~__SOFF;

	return ((int) write(fp->_file, buf, n));
}
