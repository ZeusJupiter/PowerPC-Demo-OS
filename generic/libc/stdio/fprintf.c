/*
 *   File name: fprintf.c
 *
 *  Created on: 2017年7月12日, 下午10:14:39
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#include <macros.h>
#include <types.h>
#include <stdio.h>
#include <stdarg.h>

int fprintf(FILE *fp, const char *fmt, ...)
{
	int ret;
	va_list ap;

	va_start(ap, fmt);
	ret = vfprintf(fp, fmt, ap);
	va_end(ap);
	return (ret);
}

