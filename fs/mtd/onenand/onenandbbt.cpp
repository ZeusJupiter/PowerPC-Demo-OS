/*
 *   File name: onenandbbt.cpp
 *
 *  Created on: 2017年7月27日, 下午4:55:24
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */
#include <macros.h>
#include <types.h>
#include <debug.h>
#include <assert.h>
#include <errno.h>
#include <string.h>

#include <mm/heap.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/bbm.h>
#include <linux/mtd/onenand.h>

static int check_short_pattern(uint8_t * buf, int len, int paglen,
			       struct nand_bbt_descr *td)
{
	int i;
	uint8_t *p = buf;

	for (i = 0; i < td->len; i++) {
		if (p[i] != td->pattern[i])
			return -1;
	}
	return 0;
}

static int UNUSED create_bbt(struct mtd_info *mtd, uint8_t * buf,
		      struct nand_bbt_descr *bd, int chip)
{
	struct onenand_chip *thisChip = (struct onenand_chip *)mtd->priv;
	struct bbm_info *bbm = (struct bbm_info *)thisChip->bbm;
	int i, j, numblocks, len, scanlen;
	int startblock;
	loff_t from;
	size_t readlen, ooblen;
	struct mtd_oob_ops ops;
	int rgn;

	kdebug("Scanning device for bad blocks");

	len = 1;

	scanlen = ooblen = 0;
	readlen = bd->len;

	numblocks = thisChip->chipsize >> (bbm->bbt_erase_shift - 1);
	startblock = 0;
	from = 0;

	ops.mode = MTD_OPS_PLACE_OOB;
	ops.ooblen = readlen;
	ops.oobbuf = buf;
	ops.len = ops.ooboffs = ops.retlen = ops.oobretlen = 0;

	for (i = startblock; i < numblocks;) {
		int ret;

		for (j = 0; j < len; j++) {
			ret = onenand_bbt_read_oob(mtd,
					     from + j * mtd->writesize +
					     bd->offs, &ops);
			if (ret == ONENAND_BBT_READ_FATAL_ERROR)
				return -static_cast<sint>(ErrNo::EIO);

			if (ret || check_short_pattern
			    (&buf[j * scanlen], scanlen, mtd->writesize, bd)) {
				bbm->bbt[i >> 3] |= 0x03 << (i & 0x6);
				kformatln("Bad eraseblock %d at 0x%08x", i >> 1, (unsigned int)from);
				break;
			}
		}
		i += 2;

		if (FLEXONENAND(thisChip)) {
			rgn = flexonenand_region(mtd, from);
			from += mtd->eraseregions[rgn].erasesize;
		} else
			from += (1 << bbm->bbt_erase_shift);
	}

	return 0;
}

inline int onenand_memory_bbt(struct mtd_info *mtd,
				     struct nand_bbt_descr *bd)
{
	unsigned char data_buf[MAX_ONENAND_PAGESIZE];

	bd->options &= ~NAND_BBT_SCANEMPTY;
	return create_bbt(mtd, data_buf, bd, -1);
}

static int UNUSED onenand_isbad_bbt(struct mtd_info *mtd, loff_t offs, int allowbbt)
{
	struct onenand_chip *thisChip = (struct onenand_chip *)mtd->priv;
	struct bbm_info *bbm = (struct bbm_info *)thisChip->bbm;
	int block;
	uint8_t res;

	block = (int) (onenand_block(thisChip, offs) << 1);
	res = (bbm->bbt[block >> 3] >> (block & 0x06)) & 0x03;

	kformatln ("onenand_isbad_bbt: bbt info for offs 0x%08x: (block %d) 0x%02x",
		(unsigned int)offs, block >> 1, res);

	switch ((int)res) {
	case 0x00:
		return 0;
	case 0x01:
		return 1;
	case 0x02:
		return allowbbt ? 0 : 1;
	}

	return 1;
}

static uint8_t scan_ff_pattern[2] = {0xff, 0xff};
static struct nand_bbt_descr largepage_memorybased = { 0, { 0 }, 0, 0, { 0 }, 2,
		0, 0, scan_ff_pattern };

BEG_EXT_C

int onenand_scan_bbt(struct mtd_info *mtd, struct nand_bbt_descr *bd)
{
	struct onenand_chip *thisChip = (struct onenand_chip *)mtd->priv;
	struct bbm_info *bbm = (struct bbm_info *)thisChip->bbm;
	int len, ret = 0;

	len = thisChip->chipsize >> (thisChip->erase_shift + 2);

	bbm->bbt = (u8*)kheap.allocate(len);
	if (!bbm->bbt)
		return -static_cast<sint>(ErrNo::ENOMEM);

	memset(bbm->bbt, 0x00, len);
	bbm->badblockpos = ONENAND_BADBLOCK_POS;
	bbm->bbt_erase_shift = thisChip->erase_shift;

	if (!bbm->isbad_bbt)
		bbm->isbad_bbt = onenand_isbad_bbt;

	if ((ret = onenand_memory_bbt(mtd, bd))) {
		kdebugln("onenand_scan_bbt: Can't scan flash and build the RAM-based BBT");
		kheap.free(bbm->bbt);
		bbm->bbt = NULL;
	}

	return ret;
}

int onenand_default_bbt(struct mtd_info *mtd)
{
	struct onenand_chip *thisChip = (struct onenand_chip *)mtd->priv;
	struct bbm_info *bbm;

	thisChip->bbm = kheap.allocate(sizeof(struct bbm_info));
	if (!thisChip->bbm)
		return -static_cast<sint>(ErrNo::ENOMEM);

	bbm = (struct bbm_info *)thisChip->bbm;
	memset(bbm, 0, sizeof(struct bbm_info));

	if (!bbm->badblock_pattern)
		bbm->badblock_pattern = &largepage_memorybased;

	return onenand_scan_bbt(mtd, bbm->badblock_pattern);
}

END_EXT_C
