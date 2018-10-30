/*
 *   File name: swhatbuf.c
 *
 *  Created on: 2017年7月13日, 下午4:01:05
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */
#include <macros.h>
#include <types.h>
#include <stdio.h>

#include <sys/stat.h>

#include "../private.h"

int __swhatbuf(register FILE *fp, size_t *bufsize, int *couldbetty)
{
	struct stat st;

	if (fp->_file < 0 || fstat(fp->_file, &st) < 0)
	{
		*couldbetty = 0;
		*bufsize = BUFSIZ;
		return (__SNPT);
	}

	*couldbetty = (st.st_mode & S_IFMT) == S_IFCHR;
	if (st.st_blksize <= 0)
	{
		*bufsize = BUFSIZ;
		return (__SNPT);
	}

	*bufsize = st.st_blksize;
	fp->_blksize = st.st_blksize;
	return ((st.st_mode & S_IFMT) == S_IFREG && fp->_seek == __sseek ? __SOPT :__SNPT);
}

