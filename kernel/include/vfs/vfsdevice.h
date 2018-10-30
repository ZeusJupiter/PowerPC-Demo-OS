/*
 *   File name: vfsdevice.h
 *
 *  Created on: 2017年5月7日, 下午2:28:05
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __INCLUDE_VFS_VFSDEVICE_H__
#define __INCLUDE_VFS_VFSDEVICE_H__

#include "vfsnode.h"

namespace FS
{
	class VFSDir;
	class VFS;
	class VFSDevice : public VFSNode
	{
		friend class VFS;
		friend class VFSNode;
	public:

		sint getSize(pid_t  )const{ return 0; }

		void close(pid_t pid );
		void print(void)const;
	private:
		void init(pid_t pid, VFSDir* parent, char* name, mode_t mode);

		explicit VFSDevice(void){}
		VFSDevice(const VFSDevice&) = delete;
		VFSDevice& operator=(const VFSDevice&) = delete;

	};
}

#endif

