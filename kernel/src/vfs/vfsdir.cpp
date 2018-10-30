/*
 *   File name: vfsdir.cpp
 *
 *  Created on: 2017年5月9日, 下午6:00:37
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

#include <mm/heap.h>
#include <vfs/vfs.h>
#include <vfs/vfsdir.h>

#include <stdio.h>

#include <lockguard.h>

namespace FS
{
	void VFSDir::init(pid_t pid, VFSDir* parent, c8* name, mode_t mode)
	{
		_nodeType = VFSNodeType::Directory;
		initRingDoubleList(&_childDirListHeader);
		initRingDoubleList(&_childFileListHeader);
		VFSNode::init(pid, parent, name, mode);
	}

	off_t VFSDir::seek(pid_t pid, off_t position, off_t offset, uint whence)const
	{}

	sint VFSDir::getSize(pid_t pid)const
	{
		assert(pid == this->_ower);

		register sint byteCount = 0;
		DoubleList *p;
		RingList_Foreach(p, &_childFileListHeader)
		{
			VFSNode* n = List_Entry(p, VFSNode, _pList);
			byteCount += sizeof(VFSDirEntry) + _nameLen;
		}

		RingList_Foreach(p, &_childDirListHeader)
		{
			VFSNode* n = List_Entry(p, VFSNode, _pList);
			byteCount += sizeof(VFSDirEntry) + _nameLen;
		}

		return byteCount;
	}

	sint VFSDir::read(pid_t pid, void* buffer, off_t offset, uint count)
	{}

	VFSNode* VFSDir::renameChild(pid_t pid, const char* oldName, VFSDir* newDir, const char* newName)
	{
		uint nameLen = strlen(newName);
		char* namecpy = (char*)kheap.allocate(nameLen + 1);
		if(namecpy == nullptr)
			return nullptr;
		memcpy(namecpy, newName, nameLen + 1);

		VFS::lockTree();
		VFSNode* target = const_cast<VFSNode*>(static_cast<VFSDir*>(this)->findAllInDir(oldName, nameLen, false));

		if(!target || !canRemove(pid, target))
		{
			kheap.free(namecpy);
			VFS::unlockTree();
			return nullptr;
		}

		if(newDir != this)
		{
			target->detach(true);

			newDir->addChild(target);

		}
		else
		{
			if(target->_needReleased)
			{
				kheap.free(const_cast<char*>(target->_name));
				target->_name = nullptr;
			}
		}

		target->_name = namecpy;
		target->_nameLen = nameLen;

		VFS::unlockTree();

		return target;
	}

	const VFSNode* VFSDir::findFileInDir(const char* subName, uint nameLen, bool lockNeeded) const
	{
		if(subName == nullptr || nameLen == 0) return nullptr;

		if(lockNeeded) _nodeSpinlock.lock();

		register DoubleList* p;
		register const VFSNode* n = nullptr;

		RingList_Foreach(p, &_childFileListHeader)
		{
			n = List_Entry(p, VFSNode, _pList);

			if(n->_nameLen == nameLen && strncmp(n->_name, subName, nameLen) == 0)
			{
				if(lockNeeded) _nodeSpinlock.unlock();
				return n;
			}
		}
		if(lockNeeded) _nodeSpinlock.unlock();

		return nullptr;
	}

	const VFSDir* VFSDir::findDirInDir(const char* subName, uint nameLen, bool lockNeeded) const
	{
		if(subName == nullptr || nameLen == 0) return nullptr;

		if(lockNeeded) _nodeSpinlock.lock();

		register DoubleList* p;
		register const VFSDir* n = nullptr;

		RingList_Foreach(p, &_childDirListHeader)
		{
			n = List_Entry(p, VFSDir, _pList);

			if(n->_nameLen == nameLen && strncmp(n->_name, subName, nameLen) == 0)
			{
				if(lockNeeded) _nodeSpinlock.unlock();
				return n;
			}
		}

		if(lockNeeded) _nodeSpinlock.unlock();

		return nullptr;
	}

	const VFSNode* VFSDir::findAllInDir(const char* subName, uint nameLen, bool lockNeeded) const
	{
		if(subName == nullptr || nameLen == 0) return nullptr;

		if(lockNeeded) _nodeSpinlock.lock();

		register DoubleList* p;
		register const VFSNode* n = nullptr;

		RingList_Foreach(p, &_childFileListHeader)
		{
			n = List_Entry(p, VFSNode, _pList);

			if(n->_nameLen == nameLen && strncmp(n->_name, subName, nameLen) == 0)
			{
				if(lockNeeded) _nodeSpinlock.unlock();
				return n;
			}
		}

		RingList_Foreach(p, &_childDirListHeader)
		{
			n = List_Entry(p, VFSNode, _pList);

			if(n->_nameLen == nameLen && strncmp(n->_name, subName, nameLen) == 0)
			{
				if(lockNeeded) _nodeSpinlock.unlock();
				return n;
			}
		}

		if(lockNeeded) _nodeSpinlock.unlock();

		return nullptr;
	}

	void VFSDir::show(void)
	{
		register DoubleList* p;
		register const VFSNode* n;

		{
			LockGuard<Spinlock> g(_nodeSpinlock);
			RingList_Foreach(p, &_childDirListHeader)
			{
				n = List_Entry(p, VFSDir, _pList);
				printf("%s ", n->_name);
			}

			RingList_Foreach(p, &_childFileListHeader)
			{
				n = List_Entry(p, VFSNode, _pList);
				printf("%s ", n->_name);
			}
		}
	}
}
