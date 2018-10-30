/*
 *   File name: schedule.h
 *
 *  Created on: 2017年4月2日, 下午10:23:15
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __KERNEL_INCLUDE_SCHEDULE_H__
#define __KERNEL_INCLUDE_SCHEDULE_H__

#include <errno.h>
#include <list.h>
#include <spinlock.h>
#include <arch/powerpc/context.h>

#include <kconst.h>

#include <task/thread.h>
#include <task/coroutine.h>

namespace Kernel
{
	namespace Procedure
	{
		class Process;
	}

	typedef struct
	{
	    KThread*  _candidateThread;
	    bool      _isPriorityRotated;
	} CandidateThreadInfo;

	enum class IPIType : uint
	{
		IPI_NOP     =   0,
		IPI_SCHED   =   1,
		IPI_DOWN    =   2,
		IPI_CALL    =   3,
	};

	enum class IPIMask : uint
	{
		IPI_NOP_MSK    =  (1 << static_cast<uint>(IPIType::IPI_NOP) ),
		IPI_SCHED_MSK  =  (1 << static_cast<uint>(IPIType::IPI_SCHED)),
		IPI_DOWN_MSK   =  (1 << static_cast<uint>(IPIType::IPI_DOWN)),
		IPI_CALL_MSK   =  (1 << static_cast<uint>(IPIType::IPI_CALL)),
	};

	class Scheduler
	{
	public:

		explicit Scheduler(void){}
		Scheduler(const Scheduler&) = delete;
		Scheduler& operator=(const Scheduler&) = delete;

		void init(void);
		void start(void);

		void push(Procedure::Thread* newThread);
		void append(Procedure::Thread* newThread);
		void remove(Procedure::Thread* thread);

		sint scheduleInKernel(void);
		void scheduleInInterrupt(void);
		void scheduleInSemaphore(void){ scheduleInInterrupt(); }
		Procedure::Thread* getCurrentThread(void)const{ return _curThread; }

		void curThreadSafe(void)const{ _curThread->ref(); }
		void curthreadUnsafe(void)const{ _curThread->unref(); }

		bool isNesting(void){ return _interruptNestingCounter != 0; }

		void setErrorInNesting(ErrNo err){ _interError[_interruptNestingCounter] = err; }

#if defined(_DEBUG)
		void verify(void);
#else
		void verify(void){}
#endif
	private:

		Procedure::Thread* findNextThread(void);
		Procedure::Thread* _curThread;
		Procedure::Thread* _higherPriorityThread;

		Procedure::CoRoutine* _currentCoRoutine;
		Procedure::CoRoutine* _nextCoRoutine;

		bool _isScheduledByInterrupt;
		s32 _maxPrio;

		CandidateThreadInfo _candThreadInfo;
		DoubleList _readyThreadList;

		DoubleList _ipiMsgListHeader;
		Spinlock _ipiLock;
		uint _ipiMsgCount;

		ulong _kernelStateCounter;

		word_t _ipiVec;
		S32FuncPtrVar _ipiClearFunc;
		pvoid _ipiArg;

		s64 _ipiCount;
		word_t _ipiPendCode;

		cpuid_t _cpuId;
		u32 _cpuState;

		ulong _interruptNestingCounter;
		ulong _interruptNestingMaxVal;
		ErrNo _interError[128];

		FpContext _fpuctxContext[128];

		struct
		{
			DoubleList _header;
			DoubleList* _curNodeInGroup;
			bool isEmpty(void)const{ return ringListIsEmpty<DoubleList>(&_header); }
		} _priorityArray[Procedure::MaxPriority + 1];
	};
}

#if !defined(_KERNEL_SCHEDULER_ALIAS_)
#define _KERNEL_SCHEDULER_ALIAS_ 1
#define KScheduler Kernel::Scheduler
#endif

#endif

