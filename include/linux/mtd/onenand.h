/*
 *   File name: onenand.h
 *
 *  Created on: 2017年7月25日, 下午7:34:28
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description: from linux source code
 */

#include <stddef.h>
#include "../port.h"
#include "bbm.h"
#include "onenand_regs.h"

#ifndef __INCLUDE_LINUX_MTD_ONENAND_H__
#define __INCLUDE_LINUX_MTD_ONENAND_H__

#define MAX_DIES		2
#define MAX_BUFFERRAM		2
#define MAX_ONENAND_PAGESIZE	(4096 + 128)

struct onenand_bufferram {
	int blockpage;
};

struct onenand_chip {
	void __iomem *base;
	unsigned int dies;
	unsigned int boundary[MAX_DIES];
	unsigned int diesize[MAX_DIES];
	unsigned int chipsize;
	unsigned int device_id;
	unsigned int version_id;
	unsigned int technology;
	unsigned int density_mask;
	unsigned int options;

	unsigned int erase_shift;
	unsigned int page_shift;
	unsigned int ppb_shift;

	unsigned int page_mask;
	unsigned int writesize;

	unsigned int bufferram_index;
	struct onenand_bufferram bufferram[MAX_BUFFERRAM];

	int (*command) (struct mtd_info *mtd, int cmd, loff_t address,
			size_t len);
	int (*wait) (struct mtd_info *mtd, int state);
	int (*bbt_wait) (struct mtd_info *mtd, int state);
	void (*unlock_all)(struct mtd_info *mtd);
	int (*read_bufferram) (struct mtd_info *mtd, loff_t addr, int area,
			       unsigned char *buffer, int offset, size_t count);
	int (*write_bufferram) (struct mtd_info *mtd, loff_t addr, int area,
				const unsigned char *buffer, int offset,
				size_t count);
	unsigned short (*read_word) (void __iomem *addr);
	void (*write_word) (unsigned short value, void __iomem *addr);
	int (*chip_probe)(struct mtd_info *mtd);
	void (*mmcontrol) (struct mtd_info *mtd, int sync_read);
	int (*block_markbad)(struct mtd_info *mtd, loff_t ofs);
	int (*scan_bbt)(struct mtd_info *mtd);

	unsigned char		*main_buf;
	unsigned char		*spare_buf;

	int state;
	unsigned char		*page_buf;
	unsigned char		*oob_buf;

	struct nand_oobinfo *autooob;
	int			subpagesize;
	struct nand_ecclayout	*ecclayout;

	void *bbm;

	void *priv;
};

#define ONENAND_CURRENT_BUFFERRAM(this)		(this->bufferram_index)
#define ONENAND_NEXT_BUFFERRAM(this)		(this->bufferram_index ^ 1)
#define ONENAND_SET_NEXT_BUFFERRAM(this)	(this->bufferram_index ^= 1)
#define ONENAND_SET_PREV_BUFFERRAM(this)	(this->bufferram_index ^= 1)
#define ONENAND_SET_BUFFERRAM0(this)		(this->bufferram_index = 0)
#define ONENAND_SET_BUFFERRAM1(this)		(this->bufferram_index = 1)

#define FLEXONENAND(this)	(this->device_id & DEVICE_IS_FLEXONENAND)
#define ONENAND_IS_MLC(this)	(this->technology & ONENAND_TECHNOLOGY_IS_MLC)
#define ONENAND_IS_DDP(this)						\
	(this->device_id & ONENAND_DEVICE_IS_DDP)

#define ONENAND_IS_4KB_PAGE(this)					\
	(this->options & ONENAND_HAS_4KB_PAGE)

#define ONENAND_IS_2PLANE(this)			(0)

#define ONENAND_HAS_CONT_LOCK		(0x0001)
#define ONENAND_HAS_UNLOCK_ALL		(0x0002)
#define ONENAND_HAS_2PLANE		(0x0004)
#define ONENAND_HAS_4KB_PAGE            (0x0008)
#define ONENAND_RUNTIME_BADBLOCK_CHECK	(0x0200)
#define ONENAND_PAGEBUF_ALLOC		(0x1000)
#define ONENAND_OOBBUF_ALLOC		(0x2000)

#define ONENAND_MFR_NUMONYX	0x20
#define ONENAND_MFR_SAMSUNG	0xec

struct onenand_manufacturers {
	int id;
	char *name;
};

BEG_EXT_C

extern int onenand_scan (struct mtd_info *mtd, int max_chips);
extern void onenand_release (struct mtd_info *mtd);
extern int onenand_bbt_read_oob(struct mtd_info *mtd, loff_t from,
			struct mtd_oob_ops *ops);

extern unsigned int onenand_block(struct onenand_chip *thisChip, loff_t addr);
extern int flexonenand_region(struct mtd_info *mtd, loff_t addr);

END_EXT_C
#endif

