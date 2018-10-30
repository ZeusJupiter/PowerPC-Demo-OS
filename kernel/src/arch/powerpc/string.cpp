/*
 *   File name: string.cpp
 *
 *  Created on: 2017年3月3日, 下午11:26:55
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
#include <arch/powerpc/cache.h>

EXTERN_C const char __hexChars[] = "0123456789abcdef";

void hex( word_t num, char str[] )
{
#define hexChars(x)  __hexChars[x]

	assert(str != nullptr);

	for (word_t i = 0; i < (sizeof(word_t) << 1); ++i)
	{
		str[(sizeof(word_t) << 1) - 1 - i] = hexChars( (num >> (i << 2)) & 0xf );
	}
	str[ (sizeof(word_t) << 1) ] = '\0';

#undef hexChars
}

void zero_block( void* dst, word_t size )
{
	assert(dst != nullptr);

	register addr_t start = (addr_t)dst;
	register addr_t end = (addr_t)start + (size - 1);
	register addr_t tmp = alignUp((addr_t)start, 4);

	if(tmp >= end)
	{
		byte1:
		for(; start < end; ++start)
			*(u8*)start = 0;
		*(u8*)start = 0;
		return;
	}

	for(; start < tmp; ++start)
		*(u8*)start = 0;

	tmp = alignUp((addr_t)start, Powerpc::Cache::POWERPC_CACHE_LINE_SIZE);
	if(tmp >= end)
	{
		word4:
		tmp = alignDown(end, 4);
		for( ; start < tmp; start += 4)
			*(word_t*)start = 0;
		goto byte1;
	}
	for(; start < tmp; start += 4)
		*(word_t*)start = 0;

	tmp = alignDown(end, Powerpc::Cache::POWERPC_CACHE_LINE_SIZE);
	for(; start < tmp; start += Powerpc::Cache::POWERPC_CACHE_LINE_SIZE)
	{
		Powerpc::Cache::cacheZeroBlock(start);
	}
	goto word4;
}

void memcpy_cache_flush( word_t *dst, const word_t *src, word_t size )
{
	assert(dst != nullptr && src != nullptr);

	int lineWords = Powerpc::Cache::POWERPC_CACHE_LINE_SIZE / sizeof(word_t);
	word_t cnt;
	for(cnt = 0; cnt < size/sizeof(word_t); ++cnt)
	{
		dst[cnt] = src[cnt];
		if( cnt && ((cnt%lineWords) == 0) )
		{
			Powerpc::Cache::cachePartialCodeSync((word_t)dst + cnt);
		}
	}

	Powerpc::Cache::cachePartialCodeSync((word_t)&dst[cnt-1]);
	Powerpc::Cache::cacheCompleteCodeSync();
}

void *memcpy_aligned( word_t *dst, const word_t *src, size_t size )
{
	assert(dst != nullptr && src != nullptr);

	for(word_t i = 0; i < size/sizeof(word_t); ++i)
		dst[i] = src[i];
	return dst;
}

BEG_EXT_C

uint sstrncpy( char *dst, const char *src, size_t n )
{
	assert(dst != nullptr && src != nullptr);

	size_t index = 0;
	while( (index < n) && src[index] )
	{
		dst[index] = src[index];
		++index;
	}

	if(index >= n)
		index = n - 1;
	dst[index] = '\0';

	return (index + 1);
}

END_EXT_C
