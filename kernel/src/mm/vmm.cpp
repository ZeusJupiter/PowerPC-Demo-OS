/*
 *   File name: vmm.cpp
 *
 *  Created on: 2017年3月23日, 下午6:05:01
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */
#include <macros.h>
#include <types.h>
#include <assert.h>
#include <debug.h>
#include <string.h>

#include <mm/heap.h>
#include <mm/vmm.h>

#include <task/procmanager.h>
#include <task/generalproc.h>
#include <arch/powerpc/mmu.h>

namespace Kernel
{
	namespace MM
	{
		Vmm vmm;

		void Vmm::firstInit(void)
		{
			initRingDoubleList(&_freePageList);
			_freePageCounter = 0;
			for (u32 i = 0; i < OMLayout::VmmSegmentCount; ++i)
			{
				addr_t start = OMLayout::VmmSpaces[i].baseAddr;
				addr_t end = start + OMLayout::VmmSpaces[i].size;

				start = alignUp(start, MM::PageSize);
				end = alignDown(end, MM::PageSize);

				_freePageCounter += end - start;
			}
			_freePageCounter >>= 12;
			kformatln("VMM has %d free page\r\n", _freePageCounter);
		}

		ErrNo Vmm::secondInit(void)
		{
			word_t tmp = _freePageCounter * sizeof(Page);
			addr_t startCtlBlock = (addr_t)kheap.separateFreeBlock(tmp);

			if(startCtlBlock == 0)
			{
				kdebugln("Error, Kernel Memory is too small !!!");
				return ErrNo::ENOMEM;
			}

			addr_t endCtlBlock = startCtlBlock + tmp;
			Page *p;

			tmp = 0;
			for (u32 i = 0; i < OMLayout::VmmSegmentCount && startCtlBlock < endCtlBlock; ++i)
			{
				addr_t start = OMLayout::VmmSpaces[i].baseAddr;
				addr_t end = start + OMLayout::VmmSpaces[i].size;

				start = alignUp(start, MM::PageSize);
				end = alignDown(end, MM::PageSize);

				kformat("PhyMemBlock[%d] start=0x%x, end=0x%x\r\n", i, start, end);

				while(start < end)
				{
					p = (Page*)(startCtlBlock);
					p->_phyAddr = start;
					p->_virAddr = 0;
					addNodeToRingDoubleListTail(&_freePageList, &p->_doubleList);

					start += MM::PageSize;
					startCtlBlock += sizeof(Page);
					++tmp;
				}
			}

			if(tmp < _freePageCounter)
			{
				kformatln("Warning, The number of virtual pages is wrong, and is %d !!!", tmp);
			}

			kdebugln("Virtual Memory Manager has been initialized");
			return ErrNo::ENONE;
		}

		ErrNo Vmm::init(addr_t *from , const addr_t to)
		{
			addr_t header;
			Page *p;

			_freePageCounter = 0;
			header = *from;
			kformat("from=0x%x, to=0x%x\r\n", header, to);
			header = alignUp(header, 4);
			kformat("after align, from=0x%x\r\n", header);

			initRingDoubleList(&_freePageList);
			for (u32 i = 0; i < OMLayout::VmmSegmentCount && header < to; ++i)
			{
				addr_t start = OMLayout::VmmSpaces[i].baseAddr;
				addr_t end = start + OMLayout::VmmSpaces[i].size;
				kformat("PhyMemBlock[%d] Raw start=0x%x, end=0x%x\r\n", i, start, end);

				start = alignUp(start, MM::PageSize);
				end = alignDown(end, MM::PageSize);

				kformat("PhyMemBlock[%d] start=0x%x, end=0x%x\r\n", i, start, end);

				while(start < end && (header + sizeof(Page)) < to)
				{
					p = (Page*)(header);
					p->_phyAddr = start;
					p->_virAddr = 0;
					addNodeToRingDoubleListTail(&_freePageList, &p->_doubleList);

					start += MM::PageSize;
					header += sizeof(Page);
					++_freePageCounter;
				}
			}

			ErrNo ret;
			if (header + sizeof(Page) > to)
			{
				kformatinfoln("%s", "Error!!! OS have No Enough Memory");
				ret = ErrNo::ENOMEM;
			}
			else
			{
				kformatln("VMM has %d free page\r\n", _freePageCounter);
				*from = header;

				_freePageLock.init();
			}

			kdebugln("Virtual Memory Manager has been initialized");
			return ret;
		}

		Page* Vmm::getFreePage(void)
		{
			register Page* ret = 0;
			_freePageLock.lock();
			if(!ringListIsEmpty<DoubleList>(&_freePageList))
			{
				register DoubleList *p = _freePageList.next;
				ret = List_Entry(p, Page, _doubleList);
				delNodeFromRingDoubleList(p);
				--_freePageCounter;
			}
			_freePageLock.unlock();
			return ret;
		}

		void Vmm::releaseProcAllPages(Procedure::GeneralProc* proc)
		{
			DoubleList *p, *n;
			Page* pg;

			_freePageLock.lock();
			RingList_Foreach_Safe(p, n, &proc->_pageHeader)
			{
				pg = List_Entry(p, Page, _doubleList);

				phtab.invalidPTE(pg->_virAddr);
				delNodeFromRingDoubleList(p);
				addNodeToRingDoubleListTail(&_freePageList, p);

				++_freePageCounter;
			}
			_freePageLock.unlock();
		}

		ErrNo Vmm::mapping4KB(Procedure::GeneralProc* const gproc, virtual_addr virAddr, word_t flags, bool isIdentityMapping)
		{
			Page* page = getFreePage();
			if(page == NULL){ return ErrNo::ENOMEM; }

			virAddr = alignDown(virAddr, MM::PageSize);

			gproc->_spinlock.lock();
			addNodeToRingDoubleListTail(&gproc->_pageHeader, &page->_doubleList);
			gproc->_spinlock.unlock();

			isIdentityMapping ? (page->_virAddr = page->_phyAddr) : (page->_virAddr = virAddr);
			phtab.insertPTE(virAddr, page->_phyAddr | flags);
			return ErrNo::ENONE;
		}

		ErrNo Vmm::unmapping4KB(Procedure::GeneralProc* const gproc, virtual_addr virAddr)
		{
			assert(gproc != 0);

			DoubleList * p;
			Page* page = 0;
			ErrNo ret = ErrNo::EFAULT;
			virAddr = alignDown(virAddr, MM::PageSize);

			LockGuard<Spinlock> g(gproc->_spinlock);

			RingList_Foreach(p, &gproc->_pageHeader)
			{
				page = List_Entry(p, Page, _doubleList);
				if(page->_virAddr == virAddr)
				{
					delNodeFromRingDoubleList(p);
					phtab.invalidPTE(virAddr);

					_freePageLock.lock();
					addNodeToRingDoubleListTail(&_freePageList, p);
					++_freePageCounter;
					_freePageLock.unlock();

					ret = ErrNo::ENONE;
					break;
				}
			}

			return ret;
		}
	}
}

