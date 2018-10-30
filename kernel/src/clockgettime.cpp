/*
 *   File name: clockgettime.cpp
 *
 *  Created on: 2017年7月23日, 下午4:34:08
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#include <macros.h>
#include <types.h>
#include <errno.h>

#include <task/generalproc.h>
#include <kernel.h>
#include <motherboard.h>

EXTERN_C void __setError(ErrNo err);

EXTERN_C
int	clock_gettime (clockid_t clockid, struct timespec *tv)
{
	if (tv == nullptr)
	{
		__setError(ErrNo::EINVAL);
		return -static_cast<sint>(ErrNo::EINVAL);
	}

	volatile u32 msr;

	switch (clockid)
	{
	case CLOCK_REALTIME:
		KCommKernel::lockKernelAndCloseInterrupt(&msr);
		*tv = KCommKernel::_currentTOD;
		KCommKernel::unlockKernelAndRestoreInterrupt(msr);
		break;

	case CLOCK_MONOTONIC:
		KCommKernel::lockKernelAndCloseInterrupt(&msr);
		*tv = KCommKernel::_monoTOD;
		KCommKernel::unlockKernelAndRestoreInterrupt(msr);
		break;

	case CLOCK_PROCESS_CPUTIME_ID:
		{
			KGProc *gproc = KCommKernel::getCurrentProc();
			if (gproc == nullptr)
			{
				__setError(ErrNo::ESRCH);
				return -static_cast<sint>(ErrNo::ESRCH);
			}
			KCommKernel::lockKernelAndCloseInterrupt(&msr);
			KCommKernel::tickToTimespec(gproc->getClockUser() + gproc->getClockSystem(), tv);
			KCommKernel::unlockKernelAndRestoreInterrupt(msr);
		}
		break;

	case CLOCK_THREAD_CPUTIME_ID:
		{
			KCommKernel::lockKernelAndCloseInterrupt(&msr);
			KThread* t = KCommKernel::curThread();
			KCommKernel::tickToTimespec(t->getExecTicks(), tv);
			KCommKernel::unlockKernelAndRestoreInterrupt(msr);
		}
		break;

	default:
		__setError(ErrNo::EINVAL);
		return -static_cast<sint>(ErrNo::EINVAL);
	}

	return static_cast<sint>(ErrNo::ENONE);
}
