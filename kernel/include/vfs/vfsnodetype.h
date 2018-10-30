/*
 *   File name: vfsnodetype.h
 *
 *  Created on: 2017年5月8日, 下午3:18:33
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __INCLUDE_VFS_VFSNODETYPE_H__
#define __INCLUDE_VFS_VFSNODETYPE_H__

enum class VFSNodeType : word_t
{
	Invalid,
	Directory,
	HardLink,
	SymLink,

	File,
	Device
};

#endif

