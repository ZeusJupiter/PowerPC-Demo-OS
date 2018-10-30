/*
 *   File name: stdint.h
 *
 *  Created on: 2017年7月13日, 下午4:51:51
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __INCLUDE_STDINT_H__
#define __INCLUDE_STDINT_H__

#if ( defined(__GNUC__) && __GNUC__ > 3)

#define __STDINT_EXP(x) __##x##__

typedef char c8;

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned int u64 __attribute__((__mode__(__DI__)));

typedef signed char s8;
typedef signed short s16;
typedef signed int s32;
typedef signed int s64 __attribute__((__mode__(__DI__)));

typedef unsigned char uint8_t;
typedef signed char int8_t;
typedef unsigned short uint16_t;
typedef signed short int16_t;
typedef unsigned int uint32_t;
typedef signed int int32_t;

typedef u64 uint64_t;
typedef s64 int64_t;

#include "arch/powerpc/stdint.h"

typedef int8_t int_least8_t;
typedef int16_t int_least16_t;
typedef int32_t int_least32_t;
typedef int64_t int_least64_t;

typedef uint8_t uint_least8_t;
typedef uint16_t uint_least16_t;
typedef uint32_t uint_least32_t;
typedef uint64_t uint_least64_t;

typedef int8_t int_fast8_t;
typedef int16_t int_fast16_t;
typedef int32_t int_fast32_t;
typedef int64_t int_fast64_t;

typedef uint8_t uint_fast8_t;
typedef uint16_t uint_fast16_t;
typedef uint32_t uint_fast32_t;
typedef uint64_t uint_fast64_t;

typedef s64 intmax_t;
typedef u64 uintmax_t;

#define INT8_MIN 	(-128)
#define INT8_MAX 	 (127)
#define UINT8_MAX 	 (255)

#define INT_LEAST8_MIN 	(-128)
#define INT_LEAST8_MAX 	 (127)
#define UINT_LEAST8_MAX	 (255)

#define INT16_MIN 	(-32768)
#define INT16_MAX 	 (32767)
#define UINT16_MAX 	 (65535)

#define INT_LEAST16_MIN	(-32768)
#define INT_LEAST16_MAX	 (32767)
#define UINT_LEAST16_MAX (65535)

#if defined (_INT32_EQ_LONG)
#define INT32_MIN 	 (-2147483647L-1)
#define INT32_MAX 	 (2147483647L)
#define UINT32_MAX       (4294967295UL)
#else
#define INT32_MIN 	 (-2147483647-1)
#define INT32_MAX 	 (2147483647)
#define UINT32_MAX   (4294967295U)
#endif

#if defined (_INT32_EQ_LONG)
#define INT_LEAST32_MIN  (-2147483647L-1)
#define INT_LEAST32_MAX  (2147483647L)
#define UINT_LEAST32_MAX (4294967295UL)
#else
#define INT_LEAST32_MIN  (-2147483647-1)
#define INT_LEAST32_MAX  (2147483647)
#define UINT_LEAST32_MAX (4294967295U)
#endif

#if __have_long64
#define INT64_MIN 	(-9223372036854775807L-1L)
#define INT64_MAX 	 (9223372036854775807L)
#define UINT64_MAX 	(18446744073709551615U)
#elif __have_longlong64
#define INT64_MIN 	 (-9223372036854775807LL-1LL)
#define INT64_MAX 	 (9223372036854775807LL)
#define UINT64_MAX 	 (18446744073709551615ULL)
#endif

#if __have_long64
#define INT_LEAST64_MIN  (-9223372036854775807L-1L)
#define INT_LEAST64_MAX  (9223372036854775807L)
#define UINT_LEAST64_MAX (18446744073709551615U)
#elif __have_longlong64
#define INT_LEAST64_MIN  (-9223372036854775807LL-1LL)
#define INT_LEAST64_MAX  (9223372036854775807LL)
#define UINT_LEAST64_MAX (18446744073709551615ULL)
#endif

#define INT_FAST8_MIN	INT_LEAST8_MIN
#define INT_FAST8_MAX	INT_LEAST8_MAX
#define UINT_FAST8_MAX	UINT_LEAST8_MAX

#define INT_FAST16_MIN	INT_LEAST16_MIN
#define INT_FAST16_MAX	INT_LEAST16_MAX
#define UINT_FAST16_MAX	UINT_LEAST16_MAX

#define INT_FAST32_MIN	INT_LEAST32_MIN
#define INT_FAST32_MAX	INT_LEAST32_MAX
#define UINT_FAST32_MAX	UINT_LEAST32_MAX

#define INT_FAST64_MIN	INT_LEAST64_MIN
#define INT_FAST64_MAX	INT_LEAST64_MAX
#define UINT_FAST64_MAX	UINT_LEAST64_MAX

#define INTMAX_MAX INT64_MAX
#define INTMAX_MIN INT64_MIN

#define UINTMAX_MAX UINT64_MAX

#include "limits.h"

#define SIZE_MAX (__STDINT_EXP(LONG_MAX) * 2UL + 1)

#define SIG_ATOMIC_MIN (-__STDINT_EXP(INT_MAX) - 1)
#define SIG_ATOMIC_MAX (__STDINT_EXP(INT_MAX))

#define INTPTR_MAX (__STDINT_EXP(LONG_MAX))
#define INTPTR_MIN (-__STDINT_EXP(LONG_MAX) - 1)
#define UINTPTR_MAX (__STDINT_EXP(LONG_MAX) * 2UL + 1)

#define PTRDIFF_MAX (__STDINT_EXP(LONG_MAX))
#define PTRDIFF_MIN (-PTRDIFF_MAX - 1)

#ifndef WCHAR_MIN
#ifdef __WCHAR_MIN__
#define WCHAR_MIN (__WCHAR_MIN__)
#elif defined(__WCHAR_UNSIGNED__) || (L'\0' - 1 > 0)
#define WCHAR_MIN (0 + L'\0')
#else
#define WCHAR_MIN (-0x7fffffff - 1 + L'\0')
#endif
#endif

#ifndef WCHAR_MAX
#ifdef __WCHAR_MAX__
#define WCHAR_MAX (__WCHAR_MAX__)
#elif defined(__WCHAR_UNSIGNED__) || (L'\0' - 1 > 0)
#define WCHAR_MAX (0xffffffffu + L'\0')
#else
#define WCHAR_MAX (0x7fffffff + L'\0')
#endif
#endif

#ifdef __WINT_MAX__
#define WINT_MAX (__WINT_MAX__)
#else
#define WINT_MAX (__STDINT_EXP(INT_MAX) * 2U + 1U)
#endif
#ifdef __WINT_MIN__
#define WINT_MIN (__WINT_MIN__)
#else
#define WINT_MIN (0U)
#endif

#define INT8_C(x)	x
#define UINT8_C(x)	x##U

#define INT16_C(x)	x
#define UINT16_C(x)	x##U

#if defined (_INT32_EQ_LONG)
#define INT32_C(x)	x##L
#define UINT32_C(x)	x##UL
#else
#define INT32_C(x)	x
#define UINT32_C(x)	x##U
#endif

#if __have_long64
#define INT64_C(x)	x##L
#define UINT64_C(x)	x##UL
#else
#define INT64_C(x)	x##LL
#define UINT64_C(x)	x##ULL
#endif

#if __have_long64
#define INTMAX_C(x)	x##L
#define UINTMAX_C(x)	x##UL
#else
#define INTMAX_C(x)	x##LL
#define UINTMAX_C(x)	x##ULL
#endif

#endif

#endif

