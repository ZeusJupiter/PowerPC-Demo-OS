/*
 *   File name: string.h
 *
 *  Created on: 2017年3月3日, 下午11:16:36
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */
#include <stddef.h>

#ifndef __INCLUDE_STRING_H__
#define __INCLUDE_STRING_H__

extern void hex( word_t num, char str[] );

extern void zero_block( void* dst, word_t size );

extern void memcpy_cache_flush( word_t *dst, const word_t *src, word_t size );
extern void *memcpy_aligned( word_t *dst, const word_t *src, size_t size );

BEG_EXT_C

__DeclareFunc__(void*, memcpy, ( void *dst, const void *src, size_t size ));

__DeclareFunc__(void *, memset, (void * dst, unsigned int c, size_t len));

__DeclareFunc__(void *, memchr, (const void *buf, int ch, size_t count));

__DeclareFunc__(int, memcmp, (const void *a1, const void *a2, size_t size));

__DeclareFunc__(char*, strcat, (char *RESTRICT to, const char *RESTRICT from));

__DeclareFunc__(char*, strncat, ( char * RESTRICT dst, const char * RESTRICT src, size_t n ));

__DeclareFunc__(char*, strncpy, ( char *dst, const char *src, size_t n ));
__DeclareFunc__(uint, sstrncpy, ( char *dst, const char *src, size_t n ));
__DeclareFunc__(word_t, strlen, ( const char *src ));

__DeclareFunc__(int, strcmp, ( const char *s1, const char *s2 ));

__DeclareFunc__( int, strncmp, ( const char *s1, const char *s2, word_t len ));

__DeclareFunc__(char *, strchr, (const char *str, char ch));

__DeclareFunc__( char*, strrchr, (const char *str,int ch));

__DeclareFunc__( char*, strstr, (const char *str1,const char *str2));

__DeclareFunc__(char*, strcpy, (char * RESTRICT _Dest, const char * RESTRICT _Source));

__DeclareFunc__(size_t, strlcpy,(char * dest, const char * src, size_t n));

__DeclareFunc__(size_t,	strnlen, (const char *s, size_t maxlen));

__DeclareFunc__(char *, strsep, (char **string_ptr, const char *delimiter));

END_EXT_C

#endif

