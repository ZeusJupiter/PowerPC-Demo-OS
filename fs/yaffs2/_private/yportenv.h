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

#ifndef __YPORTENV_H__
#define __YPORTENV_H__

#include "../yaffs_cfg.h"

#define MTD_VERSION(a, b, c) (((a) << 16) + ((b) << 8) + (c))

#ifdef YAFFS_OUT_OF_TREE
#include "moduleconfig.h"
#endif

#define MTD_VERSION_CODE LINUX_VERSION_CODE

#include <string.h>

#include "direct/yportenv.h"

#define YCHAR char
#define YUCHAR unsigned char
#define _Y(x)     x

#define YAFFS_LOSTNFOUND_NAME		"lost+found"
#define YAFFS_LOSTNFOUND_PREFIX		"obj"

#define compile_time_assertion(assertion) \
	({ int x = __builtin_choose_expr(assertion, 0, (void)0); (void) x; })

#define yaffs_printf(msk, fmt, ...) \
	printk(KERN_DEBUG "yaffs: " fmt "\n", ##__VA_ARGS__)

#endif
