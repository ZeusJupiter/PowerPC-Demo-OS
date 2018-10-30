/*
 *   File name: vfsmanager.h
 *
 *  Created on: 2017年5月16日, 下午5:39:40
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __INCLUDE_VFS_VFSMANAGER_H__
#define __INCLUDE_VFS_VFSMANAGER_H__

#include <mk/oscfg.h>
#include <list.h>
#include <mutex.h>
#include <lockguard.h>
#include "vfsfile.h"

namespace FS
{
	class VFS;
	class VFSNode;
	struct VFSDescriptor
	{
		VFSNode* _node;
		bool _forceClose;
		word_t _refCount;
	};

	class VFSManager
	{
	private:
		friend class VFS;

		explicit VFSManager(void) = delete;
		VFSManager(const VFSManager&) = delete;
		VFSManager& operator=(const VFSManager&) = delete;

		static Mutex vfsTreeMutex;

		static DoubleList nodeArrayListHeader;

		static DoubleList freeListHeader;

		static uint allocatedCount;
		static VFSDescriptor fsDescTable[];
		static VFSFile nodeArray[];

		static void init(void);
		static VFSNode* allocate(void);
		static void deallocate(VFSNode* node);

	#if defined(_DEBUG)
		static void verify(void);
	#else
		static void verify(void)
		{}
	#endif

	};
}

#endif

