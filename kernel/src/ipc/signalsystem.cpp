/*
 *   File name: signalcontextmanager.cpp
 *
 *  Created on: 2017年6月16日, 下午9:54:12
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */
#include <macros.h>
#include <types.h>
#include <debug.h>
#include <assert.h>
#include <lockguard.h>
#include <errno.h>

#include <kconst.h>
#include <ipc/signalsystem.h>

Mutex SignalSystem::_mutex;
DoubleList   SignalSystem::_sigPendFreeRingListHeader;
sigcontext_t SignalSystem::_sigContextTable[Kernel::IPC::SignalContextCount];
sigpend_t    SignalSystem::_sigPendTable[100];

SingleList SignalSystem::_freeSignalEventListHeader;
SignalSystem::SignalEventArg SignalSystem::_sigEventTable[20 + _NSIGS];

void SignalSystem::sigShell(sigctlmsg_t* sigCtlMsgPtr)
{}

sigpend_t* SignalSystem::allocateSigPend(void)
{
	sigpend_t* ret = nullptr;
	if(!ringListIsEmpty<DoubleList>(&_sigPendFreeRingListHeader))
	{
		DoubleList * p = _sigPendFreeRingListHeader.next;
		delNodeFromRingDoubleList(p);

		ret = List_Entry(p, sigpend_t, _dList);
	}

	return ret;
}

void SignalSystem::cancelSignal(sint sigNo, struct siginfo *sigInfoPtr)
{}
void SignalSystem::exitSignal(sint sigNo, struct siginfo *sigInfoPtr)
{}
void SignalSystem::waitSignal(sint sigNo, struct siginfo* sigInfoPtr)
{}
void SignalSystem::stopSignal(sint sigNo, struct siginfo* sigInfoPtr)
{}
void SignalSystem::signalHookWhenCreateThread(KThread* thread)
{}
void SignalSystem::signalHookWhenDeleteThread(KThread* thread)
{}

void SignalSystem::signalMakeReady(KThread* thread, sint sigNo, sint* schedulerRetVal, sint signalType)
{}

void SignalSystem::createSignalContext(KThread* thread, sigcontext_t* sigContextPtr, struct siginfo* siginfoPtr, sint schedulerRet, sigset_t* newMask)
{}

void SignalSystem::signalReturnToTask(sigcontext_t* sigcontextPtr, KThread* thread, sigctlmsg_t* sigCtlMsgPtr)
{}
void SignalSystem::runSignal(sigcontext_t* sigctxPtr, sint sigNo, struct siginfo* sigInfoPtr, sigctlmsg_t* sigCtlMsgPtr)
{}

bool SignalSystem::sigPendRunSelf(void)
{}

void SignalSystem::init(void)
{
	register uint i;
	_freeSignalEventListHeader.next = &_sigEventTable[0]._sList;
	for (i = 0; i < 20 + _NSIGS - 1; ++i)
	{
		_sigEventTable[i]._sList.next = &_sigEventTable[i + 1]._sList;
	}
	_sigEventTable[i]._sList.next = &_freeSignalEventListHeader;

	_mutex.init();
	initRingDoubleList(&_sigPendFreeRingListHeader);
	for (i = 0; i < 100; ++i)
	{
		addNodeToRingDoubleListTail(&_sigPendFreeRingListHeader,
				&_sigPendTable[i]._dList);
	}

}

sigcontext_t* SignalSystem::getSignalContextByThread(KThread* thread)
{

	return &_sigContextTable[thread->id()];
}

void SignalSystem::deallocateSigPend(sigpend_t * freeSigPendPtr)
{
	addNodeToRingDoubleListTail(&_sigPendFreeRingListHeader,
			&freeSigPendPtr->_dList);
}

void SignalSystem::initSigPend(sigpend_t * sigPendPtr)
{
	sigPendPtr->_times = 0;
	sigPendPtr->_notify = SIGEV_SIGNAL;
	sigPendPtr->_psigctx = nullptr;
}

sint SignalSystem::getOnePendedSignalForRun(sigcontext_t* sigctxPtr,
		const sigset_t *sigsetPtr, struct siginfo* siginfoPtr)
{
	sigset_t sigsetNeededRun;

	sigsetNeededRun = *sigsetPtr & sigctxPtr->_sigsetPending;
	if (sigsetNeededRun == 0ull)

	{
		return (0);
	}

	sint sigNumNeededRun;
	sint sigIndex;
	sigpend_t* sigpendPtr;

	sigsetNeededRun &= (-sigsetNeededRun);

	sigNumNeededRun = findMostSignificantBit64(sigsetNeededRun);
	sigIndex = signalIndex(sigNumNeededRun);

	if (sigsetNeededRun & sigctxPtr->_sigsetKill)
	{

		sigctxPtr->_sigsetKill &= ~sigsetNeededRun;

		siginfoPtr->_signo = sigNumNeededRun;
		siginfoPtr->_errno = static_cast<sint>(ErrNo::ENONE);
		siginfoPtr->_code = SI_KILL;
		siginfoPtr->_value.sival_ptr = nullptr;

	}
	else
	{

		sigpendPtr = List_FirstNode_Entry(&sigctxPtr->_ringSigPendQueue[sigIndex], sigpend_t, _dList);

		if (sigpendPtr->_times == 0)
		{

			delNodeFromRingDoubleList(&sigpendPtr->_dList);
		}
		else
		{
			sigpendPtr->_times--;
		}

		*siginfoPtr = sigpendPtr->_siginfo;

		if ((sigpendPtr->_siginfo._code != SI_KILL)
				&& (sigpendPtr->_notify == SIGEV_SIGNAL))
		{

			deallocateSigPend(sigpendPtr);

		}
	}

	if (ringListIsEmpty<DoubleList>(&(sigctxPtr->_ringSigPendQueue[sigIndex])))
	{

		sigctxPtr->_sigsetPending &= ~sigsetNeededRun;
	}

	return (sigNumNeededRun);
}

