/*
 *   File name: sscanf.c
 *
 *  Created on: 2017年7月12日, 下午10:19:02
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */
#include <macros.h>
#include <types.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

static int eofread(void *cookie, char *buf, int len)
{
	(void)cookie;
	(void)buf;
	(void)len;
	return (0);
}

int sscanf(const char *str, char const *fmt, ...)
{
	int ret;
	va_list ap;
	FILE f;

	f._flags = __SRD;
	f._bf._base = f._p = (unsigned char *)str;
	f._bf._size = f._r = strlen(str);
	f._read = eofread;
	f._ub._base = NULL;
	f._lb._base = NULL;

	va_start(ap, fmt);

	ret = __svfscanf(&f, fmt, ap);
	va_end(ap);
	return (ret);
}

