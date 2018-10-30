/*
 *   File name: semmanager.cpp
 *
 *  Created on: 2017年5月2日, 下午5:07:45
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

#include <ipc/semmanager.h>

namespace Kernel
{
	SemManager semMng;

	void SemManager::init(void)
	{
		u32 i;

		initRingDoubleList(&_semListHeader);
		initRingDoubleList(&_mutexListHeader);

		for(i = 0; i < Procedure::InherentSemaphoreNum; ++i)
		{
			addNodeToRingDoubleListTail(&_semListHeader, &_sems[i]._waiterList);
		}

		for(i = 2; i < Procedure::InherentMutexNum; ++i)
		{
			addNodeToRingDoubleListTail(&_mutexListHeader, &_mutexes[i]._sem._waiterList);
		}
	}

	Semaphore* SemManager::allocate(sint initVal)
	{
		Semaphore* ret = nullptr;
		{
			LockGuard<Mutex> g(_mutexes[0]);
			if(!ringListIsEmpty<DoubleList>(&_semListHeader))
			{
				DoubleList* p = _semListHeader.next;
				ret = List_Entry(p, Semaphore, _waiterList);
				delNodeFromRingDoubleList(p);

				ret->init(initVal);
			}
		}

		return ret;
	}

	void SemManager::deallocate(Semaphore* sem)
	{
		assert(sem->_waiterList.next == &sem->_waiterList);
		LockGuard<Mutex> g(_mutexes[0]);
		addNodeToRingDoubleListTail(&_semListHeader, &sem->_waiterList);
	}

	Mutex* SemManager::allocate(void)
	{
		Mutex* ret = nullptr;
		{
			LockGuard<Mutex> g(_mutexes[1]);
			if(!ringListIsEmpty<DoubleList>(&_mutexListHeader))
			{
				DoubleList* p = _mutexListHeader.next;
				ret = List_Entry(p, Mutex, _sem._waiterList);
				delNodeFromRingDoubleList(p);

				ret->init();
			}
		}
		return ret;
	}

	void SemManager::deallocate(Mutex* m)
	{
		assert(m->_sem._waiterList.next == &m->_sem._waiterList);
		LockGuard<Mutex> g(_mutexes[1]);
		addNodeToRingDoubleListTail(&_mutexListHeader, &m->_sem._waiterList);
	}
}

