/*
 *   File name: signal.cpp
 *
 *  Created on: 2017年6月13日, 下午3:49:06
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */
#include <macros.h>
#include <types.h>
#include <debug.h>
#include <assert.h>
#include <signal.h>
#include <errno.h>

BEG_EXT_C

sint sigstack(sigstack_t * newSigStack, sigstack_t * oldSigStack)
{
	signalstack_t stackNew, stackOld;

	sint ret = static_cast<word_t>(ErrNo::ENONE);

	if (newSigStack)
	{
		stackNew.ss_sp = newSigStack->ss_sp;
		stackNew.ss_size = SIGSTKSZ;

		if (newSigStack->ss_onstack)
		{
			stackNew.ss_flags = SS_ONSTACK;
		}
		else
		{
			stackNew.ss_flags = SS_DISABLE;
		}

		if ((ret = sigaltstack(&stackNew, &stackOld)) < 0)
		{
			return (ret);
		}

	}
	else if (oldSigStack)
	{
		sigaltstack(nullptr, &stackOld);
	}

	if (oldSigStack)
	{
		oldSigStack->ss_sp = stackOld.ss_sp;
		if (stackOld.ss_flags & SS_ONSTACK)
		{
			oldSigStack->ss_onstack = 1;
		}
		else
		{
			oldSigStack->ss_onstack = 0;
		}
	}

	return (ret);
}

sighandler_t bsdsignal(sint sigNo, sighandler_t sigHandler)
{}
sighandler_t signal(sint sigNo, sighandler_t sigHandler)
{}

sint raise(sint signum)
{}
sint kill(uint Id, sint sigNo)
{}
sint sigqueue(uint Id, sint sigNo, const union sigval sigvalue)
{}

END_EXT_C

