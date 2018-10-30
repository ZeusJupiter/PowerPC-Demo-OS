/*
 *   File name: strchr.cpp
 *
 *  Created on: 2017年6月28日, 上午12:22:59
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
char *strchr(const char *s, char c)
{
	assert(s != nullptr);

	while(!*s)
	{
		if(*s == c) return (char*)s;
		++s;
	}

	return NULL;
}

