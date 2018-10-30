/*
 *   File name: listenermanager.cpp
 *
 *  Created on: 2017年5月5日, 下午5:15:18
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#include <macros.h>
#include <types.h>
#include <listenermanager.h>

namespace Kernel
{
	ListenerManagerImp ListenerManager::_listenerMng;

	Listener* ListenerManager::allocate(void)
	{
		register Listener* ret = nullptr;

		{
			LockGuard<Mutex> g(_listenerMng._freeListenerListLock);
			ret = _listenerMng._freeListenerListHeader;
			if(ret != nullptr)
			{
				_listenerMng._freeListenerListHeader = ret->_next;
				ret->_next = nullptr;
			}
		}

		return ret;
	}
}

