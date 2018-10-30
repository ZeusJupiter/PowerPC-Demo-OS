/*
 *   File name: macros.h
 *
 *  Created on: Nov 9, 2016, 11:54:46 PM
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 */

#ifndef __INCLUDE_TOOLCHAIN_GNU_MACROS_H__
#define __INCLUDE_TOOLCHAIN_GNU_MACROS_H__

#define ALIGNED(x)          __attribute__((__aligned__(x)))
#define DEPRECATED          __attribute__((__deprecated__))
#define NORETURN            __attribute__((__noreturn__))
#define PACKED              __attribute__((__packed__))
#define PACKEDX(x)          x __attribute__((__packed__))
#define PURE                __attribute__((__pure__))
#define SECTION(x)          __attribute__((__section__(x)))
#define TRANSPARENT_UNION   __attribute__((__transparent_union__))
#define USED                __attribute__((__used__))
#define UNUSED              __attribute__((__unused__))
#define WEAK                __attribute__((__weak__))
#define ATTRIBUTE(X)        __attribute__((X))

#define ATTRFORMATPRNT(fun, x)  __attribute__((__format__(fun, x)))
#define ATTRFORMATPRNT2(fun, x, y)  __attribute__((__format__(fun, x, y)))
#define ATTRNONULL(x) __attribute__((nonnull (x)))
#define ATTRNONULL2(x, y) __attribute__((nonnull(x, y)))

#define __constant_p(x)         __builtin_constant_p (x)

#define __frame_address()       __builtin_frame_address (0)
#define __likely(x)             __builtin_expect(!!(x), 1)

#define __unlikely(x)           __builtin_expect(!!(x), 0)
#define __clz(x)                __builtin_clz (x)

#define likely(x)               __likely(x)
#define unlikely(x)             __unlikely(x)

#define RESTRICT                __restrict

#define FUNC_ALIAS(func, aliasToFunc, retType) 			\
    retType aliasToFunc () __attribute__ ((alias (#func)));

#endif

