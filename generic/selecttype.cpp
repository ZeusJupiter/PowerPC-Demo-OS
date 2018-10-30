/*
 *   File name: selecttype.cpp
 *
 *  Created on: 2017年5月18日, 下午4:04:52
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
#include <selecttype.h>

bool SelectWakeupList::add(SelectWakeupNode* n)
{
	assert(n != nullptr);
	{
		LockGuard<Mutex> g(_mutex);
		addNodeToRingDoubleListTail(&_wakeupListHeader, &n->_wakeupList);
		++_nodeCount;
	}
	return true;
}

void SelectWakeupList::removeNode(SelectWakeupNode* n)
{
	assert(n != nullptr);
	LockGuard<Mutex> g(_mutex);
	delNodeFromRingDoubleList(&n->_wakeupList);
	--_nodeCount;
}

void SelectWakeupList::wakeup(SelectWakeupNode* n)
{

}

void SelectWakeupList::wakeupAll(SelectType selectType)
{

}

void SelectWakeupList::terminateWakeupList(void)
{

}

void SelectWakeupList::init(void)
{
	_mutex.init();
	initRingDoubleList(&_wakeupListHeader);

	_nodeCount = 0;
}

void SelectWakeupList::terminateWakeupNode(SelectWakeupNode* n)
{

}

