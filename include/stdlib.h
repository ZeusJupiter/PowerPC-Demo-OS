/*
 *   File name: stdlib.h
 *
 *  Created on: 2017年6月26日, 上午12:27:21
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */
#include "stddef.h"

#ifndef __INCLUDE_STDLIB_H__
#define __INCLUDE_STDLIB_H__

#define EXIT_FAILURE  1
#define EXIT_SUCCESS  0

BEG_EXT_C

typedef struct
{
  int quot;

  int rem;

} div_t;

typedef struct
{
  long quot;

  long rem;

} ldiv_t;

typedef struct
{
  long long int quot;

  long long int rem;

} lldiv_t;

__DeclareFunc__(void *, malloc, (size_t  sizeInByte));
__DeclareFunc__(void *, mallocalign, (size_t  sizeInByte, size_t  alignVal));
__DeclareFunc__(void *, aligned_malloc, (size_t  sizeInByte, size_t  alignVal));
__DeclareFunc__(void, aligned_free, (void *freeAddr));
__DeclareFunc__(void *, memalign, (size_t  alignVal, size_t  sizeInByte));
__DeclareFunc__(void, free, (void *freeAddr));
__DeclareFunc__(void *, calloc, (size_t numElements,size_t sizeOfElement));
__DeclareFunc__(void *, realloc, (void *oldAddr, size_t  newSizeByByte));

__DeclareFunc__(void *, xmalloc, (size_t  sizeInByte));
__DeclareFunc__(void *, xmallocalign, (size_t  sizeInByte, size_t  alignVal));
__DeclareFunc__(void *, xmemalign, (size_t  alignVal, size_t  sizeInByte));
__DeclareFunc__(void *, xcalloc, (size_t numElements,size_t sizeOfElement));
__DeclareFunc__(void *, xrealloc, (void *oldAddr, size_t  newSizeByByte));

__DeclareFunc__(int, posix_memalign, (void **memptr, size_t alignVal, size_t sizeInByte));

__DeclareFunc__(void *, malloc_new, (size_t  sizeInByte));

__DeclareFunc__(int, rand, (void));
__DeclareFunc__(int, rand_r, (uint *seedPtr));
__DeclareFunc__(void, srand, (uint  seed));
__DeclareFunc__(void, srandom, (uint seed));
__DeclareFunc__(long, random, (void));

__DeclareFunc__(void, lcong48, (unsigned short p[7]));
__DeclareFunc__(double, erand48, (unsigned short seed[3]));
__DeclareFunc__(double, drand48, (void));
__DeclareFunc__(long, lrand48, (void));
__DeclareFunc__(long, mrand48, (void));
__DeclareFunc__(long, nrand48, (unsigned short seed[3]));
__DeclareFunc__(long, jrand48, (unsigned short seed[3]));
__DeclareFunc__(unsigned short *, seed48, (unsigned short seed[3]));
__DeclareFunc__(void, srand48, (long seed));

__DeclareFunc__(div_t, div, (int numer, int  denom));
__DeclareFunc__(ldiv_t, ldiv, (long  numer, long  denom));
__DeclareFunc__(lldiv_t, lldiv, (s64  numer, s64  denom));
__DeclareFunc__(int, abs, (int  val));
__DeclareFunc__(long, labs, (long  val));
__DeclareFunc__(long long, llabs, (long long val));

__DeclareFunc__(long, strtol, (const char * RESTRICT n, char ** RESTRICT end, register int base));
__DeclareFunc__(unsigned long, strtoul, (const char * RESTRICT str, char ** RESTRICT tailPtr, register int base));
__DeclareFunc__(long long, strtoll, (const char * RESTRICT str, char ** RESTRICT tailPtr, int base));
__DeclareFunc__(unsigned long long, strtoull, (const char * RESTRICT str, char ** RESTRICT tailPtr, int base));

__DeclareFunc__(char *, itoa, (int val, char *str, int radix));
__DeclareFunc__(int, atoi, (const char *str));
__DeclareFunc__(long, atol, (const char *str));
__DeclareFunc__(long long, atoll, (const char *str));

__DeclareFunc__(long double, strtold, (const char * RESTRICT str, char ** RESTRICT tailPtr));
__DeclareFunc__(double, strtod, (const char * RESTRICT str, char ** RESTRICT tailPtr));
__DeclareFunc__(float, strtof, (const char * RESTRICT str, char ** RESTRICT tailPtr));
__DeclareFunc__(double, atof, (const char *str));

__DeclareFunc__(void, qsort, (void *array, size_t elementCount, size_t elementSize,
		int (*cmp)(const void *em1, const void *em2)));

__DeclareFunc__(void *, bsearch, (const void *key, const void * array,size_t count, size_t size,
				int (*compar)(const void * em1, const void * em2)));

__DeclareFunc__(void, abort, (void));
__DeclareFunc__(int, system, (const char *cmd));

__DeclareFunc__(int, atexit, (void (*__func)(void)) );

__DeclareFunc__(int, getenv_r, (const char *namePtr, char * buffer, int bufLen));
__DeclareFunc__(char*, getenv, (const char *namePtr));
__DeclareFunc__(int, putenv, (char *envstr));
__DeclareFunc__(int, setenv, (const char *namePtr, const char *val, int replace));
__DeclareFunc__(int, unsetenv, (const char *namePtr));

__DeclareFunc__(size_t, mbstowcs, (wchar_t * RESTRICT dst, const char * RESTRICT src, size_t len));
__DeclareFunc__(size_t, wcstombs, (char * RESTRICT dst, const wchar_t * RESTRICT src, size_t len));
__DeclareFunc__(int, wctomb, (char *s, wchar_t wch));
__DeclareFunc__(int, mblen, (const char *s, size_t n));
__DeclareFunc__(int, mbtowc, (wchar_t * RESTRICT pwc, const char * RESTRICT s, size_t n) );

END_EXT_C

#endif

