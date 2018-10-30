/*
 *   File name: clocknanosleep.cpp
 *
 *  Created on: 2017年7月23日, 下午10:22:17
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
int clock_nanosleep(clockid_t clockid, int flags, const struct timespec * rqtp,
		struct timespec * rmtp)
{
	if ((clockid != CLOCK_REALTIME) && (clockid != CLOCK_MONOTONIC))
	{
		__setError(ErrNo::ENOTSUP);
		return -static_cast<sint>(ErrNo::ENOTSUP);
	}

	if ((!rqtp) || (rqtp->tv_nsec < 0) ||
		(rqtp->tv_nsec >= TIMEVAL_NANOSEC_MAX))
	{
		__setError(ErrNo::EINVAL);
		return -static_cast<sint>(ErrNo::EINVAL);
	}

    struct timespec  tsv;
    tsv = *rqtp;

	if (flags == TIMER_ABSTIME)
	{
		struct timespec tvNow;
		volatile u32 msr;
		KCommKernel::lockKernelAndCloseInterrupt(&msr);
		if (clockid == CLOCK_REALTIME)
		{
			tvNow = KCommKernel::_currentTOD;
		}
		else
		{
			tvNow = KCommKernel::_monoTOD;
		}

		KCommKernel::unlockKernelAndRestoreInterrupt(msr);

		if (lessThanByTimespec(rqtp, &tvNow))
		{
			__setError(ErrNo::EINVAL);
			return -static_cast<sint>(ErrNo::EINVAL);
		}

		subTimespec2(&tsv, &tvNow);

		return (nanosleep(&tsv, nullptr));
	}
	else
	{
		return (nanosleep(&tsv, rmtp));
	}
}
