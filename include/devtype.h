/*
 *   File name: devtype.h
 *
 *  Created on: 2017年5月8日, 下午4:47:26
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __INCLUDE_DEVTYPE_H__
#define __INCLUDE_DEVTYPE_H__

enum class DevType : word_t
{
	DEV_TYPE_CHAR,
	DEV_TYPE_BLOCK,
	DEV_TYPE_SERVICE,
	DEV_TYPE_FILESYS,
	DEV_TYPE_FILE
};

#endif

