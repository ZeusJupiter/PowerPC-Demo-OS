/*
 *   File name: macros.h
 *
 *  Created on: 2016年11月15日, 上午1:03:42
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __INCLUDE_MACROS_H__
#define __INCLUDE_MACROS_H__

#if defined(__GNUC__)
#include "toolchain/gnu/macros.h"
#endif

#if defined(__GNUC__)
	#define __FUNC__     ((const char*) (__PRETTY_FUNCTION__))
#elif defined (__STDC_VERSION__) && __STDC_VERSION__ >= 19901L
	#define __FUNC__     ((const char*) (__func__))
#else
	#define __FUNC__     ((const char*) (__FUNCTION__))
#endif

#ifndef NULL
#define NULL   0
#endif

#define EOS  '\0'

#define PATH_SEPARATOR '/'

#if defined(_DEBUG)
#define InlineStatic static inline
#else
#define InlineStatic inline
#endif

#ifndef __DeclareFunc__
#define __DeclareFunc__(retType, func, args) \
	retType func args

#endif

#if !defined(SSTRLEN)

#define SStrLen(str)  (sizeof((str)) - 1)
#endif

#if !defined(ARRAY_SIZE)
#define ARRAY_SIZE(array) (sizeof((array)) / sizeof((array)[0]))
#endif

#if (defined(__GNUC__) && (__GNUC__ > 3))
#define MemberOffset(type, member)  __builtin_offsetof(type, member)
#else
#define MemberOffset(type, field)	((char *) (&((type *) 0)->field))
#endif

#define BaseAddr(memberAddr, type, member) \
	( (type *)( (char*)memberAddr - MemberOffset(type, member) ) )

#define DIV_ROUND(n, d)     (((n) + ((d) / 2)) / (d))
#define DIV_ROUND_UP(n, d)  (((n) + (d) - 1) / (d))

#if (!defined(ASSEMBLY) && !defined(__cplusplus))
typedef int   bool;
#define true  1
#define false 0
#endif

#if defined( __cplusplus )

#if !defined( _EXTERN_C_ )
	#define _EXTERN_C_
	#define EXTERN_C extern "C"
#endif

#if !defined( _BEG_EXT_C_ )
	#define _BEG_EXT_C_
	#define BEG_EXT_C extern "C" {
	#define END_EXT_C }
#endif

#else

#define EXTERN_C
#define BEG_EXT_C
#define END_EXT_C

#endif

#endif

