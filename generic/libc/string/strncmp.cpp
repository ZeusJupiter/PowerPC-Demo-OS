/*
 *   File name: strncmp.cpp
 *
 *  Created on: 2017年6月28日, 上午12:22:14
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
int strncmp( const char *s1, const char *s2, word_t len )
{
	assert(s1 != nullptr && s2 != nullptr);

	while( len > 0 ) {
		if( !*s1 && !*s2 )
			return 0;
		if( *s1 < *s2 )
			return -1;
		if( *s1 > *s2 )
			return 1;
		s1++;
		s2++;
		len--;
	}
	return 0;
}

