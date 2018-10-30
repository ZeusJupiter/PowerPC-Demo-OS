/*
 *   File name: memchr.cpp
 *
 *  Created on: 2017年6月28日, 上午2:11:38
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
void * memchr (const void *buf, int ch, size_t count)
{
	register char* bufPtr = (char*) (buf);

	bufPtr += (count - 1);

	while (bufPtr >= (char*) buf)
	{
		if (*bufPtr == (char) ch)
		{
			return ((pvoid) bufPtr);
		}
		bufPtr--;
	}

	return (nullptr);
}

