/*
 *   File name: msgqueue.cpp
 *
 *  Created on: 2017年6月2日, 下午4:28:37
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */
#include <macros.h>
#include <types.h>
#include <debug.h>
#include <assert.h>
#include <string.h>

#include <ipc/msgqueue.h>
#include <mm/heap.h>

bool MsgQueue::init(sint maxMsgSize, sint capacity)
{
	_msgBlockSize = alignUp(maxMsgSize + sizeof(uint), sizeof(uint));
	register uint size = capacity * (_msgBlockSize);
	_bufferLowAddr = (u8*)kheap.allocate(size);

	if (_bufferLowAddr == nullptr)
		return false;

	_bufferHighAddr = _bufferLowAddr + size;
	_readPtr = _writePtr = _bufferLowAddr;

	_msgCount = 0;
	_mutexLock.init();
	_msgQueueSem.init(0);

	return true;
}

ErrNo MsgQueue::getMessage(pvoid msgBuffer, uint bufferSize)
{
	if (msgBuffer == nullptr)
	{
		return ErrNo::EFAULT;
	}

	_msgQueueSem.lock();

	LockGuard<Mutex> g(_mutexLock);

	if (_readPtr == _writePtr)
		return ErrNo::EBUSY;

	register GMessagePtr msg = (GMessagePtr) (_readPtr);

	assert(bufferSize >= msg->_len);

	memcpy(msgBuffer, msg->_body, msg->_len);

	_readPtr += _msgBlockSize;

	--_msgCount;

	if (_readPtr >= _bufferHighAddr)
	{
		_readPtr = _bufferLowAddr;
	}

	return ErrNo::ENONE;
}

ErrNo MsgQueue::addMessage(pvoid msgBuffer, uint msgSize)
{
	if (msgBuffer == nullptr || msgSize + sizeof(uint) > _msgBlockSize || msgSize < 1)
		return ErrNo::EFAULT;

	LockGuard<Mutex> g(_mutexLock);

	register GMessagePtr msg = (GMessagePtr)(_writePtr);
	msg->_len = msgSize;
	memcpy(msgBuffer, msg->_body, msgSize);

	_writePtr += _msgBlockSize;

	++_msgCount;

	if (_writePtr >= _bufferHighAddr)
	{
		_writePtr = _bufferLowAddr;
	}

	_msgQueueSem.unlock();

	return ErrNo::ENONE;
}

ErrNo MsgQueue::addUrgentMessage(pvoid msgBuffer, uint msgSize)
{
	if (msgBuffer == nullptr || msgSize + sizeof(uint) > _msgBlockSize || msgSize < 1)
		return ErrNo::EFAULT;

	LockGuard<Mutex> g(_mutexLock);

	if (_readPtr <= _bufferLowAddr)
	{
		_readPtr = _bufferHighAddr;
	}

	_readPtr -= _msgBlockSize;

	register GMessagePtr msg = (GMessagePtr)(_readPtr);
	msg->_len = msgSize;
	memcpy(msgBuffer, msg->_body, msgSize);

	++_msgCount;

	_msgQueueSem.unlock();

	return ErrNo::ENONE;
}

void MsgQueue::clear(void)
{
	_readPtr = _writePtr = _bufferLowAddr;
	_msgCount = 0;
	_msgQueueSem.destroy();
	_mutexLock.destroy();
}

void MsgQueue::destroy(void)
{
	_msgQueueSem.destroy();
	_mutexLock.destroy();

	kheap.free(_bufferLowAddr);

	_bufferHighAddr = _bufferLowAddr = nullptr;
	_readPtr = _writePtr = nullptr;

	_msgBlockSize = _msgCount = 0;
}

