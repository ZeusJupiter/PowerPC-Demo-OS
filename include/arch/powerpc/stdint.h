/*
 *   File name: stdint.h
 *
 *  Created on: 2017年7月14日, 下午4:04:38
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __INCLUDE_ARCH_POWERPC_STDINT_H__
#define __INCLUDE_ARCH_POWERPC_STDINT_H__

#if !defined(ASSEMBLY)

#define _INT32_EQ_LONG 1
#define __have_longlong64 1
#define __have_long64 0

typedef u8  uchar;
typedef u16 ushort;
typedef s32 sint;
typedef u32 uint;

typedef u32 ulong;
typedef s32 slong;
typedef u64 ullong;
typedef s64 sllong;

typedef u32 word_t;
typedef u32 addr_t;

typedef s32 ptrdiff_t;
typedef u32 uintptr_t;

#endif

#endif

