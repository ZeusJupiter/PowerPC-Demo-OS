/*
 *   File name: fdset.h
 *
 *  Created on: 2017年5月17日, 下午10:20:23
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __INCLUDE_FDSET_H__
#define __INCLUDE_FDSET_H__

#ifndef FD_SETSIZE
const u32 fdsize = 1024;

#define FD_SETSIZE   fdsize
#endif

typedef u32 fd_mask;

#ifndef _howmany
#define _howmany(x, y) (((x) + ((y) - 1)) / (y))
#endif

#ifndef howmany
#define howmany(x, y) (((x) + ((y) - 1)) / (y))
#endif

#define NFDBITS (sizeof(fd_mask) * 8)

typedef struct fd_set {
	fd_mask bits[_howmany(FD_SETSIZE, NFDBITS)];
} FdSet;

#define _FD_BITSINDEX(fd) ((fd) / NFDBITS)
#define _FD_BIT(fd) (1L << ((fd) % NFDBITS))

#include <memory.h>

#define FD_ZERO(set) memset((set), 0, sizeof(fd_set))
#define FD_SET(fd, set) ((set)->bits[_FD_BITSINDEX(fd)] |= _FD_BIT(fd))
#define FD_CLR(fd, set) ((set)->bits[_FD_BITSINDEX(fd)] &= ~_FD_BIT(fd))
#define FD_ISSET(fd, set) ((set)->bits[_FD_BITSINDEX(fd)] & _FD_BIT(fd))
#define FD_COPY(source, target) (*(target) = *(source))

#endif

