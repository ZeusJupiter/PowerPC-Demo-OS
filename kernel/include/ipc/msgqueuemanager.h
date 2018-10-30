/*
 *   File name: msgqueuemanager.h
 *
 *  Created on: 2017年6月2日, 下午9:14:47
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __KERNEL_INCLUDE_MSGQUEUEMANAGER_H__
#define __KERNEL_INCLUDE_MSGQUEUEMANAGER_H__

#include <mutex.h>
#include "msgqueue.h"

class MsgQueueManager
{
	explicit MsgQueueManager(void) = delete;
	MsgQueueManager(const MsgQueueManager&) = delete;
	MsgQueueManager& operator=(const MsgQueueManager&) = delete;

	static Mutex mutexLock;
	static uint usedCount;
	static uint maxUsedCount;
	static SingleList freeMsgQueueListHeader;
	static MsgQueue msgQueueArray[];

	static void init(void);

public:
	static MsgQueue* allocate(sint maxMsgSize, sint msgCount);
	static void deallocate(MsgQueue* );
};

#endif

