/*
 *   File name: listenermanager.h
 *
 *  Created on: 2017年5月5日, 下午5:11:06
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __KERNEL_INCLUDE_LISTENERMANAGER_H__
#define __KERNEL_INCLUDE_LISTENERMANAGER_H__

#include <listener.h>
#include <mutex.h>
#include <lockguard.h>
#include <kconst.h>

namespace Kernel
{
	class ListenerManager;
	class ListenerManagerImp
	{
		friend class ListenerManager;

		Mutex _freeListenerListLock;
		Listener* _freeListenerListHeader;

		Listener _inherentTimer[100];

		ListenerManagerImp(const ListenerManagerImp&) = delete;
		ListenerManagerImp operator=(const ListenerManagerImp&) = delete;

		explicit ListenerManagerImp(void){}

		void init(void)
		{
			_inherentTimer->_next = nullptr;
			_freeListenerListLock.init();
			_freeListenerListHeader = _inherentTimer;
			for(u32 i = 1; i < 100; ++i)
			{
				_inherentTimer[i]._next = _freeListenerListHeader;
				++_freeListenerListHeader;
			}
		}
	};

	class ListenerManager
	{
		static ListenerManagerImp _listenerMng;
		explicit ListenerManager() = delete;
		ListenerManager(const ListenerManager&) = delete;
		ListenerManager& operator=(const ListenerManager&) = delete;
	public:
		static void init(void){ _listenerMng.init(); }

		static Listener* allocate(void);

		static void deallocate(Listener* const tm)
		{
			LockGuard<Mutex> g(_listenerMng._freeListenerListLock);
			tm->_time = tm->_block = false;
			tm->_next = _listenerMng._freeListenerListHeader;
			_listenerMng._freeListenerListHeader = tm;
		}
	};
}

#if !defined(_KERNEL_TIMERMANAGER_ALIAS_)
#define _KERNEL_TIMERMANAGER_ALIAS_ 1
#define klistenerMng Kernel::ListenerManager
#endif

#endif

