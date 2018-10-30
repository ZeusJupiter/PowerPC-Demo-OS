/*
 *   File name: vfslink.cpp
 *
 *  Created on: 2017年5月9日, 下午7:28:22
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#include <macros.h>
#include <types.h>
#include <debug.h>
#include <assert.h>
#include <sys/stat.h>
#include <vfs/vfslink.h>

namespace FS
{
	void VFSLink::init(pid_t pid, VFSDir* parent, char* name, const VFSNode* target)
	{
		_nodeType = VFSNodeType::HardLink;
		VFSNode::init(pid, parent, name, (target->getMode() & MODE_PERM) | MODE_TYPE_HARDLINK);
	}
}

