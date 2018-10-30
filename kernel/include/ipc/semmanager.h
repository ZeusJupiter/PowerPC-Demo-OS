/*
 *   File name: semmanager.h
 *
 *  Created on: 2017年5月2日, 下午4:32:42
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __KERNEL_INCLUDE_SEMMANAGER_H__
#define __KERNEL_INCLUDE_SEMMANAGER_H__

#include <mutex.h>
#include "../kconst.h"

namespace Kernel
{
	class SemManager
	{
		DoubleList _semListHeader;
		DoubleList _mutexListHeader;

		Semaphore _sems[Procedure::InherentSemaphoreNum];
		Mutex     _mutexes[Procedure::InherentMutexNum];

		SemManager(const SemManager&) = delete;
		SemManager& operator=(const SemManager&) = delete;

	public:
		explicit SemManager(void){}
		void init(void);

		Semaphore* allocate(sint initVal);
		Mutex* allocate(void);

		void deallocate(Semaphore* sem);
		void deallocate(Mutex* m);
	};

	extern SemManager semMng;
}

#endif

