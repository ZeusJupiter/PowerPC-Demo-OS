/*
 *   File name: stddef.h
 *
 *  Created on: 2017年3月21日, 下午11:09:03
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __INCLUDE_STDDEF_H__
#define __INCLUDE_STDDEF_H__

#ifndef	NULL
#define NULL		0
#endif

#ifndef _SIZE_T
#define _SIZE_T
#define _HAVE_SIZE_T
typedef unsigned long size_t;
#endif

#if ( (! defined _HAVE_WCHAR_T) && (! defined __cplusplus ) )
#define _HAVE_WCHAR_T
typedef int	wchar_t;
#endif

#ifndef _SSIZE_T
#define _SSIZE_T
#define _HAVE_SSIZE_T
typedef signed long ssize_t;
#endif

#endif

