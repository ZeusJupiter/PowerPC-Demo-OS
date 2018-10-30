/*
 *   File name: vfsmanager.cpp
 *
 *  Created on: 2017年5月16日, 下午5:41:31
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#include <macros.h>
#include <types.h>
#include <debug.h>
#include <assert.h>
#include <mm/heap.h>
#include <vfs/vfsmanager.h>

namespace FS
{
	Mutex VFSManager::vfsTreeMutex;
	uint VFSManager::allocatedCount;

	DoubleList VFSManager::freeListHeader;

	VFSDescriptor VFSManager::fsDescTable[200];
	VFSFile VFSManager::nodeArray[200];

	void VFSManager::init(void)
	{
		vfsTreeMutex.init();

		initRingDoubleList(&freeListHeader);

		for(uint i = 0; i < 200; ++i)
		{
			nodeArray[i]._nodeId = i;

			addNodeToRingDoubleListTail(&freeListHeader, &nodeArray[i]._pList);
		}
	}

	VFSNode* VFSManager::allocate(void)
	{
		DoubleList* p ;
		VFSFile* ret = nullptr;

		vfsTreeMutex.lock();
		if(!ringListIsEmpty<DoubleList>(&freeListHeader))
		{
			p = freeListHeader.next;
			delNodeFromRingDoubleList(p);
			ret = List_Entry(p, VFSFile, _pList);

			++allocatedCount;

			fsDescTable[ret->_nodeId]._node = ret;
			fsDescTable[ret->_nodeId]._refCount = 1;
			fsDescTable[ret->_nodeId]._forceClose = false;
		}

		vfsTreeMutex.unlock();
		return ret;
	}

	void VFSManager::deallocate(VFSNode* node)
	{
		vfsTreeMutex.lock();
		--allocatedCount;
		{
			addNodeToRingDoubleListTail(&freeListHeader, &node->_pList);
			node->invalidate();
			fsDescTable[node->_nodeId]._node = nullptr;
			fsDescTable[node->_nodeId]._refCount = 0;
		}
		vfsTreeMutex.unlock();
	}

#if defined(_DEBUG)
	void VFSManager::verify(void)
	{
		LockGuard<Mutex> g(vfsTreeMutex);

		uint freeTotalCounter = 0;
		DoubleList* p;

		RingList_Foreach(p, &freeListHeader)
		{
			++freeTotalCounter;
		}

		kformatln("free Node count=%d, allocated Node Count=%d, total Node Count=%d",
				freeTotalCounter, allocatedCount, 200);

		if(freeTotalCounter + allocatedCount == 200)
		{
			kdebugln("-------- the result of VFSNodeMng::verify is ok -------------");
		}
		else
		{
			kdebugln("-------- the result of VFSNodeMng::verify is failed -------------");
		}
	}
#endif

}

