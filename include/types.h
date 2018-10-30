/*
 *   File name: types.h
 *
 *  Created on: 2016年11月16日, 下午9:22:48
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:

 */

#ifndef __INCLUDE_TYPES_H__
#define __INCLUDE_TYPES_H__

#if !defined(ASSEMBLY)

#include <platform/cpucfg.h>

#include "stdint.h"

typedef s32 off_t;
typedef s64 off64_t;
typedef s64 loff_t;

typedef s32 time_t;
typedef u32 clock_t;
typedef s32 clockid_t;
typedef u32 timer_t;

typedef u32 cpuid_t;

typedef u16 tid_t;
typedef u16 pid_t;
typedef u16 uid_t;
typedef u16 gid_t;
typedef u16 mode_t;

typedef u32 ino_t;
typedef u32 block_t;
typedef u32 dev_t;
typedef u16 nlink_t;
typedef u32 blksize_t;
typedef u32 blkcnt_t;

typedef s32 errno_t;

#include "arch/powerpc/types.h"

typedef void * pvoid;

typedef void (*VoidFuncPtrVoid)(void);
typedef u32  (*U32FuncPtrVoid)(void);
typedef void (*VoidFuncPtrU32)(u32);
#if defined(__cplusplus)
typedef void (*VoidFuncPtrVar)(...);
typedef  s32 (*S32FuncPtrVar)(...);
typedef uint (*UintFuncPtrVar)(...);
#else
typedef void (*VoidFuncPtrVar)();
typedef  s32 (*S32FuncPtrVar)();
typedef uint (*UintFuncPtrVar)();
#endif
typedef void (*VoidFuncPtrSint)(sint);
typedef void (*VoidFuncPtrPvoid)(pvoid);
typedef void (*VoidFuncPtrWord)(word_t);
typedef void (*VoidFuncPtrAddr)(addr_t);
typedef word_t (*WordFuncPtrVoid)(void);
typedef s32  (*S32FuncPtrVoid)(void);

inline word_t UNUSED addOffset(word_t arg, word_t offset){ return (arg + offset); }
inline word_t UNUSED mask(word_t arg, word_t mask){ return (arg & mask); }
inline word_t UNUSED alignDown(word_t arg, word_t align){ return arg & (~(align - 1)) ; }
inline word_t UNUSED alignUp(word_t arg, word_t align){ return (arg + (align - 1)) & (~(align - 1)); }
inline bool UNUSED isAlligned(word_t arg, word_t align){ return !(arg & (~(align - 1))); }

#if defined(__cplusplus)
extern "C" {
#endif

extern sint UNUSED findMostSignificantBit64(u64 val);
extern sint UNUSED findLeastSignificantBit64(u64 val);
extern sint UNUSED findMostSignificantBit32(u32 val);
extern sint UNUSED findLeastSignificantBit32(u32 val);

#if defined(__cplusplus)
}
#endif

#if defined(__cplusplus)

template<typename T> inline const T& min(const T& a, const T& b)
{return b > a ? a : b;}

template<typename T> inline const T& max(const T& a, const T& b)
{return b > a ? b : a;}

#endif

#endif

#endif

