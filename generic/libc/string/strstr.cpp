/*
 *   File name: strstr.cpp
 *
 *  Created on: 2017年6月28日, 上午12:25:02
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
char *strstr(const char *srcStr,const char *subStr)
{
	assert(srcStr != nullptr && subStr != nullptr);

	char *res = NULL;
	char *sub;

	if(!*subStr)
		return (char*)srcStr;
	while(*srcStr) {

		if(*srcStr++ == *subStr) {
			res = (char*)--srcStr;
			sub = (char*)subStr;
	
			while(*sub && *srcStr == *sub) {
				srcStr++;
				sub++;
			}

			if(!*sub)
				return res;
		}
	}
	return NULL;
}

