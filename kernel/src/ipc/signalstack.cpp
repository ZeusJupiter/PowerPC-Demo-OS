/*
 *   File name: signalstack.cpp
 *
 *  Created on: 2017年6月13日, 下午4:13:40
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */
#include <macros.h>
#include <types.h>
#include <debug.h>
#include <assert.h>
#include <errno.h>

#include <kconst.h>
#include <kernel.h>

#include <ipc/ksignal.h>
#include <ipc/signalsystem.h>
#include <task/schedule.h>

EXTERN_C
void __setError(ErrNo err);

EXTERN_C
sint sigaltstack(const signalstack_t * newSignalStack, signalstack_t *oldSignalStack)
{
    sigcontext_t*  sigctxPtr;
    signalstack_t* sigstackPtr;

	if (newSignalStack && (newSignalStack->ss_flags & SS_ONSTACK))
	{
		if (!newSignalStack->ss_sp || !isAlligned((word_t)(newSignalStack->ss_sp), 4))
		{
			__setError(ErrNo::EINVAL);

			return -(static_cast<sint>(ErrNo::EINVAL));
		}

		if (newSignalStack->ss_size < MINSIGSTKSZ)
		{
			__setError(ErrNo::ENOMEM);
			return -(static_cast<sint>(ErrNo::ENOMEM));
		}
	}

    KThread* curThread = KCommKernel::curThread();

	sigctxPtr = SignalSystem::getSignalContextByThread(curThread);
	if (oldSignalStack)
	{
		*oldSignalStack = sigctxPtr->_stack;
	}
	if (newSignalStack)
	{
		sigstackPtr = &sigctxPtr->_stack;
		if (sigstackPtr->ss_flags & SS_ONSTACK)
		{
			if ((curThread->_context >= (Context*) sigstackPtr->ss_sp)
					&& (curThread->_context < (Context*)((size_t) sigstackPtr->ss_sp + sigstackPtr->ss_size)))
			{
				__setError(ErrNo::EPERM);
				KCommKernel::exitKernel();
				return -(static_cast<sint>(ErrNo::EINVAL));
			}
		}
		*sigstackPtr = *newSignalStack;

		alignDown(sigstackPtr->ss_size, 4);
	}

    return -(static_cast<sint>(ErrNo::ENONE));
}

