/*
 *   File name: schedule.cpp
 *
 *  Created on: 2017年4月7日, 下午8:26:27
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#include <macros.h>
#include <types.h>

#include <debug.h>
#include <assert.h>

#include <barrier.h>
#include <lockguard.h>
#include <stdio.h>

#include <kernel.h>
#include <task/threadmanager.h>
#include <task/procmanager.h>

namespace Kernel
{
	void Scheduler::init(void)
	{
		_maxPrio = -1;
		_higherPriorityThread = nullptr;
		for(u32 i = 0; i < Procedure::MaxPriority + 1; ++i)
		{
			_priorityArray[i]._curNodeInGroup = &(_priorityArray[i]._header);
			initRingDoubleList(_priorityArray[i]._curNodeInGroup);
		}

		_curThread = kthreadMng.createThread((VoidFuncPtrPvoid)CommonKernel::idle, 2 << 10);
		KProcMng::_rootProc.addThread(_curThread);
		append(_curThread);
		_curThread = kthreadMng.createThread((VoidFuncPtrPvoid)CommonKernel::idle2, 2 << 10);
		KProcMng::_rootProc.addThread(_curThread);
		append(_curThread);

		kdebugln("Scheduler has been initialized");
	}

	void Scheduler::start(void)
	{
    	_curThread->_state = Procedure::State::Running;
		_priorityArray[_curThread->getPriority()]._curNodeInGroup = &(_curThread->_commList);
		CPU::setTick();

		__asm__ __volatile__( \
			"mr     1,    %0   \n" \
			"lwz  19 ,  80(1)  \n" \
			"lwz  20 ,  84(1)  \n" \
			"lwz  21 ,  88(1)  \n" \
			"lwz  22 ,  92(1)  \n" \
			"lwz  23 ,  96(1)  \n" \
			"lwz  24 ,  100(1) \n" \
			"lwz  25 ,  104(1) \n" \
			"lwz  26 ,  108(1) \n" \
			"lwz  27 ,  112(1) \n" \
			"lwz  28 ,  116(1) \n" \
			"lwz  29 ,  120(1) \n" \
			"lwz  30 ,  124(1) \n" \
			"lwz  31 ,  128(1) \n" \
			"lwz  13 ,  56(1)  \n" \
			"lwz  12 ,  52(1)  \n" \
			"lwz  11 ,  48(1)  \n" \
			"lwz  10 ,  44(1)  \n" \
			"lwz   9 ,  40(1)  \n" \
			"lwz   8 ,  36(1)  \n" \
			"lwz   7 ,  32(1)  \n" \
			"lwz   6 ,  28(1)  \n" \
			"lwz   5 ,  24(1)  \n" \
			"lwz   4 ,  20(1)  \n" \
			"lwz   3 ,  16(1)  \n" \
			"lwz   2 ,  12(1)  \n" \
			"lwz   0, 148(1)  \n" \
			"mtlr  0          \n" \
			"addi  1, 1, 156  \n" \
			"xor   0, 0, 0    \n" \
			"mtcr  0          \n" \
			"mtctr 0          \n" \
			"mtxer 0          \n" \
			"blr              \n"
			:
			: "r"(_curThread->_context)
		);
	}

	void Scheduler::push(Procedure::Thread* newThread)
	{
		assert(newThread != nullptr);
		if(newThread->state() == Procedure::State::Ready)
		{
			s32 newPrio = newThread->getPriority();
			LockGuard<Spinlock> g(_ipiLock);
			addNodeToRingDoubleListHeader(_priorityArray[newPrio]._curNodeInGroup, &newThread->_commList);

			if(_maxPrio < newPrio)
			{
				_maxPrio = newPrio;
				_higherPriorityThread = newThread;
			}
		}
	}
	void Scheduler::append(Procedure::Thread* newThread)
	{
		assert(newThread != nullptr);
		if(newThread->state() == Procedure::State::Ready)
		{
			s32 newPrio = newThread->getPriority();
			LockGuard<Spinlock> g(_ipiLock);
			addNodeToRingDoubleListTail(_priorityArray[newPrio]._curNodeInGroup, &newThread->_commList);

			if(_maxPrio < newPrio)
			{
				_maxPrio = newPrio;
				_higherPriorityThread = newThread;
			}
		}
	}

	void Scheduler::remove(Procedure::Thread* delThread)
	{
		assert(delThread != nullptr);
		LockGuard<Spinlock> g(_ipiLock);
		_priorityArray[delThread->getPriority()]._curNodeInGroup = delThread->_commList.next;
		delNodeFromRingDoubleList(&delThread->_commList);
	}

	void Scheduler::scheduleInInterrupt(void)
	{
		++_curThread->_execTicks;
		if(_maxPrio > _curThread->_priority)
		{
			_priorityArray[_maxPrio]._curNodeInGroup = &(_higherPriorityThread->_commList);
			if(_curThread->_execTicks >= _curThread->_totalTicks)
				_curThread->_execTicks = 0;
			goto switchThread;
		}
		else if(_curThread->_execTicks >= _curThread->_totalTicks)
		{
			_curThread->_execTicks = 0;
			KThread * nextThread = findNextThread();
			if(nextThread == _curThread)
				goto back;

			_curThread->_state = Procedure::State::Ready;
			nextThread->_state = Procedure::State::Running;

			_higherPriorityThread = nextThread;
			goto switchThread;
		}

		goto back;

switchThread :
		printk("---Next Thread id = %d, priority=%d---\r\n",
				_higherPriorityThread->id(), _higherPriorityThread->getPriority());
		KCommKernel::switchContext(this);
back:
		return;
	}

	sint Scheduler::scheduleInKernel(void)
	{
		KThread * nextThread = findNextThread();

		printk("---Next Thread id = %d---\r\n", nextThread->id());
		if(nextThread == _curThread)
			return 0;

		_curThread->_state = Procedure::State::Ready;
		nextThread->_state = Procedure::State::Running;
	}

	Procedure::Thread* Scheduler::findNextThread(void)
	{
		DoubleList *p;
		Procedure::Thread* nextThread;

		assert(_maxPrio == _curThread->_priority);

		RingList_Foreach(p, _priorityArray[_maxPrio]._curNodeInGroup)
		{
			if(p != &_priorityArray[_maxPrio]._header)
			{
				nextThread = List_Entry(p, Procedure::Thread, _commList);
				if(nextThread->state() == Procedure::State::Ready)
				{
					_priorityArray[_maxPrio]._curNodeInGroup = p;
					return nextThread;
				}
			}
		}

		nextThread = List_Entry(p, Procedure::Thread, _commList);

		return nextThread;;
	}

#if defined(_DEBUG)
	void Scheduler::verify(void)
	{
		DoubleList *p = nullptr;
		Procedure::Thread* nextThread = nullptr;
		int nodeCounter;
		for(s32 prio = 0; prio < (Procedure::MaxPriority + 1); ++prio)
		{
			if( ! _priorityArray[prio].isEmpty() )
			{
				kformatln("queue[%d] is not empty", prio);
				nodeCounter = 0;
				RingList_Foreach(p, &_priorityArray[prio]._header)
				{
					nextThread = List_Entry(p, Procedure::Thread, _commList);
					kformatln("p=0x%x, thread addr=0x%x ", (addr_t)p, (addr_t)nextThread);

					++nodeCounter;
				}
				kformatln("queue[%d] have %d effective node", prio, nodeCounter);
			}
		}
	}
#endif

}

