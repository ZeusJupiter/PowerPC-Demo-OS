/*
 *   File name: ext.h
 *
 *  Created on: 2017年6月28日, 下午10:38:07
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __GENERIC_LIBC_MATH_EXT_EXT_H__
#define __GENERIC_LIBC_MATH_EXT_EXT_H__

typedef u8  UQItype;
typedef s32 SItype;
typedef u32 USItype;
typedef s64 DItype;
typedef u64 UDItype;

#define BITS_PER_UNIT 8

#define W_TYPE_SIZE (4 * BITS_PER_UNIT)

#define Wtype	SItype
#define UWtype	USItype

#define HWtype	SItype
#define UHWtype	USItype

#define DWtype	DItype
#define UDWtype	UDItype

#define L_umoddi3
#define L_udivdi3
#define L_moddi3
#define L_divdi3
#define L_udivmoddi4

struct DWstruct {Wtype high, low;};

typedef union
{
  struct DWstruct s;
  DWtype ll;
} DWunion;

#include "ll.h"

#endif

