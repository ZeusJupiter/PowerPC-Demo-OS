/*
 *   File name: assert.h
 *
 *  Created on: 2017年3月16日, 下午11:59:38
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __INCLUDE_ASSERT_H__
#define __INCLUDE_ASSERT_H__

#if ( defined(__GNUC__) && ( __GNUC__  > 3) )

#if defined(_DEBUG)
#define assert(x)  \
do{ \
	if(! __builtin_expect( (x), true) ) \
	{ \
		kformatinfo("%s", "Assertion " #x " failed !!!\r\n"); \
		while(1); \
	} \
}while(false)

#else
#define assert(x) do{}while(0)
#endif

#endif

#endif

