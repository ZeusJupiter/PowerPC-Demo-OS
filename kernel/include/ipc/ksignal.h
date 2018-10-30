/*
 *   File name: ksignal.h
 *
 *  Created on: 2017年6月13日, 下午3:41:30
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __KERNEL_INCLUDE_KSIGNAL_H__
#define __KERNEL_INCLUDE_KSIGNAL_H__

#include <list.h>
#include <signal.h>

BEG_EXT_C

typedef enum
{
	SEND_OK,
	SEND_ERROR,
	SEND_INFO,
	SEND_IGN,
	SEND_BLOCK
} signal_send_val;

typedef struct sigWait
{
	sigset_t _sigset;
	struct siginfo _siginfo;
} sigwait_t;

typedef struct sigcontext
{
	sigset_t _sigsetSigBlockMask;
	sigset_t _sigsetPending;
	sigset_t _sigsetKill;
	struct sigaction _sigaction[_NSIGS];
	DoubleList   _ringSigPendQueue[_NSIGS];
	signalstack_t _stack;
	sigwait_t* _sigwait;

#if SIGNALFD_EN > 0
	bool _isInReadSigSet;

	sigset_t _waitingSigset;
	SelectWakeupList _selectListForSignalFd;
#endif
} sigcontext_t;

typedef struct
{
	DoubleList _dList;
	struct siginfo _siginfo;
	sigcontext_t* _psigctx;
	uint _times;
	sint _notify;
} sigpend_t;

typedef struct
{
	pvoid _stackRet;
	sint _sigSchedulerRetVal;
	sint _kernelSpace;
	sigset_t _sigsetMask;
	struct siginfo _siginfo;
	errno_t _lastErrno;

#if CPU_FPU_EN > 0
	bool _isNeededRestoreFpu;

	FpContext _fpuContext;
#endif
} sigctlmsg_t;

sint sigTrap(uint  ulId, const union sigval  sigvalue);

END_EXT_C

#endif

