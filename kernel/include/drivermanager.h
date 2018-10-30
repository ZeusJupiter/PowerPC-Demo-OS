/*
 *   File name: drivermanager.h
 *
 *  Created on: 2017年5月25日, 下午4:06:22
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __DRIVER_DRIVERMANAGER_H__
#define __DRIVER_DRIVERMANAGER_H__

#include <list.h>
#include <mutex.h>
#include <driverinterface.h>

EXTERN_C void sysBoot(void);

namespace Driver
{
	class DriverManager
	{
		friend void ::sysBoot(void);
		explicit DriverManager(void) = delete;
		DriverManager(const DriverManager&) = delete;
		DriverManager& operator=(const DriverManager&) = delete;

		struct DriverNode
		{
			DoubleList      _listLink;
			DriverInterface _driver;
			sint            _index;
		};

		static sint _nextId;
		static Mutex _mutex;
		static DoubleList _usedListHeader;
		static DoubleList _freeInherentListHeader;

		static DriverNode drivers[];

		static sint nextId(void){ return _nextId++; }

		static void init(void);
	public:

		static sint getFreeDriver(DriverInterface** drv);
		static DriverInterface* getDriver(sint dIndex);
		static inline bool isValidDrvIndex(const sint drvIndex)
		{
			return (drvIndex > 0 && drvIndex < _nextId);
		}
	};
}

#endif

