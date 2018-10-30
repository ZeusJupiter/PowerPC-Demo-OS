/*
 *   File name: heap.cpp
 *
 *  Created on: 2017年3月14日, 下午3:56:26
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#include <macros.h>
#include <types.h>
#include <debug.h>
#include <string.h>

#include <kconst.h>
#include <mm/heap.h>
#include <mm/vmm.h>

namespace
{
	struct HeapSegmentDesc
	{
		struct HeapSegmentDesc * left;
		struct HeapSegmentDesc * right;
		DoubleList doubleList;
		word_t availableSize;
		word_t headerOffset;
	};
#if defined(_DEBUG)
	struct PhyBlockHeader
	{
		struct PhyBlockHeader* next;
	};
#endif
}

namespace Kernel
{
    namespace MM
    {
        Heap kernelHeap;

    	void Heap::addHeapBlock(word_t from, word_t to)

        {
    		kformat("add free Block for heap, from=0x%x, to=0x%x\r\n", from, to);

    		if(_freeRingListHeader.next == nullptr)
    		{
    			initRingDoubleList(&_freeRingListHeader);
    			_start = from;
    			_end = to;
    		}

#if defined(_DEBUG)
    		((PhyBlockHeader*)from)->next = (PhyBlockHeader*)phyBlockHeader;
    		phyBlockHeader = (addr_t)from;

            from += sizeof(PhyBlockHeader);
#endif

    		if(_start > from) _start = from;
    		if(_end < to) _end = to;

            HeapSegmentDesc * node = (HeapSegmentDesc *)from;
            node->right = 0;
            node->left = 0;
            node->headerOffset = sizeof(HeapSegmentDesc);
            node->availableSize = to - from - sizeof(HeapSegmentDesc);

            addNodeToRingDoubleListTail(&_freeRingListHeader, &node->doubleList);
#if defined(_DEBUG)
            counter++;
#endif

            kformat("node=%d aSize=%d hOffset=%d\r\n", (word_t)node, node->availableSize, node->headerOffset);
        }

    	void* Heap::separateFreeBlock(u32 size)
    	{
			DoubleList* p;
			HeapSegmentDesc* ret = nullptr;
			size = alignUp(size, sizeof(addr_t));
			RingList_Foreach(p, &_freeRingListHeader)
			{
				HeapSegmentDesc * node = List_Entry(p, HeapSegmentDesc, doubleList);
				if(node->availableSize + sizeof(HeapSegmentDesc) > size)
				{
					ret = node;
					word_t leftSize = node->availableSize + sizeof(HeapSegmentDesc) - size;
					if(leftSize > sizeof(HeapSegmentDesc))
					{
				

						node = (HeapSegmentDesc *)((addr_t)ret + size);
						node->availableSize = leftSize - sizeof(HeapSegmentDesc);
						node->headerOffset = sizeof(HeapSegmentDesc);

						node->right = ret->right;
						if(node->right) node->right->left = node;

						node->left = nullptr;
						if(ret->left) ret->left->right = nullptr;

						node->availableSize > MM::PageSize ? addNodeToRingDoubleListTail(&_freeRingListHeader, &node->doubleList) :
						        						addNodeToRingDoubleListHeader(&_freeRingListHeader, &node->doubleList);
					}
					else
					{
						if(node->right) node->right->left = nullptr;
						if(node->left) node->left->right = nullptr;
					}
					delNodeFromRingDoubleList(p);

					break;
				}
			}

			return (pvoid)ret;
    	}

        void* Heap::allocate(word_t neededSize)
        {
        	DoubleList * p = 0;
        	addr_t retAddr = 0;
        	HeapSegmentDesc * node = 0;

        	neededSize = alignUp(neededSize, 4);
        	_heapLock.lock();

        	RingList_Foreach(p, &_freeRingListHeader)
        	{
        		node = List_Entry(p, HeapSegmentDesc, doubleList);

          		if(node->availableSize >= neededSize)
        		{
        	

        			word_t leftSize = node->availableSize - neededSize;
        			retAddr = (addr_t)node + node->headerOffset;
        			if(leftSize > sizeof(HeapSegmentDesc))
        			{
        		

        		

        				HeapSegmentDesc * newNode = (HeapSegmentDesc *)(retAddr + neededSize);

        				newNode->right = node->right;
        				newNode->left = node;
        				node->right = newNode;
        				if(newNode->right != 0)  { (newNode->right)->left = newNode; }

        				newNode->availableSize = leftSize - sizeof(HeapSegmentDesc);
        				newNode->headerOffset = sizeof(HeapSegmentDesc);

        				node->availableSize = neededSize;
        				newNode->availableSize > MM::PageSize ? addNodeToRingDoubleListTail(&_freeRingListHeader, &newNode->doubleList) :
        						addNodeToRingDoubleListHeader(&_freeRingListHeader, &newNode->doubleList);
#if defined(_DEBUG)
        				this->counter++;
#endif
        			}
        	

        			delNodeFromRingDoubleList(p);
#if defined(_DEBUG)
        			this->counter--;
#endif
        			break;
        		}
        	}
        	_heapLock.unlock();
        	return (void *)retAddr;
        }

        void* Heap::allocate(word_t neededSize, word_t neededAlignment)
        {
        	addr_t retAddr = 0;
        	HeapSegmentDesc * node = 0;
        	DoubleList * p = 0;
        	word_t availableSize = 0;
        	word_t leftSize = 0;

        	neededSize = alignUp(neededSize, 4);
        	_heapLock.lock();
        	RingList_Foreach(p, &_freeRingListHeader)
        	{
        		node = List_Entry(p, HeapSegmentDesc, doubleList);

        		leftSize = (addr_t)node + node->headerOffset;
        		retAddr = alignUp(leftSize, neededAlignment);
        		availableSize = leftSize + node->availableSize - retAddr;

        		if(availableSize >= neededSize)
        		{
        	

        			HeapSegmentDesc * newNode = (HeapSegmentDesc*)alignDown( retAddr - sizeof(HeapSegmentDesc), sizeof(node->left));
        			if((word_t)newNode > leftSize)
        			{
        		

        				newNode->left = node;
        				newNode->right = node->right;
        				node->right = newNode;
        				if(0 != newNode->right) { (newNode->right)->left = newNode; }

        				node->availableSize = (word_t)newNode - leftSize;
        				newNode->doubleList.next = newNode->doubleList.prev = 0;

        				node = newNode;
        			}
        			else
        			{
        		

        				delNodeFromRingDoubleList(p);
#if defined(_DEBUG)
        				this->counter--;
#endif
        			}

        	

        			node->availableSize = availableSize;
        			node->headerOffset = (word_t)(retAddr - (addr_t)node) ;
        			*((word_t *)retAddr - 1) = node->headerOffset;

        			leftSize = availableSize - neededSize;
        			if(leftSize > sizeof(HeapSegmentDesc))
        			{
        		

        		

        				HeapSegmentDesc * newNode = (HeapSegmentDesc *)(retAddr + neededSize);

        				newNode->right = node->right;
        				newNode->left = node;
        				node->right = newNode;
        				if(0 != newNode->right) { (newNode->right)->left = newNode; }

        				newNode->availableSize = leftSize - sizeof(HeapSegmentDesc);
        				newNode->headerOffset = sizeof(HeapSegmentDesc);

        				node->availableSize = neededSize;
        				newNode->availableSize > MM::PageSize ? addNodeToRingDoubleListTail(&_freeRingListHeader, &newNode->doubleList) :
        				        						addNodeToRingDoubleListHeader(&_freeRingListHeader, &newNode->doubleList);
#if defined(_DEBUG)
        				this->counter++;
#endif
        			}

        			break;
        		}
        	}

        	_heapLock.unlock();
        	return (void *)retAddr;
        }

        void  Heap::free(pvoid baseAddr)
        {
        	if(baseAddr == 0) return;

        	if((addr_t)baseAddr < _start || (addr_t)baseAddr > _end) return;

        	word_t offset = *((word_t *)baseAddr -  1);
        	if(offset < sizeof(HeapSegmentDesc))return;

        	register HeapSegmentDesc * node = (HeapSegmentDesc *)( (word_t)baseAddr - offset);
        	if(node->doubleList.next != 0)

        		return;

        	_heapLock.lock();

        	HeapSegmentDesc* theNode = node->right;
        	if(theNode && 0 != theNode->doubleList.prev)
        	{
        

        		delNodeFromRingDoubleList(&theNode->doubleList);
#if defined(_DEBUG)
        		this->counter--;
#endif

        

				node->right = theNode->right;
				if(0 != theNode->right) { (theNode->right)->left = node; }
				node->availableSize += theNode->availableSize + theNode->headerOffset;
				zero_block((addr_t *)theNode, theNode->headerOffset);

        	}

        	theNode = node->left;
        	if(theNode && 0 != theNode->doubleList.prev)
        	{
        

				theNode->right = node->right;
				if(0 != node->right) { (node->right)->left = theNode; }
				theNode->availableSize += node->availableSize + node->headerOffset;
				zero_block((addr_t *)node, node->headerOffset);

        	}
        	else
        	{
        		if(node->headerOffset > sizeof(HeapSegmentDesc))

        		{
        			node->availableSize += node->headerOffset - sizeof(HeapSegmentDesc);
        			node->headerOffset = sizeof(HeapSegmentDesc);
        		}
        		node->availableSize > MM::PageSize ? addNodeToRingDoubleListTail(&_freeRingListHeader, &node->doubleList) :
        		        						addNodeToRingDoubleListHeader(&_freeRingListHeader, &node->doubleList);
#if defined(_DEBUG)
        		this->counter++;
#endif
        	}

        	_heapLock.unlock();
        }
#if defined(_DEBUG)

        bool Heap::verifyPhyBlock(addr_t addr)
        {
        	register HeapSegmentDesc * n = 0;

        	for(register HeapSegmentDesc * p = (HeapSegmentDesc*)(addr); p ; p = p->right)
        	{
        		if((n = p->right) != 0)
        		{
        			if((addr_t)p + p->headerOffset + p->availableSize != (addr_t)n)
        			{
                		kformat("curNode=0x%x aSize=%d hOffset=%d isUsed=%d\r\n", (addr_t)p, p->availableSize, p->headerOffset);
						kformat("(curNode + curNode->headerOffset + curNode->availableSize) = 0x%x, nextNode =0x%d\r\n",
								(addr_t)p + p->headerOffset + p->availableSize, (addr_t)n);
						return false;
        			}
        		}
        	}

        	return true;
        }

        void Heap::verify(void)
        {
        	kformat("-------------Heap::verify, free block=%d------------\r\n", counter);
        	kformat("---------freelist header=0x%x\r\n", (addr_t)List_FirstNode_Entry(&_freeRingListHeader, HeapSegmentDesc, doubleList));
        	uint index = 0;
        	for(PhyBlockHeader* p = (PhyBlockHeader*)phyBlockHeader; p; p = p->next, ++index)
        	{
        		if(verifyPhyBlock((addr_t)p + sizeof(PhyBlockHeader)))
        		{
        			kformatln("----- Heap::verifyPhyBlock of physic block(%d) is ok -----", index);
        		}
        		else
        		{
        			kformatln("----- Heap::verifyPhyBlock of physic block(%d) is failed -----", index);
        		}
        	}
        }
#endif
    }
}
