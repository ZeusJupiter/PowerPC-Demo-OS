/*
 *   File name: stdarg.h
 *
 *  Created on: 2017年3月19日, 下午8:39:39
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __INCLUDE_STDARG_H__
#define __INCLUDE_STDARG_H__

#define va_start(v,l) __builtin_va_start(v,l)
#define va_end(v) __builtin_va_end(v)
#define va_arg(v,l) __builtin_va_arg(v,l)
#define __va_copy(d,s) __builtin_va_copy((d),(s))
typedef __builtin_va_list va_list;

#endif

