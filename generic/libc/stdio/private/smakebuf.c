/*
 *   File name: smakebuf.c
 *
 *  Created on: 2017年7月13日, 下午4:03:35
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */
#include <macros.h>
#include <types.h>
#include <stdio.h>

#include <unistd.h>
#include <stdlib.h>

#include "../private.h"

void __smakebuf(register FILE *fp)
{
	register void *p;
	register int flags;
	size_t size;
	int couldbetty;

	if (fp->_flags & __SNBF)
	{
		fp->_bf._base = fp->_p = fp->_nbuf;
		fp->_bf._size = 1;
		return;
	}
	flags = __swhatbuf(fp, &size, &couldbetty);
	if ((p = malloc(size)) == NULL)
	{
		fp->_flags |= __SNBF;
		fp->_bf._base = fp->_p = fp->_nbuf;
		fp->_bf._size = 1;
		return;
	}

	flags |= __SMBF;
	fp->_bf._base = fp->_p = p;
	fp->_bf._size = size;
	if (couldbetty && isatty(fp->_file))
		flags |= __SLBF;
	fp->_flags |= flags;
}
