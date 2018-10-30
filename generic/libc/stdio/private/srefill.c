/*
 *   File name: srefill.c
 *
 *  Created on: 2017年7月12日, 下午10:43:17
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */
#include <macros.h>
#include <types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "../private.h"

static int __lflush(FILE *fp)
{
	if ((fp->_flags & (__SLBF|__SWR)) == (__SLBF|__SWR))
		return (__sflush(fp));
	return (0);
}

int __srefill(register FILE *fp)
{

	if (!__stdIOIsInitilized__)
		__sinit();

	fp->_r = 0;

	if (fp->_flags & __SEOF)
		return (EOF);

	if ((fp->_flags & __SRD) == 0) {
		if ((fp->_flags & __SRW) == 0) {
			errno = EBADF;
			return (EOF);
		}

		if (fp->_flags & __SWR) {
			if (__sflush(fp))
				return (EOF);
			fp->_flags &= ~__SWR;
			fp->_w = 0;
			fp->_lbfsize = 0;
		}
		fp->_flags |= __SRD;
	} else {

		if (HASUB(fp)) {
			FREEUB(fp);
			if ((fp->_r = fp->_ur) != 0) {
				fp->_p = fp->_up;
				return (0);
			}
		}
	}

	if (fp->_bf._base == NULL)
		__smakebuf(fp);

	if (fp->_flags & (__SLBF|__SNBF))
		(void) _fwalk(__lflush);
	fp->_p = fp->_bf._base;
	fp->_r = (*fp->_read)(fp->_cookie, (char *)fp->_p, fp->_bf._size);
	fp->_flags &= ~__SMOD;

	if (fp->_r <= 0) {
		if (fp->_r == 0)
			fp->_flags |= __SEOF;
		else {
			fp->_r = 0;
			fp->_flags |= __SERR;
		}
		return (EOF);
	}
	return (0);
}

