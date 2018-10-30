/*
 *   File name: vfslink.h
 *
 *  Created on: 2017年5月7日, 下午2:31:27
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __INCLUDE_VFS_VFSLINK_H__
#define __INCLUDE_VFS_VFSLINK_H__

#include "vfsnode.h"

namespace FS
{
	class VFS;
	class VFSManager;
	class VFSInterface;
	class VFSDir;
	class VFSLink : public VFSNode
	{
		friend class VFS;
		friend class VFSManager;

		const VFSNode* _target;

		explicit VFSLink(void){}
		VFSLink(const VFSLink&) = delete;
		VFSLink& operator=(const VFSLink&) = delete;

		virtual void init(pid_t pid, VFSDir* parent, char* name, const VFSNode* target);

	public:
		void setTarget(const VFSNode* n){ _target = n; }
		const VFSNode* resolve(void)const{ return _target; }

		sint  getSize(pid_t pid)const{ return _target->getSize(pid); }

	};
}

#endif

