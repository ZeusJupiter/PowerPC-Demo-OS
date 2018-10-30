/*
 *   File name: unistd.h
 *
 *  Created on: 2017年6月26日, 上午6:29:24
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#include "macros.h"
#include "stddef.h"

#ifndef __INCLUDE_UNISTD_H__
#define __INCLUDE_UNISTD_H__

#ifndef SEEK_SET
#define SEEK_SET    0
#define SEEK_CUR    1
#define SEEK_END    2

#endif

#define R_OK        4
#define W_OK        2
#define X_OK        1
#define F_OK        0

#define _PC_2_SYMLINKS          1
#define _PC_ALLOC_SIZE_MIN      2
#define _PC_ASYNC_IO            3
#define _PC_CHOWN_RESTRICTED    4
#define _PC_FILESIZEBITS        5
#define _PC_LINK_MAX            6
#define _PC_MAX_CANON           7
#define _PC_MAX_INPUT           8
#define _PC_NAME_MAX            9
#define _PC_NO_TRUNC            10
#define _PC_PATH_MAX            11
#define _PC_PIPE_BUF            12
#define _PC_PRIO_IO             13
#define _PC_REC_INCR_XFER_SIZE  14
#define _PC_REC_MAX_XFER_SIZE   15
#define _PC_REC_MIN_XFER_SIZE   16
#define _PC_REC_XFER_ALIGN      17
#define _PC_SYMLINK_MAX         18
#define _PC_SYNC_IO             19
#define _PC_VDISABLE            20

#define _POSIX_ASYNC_IO         0
#define _POSIX_PRIO_IO          0
#define _POSIX_SYNC_IO          0
#define _POSIX_NO_TRUNC         0
#define _POSIX_SYNCHRONIZED_IO  0

struct getopt_s
{
	int opterr;
	int optind;
	int optopt;
	int optreset;
	char *optarg;
	char *place;
};

BEG_EXT_C

extern char *optarg;
extern int optind, opterr, optopt;
extern int optreset;

typedef struct getopt_s GETOPT;
typedef struct getopt_s * GETOPT_ID;

__DeclareFunc__(NORETURN void, _exit, (int __status ));

__DeclareFunc__(sint, pause, (void));

__DeclareFunc__(int, unlink, (const char *name));

__DeclareFunc__(int, link, (const char *, const char *));

__DeclareFunc__(int, fdatasync, (int));

__DeclareFunc__(long, fpathconf, (int, int));

__DeclareFunc__(long, pathconf, (const char *, int));

__DeclareFunc__(int, access, (const char *, int));

__DeclareFunc__(int, close, (int fd));

__DeclareFunc__(ssize_t, read, (int fd, void * buffer, size_t maxbytes));

__DeclareFunc__(ssize_t, write, (int fd, char * buffer, size_t nbytes));

__DeclareFunc__(off_t, lseek, (int fd, off_t offset, int whence));

__DeclareFunc__(int, chdir, (const char *pathname));

__DeclareFunc__(int, pause, (void));

__DeclareFunc__(int, isatty, (int fd));

__DeclareFunc__(int, rmdir, (const char *dirName));

__DeclareFunc__(char*, getcwd, (char *buffer, size_t size));

__DeclareFunc__(int , ftruncate, (int fildes, off_t length));

__DeclareFunc__(unsigned int, sleep, (unsigned int));

__DeclareFunc__(unsigned int, alarm, (unsigned int));

__DeclareFunc__(int, getopt, (int, char * const [], const char *));
__DeclareFunc__(int, getopt_r, (int, char * const [], const char *, GETOPT_ID pGetOpt));
__DeclareFunc__(void, getoptInit, (GETOPT_ID pArg));
__DeclareFunc__(int, getOptServ, (char * ParamString, const char * progName, int * argc,
				char * argvloc[], int argvlen));

__DeclareFunc__( pid_t, getpid, (void));

END_EXT_C

#endif

