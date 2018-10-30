/*
 *   File name: strncat.cpp
 *
 *  Created on: 2017年6月28日, 上午12:34:53
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#include <macros.h>
#include <types.h>
#include <debug.h>
#include <assert.h>
#include <string.h>

EXTERN_C
char* strncat( char *dst, const char *src, size_t n )
{
	assert(dst != nullptr && src != nullptr);
	if (n != 0)
	{
		char *d = dst;

		while (*d++ != EOS) ;

		d--;

		while (((*d++ = *src++) != EOS) && (--n > 0)) ;

		if (n == 0)
			*d = EOS;

	}

	return dst;
}

