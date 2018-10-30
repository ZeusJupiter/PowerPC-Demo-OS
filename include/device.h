/*
 *   File name: device.h
 *
 *  Created on: 2017年5月6日, 下午2:13:00
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __INCLUDE_DEVICE_H__
#define __INCLUDE_DEVICE_H__

#include "assert.h"
#include "string.h"
#include "devtype.h"
#include "atomic.h"

namespace FS
{
	class VFSNode;
	class VFS;
}

namespace Driver
{
	class DriverInterface;
}

class BaseDevice
{
	friend class FS::VFSNode;
	friend class FS::VFS;

	static u32 _nextId;
	static inline u32 nextId(void){ return ++_nextId; }
public:

	DevType devType(void)const{ return _devType; }

	u32 devId(void)const{ return _devId; }
	u32 openCount(void)const{ return _openCount; }

protected:
	explicit BaseDevice(void){}
	BaseDevice(const BaseDevice&) = delete;
	BaseDevice& operator=(const BaseDevice&) = delete;

	virtual void init(DevType devType, Driver::DriverInterface* driver)
	{
		assert(driver != nullptr);

		_devType = devType;
		_driver = driver;
		_devId = nextId();
		_node = nullptr;
	}

	virtual void destroy(void){ _node = nullptr; }

	DevType _devType;
	u32     _devId;
	Driver::DriverInterface* _driver;
	FS::VFSNode* _node;
	Atomic _openCount;
};

#endif

