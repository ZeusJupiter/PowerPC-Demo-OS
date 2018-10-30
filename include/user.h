/*
 *   File name: user.h
 *
 *  Created on: 2017年5月10日, 下午10:48:37
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __INCLUDE_USER_H__
#define __INCLUDE_USER_H__

struct User
{
	uid_t uid;
	gid_t gid;
	pid_t pid;
};

#endif

