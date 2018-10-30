/*
 *   File name: clocksettime.cpp
 *
 *  Created on: 2017年7月23日, 下午4:01:18
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */
#include <macros.h>
#include <types.h>
#include <errno.h>

#include <kernel.h>

EXTERN_C void __setError(ErrNo err);

EXTERN_C
int	clock_settime (clockid_t clockid, const struct timespec *tv)
{
	volatile u32 iregInterLevel;

	if (tv == nullptr)
	{
		__setError(ErrNo::EINVAL);
		return -static_cast<sint>(ErrNo::EINVAL);
	}

	if (clockid != CLOCK_REALTIME)
	{
		__setError(ErrNo::EINVAL);
		return -static_cast<sint>(ErrNo::EINVAL);
	}

	KCommKernel::lockKernelAndCloseInterrupt(&iregInterLevel);
    KCommKernel::_currentTOD = *tv;
    KCommKernel::unlockKernelAndRestoreInterrupt(iregInterLevel);

	return 0;
}

