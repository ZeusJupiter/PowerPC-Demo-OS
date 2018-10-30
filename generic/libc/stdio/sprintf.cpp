/*
 *   File name: sprintf.c
 *
 *  Created on: 2017年6月26日, 上午2:52:33
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */
#include <macros.h>
#include <types.h>
#include <stdio.h>
#include <limits.h>
#include <stdarg.h>

BEG_EXT_C
int sprintf(char *str, char const *fmt, ...)
{
	int ret;
	va_list ap;
	FILE f;

	f._flags = __SWR | __SSTR;
	f._bf._base = f._p = (u8 *)str;
	f._bf._size = f._w = INT_MAX;

	va_start(ap, fmt);
	ret = vfprintf(&f, fmt, ap);
	va_end(ap);
	*f._p = 0;
	return (ret);
}
END_EXT_C

