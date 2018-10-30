/*
 *   File name: strcpy.c
 *
 *  Created on: 2017年6月25日, 下午11:15:24
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

char* strcpy(char * to, const char * from)
{
	register char *res = to;

	assert(from != NULL);
	assert(to != NULL);

	while(*from) {
		*to++ = *from++;
	}
	*to = EOS;
	return res;
}

