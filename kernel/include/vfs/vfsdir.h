/*
 *   File name: vfsdir.h
 *
 *  Created on: 2017年5月7日, 下午2:29:43
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __INCLUDE_VFS_VFSDIR_H__
#define __INCLUDE_VFS_VFSDIR_H__

#include <lockguard.h>
#include "vfsnode.h"

namespace FS
{
	class VFS;
	class VFSDir : public VFSNode
	{
		friend class VFS;
	public:
		struct VFSDirEntry
		{
			ino_t _nodeId;

			ushort _recLen;
			ushort _nameLen;
		};

		off_t seek(pid_t pid, off_t position, off_t offset, uint whence)const;

		sint getSize(pid_t pid)const;
		sint read(pid_t pid, void* buffer, off_t offset, uint count);
		bool isEmptyDir(void)const{ return ringListIsEmpty<DoubleList>(&_childDirListHeader); }
		void addChild(VFSNode* const child)
		{
			LockGuard<Spinlock> g(_nodeSpinlock);

			ref();
			if(child->nodeType() == VFSNodeType::Directory)
				addNodeToRingDoubleListTail(&_childDirListHeader, &child->_pList);
			else
				addNodeToRingDoubleListTail(&_childFileListHeader, &child->_pList);
		}
		void removeChild(VFSNode* const child){ LockGuard<Spinlock> g(_nodeSpinlock);  _refCount -= 1; delNodeFromRingDoubleList(&child->_pList); }
		void removeChildren(void)
		{
			DoubleList *p, *n;
			VFSNode* child;
			RingList_Foreach_Safe(p, n, &_childDirListHeader)
			{
				child = List_Entry(p, VFSNode, _pList);
				child->doUnref(true);
			}
		}

		VFSNode* renameChild(pid_t pid, const char* oldName, VFSDir* newDir, const char* newName);

		const VFSNode* findAllInDir(const char* subName, uint nameLen, bool lockNeeded = true) const;

		const VFSNode* findFileInDir(const char* subName, uint nameLen, bool lockNeeded = true) const;
		const VFSDir* findDirInDir(const char* subName, uint nameLen, bool lockNeeded = true) const;

		void show(void);

	private:
		explicit VFSDir(void){}
		VFSDir(const VFSDir&) = delete;
		VFSDir& operator=(const VFSDir&) = delete;

		virtual void init(pid_t pid, VFSDir* parent, c8* name, mode_t mode);

		DoubleList _childDirListHeader;
		DoubleList _childFileListHeader;
	};
}

#endif

