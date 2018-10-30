/*
 *   File name: kernel.h
 *
 *  Created on: 2017年6月24日, 下午8:42:48
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __KERNEL_INCLUDE_KERNEL_H__
#define __KERNEL_INCLUDE_KERNEL_H__

#include <time.h>
#include <errno.h>
#include <spinlock.h>

#include <mk/oscfg.h>

#include "task/schedule.h"
#include "cpu.h"
#include "vfs/vfs.h"

namespace Kernel
{
	class Scheduler;

	namespace Procedure
	{
		class Process;
		class GeneralProc;
	}

	class CommonKernel
	{
	public:

		typedef void (*VoidFuncPtrSchedulerPtr)(Scheduler*);

		static const VoidFuncPtrSchedulerPtr switchContext;

    	static const U32FuncPtrVoid closeInterrupt;
    	static const VoidFuncPtrU32 restoreInterrupt;

    	static const VoidFuncPtrVoid ioLock;
    	static const VoidFuncPtrVoid ioUnlock;

		static void handleSysTickInterrupt(void);

		static Scheduler* curSheduler(void){ return &_schedulerTable[CPU::id()]; }
		static Procedure::Thread* curThread(void){ return _schedulerTable[CPU::id()].getCurrentThread(); }

    	static void enterKernel(c8* func);
    	static sint exitKernel(void);

		static inline void init(void)
		{
			_lastTime = CPU::timeStamp();

			_schedulerTable[CPU::id()].init();
			_schedulerTable[CPU::id()].verify();
		}

		static Procedure::GeneralProc* getCurrentProc(void){ return curThread()->_proc; }

		static void setErrorWhenNotRun(ErrNo err){ _errorWhenNotRun = err; }

		static void tickToTimespec(ulong ticks, struct timespec* ts)
		{
		    ts->tv_sec  = (time_t)(ticks / OSCfg::Clock::SystemTicksPreSecond);
		    ts->tv_nsec = (slong)(ticks % OSCfg::Clock::SystemTicksPreSecond) * ((1000 * 1000 * 1000) / OSCfg::Clock::SystemTicksPreSecond);
		}

		static u64 ticks(void){ return _tickCounter; }

	private:
    	static void idle(void);
    	static void idle2(void);
    	static void lockKernelAndCloseInterrupt(volatile u32 *intVal);
    	static void unlockKernelAndRestoreInterrupt(u32 intVal);

    	static void unlockKernelWhenSwitchContext(void);
    	static void justLockKernel(void);

    	static struct timespec _currentTOD;

    	static struct timespec _monoTOD;

		static u64 _lastTime;
		static u64 _tickCounter;

		static u32 _interruptNestingCounter;
		static Spinlock _kernelLock;
		static u32 _kernelCounter;
		static pvoid _kernelOwer;
		static c8* _kernelEnterFunc;
		static ErrNo _errorWhenNotRun;

		static Scheduler _schedulerTable[];

		friend class Scheduler;
		friend int ::clock_settime(clockid_t, const struct timespec *);
		friend int ::clock_gettime(clockid_t, struct timespec *);
		friend int ::clock_nanosleep(clockid_t, int, const struct timespec*, struct timespec*);
	};
};
#if !defined(_COMMON_KERNEL_ALIAS_)
#define _COMMON_KERNEL_ALIAS_ 1
#define KCommKernel Kernel::CommonKernel
#endif

#endif

