/*
 *   File name: cpu.h
 *
 *  Created on: 2017年6月30日, 下午5:38:21
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __KERNEL_INCLUDE_ARCH_POWERPC_CPU_H__
#define __KERNEL_INCLUDE_ARCH_POWERPC_CPU_H__

#include "powerpc.h"

class CPU : public CPUBase
{
	friend class CPUBase;

	explicit CPU() = delete;
	CPU(const CPU&) = delete;
	CPU& operator = (const CPU&) = delete;

public:

};

inline void CPUBase::sleep(void)
{
	register word_t msr = getMSR();

	MSR_SET(msr, PPC_MSR_POW_BIT);
	MSR_SET(msr, PPC_MSR_EE_BIT);

	__asm__ __volatile__("mtmsr %0 \n" : : "r"(msr));
}

inline void CPUBase::setTick(void)
{
	Powerpc::setDEC(Powerpc::TimerInterval);
}

inline void CPUBase::halt(void)
{
	while(true);
}

inline void CPUBase::init(void)
{
	Powerpc::init();
}

inline u64 CPUBase::timeStamp(void)
{
	union
	{
		u64 d;
		struct
		{
			u32 u;
			u32 l;
		};
	} ret;
	ret.u = getTbu();
	ret.l= getTbl();
	return ret.d;
}

inline u32 CPUBase::id(void)
{
	return Powerpc::powerpcId();
}

inline void CPUBase::microsecondDelay(ulong microsecond)
{
    volatile uint  i;

    while (microsecond) {
    	microsecond--;
        for (i = 0; i < 600; i++) {
        }
    }
}

inline void CPUBase::nanosecondDelay(ulong nanosecond)
{
    volatile int  i;
    while (nanosecond) {
    	nanosecond = (nanosecond < 100u) ? (0u) : (nanosecond - 100u);
        for (i = 0; i < 40; i++) {
        }
    }
}

inline void CPUBase::reportSelf(void)
{
	Powerpc::ProcessorVesion pv;
	pv.raw = Powerpc::processorVesion();

	pv.raw = Powerpc::getPLL();
}

#endif

