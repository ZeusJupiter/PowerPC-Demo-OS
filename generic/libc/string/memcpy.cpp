/*
 *   File name: memcpy.cpp
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

BEG_EXT_C

void *memcpy( void *dst, const void *src, size_t size )
{
	assert(dst != nullptr && src != nullptr);

	if( !((word_t)dst % sizeof(word_t)) &&
		!((word_t)src % sizeof(word_t)) &&
		!((word_t)size % sizeof(word_t)) )
		return memcpy_aligned((word_t *)dst, (word_t *)src, size);

	word_t index;
	for(index = 0; index < size; ++index)
		((u8 *)dst)[index] = ((u8 *)src)[index];
	return dst;
}

END_EXT_C
