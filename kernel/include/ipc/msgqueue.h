/*
 *   File name: msgqueue.h
 *
 *  Created on: 2017年6月2日, 下午6:26:55
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __KERNEL_INCLUDE_MSGQUEUE_H__
#define __KERNEL_INCLUDE_MSGQUEUE_H__

#include <list.h>
#include <lockguard.h>
#include <mutex.h>
#include <errno.h>
#include "message.h"

class MsgQueue
{
public:
	inline explicit MsgQueue(void){}
	MsgQueue(const MsgQueue&) = delete;
	MsgQueue& operator=(const MsgQueue&) = delete;

	bool isEmpty(void)const{ LockGuard<Mutex> g(_mutexLock); return _msgCount == 0; }

	virtual ErrNo getMessage(pvoid msgBuffer, uint bufferSize);
	virtual ErrNo addMessage(pvoid msgBuffer, uint msgSize);
	virtual ErrNo addUrgentMessage(pvoid msgBuffer, uint msgSize);
	uint getCurrentMsgLength(void) const
	{
		LockGuard<Mutex> g(_mutexLock);
		if (_readPtr == _writePtr)
			return 0;
		return ((const GMessagePtr) _readPtr)->_len;
	}

	uint msgCount(void)const { return _msgCount; }

	virtual void clear(void);
	virtual void destroy(void);

private:
	bool init(sint maxMsgSize, sint msgCount);

	u8*  _readPtr;
	u8*  _writePtr;
	u8* _bufferLowAddr;
	u8* _bufferHighAddr;
	uint _msgBlockSize;
	uint _msgCount;

	mutable Mutex _mutexLock;
	Semaphore _msgQueueSem;

	SingleList _sList;

	friend class MsgQueueManager;
};

#endif

