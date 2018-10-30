/*
 *   File name: msgqueuemanager.cpp
 *
 *  Created on: 2017年6月3日, 下午3:40:18
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

#include <kconst.h>
#include <ipc/msgqueuemanager.h>

Mutex MsgQueueManager::mutexLock;
uint MsgQueueManager::usedCount;
uint MsgQueueManager::maxUsedCount;
SingleList MsgQueueManager::freeMsgQueueListHeader;
MsgQueue MsgQueueManager::msgQueueArray[256];

void MsgQueueManager::init(void)
{
	uint i;
	mutexLock.init();
	freeMsgQueueListHeader.next = &msgQueueArray[0]._sList;
	for (i = 0; i < 255; ++i)
	{
		msgQueueArray[i]._sList.next = &msgQueueArray[i + 1]._sList;
	}
	msgQueueArray[i]._sList.next = &freeMsgQueueListHeader;
}

MsgQueue* MsgQueueManager::allocate(sint maxMsgSize, sint msgCount)
{
	if (maxMsgSize < 1 || msgCount < 1)
	{
		return nullptr;
	}

	MsgQueue* ret = nullptr;
	{
		LockGuard<Mutex> g(mutexLock);
		if (freeMsgQueueListHeader.next != &freeMsgQueueListHeader)
		{
			ret = List_Entry(delNodeFromRingSingleListHeader(&freeMsgQueueListHeader), MsgQueue, _sList);

			++usedCount;
			if (usedCount > maxUsedCount)
				maxUsedCount = usedCount;
		}
	}

	if (ret && !ret->init(maxMsgSize, msgCount))
	{
		LockGuard<Mutex> g(mutexLock);
		addNodeToRingSingleListHeader(&freeMsgQueueListHeader, &ret->_sList);
		--usedCount;
		ret = nullptr;
	}

	return ret;
}

void MsgQueueManager::deallocate(MsgQueue* ret)
{
	LockGuard<Mutex> g(mutexLock);
	ret->destroy();
	addNodeToRingSingleListHeader(&freeMsgQueueListHeader, &ret->_sList);
	--usedCount;
}
