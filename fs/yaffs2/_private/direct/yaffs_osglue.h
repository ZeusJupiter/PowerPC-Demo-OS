/*
 * YAFFS: Yet another Flash File System . A NAND-flash specific file system.
 *
 * Copyright (C) 2002-2011 Aleph One Ltd.
 *   for Toby Churchill Ltd and Brightstar Engineering
 *
 * Created by Charles Manning <charles@aleph1.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 2.1 as
 * published by the Free Software Foundation.
 *
 * Note: Only YAFFS headers are LGPL, YAFFS C code is covered by GPL.
 */

#ifndef __YAFFS_OSGLUE_H__
#define __YAFFS_OSGLUE_H__

#include "yportenv.h"

static inline void yaffsfs_Lock(void){}
static inline void yaffsfs_Unlock(void){}
static inline u32 yaffsfs_CurrentTime(void){ return (u32)time(NULL); }
static inline void yaffsfs_SetError(int err){errno = (err > 0) ? (err) : (-err);}
static inline void *yaffsfs_malloc(size_t size){ return malloc(size); }
static inline void yaffsfs_free(void *ptr){ free(ptr); }
static inline int yaffsfs_CheckMemRegion(const void *addr, size_t size, int write_request)
{ return (addr != NULL) ? 0 : -1; }
static inline void yaffsfs_OSInitialisation(void){}

#endif

