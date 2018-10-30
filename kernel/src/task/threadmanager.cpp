/*
 *   File name: threadmanager.cpp
 *
 *  Created on: 2017年4月7日, 上午11:29:31
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */
#include <macros.h>
#include <types.h>
#include <string.h>
#include <lockguard.h>

#include <mm/heap.h>
#include <task/procmanager.h>
#include <task/threadmanager.h>
#include <arch/powerpc/context.h>
#include <arch/powerpc/msr.h>

EXTERN_C void archPpcIRet(void);

namespace Kernel
{
	word_t ThreadManager::ThreadId;
	ThreadManager threadMng;

	void ThreadManager::init(void)
	{
		ThreadId = Procedure::InherentThreadNum;
		_curUsedThreadNum = _maxUsedThreadNum = 0;
		initRingDoubleList(&_freeInherentThreadList);
		for(u32 i = 0; i < Procedure::InherentThreadNum; ++i)
		{
			addNodeToRingDoubleListTail(&_freeInherentThreadList, &(_threadPool[i]._commList));
			_threadPool[i]._id = i;
		}

		kformatln("Thread Size=%d", sizeof(Procedure::Thread));
		kdebugln("Thread Manager has been initialized");
	}

	Procedure::Thread* ThreadManager::allocate(void)
	{

		Procedure::Thread* ret = nullptr;

		{
			LockGuard<Spinlock> g(_inherentThreadSpinlock);
			assert( !ringListIsEmpty<DoubleList>(&_freeInherentThreadList) );
			DoubleList * p = _freeInherentThreadList.next;
			ret = List_Entry(p, Procedure::Thread, _commList);
			delNodeFromRingDoubleList(p);
			++_curUsedThreadNum;
			if(_curUsedThreadNum > _maxUsedThreadNum) _maxUsedThreadNum = _curUsedThreadNum;
		}

		return ret;
	}

	void ThreadManager::deallocate(Procedure::Thread* const thread)
	{
		{
			assert(thread->id() < Procedure::InherentThreadNum);
			LockGuard<Spinlock> g(_inherentThreadSpinlock);
			thread->_state = Procedure::State::Stopped;
			addNodeToRingDoubleListTail(&_freeInherentThreadList, &(thread->_commList));
			--_curUsedThreadNum;
		}
	}

	Context* ThreadManager::createContext(VoidFuncPtrPvoid entry, word_t stackSize, KThread* thread)
	{
		void* ret = kheap.allocate(stackSize, Procedure::ContextAlignment);
		Context* csp = nullptr;
		if(ret != nullptr)
		{
			thread->_startAddr = (addr_t)ret;
			thread->_endAddr = (addr_t)ret + stackSize - 1;

			Context* csp2  = (Context*)((addr_t)ret + stackSize - sizeof(Context));
			csp = (Context*)((char*)csp2 - sizeof(Context));

			zero_block(csp, sizeof(Context) << 1);

			csp2->lr = 0;
			csp2->sp = (addr_t)ret + stackSize;
			csp2->srr0 = (word_t)entry;
			csp2->srr1 = PPC_KERNEL_MSR;

			csp->sp = (word_t)(&csp2->sp);
			csp->lr = (word_t)archPpcIRet;

			thread->_context = csp;
		}

		return csp;
	}

	Procedure::Thread* ThreadManager::createThread(VoidFuncPtrPvoid entry, word_t stackSize)
	{
		Procedure::Thread* thread = allocate();
		if(thread != nullptr)
		{
			thread->_entry = entry;
			createContext(entry, stackSize, thread);

			thread->_proc = nullptr;

			thread->_state = Procedure::State::Ready;
			thread->_io = &thread->_proc->_io;
			thread->_execTicks = 0;
			thread->_slices = 1;
			thread->_totalTicks = thread->_slices * 10;
		}
		return thread;
	}
	void ThreadManager::destroyThread(Procedure::Thread* const thread)
	{
		{
			LockGuard<Spinlock> g(thread->_proc->_spinlock);
			--(thread->_proc->_threadCounter);
			thread->_proc->delThread(thread);
		}

		deallocate(thread);
	}
}

