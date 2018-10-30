/*
 *   File name: strrchr.cpp
 *
 *  Created on: 2017年6月28日, 上午12:23:47
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
char *strrchr(const char *str,int ch)
{
	assert(str != nullptr);

	char *pos = NULL;
	while(*str)
	{
		if(*str == ch)
		{
			pos = const_cast<char*>(str);
		}
		++str;
	}
	return pos;
}

