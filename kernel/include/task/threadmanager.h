/*
 *   File name: threadmanager.h
 *
 *  Created on: 2017年4月7日, 上午11:15:44
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __KERNEL_INCLUDE_THREADMANAGER_H__
#define __KERNEL_INCLUDE_THREADMANAGER_H__

#include <debug.h>
#include <assert.h>
#include "thread.h"
#include "../kconst.h"

typedef struct stContext Context;

namespace Kernel
{
	class ThreadManager
	{
		static word_t ThreadId;
		static word_t getNewThreadId(void)
		{
			register word_t ret = ThreadId++;
			return ret;
		}

		DoubleList _freeInherentThreadList;

		word_t _curUsedThreadNum;
		word_t _maxUsedThreadNum;

		Procedure::Thread _threadPool[Procedure::InherentThreadNum];
		Spinlock _inherentThreadSpinlock;

		Context* createContext(VoidFuncPtrPvoid entry, word_t stackSize, KThread* thread);
		Procedure::Thread* allocate(void);
		void deallocate(Procedure::Thread* const thread);

		ThreadManager(const ThreadManager&) = delete;
		ThreadManager& operator=(const ThreadManager&) = delete;
	public:

		inline explicit ThreadManager(void){}

		void init(void);
		Procedure::Thread* createThread(VoidFuncPtrPvoid entry, word_t stackSize);
		void destroyThread(Procedure::Thread* const thread);
		Procedure::Thread* getThreadById(word_t tid)
		{
			assert((tid <= Procedure::InherentThreadNum));
			return &_threadPool[tid];
		}
	};

	extern ThreadManager threadMng;
}

#if !defined(_KERNEL_THREADMANAGER_ALIAS_)
#define _KERNEL_THREADMANAGER_ALIAS_ 1
#define kthreadMng Kernel::threadMng
#endif

#endif

