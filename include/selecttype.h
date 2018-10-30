/*
 *   File name: selecttype.h
 *
 *  Created on: 2017年5月18日, 下午4:03:53
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __INCLUDE_SELECTTYPE_H__
#define __INCLUDE_SELECTTYPE_H__

#include <list.h>
#include <mutex.h>

enum class SelectType : word_t
{
	SelectRead,
	SelectWrite,
	SelectExcept
};

namespace Kernel
{
	namespace Procedure
	{
		class Thread;
	}
}

class SelectWakeupList;

class SelectWakeupNode
{
	friend class SelectWakeupList;

	DoubleList _wakeupList;
	Kernel::Procedure::Thread* _thread;
	u16 _fd;
	u16 _isReady;
	SelectType _selectType;

public:
	SelectType selectType(void)const{ return _selectType; }
	bool isReady(void)const{ return _isReady; }
	u16 fd(void)const{ return _fd; }
};

class SelectWakeupList
{
public:
	bool add(SelectWakeupNode* n);
	void removeNode(SelectWakeupNode* n);
	uint nodeCount(void)const{ return _nodeCount; }
	void wakeup(SelectWakeupNode* n);
	void wakeupAll(SelectType selectType);
	void terminateWakeupList(void);

	explicit SelectWakeupList(void){}
	SelectWakeupList(const SelectWakeupList&) = delete;
	SelectWakeupList& operator=(const SelectWakeupList&) = delete;

	void init(void);
private:
	void terminateWakeupNode(SelectWakeupNode* n);

	Mutex _mutex;
	DoubleList _wakeupListHeader;

	uint _nodeCount;
};

#endif

