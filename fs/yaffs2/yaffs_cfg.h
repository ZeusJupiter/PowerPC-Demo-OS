/*
 *   File name: yaffs_cfg.h
 *
 *  Created on: 2017年6月22日, 下午6:44:58
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __FS_YAFFS2_YAFFS_CFG_H__
#define __FS_YAFFS2_YAFFS_CFG_H__

#include <macros.h>
#include <types.h>
#include <debug.h>
#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <string.h>

#include <stdlib.h>
#include <time.h>

#include <linux/version.h>
#include <linux/log.h>
#include <linux/gfp.h>

#define CONFIG_YAFFS_DIRECT 1

#define CONFIG_YAFFS_FS 1
#define CONFIG_YAFFS_YAFFS1 1

#define CONFIG_YAFFS_YAFFS2 1
#define CONFIG_YAFFS_PROVIDE_DEFS 1

#endif

