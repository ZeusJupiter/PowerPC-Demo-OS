/*
 *   File name: semaphore.cpp
 *
 *  Created on: 2017年4月10日, 下午9:31:51
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
#include <semaphore.h>

#include <task/thread.h>
#include <kernel.h>

void Semaphore::lock(void)
{
_Lret:
	_spinlock.lock();
	--_val;
	if(_val < 0)
	{
		KThread* curThread = KCommKernel::curThread();
		curThread->suspendSelf();

		assert(curThread->_commList.next == nullptr);

		addNodeToRingDoubleListTail(&_waiterList, &curThread->_commList);
		_spinlock.unlock();

		KCommKernel::curSheduler()->scheduleInSemaphore();

		goto _Lret;

	}
	_spinlock.unlock();
}

bool Semaphore::trylock(void)
{
	bool ret = false;
	{
		LockGuard<Spinlock> g(_spinlock);
		if(_val > 0)
		{
			--_val;
			ret = true;
		}
	}
	return ret;
}

void Semaphore::unlock(void)
{
	register DoubleList *p, *n;
	register KThread* thread;
	LockGuard<Spinlock> g(_spinlock);
	++_val;

	RingList_Foreach_Safe(p, n, &_waiterList)
	{
		++_val;
		delNodeFromRingDoubleList(p);
		thread = List_Entry(p, Kernel::Procedure::Thread, _commList);
		thread->wakeup(thread->id());
	}
}

void Semaphore::destroy(void)
{
	unlock();
}

