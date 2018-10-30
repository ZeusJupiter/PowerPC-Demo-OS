/*
 *   File name: select.h
 *
 *  Created on: 2017年5月17日, 下午9:38:53
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __INCLUDE_SELECT_H__
#define __INCLUDE_SELECT_H__

#include "fdset.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct timeval timeval_t;
typedef struct timespec timespec_t;

extern int select (int width, FdSet * readFdset, FdSet * writeFdset,
		   FdSet * exceptFdset, timeval_t * timeout);
extern int pselect(int width, FdSet* readFdset, FdSet* writeFdset, FdSet* exceptFdset,
		              const timespec_t* timeout, const sigset_t* sigsetMask);

extern int waitread(int  fd, timeval_t* timeout);
extern int waitwrite(int fd, timeval_t* timeout);
extern int waitexcept(int fd, timeval_t* timeout);

#ifdef __cplusplus
}
#endif

#endif

