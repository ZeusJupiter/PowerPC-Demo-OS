/*
 *   File name: vfsdevice.cpp
 *
 *  Created on: 2017年5月9日, 下午6:47:45
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */
#include <macros.h>
#include <types.h>
#include <debug.h>
#include <assert.h>

#include <vfs/vfsdevice.h>

namespace FS
{
	void VFSDevice::init(pid_t pid, VFSDir* parent, char* name, mode_t mode )
	{
		_nodeType = VFSNodeType::Device;

		VFSNode::init(pid, parent, name, mode);
	}

	void VFSDevice::close(pid_t pid )
	{}

	void VFSDevice::print(void)const
	{}
}
