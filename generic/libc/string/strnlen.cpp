/*
 *   File name: strnlen.cpp
 *
 *  Created on: 2017年7月7日, 下午6:46:43
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
size_t  strnlen (const c8*  s, size_t  maxlen)
{
	assert(s != nullptr);

    register size_t stLen = 0;

    while (*s != EOS && maxlen > 0) {
        s++;
        stLen++;
        maxlen--;
    }

    return  (stLen);
}

