/*
 *   File name: strncpy.c
 *
 *  Created on: 2017年6月28日, 上午12:38:07
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

char *strncpy(char * s1,

		const char *s2,

		size_t n

		)
{
	register char *d = s1;

	if (n != 0)
	{
		while ((*d++ = *s2++) != 0)

		{
			if (--n == 0)
				return (s1);
		}

		while (--n > 0)
			*d++ = EOS;

	}

	return (s1);
}

