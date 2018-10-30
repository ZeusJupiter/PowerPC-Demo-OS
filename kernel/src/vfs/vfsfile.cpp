/*
 *   File name: vfsfile.cpp
 *
 *  Created on: 2017年5月9日, 下午7:02:01
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */
#include <macros.h>
#include <types.h>
#include <debug.h>
#include <assert.h>
#include <vfs/vfsfile.h>

namespace FS
{
	void VFSFile::init(pid_t pid, VFSDir* parent, c8* name, mode_t mode)
	{
		_nodeType = VFSNodeType::File;
		_isDynamic = true;
		_bufSize = 0;
		_fileSize = 0;
		_data = nullptr;
		VFSNode::init(pid, parent, name, mode);
	}

	void VFSFile::init(pid_t pid, VFSDir* parent, c8* name, mode_t mode, void* data,
			uint len)
	{
		_nodeType = VFSNodeType::File;
		_isDynamic = false;
		_bufSize = len;
		_fileSize = 0;
		_data = data;
		VFSNode::init(pid, parent, name, mode);
	}

	sint VFSFile::getSize(void) const
	{
		return _fileSize;
	}

	off_t VFSFile::seek(pid_t pid, off_t position, off_t offset, uint whence) const
	{}

	sint VFSFile::read(pid_t pid, void* buffer, off_t offset, uint count)
	{}

	sint VFSFile::write(pid_t pid, const void* buffer, off_t offset, uint count)
	{}

	int VFSFile::truncate(pid_t pid, off_t lenth)
	{}

	int VFSFile::reserve(uint newSize)
	{}
}
