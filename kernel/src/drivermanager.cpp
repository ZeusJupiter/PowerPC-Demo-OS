/*
 *   File name: drivermanager.cpp
 *
 *  Created on: 2017年5月25日, 下午4:18:16
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
#include <mk/oscfg.h>
#include <drivermanager.h>

namespace Driver
{
	sint DriverManager::_nextId;
	Mutex DriverManager::_mutex;
	DoubleList DriverManager::_usedListHeader;
	DoubleList DriverManager::_freeInherentListHeader;
	DriverManager::DriverNode DriverManager::drivers[16];

	void DriverManager::init(void)
	{
		_nextId = 16 + 1;

		initRingDoubleList(&_usedListHeader);
		initRingDoubleList(&_freeInherentListHeader);

		_mutex.init();

		for (sint i = 0; i < 16; ++i)
		{
			drivers[i]._index = i + 1;

			addNodeToRingDoubleListTail(&_freeInherentListHeader,
					&drivers[i]._listLink);
		}
	}

	sint DriverManager::getFreeDriver(DriverInterface** drv)
	{
		register sint ret = -1;

		{
			LockGuard<Mutex> g(_mutex);

			DriverNode* n = nullptr;
			DoubleList* p = nullptr;

			if (!ringListIsEmpty<DoubleList>(&_freeInherentListHeader))
			{
				p = _freeInherentListHeader.next;
				delNodeFromRingDoubleList(p);
				n = List_Entry(p, DriverNode, _listLink);
			}

			if (n)
			{
				ret = n->_index;
				if (*drv)
					*drv = &n->_driver;
			}

			if (p)
				addNodeToRingDoubleListTail(&_usedListHeader, p);
		}

		return ret;
	}

	DriverInterface* DriverManager::getDriver(sint dIndex)
	{
		register DriverInterface* ret = nullptr;
		assert(dIndex > 0 && dIndex < _nextId);
		ret = &drivers[dIndex - 1]._driver;

		return ret;
	}
}

