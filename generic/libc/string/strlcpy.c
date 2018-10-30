/*
 *   File name: strlcpy.c
 *
 *  Created on: 2017年6月26日, 上午2:28:31
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */
#include <macros.h>
#include <types.h>
#include <string.h>
#include <debug.h>
#include <assert.h>

size_t strlcpy(char * dest, const char * src, size_t destSize)
{
    assert(dest != NULL);
	assert(src != NULL);
	assert(destSize != 0);

	register c8* tmpSrcPtr = (c8*)src;

    while (*tmpSrcPtr != EOS && destSize > 1) {
        *dest++ = *tmpSrcPtr++;
        destSize--;
    }
    *dest = EOS;

    while (*tmpSrcPtr != EOS) {
        tmpSrcPtr++;
    }

    return  ((size_t)(tmpSrcPtr - src));
}
