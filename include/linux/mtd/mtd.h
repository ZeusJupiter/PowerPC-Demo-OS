/*
 *   File name: mtd.h
 *
 *  Created on: 2017年7月25日, 下午6:40:52
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description: from linux source code
 */

#include <stddef.h>
#include <errno.h>

#include "mtdabi.h"

#ifndef __INCLUDE_LINUX_MTD_MTD_H__
#define __INCLUDE_LINUX_MTD_MTD_H__

enum FlashChipState
{
	FCS_READY,
	FCS_READING,
	FCS_WRITING,
	FCS_ERASING,
	FCS_SYNCING,
	FCS_CACHEDPRG,
	FCS_RESETING,
	FCS_UNLOCKING,
	FCS_LOCKING,
	FCS_PM_SUSPENDED,
};

#define MTD_CHAR_MAJOR 90
#define MTD_BLOCK_MAJOR 31

#define MTD_ERASE_PENDING	0x01
#define MTD_ERASING		0x02
#define MTD_ERASE_SUSPEND	0x04
#define MTD_ERASE_DONE		0x08
#define MTD_ERASE_FAILED	0x10

#define MTD_FAIL_ADDR_UNKNOWN -1LL

struct erase_info
{
	struct mtd_info *mtd;
	u64 addr;
	u64 len;
	u64 fail_addr;
	ulong time;
	ulong retries;
	unsigned dev;
	unsigned cell;
	void (*callback)(struct erase_info *self);
	ulong priv;
	uchar state;
	struct erase_info *next;
	int scrub;

};

struct mtd_erase_region_info
{
	u64 offset;

	u32 erasesize;

	u32 numblocks;

	unsigned long *lockmap;

};

struct mtd_oob_ops
{
	unsigned int mode;
	size_t len;
	size_t retlen;
	size_t ooblen;
	size_t oobretlen;
	u32 ooboffs;
	u8 *datbuf;
	u8 *oobbuf;
};

struct nand_ecclayout
{
	u32 eccbytes;
	u32 eccpos[MTD_MAX_ECCPOS_ENTRIES_LARGE];
	u32 oobavail;
	struct nand_oobfree oobfree[MTD_MAX_OOBFREE_ENTRIES_LARGE];
};

struct module;

struct mtd_info
{
	uchar type;
	u32 flags;
	u64 size;

	u32 erasesize;

	u32 writesize;

	u32 oobsize;

	u32 oobavail;

	unsigned int bitflip_threshold;

	const char *name;
	int index;

	struct nand_ecclayout *ecclayout;

	unsigned int ecc_strength;

	int numeraseregions;
	struct mtd_erase_region_info *eraseregions;

	int (*_erase) (struct mtd_info *mtd, struct erase_info *instr);
	int (*_point) (struct mtd_info *mtd, loff_t from, size_t len,
		       size_t *retlen, void **virt, addr_t *phys);
	int (*_unpoint) (struct mtd_info *mtd, loff_t from, size_t len);

	int (*_read) (struct mtd_info *mtd, loff_t from, size_t len,
		      size_t *retlen, uchar *buf);
	int (*_write) (struct mtd_info *mtd, loff_t to, size_t len,
		       size_t *retlen, const uchar *buf);
	int (*_panic_write) (struct mtd_info *mtd, loff_t to, size_t len,
			     size_t *retlen, const uchar *buf);
	int (*_read_oob) (struct mtd_info *mtd, loff_t from,
			  struct mtd_oob_ops *ops);
	int (*_write_oob) (struct mtd_info *mtd, loff_t to,
			   struct mtd_oob_ops *ops);
	int (*_get_fact_prot_info) (struct mtd_info *mtd, struct otp_info *buf,
				    size_t len);
	int (*_read_fact_prot_reg) (struct mtd_info *mtd, loff_t from,
				    size_t len, size_t *retlen, uchar *buf);
	int (*_get_user_prot_info) (struct mtd_info *mtd, struct otp_info *buf,
				    size_t len);
	int (*_read_user_prot_reg) (struct mtd_info *mtd, loff_t from,
				    size_t len, size_t *retlen, uchar *buf);
	int (*_write_user_prot_reg) (struct mtd_info *mtd, loff_t to,
				     size_t len, size_t *retlen, uchar *buf);
	int (*_lock_user_prot_reg) (struct mtd_info *mtd, loff_t from,
				    size_t len);

	void (*_sync) (struct mtd_info *mtd);
	int (*_lock) (struct mtd_info *mtd, loff_t ofs, u64 len);
	int (*_unlock) (struct mtd_info *mtd, loff_t ofs, u64 len);

	int (*_block_isbad) (struct mtd_info *mtd, loff_t ofs);
	int (*_block_markbad) (struct mtd_info *mtd, loff_t ofs);

	int (*_get_device) (struct mtd_info *mtd);
	void (*_put_device) (struct mtd_info *mtd);

	struct mtd_ecc_stats ecc_stats;

	int subpage_sft;

	void *priv;

	struct module *owner;

	int usecount;
};

static inline int mtd_write_oob(struct mtd_info *mtd, loff_t to,
				struct mtd_oob_ops *ops)
{
	ops->retlen = ops->oobretlen = 0;
	if (!mtd->_write_oob)
#if defined(__cplusplus)
		return -static_cast<sint>(ErrNo::EOPNOTSUPP);
#else
		return -EOPNOTSUPP;
#endif
	if (!(mtd->flags & MTD_WRITEABLE))
#if defined(__cplusplus)
		return -static_cast<sint>(ErrNo::EROFS);
#else
		return -(EROFS);
#endif
	return mtd->_write_oob(mtd, to, ops);
}

static inline void mtd_sync(struct mtd_info *mtd)
{
	if (mtd->_sync)
		mtd->_sync(mtd);
}

static inline int mtd_has_oob(const struct mtd_info *mtd)
{
	return mtd->_read_oob && mtd->_write_oob;
}

static inline int mtd_can_have_bb(const struct mtd_info *mtd)
{
	return !!mtd->_block_isbad;
}

#ifdef CONFIG_MTD_PARTITIONS
void mtd_erase_callback(struct erase_info *instr);
#else
static inline void mtd_erase_callback(struct erase_info *instr)
{
	if (instr->callback)
		instr->callback(instr);
}
#endif

#define MTD_DEBUG_LEVEL0	(0)

#define MTD_DEBUG_LEVEL1	(1)

#define MTD_DEBUG_LEVEL2	(2)

#define MTD_DEBUG_LEVEL3	(3)

#ifdef CONFIG_MTD_DEBUG

#else

#endif

static inline int mtd_is_bitflip(int err) {
#if defined(__cplusplus)
	return err == -static_cast<sint>(ErrNo::EUCLEAN);
#else
	return err == -(EUCLEAN);
#endif
}

static inline int mtd_is_eccerr(int err) {
#if defined(__cplusplus)
	return err == -static_cast<sint>(ErrNo::EBADMSG);
#else
	return err == -EBADMSG;
#endif
}

static inline int mtd_is_bitflip_or_eccerr(int err) {
	return mtd_is_bitflip(err) || mtd_is_eccerr(err);
}

BEG_EXT_C

int mtd_erase(struct mtd_info *mtd, struct erase_info *instr);
int mtd_read(struct mtd_info *mtd, loff_t from, size_t len, size_t *retlen,  uchar *buf);
int mtd_write(struct mtd_info *mtd, loff_t to, size_t len, size_t *retlen, const uchar *buf);
int mtd_panic_write(struct mtd_info *mtd, loff_t to, size_t len, size_t *retlen, const uchar *buf);

int mtd_read_oob(struct mtd_info *mtd, loff_t from, struct mtd_oob_ops *ops);
int mtd_get_fact_prot_info(struct mtd_info *mtd, struct otp_info *buf, size_t len);
int mtd_read_fact_prot_reg(struct mtd_info *mtd, loff_t from, size_t len, size_t *retlen, uchar *buf);
int mtd_get_user_prot_info(struct mtd_info *mtd, struct otp_info *buf, size_t len);
int mtd_read_user_prot_reg(struct mtd_info *mtd, loff_t from, size_t len, size_t *retlen, uchar *buf);
int mtd_write_user_prot_reg(struct mtd_info *mtd, loff_t to, size_t len, size_t *retlen, uchar *buf);
int mtd_lock_user_prot_reg(struct mtd_info *mtd, loff_t from, size_t len);

int mtd_lock(struct mtd_info *mtd, loff_t ofs, uint64_t len);
int mtd_unlock(struct mtd_info *mtd, loff_t ofs, uint64_t len);
int mtd_is_locked(struct mtd_info *mtd, loff_t ofs, uint64_t len);
int mtd_block_isbad(struct mtd_info *mtd, loff_t ofs);
int mtd_block_markbad(struct mtd_info *mtd, loff_t ofs);

extern int add_mtd_device(struct mtd_info *mtd);
extern int del_mtd_device (struct mtd_info *mtd);

extern struct mtd_info *get_mtd_device(struct mtd_info *mtd, int num);
extern struct mtd_info *get_mtd_device_nm(const char *name);

extern void put_mtd_device(struct mtd_info *mtd);
extern void mtd_get_len_incl_bad(struct mtd_info *mtd, uint64_t offset,
				 const uint64_t length, uint64_t *len_incl_bad,
				 int *truncated);

END_EXT_C

#endif

