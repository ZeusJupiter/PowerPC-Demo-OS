/*
 *   File name: procmanager.cpp
 *
 *  Created on: 2017年4月1日, 下午4:28:54
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */
#include <macros.h>
#include <types.h>

#include <debug.h>
#include <assert.h>

#include <lockguard.h>
#include <string.h>

#include <kconst.h>
#include <mm/vmm.h>
#include <mm/heap.h>

#include <task/procmanager.h>
#include <arch/powerpc/msr.h>

#include <vfs/vfs.h>

namespace Kernel
{

	word_t ProcManager::ProcessId;
	DoubleList ProcManager::_usedProcListHeader;
	DoubleList ProcManager::_freeInherentProcListHeader;

	Spinlock ProcManager::_usedProcListSpinlock;
	Spinlock ProcManager::_inherentProcListSpinlock;

	word_t ProcManager::_curUsedProcNum;
	word_t ProcManager::_maxUsedProcNum;

	Procedure::GeneralProc  ProcManager::_rootProc;

	Procedure::Process ProcManager::_inherentProcs[Procedure::InherentProcessNum];

	void ProcManager::init(void)
	{
		ProcessId = Procedure::InherentProcessNum + 1;
		_curUsedProcNum = _maxUsedProcNum = 0;

		initRingDoubleList(&_usedProcListHeader);
		initRingDoubleList(&_freeInherentProcListHeader);

		_rootProc.init(nullptr, 255, FS::VFS::getRootUser());
		_rootProc._processId = 0;

		for(uint i = 0; i < Procedure::InherentProcessNum; ++i)
		{
			_inherentProcs[i]._processId = i + 1;

			addNodeToRingDoubleListTail(&_freeInherentProcListHeader, &_inherentProcs[i]._procList);
		}

		kformatln("Process Size=%d", sizeof(Procedure::Process));
		kdebugln("Process Manager has been initialized");
	}

	void ProcManager::destroyProc(Procedure::Process* desProc)
	{
		desProc->_state = Procedure::State::Stopped;

		if(desProc->_parent)
		{
			(desProc->_parent)->removeChild(desProc);
		}

		{
			LockGuard<Spinlock> g(_usedProcListSpinlock);
			delNodeFromRingDoubleList(&desProc->_procList);
		}

		kvmm.releaseProcAllPages(desProc);
		desProc->_mainThread = nullptr;
		desProc->_priority = 0;
		desProc->_parent = nullptr;

		deallocate(desProc);
	}

	Procedure::Process* ProcManager::createProcess(const char* filename, Procedure::ProcAttr procAttr)
	{

		Procedure::Process* newProc = allocate(procAttr);
		if(newProc != nullptr)
		{
			LockGuard<Spinlock> g(_usedProcListSpinlock);
			addNodeToRingDoubleListTail(&_usedProcListHeader, &newProc->_procList);
		}

		return newProc;
	}

	Procedure::Process* ProcManager::allocate(Procedure::ProcAttr procAttr)
	{
		Procedure::Process* ret = nullptr;
		switch(procAttr)
		{
		case Procedure::ProcAttr::RealtimeProc:
			{
				LockGuard<Spinlock> g(_inherentProcListSpinlock);
				assert( _freeInherentProcListHeader.next != &_freeInherentProcListHeader );
				DoubleList * p = _freeInherentProcListHeader.next;
				ret = List_Entry(p, Procedure::Process, _procList);
				delNodeFromRingDoubleList(p);
				++_curUsedProcNum;
				if(_curUsedProcNum > _maxUsedProcNum) _maxUsedProcNum = _curUsedProcNum;
			}
			break;
		case Procedure::ProcAttr::NormalProc:
			{
				ret = static_cast<Procedure::Process*>(kheap.allocate(sizeof(Procedure::Process)));

				zero_block((void*)ret, sizeof(Procedure::Process));

				ret->_processId = getNewProcessId();
			}
			break;
		}

		return ret;
	}

	void ProcManager::deallocate(Procedure::Process* const process)
	{
		if(process->processId() < Procedure::InherentProcessNum + 1)
		{
			LockGuard<Spinlock> g(_inherentProcListSpinlock);
			addNodeToRingDoubleListTail(&_freeInherentProcListHeader, &(process->_procList));
			--_curUsedProcNum;
		}
	}
}

