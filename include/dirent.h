/*
 *   File name: dirent.h
 *
 *  Created on: 2017年7月19日, 上午10:12:45
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __INCLUDE_DIRENT_H__
#define __INCLUDE_DIRENT_H__

#include "macros.h"
#include "types.h"
#include "mutex.h"
#include <mk/oscfg.h>

struct dirent
{
	ino_t  d_ino;
	c8 d_name[256];
};

struct DIR
{
	sint   dir_fd;
	slong  dir_pos;
	bool   dir_eof;
	struct dirent dir_dirent;
	Mutex dir_mutex;
};

#endif

