/*
 *   File name: memset.cpp
 *
 *  Created on: 2017年3月3日, 下午11:04:29
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

EXTERN_C void * memset (register void * dst, unsigned int c, register size_t len)
{
	u8 *d = (u8 *)dst;
	u8 *end = (u8* )dst + len;

	if(len >= 2 * sizeof(long) - 1)
	{
		while((long)d & (sizeof(long) - 1))
		{
			*d++ = (char)c;
		}

		register long val = (unsigned char)c;
		val |= (val << 8);
		val |= (val << 16);

	#if (CONFIG_CPU_64BITS > 0)
		val | = (val << 32);
	#endif

		register long* lBufPtr = (long*) d;
		register char* lBufEnd = (char*)((long)end & ~(sizeof(long) - 1));
		do
		{
			*lBufPtr++ = val;
		}while((char*)lBufPtr != lBufEnd);

		d = (u8 *)lBufPtr;
	}

	while(d != end) *d++ = c;
	return dst;
}

