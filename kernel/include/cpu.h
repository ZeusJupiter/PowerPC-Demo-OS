/*
 *   File name: cpu.h
 *
 *  Created on: 2017年6月26日, 下午4:13:31
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __ARCH_INCLUDE_CPU_H__
#define __ARCH_INCLUDE_CPU_H__

class CPUBase
{
protected:
	explicit CPUBase() = delete;
	CPUBase(const CPUBase&) = delete;
	CPUBase& operator=(const CPUBase&) = delete;

public:
	static void sleep(void);
	static void setTick(void);

	static void halt(void);

	static void init(void);
	static u64 timeStamp(void);
	static u32 id(void);
	static void microsecondDelay(ulong microsecond);
	static void nanosecondDelay(ulong nanosecond);

	static void reportSelf(void);
};

#if ( defined(POWERPC) && (CONFIG_CPU_FAMILY == POWERPC) )
#include <arch/powerpc/cpu.h>
#endif

#endif

