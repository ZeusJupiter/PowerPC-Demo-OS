/*
 *   File name: signalsystem.h
 *
 *  Created on: 2017年6月13日, 下午7:07:55
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __KERNEL_INCLUDE_SIGNALSYSTEM_H__
#define __KERNEL_INCLUDE_SIGNALSYSTEM_H__

#include <list.h>
#include <mutex.h>
#include "ksignal.h"
#include "../task/thread.h"

class SignalSystem
{
	union SignalEventArg {
		SingleList _sList;
	    struct {
	        struct sigevent _sigevent;
	        struct siginfo _siginfo;
	    };
	};

public:
	static void init(void);
	static sigcontext_t* getSignalContextByThread(KThread* thread);

	static void deallocateSigPend(sigpend_t * freeSigPendPtr);

	static void initSigPend(sigpend_t * sigPendPtr);
	static bool runPendedSignal(KThread* thread);

	static sint getOnePendedSignalForRun(sigcontext_t* sigctxPtr, const sigset_t *sigsetPtr, struct siginfo* siginfoPtr);

	static ulong recalcuateSiganlTimeout(ulong orgKernelTime, ulong orgTimeout);

	static signal_send_val doSignal(KThread* thread, sigpend_t* sigpendPtr);
	static signal_send_val doKill(KThread* thread, sint sigNo);
	static signal_send_val doSigQueue(KThread* thread, sint sigNo, const sigval_t sigValue);

private:

	static inline sint signalIndex(sint sigNo)
	{
		return sigNo - 1;
	}

	static void sigShell(sigctlmsg_t* sigCtlMsgPtr);

	static sigpend_t* allocateSigPend(void);

	static void cancelSignal(sint sigNo, struct siginfo *sigInfoPtr);
	static void exitSignal(sint sigNo, struct siginfo *sigInfoPtr);
	static void waitSignal(sint sigNo, struct siginfo* sigInfoPtr);
	static void stopSignal(sint sigNo, struct siginfo* sigInfoPtr);
	static void signalHookWhenCreateThread(KThread* thread);
	static void signalHookWhenDeleteThread(KThread* thread);

	static void signalMakeReady(KThread* thread, sint sigNo, sint* schedulerRetVal, sint signalType);

	static void createSignalContext(KThread* thread, sigcontext_t* sigContextPtr, struct siginfo* siginfoPtr, sint schedulerRet, sigset_t* newMask);

	static void signalReturnToTask(sigcontext_t* sigcontextPtr, KThread* thread, sigctlmsg_t* sigCtlMsgPtr);
	static void runSignal(sigcontext_t* sigctxPtr, sint sigNo, struct siginfo* sigInfoPtr, sigctlmsg_t* sigCtlMsgPtr);

	static bool sigPendRunSelf(void);

	static Mutex _mutex;
	static DoubleList _sigPendFreeRingListHeader;
	static sigcontext_t _sigContextTable[];
	static sigpend_t    _sigPendTable[];

	static SingleList _freeSignalEventListHeader;
	static SignalEventArg _sigEventTable[];

};

#endif

