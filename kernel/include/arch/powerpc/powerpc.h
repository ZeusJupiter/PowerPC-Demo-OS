/*
 *   File name: power750.h
 *
 *  Created on: 2016年11月26日, 下午7:57:24
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __POWER_H__
#define __POWER_H__

#include <mk/hardware.h>
#include <mk/oscfg.h>
#include "msr.h"

#include <platform/cpucfg.h>

#if POWERPC_FAMILY == POWERPC_MPC750

#include "60x/hid1.h"

#endif

typedef struct stContext Context;

namespace Kernel
{
	class Scheduler;
}

namespace Powerpc
{
	const u32 TimerInterval = Hardware::ISA::BusFrequency/OSCfg::Clock::SystemTicksPreSecond;

    EXTERN_C void contextSwitch(Kernel::Scheduler* scheduler);

	InlineStatic void setDEC(word_t val)
	{
		asm volatile("mtdec %0 \n" : : "r"(val));
	}

	InlineStatic word_t getDEC(void)
	{
		word_t val;
		asm volatile("mfdec %0 \n" : "=r"(val));
		return val;
	}

	InlineStatic s32 powerpcId(void){ return 0; }

	union ProcessorVesion
	{
		u32 raw;
		struct
		{
			u16 version;
			u16 revision;
		};
	};

	InlineStatic u32 processorVesion(void)
	{
		u32 pv;
		__asm__("mfspr %0, 287": "=r"(pv)::);
		return pv;
	}

	InlineStatic u32 getPLL(void){  return ::getHid1(); }

	void init(void);
}

#endif

