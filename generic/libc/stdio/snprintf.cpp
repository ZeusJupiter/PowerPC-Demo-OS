/*
 *   File name: snprintf.c
 *
 *  Created on: 2017年6月25日, 下午11:22:35
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */
#include <macros.h>
#include <types.h>
#include <stdio.h>
#include <stdarg.h>

EXTERN_C
int snprintf(char *str, size_t n, const char *fmt, ...)
{
	int res;
	va_list ap;
	va_start(ap,fmt);
	res = vsnprintf(str,n,fmt,ap);
	va_end(ap);
	return res;
}

