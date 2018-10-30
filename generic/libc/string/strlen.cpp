/*
 *   File name: strlen.cpp
 *
 *  Created on: 2017年6月28日, 上午12:18:19
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
word_t strlen( const char *src )
{
	assert(src != nullptr);

	word_t cnt = 0;
	while( src[cnt] ) ++cnt;
	return cnt;
}
