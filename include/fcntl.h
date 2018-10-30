/*
 *   File name: fcntl.h
 *
 *  Created on: 2017年5月24日, 下午10:00:05
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __INCLUDE_FCNTL_H__
#define __INCLUDE_FCNTL_H__

#ifdef __cplusplus
extern "C" {
#endif

#define	_FOPEN		(-1)
#define _FREAD          0x0001
#define _FWRITE         0x0002
#define _FAPPEND        0x0008
#define _FDSYNC         0x0020
#define _FASYNC         0x0040
#define _FSHLOCK        0x0080
#define _FEXLOCK        0x0100
#define _FCREAT         0x0200
#define _FTRUNC         0x0400
#define _FEXCL          0x0800
#define _FNBIO          0x1000
#define _FSYNC          0x2000
#define _FNONBLOCK      0x4000
#define _FNOCTTY        0x8000
#define _FBINARY       0x10000
#define _FNOFOLLOW     0x20000

#define	O_RDONLY	   0
#define	O_WRONLY	   1
#define	O_RDWR		   2
#define	O_ACCMODE	   (O_RDONLY|O_WRONLY|O_RDWR)
#define	O_APPEND	   _FAPPEND
#define	O_CREAT		   _FCREAT
#define	O_TRUNC		   _FTRUNC
#define	O_EXCL		   _FEXCL
#define O_SYNC		   _FSYNC
#define O_SHLOCK       0x0010
#define O_EXLOCK       0x0020
#define O_FSYNC        _FSYNC
#define O_DSYNC        _FDSYNC
#define O_ASYNC        _FASYNC
#define O_NONBLOCK     _FNONBLOCK
#define O_NDELAY       _FNONBLOCK
#define O_NOCTTY       _FNOCTTY
#define O_BINARY       _FBINARY
#define O_NOFOLLOW     _FNOFOLLOW
#define O_CLOEXEC      0x80000
#define O_LARGEFILE    0x100000

#define	FD_CLOEXEC	1

#define F_DUPFD       0
#define F_GETFD       1
#define F_SETFD       2
#define F_GETFL       3
#define F_SETFL       4

#ifndef	_POSIX_SOURCE
#define F_GETOWN      5
#define F_SETOWN      6
#endif

#define F_GETLK       7
#define F_SETLK       8
#define F_SETLKW      9

#ifndef	_POSIX_SOURCE
#define F_RGETLK      10
#define F_RSETLK      11
#define F_CNVT        12
#define F_RSETLKW     13

#endif

#define	F_RDLCK		1
#define	F_WRLCK		2
#define	F_UNLCK		3

#ifndef	_POSIX_SOURCE
#define	F_UNLKSYS	4
#endif

struct flock {
	short	  l_type;
	short	  l_whence;
	long long l_start;
	long long l_len;
	long	  l_pid;
};

struct eflock {
	short	  l_type;
	short	  l_whence;
	long long l_start;
	long long l_len;
	long	  l_pid;
	long	  l_rpid;
	long	  l_rsys;
};

sint  fcntl(sint fd, sint request, ...);

#ifdef __cplusplus
}
#endif
#endif

