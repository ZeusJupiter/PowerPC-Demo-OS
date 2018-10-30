/*
 *   File name: onenandcore.cpp
 *
 *  Created on: 2017年7月27日, 下午4:55:10
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#include <macros.h>
#include <types.h>
#include <stdio.h>
#include <debug.h>
#include <assert.h>

#include <errno.h>
#include <string.h>
#include <endian.h>

#include <io.h>

#include <mm/heap.h>

#include <linux/bitops.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/bbm.h>
#include <linux/mtd/onenand.h>

static struct nand_ecclayout onenand_oob_128 = { 64, { 6, 7, 8, 9, 10, 11, 12,
		13, 14, 15, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 38, 39, 40, 41, 42,
		43, 44, 45, 46, 47, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 70, 71, 72,
		73, 74, 75, 76, 77, 78, 79, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 102,
		103, 104, 105 }, 0, { { 2, 4 }, { 18, 4 }, { 34, 4 }, { 50, 4 },
		{ 66, 4 }, { 82, 4 }, { 98, 4 }, { 114, 4 } } };
static struct nand_ecclayout onenand_oob_64 = { 20, { 8, 9, 10, 11, 12, 24, 25,
		26, 27, 28, 40, 41, 42, 43, 44, 56, 57, 58, 59, 60 }, 0, { { 2, 3 }, {
		14, 2 }, { 18, 3 }, { 30, 2 }, { 34, 3 }, { 46, 2 }, { 50, 3 },
		{ 62, 2 } } };

static struct nand_ecclayout onenand_oob_32 = { 10, { 8, 9, 10, 11, 12, 24, 25,
		26, 27, 28, }, 0, { { 2, 3 }, { 14, 2 }, { 18, 3 }, { 30, 2 } } };

static unsigned char ffchars[128] = { 0xff };

inline unsigned short onenand_readw(void __iomem * addr)
{
	return read16((addr_t)addr);
}

inline void onenand_writew(unsigned short value, void __iomem * addr)
{
	write16((addr_t)addr, value);
}

static int onenand_block_address(struct onenand_chip *thisChip, int block)
{

	if (block & thisChip->density_mask)
		return ONENAND_DDP_CHIP1 | (block ^ thisChip->density_mask);

	return block;
}

static int onenand_bufferram_address(struct onenand_chip *thisChip, int block)
{

	if (block & thisChip->density_mask)
		return ONENAND_DDP_CHIP1;

	return ONENAND_DDP_CHIP0;
}

static int onenand_page_address(int page, int sector)
{

	int fpa, fsa;

	fpa = page & ONENAND_FPA_MASK;
	fsa = sector & ONENAND_FSA_MASK;

	return ((fpa << ONENAND_FPA_SHIFT) | fsa);
}

static int onenand_buffer_address(int dataram1, int sectors, int count)
{
	int bsa, bsc;

	bsa = sectors & ONENAND_BSA_MASK;

	if (dataram1)
		bsa |= ONENAND_BSA_DATARAM1;

	else
		bsa |= ONENAND_BSA_DATARAM0;

	bsc = count & ONENAND_BSC_MASK;

	return ((bsa << ONENAND_BSA_SHIFT) | bsc);
}

static unsigned int flexonenand_block(struct onenand_chip *thisChip, loff_t addr)
{
	unsigned int boundary, blk, die = 0;

	if (ONENAND_IS_DDP(thisChip) && addr >= thisChip->diesize[0]) {
		die = 1;
		addr -= thisChip->diesize[0];
	}

	boundary = thisChip->boundary[die];

	blk = addr >> (thisChip->erase_shift - 1);
	if (blk > boundary)
		blk = (blk + boundary + 1) >> 1;

	blk += die ? thisChip->density_mask : 0;
	return blk;
}

static loff_t flexonenand_addr(struct onenand_chip *thisChip, int block)
{
	loff_t ofs = 0;
	int die = 0, boundary;

	if (ONENAND_IS_DDP(thisChip) && block >= thisChip->density_mask) {
		block -= thisChip->density_mask;
		die = 1;
		ofs = thisChip->diesize[0];
	}

	boundary = thisChip->boundary[die];
	ofs += (loff_t) block << (thisChip->erase_shift - 1);
	if (block > (boundary + 1))
		ofs += (loff_t) (block - boundary - 1)
			<< (thisChip->erase_shift - 1);
	return ofs;
}

static loff_t onenand_addr(struct onenand_chip *thisChip, int block)
{
	if (!FLEXONENAND(thisChip))
		return (loff_t) block << thisChip->erase_shift;
	return flexonenand_addr(thisChip, block);
}

static inline int onenand_get_density(int dev_id)
{
	int density = dev_id >> ONENAND_DEVICE_DENSITY_SHIFT;
	return (density & ONENAND_DEVICE_DENSITY_MASK);
}

static int onenand_command(struct mtd_info *mtd, int cmd, loff_t addr,
			   size_t len)
{
	struct onenand_chip *thisChip = (struct onenand_chip *)mtd->priv;
	int value;
	int block, page;

	int sectors = 0, count = 0;

	switch (cmd) {
	case ONENAND_CMD_UNLOCK:
	case ONENAND_CMD_LOCK:
	case ONENAND_CMD_LOCK_TIGHT:
	case ONENAND_CMD_UNLOCK_ALL:
		block = -1;
		page = -1;
		break;

	case FLEXONENAND_CMD_PI_ACCESS:

		block = addr * thisChip->density_mask;
		page = -1;
		break;

	case ONENAND_CMD_ERASE:
	case ONENAND_CMD_BUFFERRAM:
		block = onenand_block(thisChip, addr);
		page = -1;
		break;

	case FLEXONENAND_CMD_READ_PI:
		cmd = ONENAND_CMD_READ;
		block = addr * thisChip->density_mask;
		page = 0;
		break;

	default:
		block = onenand_block(thisChip, addr);
		page = (int) (addr
			- onenand_addr(thisChip, block)) >> thisChip->page_shift;
		page &= thisChip->page_mask;
		break;
	}

	if (cmd == ONENAND_CMD_BUFFERRAM) {

		value = onenand_bufferram_address(thisChip, block);
		thisChip->write_word(value,
				 thisChip->base + ONENAND_REG_START_ADDRESS2);

		if (ONENAND_IS_4KB_PAGE(thisChip))
			ONENAND_SET_BUFFERRAM0(thisChip);
		else
	

			ONENAND_SET_NEXT_BUFFERRAM(thisChip);

		return 0;
	}

	if (block != -1) {

		value = onenand_block_address(thisChip, block);
		thisChip->write_word(value,
				 thisChip->base + ONENAND_REG_START_ADDRESS1);

		value = onenand_bufferram_address(thisChip, block);
		thisChip->write_word(value,
				 thisChip->base + ONENAND_REG_START_ADDRESS2);
	}

	if (page != -1) {
		int dataram;

		switch (cmd) {
		case FLEXONENAND_CMD_RECOVER_LSB:
		case ONENAND_CMD_READ:
		case ONENAND_CMD_READOOB:
			if (ONENAND_IS_4KB_PAGE(thisChip))
				dataram = ONENAND_SET_BUFFERRAM0(thisChip);
			else
				dataram = ONENAND_SET_NEXT_BUFFERRAM(thisChip);

			break;

		default:
			dataram = ONENAND_CURRENT_BUFFERRAM(thisChip);
			break;
		}

		value = onenand_page_address(page, sectors);
		thisChip->write_word(value,
				 thisChip->base + ONENAND_REG_START_ADDRESS8);

		value = onenand_buffer_address(dataram, sectors, count);
		thisChip->write_word(value, thisChip->base + ONENAND_REG_START_BUFFER);
	}

	thisChip->write_word(ONENAND_INT_CLEAR, thisChip->base + ONENAND_REG_INTERRUPT);

	thisChip->write_word(cmd, thisChip->base + ONENAND_REG_COMMAND);

	return 0;
}

static int onenand_read_ecc(struct onenand_chip *thisChip)
{
	int ecc, i;

	if (!FLEXONENAND(thisChip))
		return thisChip->read_word(thisChip->base + ONENAND_REG_ECC_STATUS);

	for (i = 0; i < 4; i++) {
		ecc = thisChip->read_word(thisChip->base
				+ ((ONENAND_REG_ECC_STATUS + i) << 1));
		if (likely(!ecc))
			continue;
		if (ecc & FLEXONENAND_UNCORRECTABLE_ERROR)
			return ONENAND_ECC_2BIT_ALL;
	}

	return 0;
}

static int onenand_wait(struct mtd_info *mtd, int state)
{
	struct onenand_chip *thisChip = (struct onenand_chip *)mtd->priv;
	unsigned int flags = ONENAND_INT_MASTER;
	unsigned int interrupt = 0;
	unsigned int ctrl;

	while (1) {
		interrupt = thisChip->read_word(thisChip->base + ONENAND_REG_INTERRUPT);
		if (interrupt & flags)
			break;
	}

	ctrl = thisChip->read_word(thisChip->base + ONENAND_REG_CTRL_STATUS);

	if (interrupt & ONENAND_INT_READ) {
		int ecc = onenand_read_ecc(thisChip);
		if (ecc & ONENAND_ECC_2BIT_ALL) {
			kformatln("onenand_wait: ECC error = 0x%04x", ecc);
			return -static_cast<sint>(ErrNo::EBADMSG);
		}
	}

	if (ctrl & ONENAND_CTRL_ERROR) {
		kformatln("onenand_wait: controller error = 0x%04x", ctrl);
		if (ctrl & ONENAND_CTRL_LOCK)
			kformatln("onenand_wait: it's locked error = 0x%04x",
				ctrl);

		return -static_cast<sint>(ErrNo::EIO);
	}

	return 0;
}

static inline int onenand_bufferram_offset(struct mtd_info *mtd, int area)
{
	struct onenand_chip *thisChip = (struct onenand_chip *)mtd->priv;

	if (ONENAND_CURRENT_BUFFERRAM(thisChip)) {
		if (area == ONENAND_DATARAM)
			return mtd->writesize;
		if (area == ONENAND_SPARERAM)
			return mtd->oobsize;
	}

	return 0;
}

static int onenand_read_bufferram(struct mtd_info *mtd, loff_t addr, int area,
				  unsigned char *buffer, int offset,
				  size_t count)
{
	struct onenand_chip *thisChip = (struct onenand_chip *)mtd->priv;
	void __iomem *bufferram;

	bufferram = thisChip->base + area;
	bufferram += onenand_bufferram_offset(mtd, area);

	memcpy(buffer, bufferram + offset, count);

	return 0;
}

static int onenand_sync_read_bufferram(struct mtd_info *mtd, loff_t addr, int area,
				       unsigned char *buffer, int offset,
				       size_t count)
{
	struct onenand_chip *thisChip = (struct onenand_chip *)mtd->priv;
	void __iomem *bufferram;

	bufferram = thisChip->base + area;
	bufferram += onenand_bufferram_offset(mtd, area);

	thisChip->mmcontrol(mtd, ONENAND_SYS_CFG1_SYNC_READ);

	memcpy(buffer, bufferram + offset, count);

	thisChip->mmcontrol(mtd, 0);

	return 0;
}

static int onenand_write_bufferram(struct mtd_info *mtd, loff_t addr, int area,
				   const unsigned char *buffer, int offset,
				   size_t count)
{
	struct onenand_chip *thisChip = (struct onenand_chip *)mtd->priv;
	void __iomem *bufferram;

	bufferram = thisChip->base + area;
	bufferram += onenand_bufferram_offset(mtd, area);

	memcpy(bufferram + offset, buffer, count);

	return 0;
}

static int onenand_get_2x_blockpage(struct mtd_info *mtd, loff_t addr)
{
	struct onenand_chip *thisChip = (struct onenand_chip *)mtd->priv;
	int blockpage, block, page;

	block = (int) (addr >> thisChip->erase_shift) & ~1;

	if (addr & thisChip->writesize)
		block++;
	page = (int) (addr >> (thisChip->page_shift + 1)) & thisChip->page_mask;
	blockpage = (block << 7) | page;

	return blockpage;
}

static int onenand_check_bufferram(struct mtd_info *mtd, loff_t addr)
{
	struct onenand_chip *thisChip = (struct onenand_chip *)mtd->priv;
	int blockpage, found = 0;
	unsigned int i;

	if (ONENAND_IS_2PLANE(thisChip))
		blockpage = onenand_get_2x_blockpage(mtd, addr);
	else
		blockpage = (int) (addr >> thisChip->page_shift);

	i = ONENAND_CURRENT_BUFFERRAM(thisChip);
	if (thisChip->bufferram[i].blockpage == blockpage)
		found = 1;
	else {

		i = ONENAND_NEXT_BUFFERRAM(thisChip);
		if (thisChip->bufferram[i].blockpage == blockpage) {
			ONENAND_SET_NEXT_BUFFERRAM(thisChip);
			found = 1;
		}
	}

	if (found && ONENAND_IS_DDP(thisChip)) {

		int block = onenand_block(thisChip, addr);
		int value = onenand_bufferram_address(thisChip, block);
		thisChip->write_word(value, thisChip->base + ONENAND_REG_START_ADDRESS2);
	}

	return found;
}

static int onenand_update_bufferram(struct mtd_info *mtd, loff_t addr,
				    int valid)
{
	struct onenand_chip *thisChip = (struct onenand_chip *)mtd->priv;
	int blockpage;
	unsigned int i;

	if (ONENAND_IS_2PLANE(thisChip))
		blockpage = onenand_get_2x_blockpage(mtd, addr);
	else
		blockpage = (int)(addr >> thisChip->page_shift);

	i = ONENAND_NEXT_BUFFERRAM(thisChip);
	if (thisChip->bufferram[i].blockpage == blockpage)
		thisChip->bufferram[i].blockpage = -1;

	i = ONENAND_CURRENT_BUFFERRAM(thisChip);
	if (valid)
		thisChip->bufferram[i].blockpage = blockpage;
	else
		thisChip->bufferram[i].blockpage = -1;

	return 0;
}

static void onenand_invalidate_bufferram(struct mtd_info *mtd, loff_t addr,
					 unsigned int len)
{
	struct onenand_chip *thisChip = (struct onenand_chip *)mtd->priv;
	int i;
	loff_t end_addr = addr + len;

	for (i = 0; i < MAX_BUFFERRAM; i++) {
		loff_t buf_addr = thisChip->bufferram[i].blockpage << thisChip->page_shift;

		if (buf_addr >= addr && buf_addr < end_addr)
			thisChip->bufferram[i].blockpage = -1;
	}
}

static void onenand_get_device(struct mtd_info *mtd, int new_state)
{

}

static void onenand_release_device(struct mtd_info *mtd)
{

}

static int onenand_transfer_auto_oob(struct mtd_info *mtd, uint8_t *buf,
					int column, int thislen)
{
	struct onenand_chip *thisChip = (struct onenand_chip *)mtd->priv;
	struct nand_oobfree *free;
	int readcol = column;
	int readend = column + thislen;
	int lastgap = 0;
	unsigned int i;
	uint8_t *oob_buf = thisChip->oob_buf;

	free = (struct nand_oobfree *)(thisChip->ecclayout->oobfree);
	for (i = 0; i < MTD_MAX_OOBFREE_ENTRIES_LARGE && free->length;
	     i++, free++) {
		if (readcol >= lastgap)
			readcol += free->offset - lastgap;
		if (readend >= lastgap)
			readend += free->offset - lastgap;
		lastgap = free->offset + free->length;
	}
	thisChip->read_bufferram(mtd, 0, ONENAND_SPARERAM, oob_buf, 0, mtd->oobsize);
	free = (struct nand_oobfree *)(thisChip->ecclayout->oobfree);
	for (i = 0; i < MTD_MAX_OOBFREE_ENTRIES_LARGE && free->length;
	     i++, free++) {
		int free_end = free->offset + free->length;
		if (free->offset < readend && free_end > readcol) {
			int st = max_t(int,free->offset,readcol);
			int ed = min_t(int,free_end,readend);
			int n = ed - st;
			memcpy(buf, (uint8_t*)((addr_t)oob_buf + st), n);
			buf += n;
		} else if (column == 0)
			break;
	}
	return 0;
}

static int onenand_recover_lsb(struct mtd_info *mtd, loff_t addr, int status)
{
	struct onenand_chip *thisChip = (struct onenand_chip *)mtd->priv;
	int i;

	if (!FLEXONENAND(thisChip))
		return status;

	if (!mtd_is_eccerr(status) && status != ONENAND_BBT_READ_ECC_ERROR)
		return status;

	i = flexonenand_region(mtd, addr);
	if (mtd->eraseregions[i].erasesize < (1 << thisChip->erase_shift))
		return status;

	kdebugln("onenand_recover_lsb: Attempting to recover from uncorrectable read");

	thisChip->command(mtd, FLEXONENAND_CMD_RECOVER_LSB, addr, thisChip->writesize);
	return thisChip->wait(mtd, FCS_READING);
}

static int onenand_read_ops_nolock(struct mtd_info *mtd, loff_t from,
		struct mtd_oob_ops *ops)
{
	struct onenand_chip *thisChip = (struct onenand_chip *)mtd->priv;
	struct mtd_ecc_stats stats;
	size_t len = ops->len;
	size_t ooblen = ops->ooblen;
	uchar *buf = ops->datbuf;
	uchar *oobbuf = ops->oobbuf;
	int read = 0, column, thislen;
	int oobread = 0, oobcolumn, thisooblen, oobsize;
	int ret = 0, boundary = 0;
	int writesize = thisChip->writesize;

	kformatln("onenand_read_ops_nolock: from = 0x%08x, len = %i", (unsigned int) from, (int) len);

	if (ops->mode == MTD_OPS_AUTO_OOB)
		oobsize = thisChip->ecclayout->oobavail;
	else
		oobsize = mtd->oobsize;

	oobcolumn = from & (mtd->oobsize - 1);

	if ((from + len) > mtd->size) {
		kdebugln("onenand_read_ops_nolock: Attempt read beyond end of device");
		ops->retlen = 0;
		ops->oobretlen = 0;
		return -static_cast<sint>(ErrNo::EINVAL);
	}

	stats = mtd->ecc_stats;

	if (read < len) {
		if (!onenand_check_bufferram(mtd, from)) {
			thisChip->main_buf = buf;
			thisChip->command(mtd, ONENAND_CMD_READ, from, writesize);
			ret = thisChip->wait(mtd, FCS_READING);
			if (unlikely(ret))
				ret = onenand_recover_lsb(mtd, from, ret);
			onenand_update_bufferram(mtd, from, !ret);
			if (ret == -static_cast<sint>(ErrNo::EBADMSG))
				ret = 0;
		}
	}

	thislen = min_t(int, writesize, len - read);
	column = from & (writesize - 1);
	if (column + thislen > writesize)
		thislen = writesize - column;

	while (!ret) {

		from += thislen;
		if (!ONENAND_IS_4KB_PAGE(thisChip) && read + thislen < len) {
			thisChip->main_buf = buf + thislen;
			thisChip->command(mtd, ONENAND_CMD_READ, from, writesize);
	

			if (ONENAND_IS_DDP(thisChip) &&
					unlikely(from == (thisChip->chipsize >> 1))) {
				thisChip->write_word(ONENAND_DDP_CHIP0, thisChip->base + ONENAND_REG_START_ADDRESS2);
				boundary = 1;
			} else
				boundary = 0;
			ONENAND_SET_PREV_BUFFERRAM(thisChip);
		}

		thisChip->read_bufferram(mtd, from - thislen, ONENAND_DATARAM, buf, column, thislen);

		if (oobbuf) {
			thisooblen = oobsize - oobcolumn;
			thisooblen = min_t(int, thisooblen, ooblen - oobread);

			if (ops->mode == MTD_OPS_AUTO_OOB)
				onenand_transfer_auto_oob(mtd, oobbuf, oobcolumn, thisooblen);
			else
				thisChip->read_bufferram(mtd, 0, ONENAND_SPARERAM, oobbuf, oobcolumn, thisooblen);
			oobread += thisooblen;
			oobbuf += thisooblen;
			oobcolumn = 0;
		}

		if (ONENAND_IS_4KB_PAGE(thisChip) && (read + thislen < len)) {
			thisChip->command(mtd, ONENAND_CMD_READ, from, writesize);
			ret = thisChip->wait(mtd, FCS_READING);
			if (unlikely(ret))
				ret = onenand_recover_lsb(mtd, from, ret);
			onenand_update_bufferram(mtd, from, !ret);
			if (mtd_is_eccerr(ret))
				ret = 0;
		}

		read += thislen;
		if (read == len)
			break;

		if (unlikely(boundary))
			thisChip->write_word(ONENAND_DDP_CHIP1, thisChip->base + ONENAND_REG_START_ADDRESS2);
		if (!ONENAND_IS_4KB_PAGE(thisChip))
			ONENAND_SET_NEXT_BUFFERRAM(thisChip);
		buf += thislen;
		thislen = min_t(int, writesize, len - read);
		column = 0;

		if (!ONENAND_IS_4KB_PAGE(thisChip)) {
	

			ret = thisChip->wait(mtd, FCS_READING);
			onenand_update_bufferram(mtd, from, !ret);
			if (mtd_is_eccerr(ret))
				ret = 0;
		}
	}

	ops->retlen = read;
	ops->oobretlen = oobread;

	if (ret)
		return ret;

	if (mtd->ecc_stats.failed - stats.failed)
		return -static_cast<sint>(ErrNo::EBADMSG);

	return mtd->ecc_stats.corrected != stats.corrected ? 1 : 0;
}

static int onenand_read_oob_nolock(struct mtd_info *mtd, loff_t from,
		struct mtd_oob_ops *ops)
{
	struct onenand_chip *thisChip = (struct onenand_chip *)mtd->priv;
	struct mtd_ecc_stats stats;
	int read = 0, thislen, column, oobsize;
	size_t len = ops->ooblen;
	unsigned int mode = ops->mode;
	uchar *buf = ops->oobbuf;
	int ret = 0, readcmd;

	from += ops->ooboffs;

	kformatln("onenand_read_oob_nolock: from = 0x%08x, len = %i", (unsigned int) from, (int) len);

	ops->oobretlen = 0;

	if (mode == MTD_OPS_AUTO_OOB)
		oobsize = thisChip->ecclayout->oobavail;
	else
		oobsize = mtd->oobsize;

	column = from & (mtd->oobsize - 1);

	if (unlikely(column >= oobsize)) {
		kdebugln("onenand_read_oob_nolock: Attempted to start read outside oob");
		return -static_cast<sint>(ErrNo::EINVAL);
	}

	if (unlikely(from >= mtd->size ||
		column + len > ((mtd->size >> thisChip->page_shift) -
				(from >> thisChip->page_shift)) * oobsize)) {
		kdebugln("onenand_read_oob_nolock: Attempted to read beyond end of device");
		return -static_cast<sint>(ErrNo::EINVAL);
	}

	stats = mtd->ecc_stats;

	readcmd = ONENAND_IS_4KB_PAGE(thisChip) ?
		ONENAND_CMD_READ : ONENAND_CMD_READOOB;

	while (read < len) {
		thislen = oobsize - column;
		thislen = min_t(int, thislen, len);

		thisChip->spare_buf = buf;
		thisChip->command(mtd, readcmd, from, mtd->oobsize);

		onenand_update_bufferram(mtd, from, 0);

		ret = thisChip->wait(mtd, FCS_READING);
		if (unlikely(ret))
			ret = onenand_recover_lsb(mtd, from, ret);

		if (ret && ret != -static_cast<sint>(ErrNo::EBADMSG))
		{
			kformatln("onenand_read_oob_nolock: read failed = 0x%x", ret);
			break;
		}

		if (mode == MTD_OPS_AUTO_OOB)
			onenand_transfer_auto_oob(mtd, buf, column, thislen);
		else
			thisChip->read_bufferram(mtd, 0, ONENAND_SPARERAM, buf, column, thislen);

		read += thislen;

		if (read == len)
			break;

		buf += thislen;

		if (read < len) {
	

			from += mtd->writesize;
			column = 0;
		}
	}

	ops->oobretlen = read;

	if (ret)
		return ret;

	if (mtd->ecc_stats.failed - stats.failed)
		return -static_cast<sint>(ErrNo::EBADMSG);

	return 0;
}

static int onenand_bbt_wait(struct mtd_info *mtd, int state)
{
	struct onenand_chip *thisChip = (struct onenand_chip *)mtd->priv;
	unsigned int flags = ONENAND_INT_MASTER;
	unsigned int interrupt;
	unsigned int ctrl;

	while (1) {
		interrupt = thisChip->read_word(thisChip->base + ONENAND_REG_INTERRUPT);
		if (interrupt & flags)
			break;
	}

	interrupt = thisChip->read_word(thisChip->base + ONENAND_REG_INTERRUPT);
	ctrl = thisChip->read_word(thisChip->base + ONENAND_REG_CTRL_STATUS);

	if (interrupt & ONENAND_INT_READ) {
		int ecc = onenand_read_ecc(thisChip);
		if (ecc & ONENAND_ECC_2BIT_ALL) {
			kformatln("onenand_bbt_wait: ecc error = 0x%04x, controller = 0x%04x", ecc, ctrl);
			return ONENAND_BBT_READ_ERROR;
		}
	} else {
		kformatln("onenand_bbt_wait: read timeout! ctrl=0x%04x intr=0x%04x", ctrl, interrupt);
		return ONENAND_BBT_READ_FATAL_ERROR;
	}

	if (ctrl & ONENAND_CTRL_ERROR) {
		kformatln("onenand_bbt_wait: controller error = 0x%04x", ctrl);
		return ONENAND_BBT_READ_ERROR;
	}

	return 0;
}

#ifdef CONFIG_MTD_ONENAND_VERIFY_WRITE

static int onenand_verify_oob(struct mtd_info *mtd, const uchar *buf, loff_t to)
{
	struct onenand_chip *thisChip = mtd->priv;
	uchar *oob_buf = thisChip->oob_buf;
	int status, i, readcmd;

	readcmd = ONENAND_IS_4KB_PAGE(thisChip) ?
		ONENAND_CMD_READ : ONENAND_CMD_READOOB;

	thisChip->command(mtd, readcmd, to, mtd->oobsize);
	onenand_update_bufferram(mtd, to, 0);
	status = thisChip->wait(mtd, FL_READING);
	if (status)
		return status;

	thisChip->read_bufferram(mtd, 0, ONENAND_SPARERAM, oob_buf, 0, mtd->oobsize);
	for (i = 0; i < mtd->oobsize; i++)
		if (buf[i] != 0xFF && buf[i] != oob_buf[i])
			return -static_cast<sint>(ErrNo::EBADMSG);

	return 0;
}

static int onenand_verify(struct mtd_info *mtd, const uchar *buf, loff_t addr, size_t len)
{
	struct onenand_chip *thisChip = mtd->priv;
	void __iomem *dataram;
	int ret = 0;
	int thislen, column;

	while (len != 0) {
		thislen = min_t(int, thisChip->writesize, len);
		column = addr & (thisChip->writesize - 1);
		if (column + thislen > thisChip->writesize)
			thislen = thisChip->writesize - column;

		thisChip->command(mtd, ONENAND_CMD_READ, addr, thisChip->writesize);

		onenand_update_bufferram(mtd, addr, 0);

		ret = thisChip->wait(mtd, FL_READING);
		if (ret)
			return ret;

		onenand_update_bufferram(mtd, addr, 1);

		dataram = thisChip->base + ONENAND_DATARAM;
		dataram += onenand_bufferram_offset(mtd, ONENAND_DATARAM);

		if (memcmp(buf, dataram + column, thislen))
			return -EBADMSG;

		len -= thislen;
		buf += thislen;
		addr += thislen;
	}

	return 0;
}
#else
#define onenand_verify(...)             (0)
#define onenand_verify_oob(...)         (0)
#endif

#define NOTALIGNED(x)	((x & (thisChip->subpagesize - 1)) != 0)

static int onenand_fill_auto_oob(struct mtd_info *mtd, uchar *oob_buf,
		const uchar *buf, int column, int thislen)
{
	struct onenand_chip *thisChip = (struct onenand_chip *)mtd->priv;
	const struct nand_oobfree *free;
	int writecol = column;
	int writeend = column + thislen;
	int lastgap = 0;
	unsigned int i;

	free = (struct nand_oobfree*)thisChip->ecclayout->oobfree;
	for (i = 0; i < MTD_MAX_OOBFREE_ENTRIES_LARGE && free->length;
	     i++, free++) {
		if (writecol >= lastgap)
			writecol += free->offset - lastgap;
		if (writeend >= lastgap)
			writeend += free->offset - lastgap;
		lastgap = free->offset + free->length;
	}
	free = (struct nand_oobfree*)(thisChip->ecclayout->oobfree);
	for (i = 0; i < MTD_MAX_OOBFREE_ENTRIES_LARGE && free->length;
	     i++, free++) {
		int free_end = free->offset + free->length;
		if (free->offset < writeend && free_end > writecol) {
			int st = max_t(int,free->offset,writecol);
			int ed = min_t(int,free_end,writeend);
			int n = ed - st;
			memcpy(oob_buf + st, buf, n);
			buf += n;
		} else if (column == 0)
			break;
	}
	return 0;
}

static int onenand_write_ops_nolock(struct mtd_info *mtd, loff_t to,
		struct mtd_oob_ops *ops)
{
	struct onenand_chip *thisChip = (struct onenand_chip *)(mtd->priv);
	int written = 0, column, thislen, subpage;
	int oobwritten = 0, oobcolumn, thisooblen, oobsize;
	size_t len = ops->len;
	size_t ooblen = ops->ooblen;
	const uchar *buf = ops->datbuf;
	const uchar *oob = ops->oobbuf;
	uchar *oobbuf;
	int ret = 0;

	kformatln("onenand_write_ops_nolock: to = 0x%08x, len = %i", (unsigned int) to, (int) len);

	ops->retlen = 0;
	ops->oobretlen = 0;

	if (unlikely(NOTALIGNED(to) || NOTALIGNED(len))) {
		kdebugln("onenand_write_ops_nolock: Attempt to write not page aligned data");
		return -static_cast<sint>(ErrNo::EINVAL);
	}

	if (ops->mode == MTD_OPS_AUTO_OOB)
		oobsize = thisChip->ecclayout->oobavail;
	else
		oobsize = mtd->oobsize;

	oobcolumn = to & (mtd->oobsize - 1);

	column = to & (mtd->writesize - 1);

	while (written < len) {
		uchar *wbuf = (uchar *) buf;

		thislen = min_t(int, mtd->writesize - column, len - written);
		thisooblen = min_t(int, oobsize - oobcolumn, ooblen - oobwritten);

		thisChip->command(mtd, ONENAND_CMD_BUFFERRAM, to, thislen);

		subpage = thislen < mtd->writesize;
		if (subpage) {
			memset(thisChip->page_buf, 0xff, mtd->writesize);
			memcpy(thisChip->page_buf + column, buf, thislen);
			wbuf = thisChip->page_buf;
		}

		thisChip->write_bufferram(mtd, to, ONENAND_DATARAM, wbuf, 0, mtd->writesize);

		if (oob) {
			oobbuf = thisChip->oob_buf;

	

			memset(oobbuf, 0xff, mtd->oobsize);
			if (ops->mode == MTD_OPS_AUTO_OOB)
				onenand_fill_auto_oob(mtd, oobbuf, oob, oobcolumn, thisooblen);
			else
				memcpy(oobbuf + oobcolumn, oob, thisooblen);

			oobwritten += thisooblen;
			oob += thisooblen;
			oobcolumn = 0;
		} else
			oobbuf = (uchar *) ffchars;

		thisChip->write_bufferram(mtd, 0, ONENAND_SPARERAM, oobbuf, 0, mtd->oobsize);

		thisChip->command(mtd, ONENAND_CMD_PROG, to, mtd->writesize);

		ret = thisChip->wait(mtd, FCS_WRITING);

		onenand_update_bufferram(mtd, to, !ret && !subpage);
		if (ONENAND_IS_2PLANE(thisChip)) {
			ONENAND_SET_BUFFERRAM1(thisChip);
			onenand_update_bufferram(mtd, to + thisChip->writesize, !ret && !subpage);
		}

		if (ret) {
			kformatln("onenand_write_ops_nolock: write filaed %d", ret);
			break;
		}

		ret = onenand_verify(mtd, buf, to, thislen);
		if (ret) {
			kformatln("onenand_write_ops_nolock: verify failed %d", ret);
			break;
		}

		written += thislen;

		if (written == len)
			break;

		column = 0;
		to += thislen;
		buf += thislen;
	}

	ops->retlen = written;

	return ret;
}

static int onenand_write_oob_nolock(struct mtd_info *mtd, loff_t to,
		struct mtd_oob_ops *ops)
{
	struct onenand_chip *thisChip = (struct onenand_chip *)mtd->priv;
	int column, ret = 0, oobsize;
	int written = 0, oobcmd;
	uchar *oobbuf;
	size_t len = ops->ooblen;
	const uchar *buf = ops->oobbuf;
	unsigned int mode = ops->mode;

	to += ops->ooboffs;

	kformatln("onenand_write_oob_nolock: to = 0x%08x, len = %i", (unsigned int) to, (int) len);

	ops->oobretlen = 0;

	if (mode == MTD_OPS_AUTO_OOB)
		oobsize = thisChip->ecclayout->oobavail;
	else
		oobsize = mtd->oobsize;

	column = to & (mtd->oobsize - 1);

	if (unlikely(column >= oobsize)) {
		kdebugln("onenand_write_oob_nolock: Attempted to start write outside oob");
		return -static_cast<sint>(ErrNo::EINVAL);
	}

	if (unlikely(column + len > oobsize)) {
		kdebugln("onenand_write_oob_nolock: Attempt to write past end of page");
		return -static_cast<sint>(ErrNo::EINVAL);
	}

	if (unlikely(to >= mtd->size ||
				column + len > ((mtd->size >> thisChip->page_shift) -
					(to >> thisChip->page_shift)) * oobsize)) {
		kdebugln("onenand_write_oob_nolock: Attempted to write past end of device");
		return -static_cast<sint>(ErrNo::EINVAL);
	}

	oobbuf = thisChip->oob_buf;

	oobcmd = ONENAND_IS_4KB_PAGE(thisChip) ?
		ONENAND_CMD_PROG : ONENAND_CMD_PROGOOB;

	while (written < len) {
		int thislen = min_t(int, oobsize, len - written);

		thisChip->command(mtd, ONENAND_CMD_BUFFERRAM, to, mtd->oobsize);

		memset(oobbuf, 0xff, mtd->oobsize);
		if (mode == MTD_OPS_AUTO_OOB)
			onenand_fill_auto_oob(mtd, oobbuf, buf, column, thislen);
		else
			memcpy(oobbuf + column, buf, thislen);
		thisChip->write_bufferram(mtd, 0, ONENAND_SPARERAM, oobbuf, 0, mtd->oobsize);

		if (ONENAND_IS_4KB_PAGE(thisChip)) {
	

			memset(thisChip->page_buf, 0xff, mtd->writesize);
			thisChip->write_bufferram(mtd, 0, ONENAND_DATARAM,
				thisChip->page_buf,	0, mtd->writesize);
		}

		thisChip->command(mtd, oobcmd, to, mtd->oobsize);

		onenand_update_bufferram(mtd, to, 0);
		if (ONENAND_IS_2PLANE(thisChip)) {
			ONENAND_SET_BUFFERRAM1(thisChip);
			onenand_update_bufferram(mtd, to + thisChip->writesize, 0);
		}

		ret = thisChip->wait(mtd, FCS_WRITING);
		if (ret) {
			kformatln("onenand_write_oob_nolock: write failed %d", ret);
			break;
		}

		ret = onenand_verify_oob(mtd, oobbuf, to);
		if (ret) {
			kformatln("onenand_write_oob_nolock: verify failed %d", ret);
			break;
		}

		written += thislen;
		if (written == len)
			break;

		to += mtd->writesize;
		buf += thislen;
		column = 0;
	}

	ops->oobretlen = written;

	return ret;
}

static int onenand_block_isbad_nolock(struct mtd_info *mtd, loff_t ofs, int allowbbt)
{
	struct onenand_chip *thisChip = (struct onenand_chip *)mtd->priv;
	struct bbm_info *bbm = (struct bbm_info *)thisChip->bbm;

	return bbm->isbad_bbt(mtd, ofs, allowbbt);
}

static int onenand_default_block_markbad(struct mtd_info *mtd, loff_t ofs)
{
	struct onenand_chip *thisChip = (struct onenand_chip *)mtd->priv;
	struct bbm_info *bbm = (struct bbm_info *)thisChip->bbm;
	uchar buf[2] = {0, 0};
	struct mtd_oob_ops ops;

	ops.mode = MTD_OPS_PLACE_OOB;
	ops.ooblen = 2;
	ops.oobbuf = buf;
	ops.ooboffs = 0;

	int block;

	block = onenand_block(thisChip, ofs);
	if (bbm->bbt)
		bbm->bbt[block >> 2] |= 0x01 << ((block & 0x03) << 1);

	ofs += mtd->oobsize + (bbm->badblockpos & ~0x01);
	return onenand_write_oob_nolock(mtd, ofs, &ops);
}

static int onenand_do_lock_cmd(struct mtd_info *mtd, loff_t ofs, size_t len, int cmd)
{
	struct onenand_chip *thisChip = (struct onenand_chip *)mtd->priv;
	int start, end, block, value, status;

	start = onenand_block(thisChip, ofs);
	end = onenand_block(thisChip, ofs + len);

	if (thisChip->options & ONENAND_HAS_CONT_LOCK) {

		thisChip->write_word(start,
				 thisChip->base + ONENAND_REG_START_BLOCK_ADDRESS);

		thisChip->write_word(end - 1,
				 thisChip->base + ONENAND_REG_END_BLOCK_ADDRESS);

		thisChip->command(mtd, cmd, 0, 0);

		thisChip->wait(mtd, FCS_UNLOCKING);

		while (thisChip->read_word(thisChip->base + ONENAND_REG_CTRL_STATUS)
		       & ONENAND_CTRL_ONGO)
			continue;

		status = thisChip->read_word(thisChip->base + ONENAND_REG_WP_STATUS);
		if (!(status & ONENAND_WP_US))
			kformatln("wp status = 0x%x", status);

		return 0;
	}

	for (block = start; block < end; block++) {

		value = onenand_block_address(thisChip, block);
		thisChip->write_word(value, thisChip->base + ONENAND_REG_START_ADDRESS1);

		value = onenand_bufferram_address(thisChip, block);
		thisChip->write_word(value, thisChip->base + ONENAND_REG_START_ADDRESS2);

		thisChip->write_word(block,
				 thisChip->base + ONENAND_REG_START_BLOCK_ADDRESS);

		thisChip->command(mtd, ONENAND_CMD_UNLOCK, 0, 0);

		thisChip->wait(mtd, FCS_UNLOCKING);

		while (thisChip->read_word(thisChip->base + ONENAND_REG_CTRL_STATUS)
		       & ONENAND_CTRL_ONGO)
			continue;

		status = thisChip->read_word(thisChip->base + ONENAND_REG_WP_STATUS);
		if (!(status & ONENAND_WP_US))
			kformatln("block = %d, wp status = 0x%x",block, status);
	}

	return 0;
}

#ifdef ONENAND_LINUX

static int onenand_lock(struct mtd_info *mtd, loff_t ofs, size_t len)
{
	int ret;

	onenand_get_device(mtd, FL_LOCKING);
	ret = onenand_do_lock_cmd(mtd, ofs, len, ONENAND_CMD_LOCK);
	onenand_release_device(mtd);
	return ret;
}

static int onenand_unlock(struct mtd_info *mtd, loff_t ofs, size_t len)
{
	int ret;

	onenand_get_device(mtd, FL_LOCKING);
	ret = onenand_do_lock_cmd(mtd, ofs, len, ONENAND_CMD_UNLOCK);
	onenand_release_device(mtd);
	return ret;
}
#endif

static int onenand_check_lock_status(struct onenand_chip *thisChip)
{
	unsigned int value, block, status;
	unsigned int end;

	end = thisChip->chipsize >> thisChip->erase_shift;
	for (block = 0; block < end; block++) {

		value = onenand_block_address(thisChip, block);
		thisChip->write_word(value, thisChip->base + ONENAND_REG_START_ADDRESS1);

		value = onenand_bufferram_address(thisChip, block);
		thisChip->write_word(value, thisChip->base + ONENAND_REG_START_ADDRESS2);

		thisChip->write_word(block, thisChip->base + ONENAND_REG_START_BLOCK_ADDRESS);

		status = thisChip->read_word(thisChip->base + ONENAND_REG_WP_STATUS);
		if (!(status & ONENAND_WP_US)) {
			kformatln("block = %d, wp status = 0x%x", block, status);
			return 0;
		}
	}

	return 1;
}

static void onenand_unlock_all(struct mtd_info *mtd)
{
	struct onenand_chip *thisChip = (struct onenand_chip *)mtd->priv;
	loff_t ofs = 0;
	size_t len = mtd->size;

	if (thisChip->options & ONENAND_HAS_UNLOCK_ALL) {

		thisChip->write_word(0, thisChip->base + ONENAND_REG_START_BLOCK_ADDRESS);

		thisChip->command(mtd, ONENAND_CMD_UNLOCK_ALL, 0, 0);

		thisChip->wait(mtd, FCS_LOCKING);

		while (thisChip->read_word(thisChip->base + ONENAND_REG_CTRL_STATUS)
				& ONENAND_CTRL_ONGO)
			continue;

		if (onenand_check_lock_status(thisChip))
			return;

		if (ONENAND_IS_DDP(thisChip) && !FLEXONENAND(thisChip)) {
	

			ofs = thisChip->chipsize >> 1;
			len = thisChip->chipsize >> 1;
		}
	}

	onenand_do_lock_cmd(mtd, ofs, len, ONENAND_CMD_UNLOCK);
}

static void onenand_check_features(struct mtd_info *mtd)
{
	struct onenand_chip *thisChip = (struct onenand_chip *)mtd->priv;
	unsigned int density, process;

	density = onenand_get_density(thisChip->device_id);
	process = thisChip->version_id >> ONENAND_VERSION_PROCESS_SHIFT;

	switch (density) {
	case ONENAND_DEVICE_DENSITY_4Gb:
		if (ONENAND_IS_DDP(thisChip))
			thisChip->options |= ONENAND_HAS_2PLANE;
		else
			thisChip->options |= ONENAND_HAS_4KB_PAGE;

	case ONENAND_DEVICE_DENSITY_2Gb:

		if (!ONENAND_IS_DDP(thisChip))
			thisChip->options |= ONENAND_HAS_2PLANE;
		thisChip->options |= ONENAND_HAS_UNLOCK_ALL;

	case ONENAND_DEVICE_DENSITY_1Gb:

		if (process)
			thisChip->options |= ONENAND_HAS_UNLOCK_ALL;
		break;

	default:

		if (!process)
			thisChip->options |= ONENAND_HAS_CONT_LOCK;
		break;
	}

	if (ONENAND_IS_MLC(thisChip))
		thisChip->options |= ONENAND_HAS_4KB_PAGE;

	if (ONENAND_IS_4KB_PAGE(thisChip))
		thisChip->options &= ~ONENAND_HAS_2PLANE;

	if (FLEXONENAND(thisChip)) {
		thisChip->options &= ~ONENAND_HAS_CONT_LOCK;
		thisChip->options |= ONENAND_HAS_UNLOCK_ALL;
	}

	if (thisChip->options & ONENAND_HAS_CONT_LOCK)
		kdebugln("Lock scheme is Continuous Lock");
	if (thisChip->options & ONENAND_HAS_UNLOCK_ALL)
		kdebugln("Chip support all block unlock");
	if (thisChip->options & ONENAND_HAS_2PLANE)
		kdebugln("Chip has 2 plane");
	if (thisChip->options & ONENAND_HAS_4KB_PAGE)
		kdebugln("Chip has 4KiB pagesize");

}

static const struct onenand_manufacturers onenand_manuf_ids[] = {
	{ONENAND_MFR_NUMONYX, "Numonyx"},
	{ONENAND_MFR_SAMSUNG, "Samsung"},
};

static int onenand_check_maf(int manuf)
{
	int size = ARRAY_SIZE(onenand_manuf_ids);
	int i;
#ifdef ONENAND_DEBUG
	char *name;
#endif

	for (i = 0; i < size; i++)
		if (manuf == onenand_manuf_ids[i].id)
			break;

#ifdef ONENAND_DEBUG
	if (i < size)
		name = onenand_manuf_ids[i].name;
	else
		name = "Unknown";

	kformatln("OneNAND Manufacturer: %s (0x%0x)", name, manuf);
#endif

	return i == size;
}

static int flexonenand_get_boundary(struct mtd_info *mtd)
{
	struct onenand_chip *thisChip = (struct onenand_chip *)mtd->priv;
	unsigned int die, bdry;
	int syscfg, locked;

	syscfg = thisChip->read_word(thisChip->base + ONENAND_REG_SYS_CFG1);
	thisChip->write_word((syscfg | 0x0100), thisChip->base + ONENAND_REG_SYS_CFG1);

	for (die = 0; die < thisChip->dies; die++) {
		thisChip->command(mtd, FLEXONENAND_CMD_PI_ACCESS, die, 0);
		thisChip->wait(mtd, FCS_SYNCING);

		thisChip->command(mtd, FLEXONENAND_CMD_READ_PI, die, 0);
		thisChip->wait(mtd, FCS_READING);

		bdry = thisChip->read_word(thisChip->base + ONENAND_DATARAM);
		if ((bdry >> FLEXONENAND_PI_UNLOCK_SHIFT) == 3)
			locked = 0;
		else
			locked = 1;
		thisChip->boundary[die] = bdry & FLEXONENAND_PI_MASK;

		thisChip->command(mtd, ONENAND_CMD_RESET, 0, 0);
		thisChip->wait(mtd, FCS_RESETING);

		kformatln("Die %d boundary: %d%s", die,
		       thisChip->boundary[die], locked ? "(Locked)" : "(Unlocked)");
	}

	thisChip->write_word(syscfg, thisChip->base + ONENAND_REG_SYS_CFG1);
	return 0;
}

static void flexonenand_get_size(struct mtd_info *mtd)
{
	struct onenand_chip *thisChip = (struct onenand_chip *)mtd->priv;
	int die, i, eraseshift, density;
	int blksperdie, maxbdry;
	loff_t ofs;

	density = onenand_get_density(thisChip->device_id);
	blksperdie = ((loff_t)(16 << density) << 20) >> (thisChip->erase_shift);
	blksperdie >>= ONENAND_IS_DDP(thisChip) ? 1 : 0;
	maxbdry = blksperdie - 1;
	eraseshift = thisChip->erase_shift - 1;

	mtd->numeraseregions = thisChip->dies << 1;

	flexonenand_get_boundary(mtd);
	die = 0;
	ofs = 0;
	i = -1;
	for (; die < thisChip->dies; die++) {
		if (!die || thisChip->boundary[die-1] != maxbdry) {
			i++;
			mtd->eraseregions[i].offset = ofs;
			mtd->eraseregions[i].erasesize = 1 << eraseshift;
			mtd->eraseregions[i].numblocks =
							thisChip->boundary[die] + 1;
			ofs += mtd->eraseregions[i].numblocks << eraseshift;
			eraseshift++;
		} else {
			mtd->numeraseregions -= 1;
			mtd->eraseregions[i].numblocks +=
							thisChip->boundary[die] + 1;
			ofs += (thisChip->boundary[die] + 1) << (eraseshift - 1);
		}
		if (thisChip->boundary[die] != maxbdry) {
			i++;
			mtd->eraseregions[i].offset = ofs;
			mtd->eraseregions[i].erasesize = 1 << eraseshift;
			mtd->eraseregions[i].numblocks = maxbdry ^
							 thisChip->boundary[die];
			ofs += mtd->eraseregions[i].numblocks << eraseshift;
			eraseshift--;
		} else
			mtd->numeraseregions -= 1;
	}

	mtd->erasesize = 1 << thisChip->erase_shift;
	if (mtd->numeraseregions == 1)
		mtd->erasesize >>= 1;

	kformatln("Device has %d eraseregions", mtd->numeraseregions);
	for (i = 0; i < mtd->numeraseregions; i++)
		kformatln("[offset: 0x%08llx, erasesize: 0x%05x, numblocks: %04u]",
				mtd->eraseregions[i].offset,
				mtd->eraseregions[i].erasesize,
				mtd->eraseregions[i].numblocks);

	for (die = 0, mtd->size = 0; die < thisChip->dies; die++) {
		thisChip->diesize[die] = (loff_t) (blksperdie << thisChip->erase_shift);
		thisChip->diesize[die] -= (loff_t) (thisChip->boundary[die] + 1)
						 << (thisChip->erase_shift - 1);
		mtd->size += thisChip->diesize[die];
	}
}

static int flexonenand_check_blocks_erased(struct mtd_info *mtd,
					int start, int end)
{
	struct onenand_chip *thisChip = (struct onenand_chip *)mtd->priv;
	int i, ret;
	int block;
	struct mtd_oob_ops ops;
	loff_t addr;

	ops.mode = MtdOperationModes::MTD_OPS_AUTO_OOB;
	ops.ooboffs = 0;
	ops.ooblen = mtd->oobsize;
	ops.datbuf = nullptr;
	ops.oobbuf = thisChip->oob_buf;

	kformatln("Check blocks from %d to %d", start, end);

	for (block = start; block <= end; block++) {
		addr = flexonenand_addr(thisChip, block);
		if (onenand_block_isbad_nolock(mtd, addr, 0))
			continue;

		ret = onenand_read_oob_nolock(mtd, addr, &ops);
		if (ret)
			return ret;

		for (i = 0; i < mtd->oobsize; i++)
			if (thisChip->oob_buf[i] != 0xff)
				break;

		if (i != mtd->oobsize) {
			kformatln("Block %d not erased.", block);
			return 1;
		}
	}

	return 0;
}

static int onenand_chip_probe(struct mtd_info *mtd)
{
	struct onenand_chip *thisChip = (struct onenand_chip *)mtd->priv;
	int bram_maf_id, bram_dev_id, maf_id, dev_id;
	int syscfg;

	syscfg = thisChip->read_word(thisChip->base + ONENAND_REG_SYS_CFG1);

	thisChip->write_word((syscfg & ~ONENAND_SYS_CFG1_SYNC_READ),
			 thisChip->base + ONENAND_REG_SYS_CFG1);

	thisChip->write_word(ONENAND_CMD_READID, thisChip->base + ONENAND_BOOTRAM);

	bram_maf_id = thisChip->read_word(thisChip->base + ONENAND_BOOTRAM + 0x0);
	bram_dev_id = thisChip->read_word(thisChip->base + ONENAND_BOOTRAM + 0x2);

	thisChip->write_word(ONENAND_CMD_RESET, thisChip->base + ONENAND_BOOTRAM);

	thisChip->wait(mtd, FCS_RESETING);

	thisChip->write_word(syscfg, thisChip->base + ONENAND_REG_SYS_CFG1);

	if (onenand_check_maf(bram_maf_id))
		return -static_cast<sint>(ErrNo::ENXIO);

	maf_id = thisChip->read_word(thisChip->base + ONENAND_REG_MANUFACTURER_ID);
	dev_id = thisChip->read_word(thisChip->base + ONENAND_REG_DEVICE_ID);

	if (maf_id != bram_maf_id || dev_id != bram_dev_id)
		return -static_cast<sint>(ErrNo::ENXIO);

	return 0;
}

BEG_EXT_C

unsigned int onenand_block(struct onenand_chip *thisChip, loff_t addr)
{
	if (!FLEXONENAND(thisChip))
		return addr >> thisChip->erase_shift;
	return flexonenand_block(thisChip, addr);
}

int flexonenand_region(struct mtd_info *mtd, loff_t addr)
{
	int i;

	for (i = 0; i < mtd->numeraseregions; i++)
		if (addr < mtd->eraseregions[i].offset)
			break;
	return i - 1;
}

int onenand_read(struct mtd_info *mtd, loff_t from, size_t len,
		 size_t * retlen, uchar * buf)
{
	struct mtd_oob_ops ops;
	int ret;

	ops.len = len;
	ops.ooblen = 0;
	ops.datbuf = buf;
	ops.oobbuf = nullptr;

	onenand_get_device(mtd, FCS_READING);
	ret = onenand_read_ops_nolock(mtd, from, &ops);
	onenand_release_device(mtd);

	*retlen = ops.retlen;
	return ret;
}

int onenand_read_oob(struct mtd_info *mtd, loff_t from,
			struct mtd_oob_ops *ops)
{
	int ret;

	switch (ops->mode) {
	case MTD_OPS_PLACE_OOB:
	case MTD_OPS_AUTO_OOB:
		break;
	case MTD_OPS_RAW:

	default:
		return -static_cast<sint>(ErrNo::EINVAL);
	}

	onenand_get_device(mtd, FCS_READING);
	if (ops->datbuf)
		ret = onenand_read_ops_nolock(mtd, from, ops);
	else
		ret = onenand_read_oob_nolock(mtd, from, ops);
	onenand_release_device(mtd);

	return ret;
}

int onenand_bbt_read_oob(struct mtd_info *mtd, loff_t from,
		struct mtd_oob_ops *ops)
{
	struct onenand_chip *thisChip = (struct onenand_chip *)mtd->priv;
	int read = 0, thislen, column;
	int ret = 0, readcmd;
	size_t len = ops->ooblen;
	uchar *buf = ops->oobbuf;

	kformatln("onenand_bbt_read_oob: from = 0x%08x, len = %zi", (unsigned int) from, len);

	readcmd = ONENAND_IS_4KB_PAGE(thisChip) ?
		ONENAND_CMD_READ : ONENAND_CMD_READOOB;

	ops->oobretlen = 0;

	if (unlikely((from + len) > mtd->size)) {
		kdebugln("onenand_bbt_read_oob: Attempt read beyond end of device");
		return ONENAND_BBT_READ_FATAL_ERROR;
	}

	onenand_get_device(mtd, FCS_READING);

	column = from & (mtd->oobsize - 1);

	while (read < len) {

		thislen = mtd->oobsize - column;
		thislen = min_t(int, thislen, len);

		thisChip->spare_buf = buf;
		thisChip->command(mtd, readcmd, from, mtd->oobsize);

		onenand_update_bufferram(mtd, from, 0);

		ret = thisChip->bbt_wait(mtd, FCS_READING);
		if (unlikely(ret))
			ret = onenand_recover_lsb(mtd, from, ret);

		if (ret)
			break;

		thisChip->read_bufferram(mtd, 0, ONENAND_SPARERAM, buf, column, thislen);
		read += thislen;
		if (read == len)
			break;

		buf += thislen;

		if (read < len) {
	

			from += thisChip->writesize;
			column = 0;
		}
	}

	onenand_release_device(mtd);

	ops->oobretlen = read;
	return ret;
}

int onenand_write(struct mtd_info *mtd, loff_t to, size_t len,
		  size_t * retlen, const uchar * buf)
{
	struct mtd_oob_ops ops;
	int ret;

	ops.len    = len;
	ops.ooblen = 0;
	ops.datbuf = (uchar *) buf;
	ops.oobbuf = nullptr;

	onenand_get_device(mtd, FCS_WRITING);
	ret = onenand_write_ops_nolock(mtd, to, &ops);
	onenand_release_device(mtd);

	*retlen = ops.retlen;
	return ret;
}

int onenand_write_oob(struct mtd_info *mtd, loff_t to,
			struct mtd_oob_ops *ops)
{
	int ret;

	switch (ops->mode) {
	case MTD_OPS_PLACE_OOB:
	case MTD_OPS_AUTO_OOB:
		break;
	case MTD_OPS_RAW:

	default:
		return -static_cast<sint>(ErrNo::EINVAL);
	}

	onenand_get_device(mtd, FCS_WRITING);
	if (ops->datbuf)
		ret = onenand_write_ops_nolock(mtd, to, ops);
	else
		ret = onenand_write_oob_nolock(mtd, to, ops);
	onenand_release_device(mtd);

	return ret;

}

int onenand_erase(struct mtd_info *mtd, struct erase_info *instr)
{
	struct onenand_chip *thisChip = (struct onenand_chip *)mtd->priv;
	unsigned int block_size;
	loff_t addr = instr->addr;
	unsigned int len = instr->len;
	int ret = 0, i;
	struct mtd_erase_region_info *region = nullptr;
	unsigned int region_end = 0;

	kformatln("onenand_erase: start = 0x%08x, len = %i",
			(unsigned int) addr, len);

	if (FLEXONENAND(thisChip)) {

		i = flexonenand_region(mtd, addr);
		region = &mtd->eraseregions[i];

		block_size = region->erasesize;
		region_end = region->offset
			+ region->erasesize * region->numblocks;

		if (unlikely((addr - region->offset) & (block_size - 1))) {
			kdebugln("onenand_erase: Unaligned address");
			return -static_cast<sint>(ErrNo::EINVAL);
		}
	} else {
		block_size = 1 << thisChip->erase_shift;

		if (unlikely(addr & (block_size - 1))) {
			kdebugln("onenand_erase: Unaligned address");
			return -static_cast<sint>(ErrNo::EINVAL);
		}
	}

	if (unlikely(len & (block_size - 1))) {
		kdebugln ("onenand_erase: Length not block aligned");
		return -static_cast<sint>(ErrNo::EINVAL);
	}

	onenand_get_device(mtd, FCS_ERASING);

	instr->state = MTD_ERASING;

	while (len) {

		if (instr->priv == 0 && onenand_block_isbad_nolock(mtd, addr, 0)) {
			kformatln("onenand_erase: attempt to erase"
				" a bad block at addr 0x%08x",
				(unsigned int) addr);
			instr->state = MTD_ERASE_FAILED;
			goto erase_exit;
		}

		thisChip->command(mtd, ONENAND_CMD_ERASE, addr, block_size);

		onenand_invalidate_bufferram(mtd, addr, block_size);

		ret = thisChip->wait(mtd, FCS_ERASING);

		if (ret) {
			if (ret == -static_cast<sint>(ErrNo::EPERM))
				kdebugln ("onenand_erase: Device is write protected!!!");
			else
				kformatln("onenand_erase: Failed erase, block %d",
					onenand_block(thisChip, addr));
			instr->state = MTD_ERASE_FAILED;
			instr->fail_addr = addr;

			goto erase_exit;
		}

		len -= block_size;
		addr += block_size;

		if (addr == region_end) {
			if (!len)
				break;
			region++;

			block_size = region->erasesize;
			region_end = region->offset
				+ region->erasesize * region->numblocks;

			if (len & (block_size - 1)) {
		

				kdebugln("onenand_erase: Unaligned address");
				goto erase_exit;
			}
		}
	}

	instr->state = MTD_ERASE_DONE;

erase_exit:

	ret = instr->state == MTD_ERASE_DONE ? 0 : -static_cast<sint>(ErrNo::EIO);

	if (!ret)
		mtd_erase_callback(instr);

	onenand_release_device(mtd);

	return ret;
}

void onenand_sync(struct mtd_info *mtd)
{
	kdebugln("onenand_sync: called");

	onenand_get_device(mtd, FCS_SYNCING);

	onenand_release_device(mtd);
}

int onenand_block_isbad(struct mtd_info *mtd, loff_t ofs)
{
	int ret;

	if (ofs > mtd->size)
		return -static_cast<sint>(ErrNo::EINVAL);

	onenand_get_device(mtd, FCS_READING);
	ret = onenand_block_isbad_nolock(mtd,ofs, 0);
	onenand_release_device(mtd);
	return ret;
}

int onenand_block_markbad(struct mtd_info *mtd, loff_t ofs)
{
	int ret;

	ret = onenand_block_isbad(mtd, ofs);
	if (ret) {

		if (ret > 0)
			return 0;
		return ret;
	}

	ret = mtd_block_markbad(mtd, ofs);
	return ret;
}

char *onenand_print_device_info(int device, int version)
{
	int vcc, demuxed, ddp, density, flexonenand;
	char *dev_info = (char*)kheap.allocate(80);
	char *p = dev_info;

	vcc = device & ONENAND_DEVICE_VCC_MASK;
	demuxed = device & ONENAND_DEVICE_IS_DEMUX;
	ddp = device & ONENAND_DEVICE_IS_DDP;
	density = onenand_get_density(device);
	flexonenand = device & DEVICE_IS_FLEXONENAND;
	p += sprintf(dev_info, "%s%sOneNAND%s %dMB %sV 16-bit (0x%02x)",
	       demuxed ? "" : "Muxed ",
	       flexonenand ? "Flex-" : "",
	       ddp ? "(DDP)" : "", (16 << density), vcc ? "2.65/3.3" : "1.8", device);

	sprintf(p, "OneNAND version = 0x%04x", version);
	kformatln("%s", dev_info);

	return dev_info;
}

int flexonenand_set_boundary(struct mtd_info *mtd, int die,
				    int boundary, int lock)
{
	struct onenand_chip *thisChip = (struct onenand_chip *)mtd->priv;
	int ret, density, blksperdie, oldVal, newVal, thisboundary;
	loff_t addr;

	if (die >= thisChip->dies)
		return -static_cast<sint>(ErrNo::EINVAL);

	if (boundary == thisChip->boundary[die])
		return 0;

	density = onenand_get_density(thisChip->device_id);
	blksperdie = ((16 << density) << 20) >> thisChip->erase_shift;
	blksperdie >>= ONENAND_IS_DDP(thisChip) ? 1 : 0;

	if (boundary >= blksperdie) {
		kdebugln("flexonenand_set_boundary: Invalid boundary value. "
			"Boundary not changed.");
		return -static_cast<sint>(ErrNo::EINVAL);
	}

	oldVal = thisChip->boundary[die] + (die * thisChip->density_mask);
	newVal = boundary + (die * thisChip->density_mask);
	ret = flexonenand_check_blocks_erased(mtd, min(oldVal, newVal)
						+ 1, max(oldVal, newVal));
	if (ret) {
		kdebugln("flexonenand_set_boundary: Please erase blocks before boundary change");
		return ret;
	}

	thisChip->command(mtd, FLEXONENAND_CMD_PI_ACCESS, die, 0);
	thisChip->wait(mtd, FCS_SYNCING);

	thisChip->command(mtd, FLEXONENAND_CMD_READ_PI, die, 0);
	ret = thisChip->wait(mtd, FCS_READING);

	thisboundary = thisChip->read_word(thisChip->base + ONENAND_DATARAM);
	if ((thisboundary >> FLEXONENAND_PI_UNLOCK_SHIFT) != 3) {
		kdebugln("flexonenand_set_boundary: boundary locked");
		goto out;
	}

	kformatln("flexonenand_set_boundary: Changing die %d boundary: %d%s",
			die, boundary, lock ? "(Locked)" : "(Unlocked)");

	boundary &= FLEXONENAND_PI_MASK;
	boundary |= lock ? 0 : (3 << FLEXONENAND_PI_UNLOCK_SHIFT);

	addr = die ? thisChip->diesize[0] : 0;
	thisChip->command(mtd, ONENAND_CMD_ERASE, addr, 0);
	ret = thisChip->wait(mtd, FCS_ERASING);
	if (ret) {
		kformatln("flexonenand_set_boundary:"
			"Failed PI erase for Die %d", die);
		goto out;
	}

	thisChip->write_word(boundary, thisChip->base + ONENAND_DATARAM);
	thisChip->command(mtd, ONENAND_CMD_PROG, addr, 0);
	ret = thisChip->wait(mtd, FCS_WRITING);
	if (ret) {
		kformatln("flexonenand_set_boundary:"
			"Failed PI write for Die %d", die);
		goto out;
	}

	thisChip->command(mtd, FLEXONENAND_CMD_PI_UPDATE, die, 0);
	ret = thisChip->wait(mtd, FCS_WRITING);
out:
	thisChip->write_word(ONENAND_CMD_RESET, thisChip->base + ONENAND_REG_COMMAND);
	thisChip->wait(mtd, FCS_RESETING);
	if (!ret)

		flexonenand_get_size(mtd);

	return ret;
}

int onenand_probe(struct mtd_info *mtd)
{
	struct onenand_chip *thisChip = (struct onenand_chip *)mtd->priv;
	int dev_id, ver_id;
	int density;
	int ret;

	ret = thisChip->chip_probe(mtd);
	if (ret)
		return ret;

	dev_id = thisChip->read_word(thisChip->base + ONENAND_REG_DEVICE_ID);
	ver_id = thisChip->read_word(thisChip->base + ONENAND_REG_VERSION_ID);
	thisChip->technology = thisChip->read_word(thisChip->base + ONENAND_REG_TECHNOLOGY);

	mtd->name = onenand_print_device_info(dev_id, ver_id);
	thisChip->device_id = dev_id;
	thisChip->version_id = ver_id;

	onenand_check_features(mtd);

	density = onenand_get_density(dev_id);
	if (FLEXONENAND(thisChip)) {
		thisChip->dies = ONENAND_IS_DDP(thisChip) ? 2 : 1;

		mtd->numeraseregions = thisChip->dies << 1;
		mtd->eraseregions = (struct mtd_erase_region_info *)kheap.allocate(
				sizeof(struct mtd_erase_region_info) * (thisChip->dies << 1));
		if (!mtd->eraseregions)
			return -static_cast<sint>(ErrNo::ENOMEM);
	}

	thisChip->chipsize = (16 << density) << 20;

	mtd->writesize =
	    thisChip->read_word(thisChip->base + ONENAND_REG_DATA_BUFFER_SIZE);

	if (ONENAND_IS_4KB_PAGE(thisChip))
		mtd->writesize <<= 1;

	mtd->oobsize = mtd->writesize >> 5;

	mtd->erasesize = mtd->writesize << 6;

	if (FLEXONENAND(thisChip))
		mtd->erasesize <<= 1;

	thisChip->erase_shift = ffs(mtd->erasesize) - 1;
	thisChip->page_shift = ffs(mtd->writesize) - 1;
	thisChip->ppb_shift = (thisChip->erase_shift - thisChip->page_shift);
	thisChip->page_mask = (mtd->erasesize / mtd->writesize) - 1;

	if (ONENAND_IS_DDP(thisChip))
		thisChip->density_mask = thisChip->chipsize >> (thisChip->erase_shift + 1);

	thisChip->writesize = mtd->writesize;

	if (FLEXONENAND(thisChip))
		flexonenand_get_size(mtd);
	else
		mtd->size = thisChip->chipsize;

	mtd->flags = MTD_CAP_NANDFLASH;
	mtd->_erase = onenand_erase;
	mtd->_read = onenand_read;
	mtd->_write = onenand_write;
	mtd->_read_oob = onenand_read_oob;
	mtd->_write_oob = onenand_write_oob;
	mtd->_sync = onenand_sync;
	mtd->_block_isbad = onenand_block_isbad;
	mtd->_block_markbad = onenand_block_markbad;

	return 0;
}

int onenand_scan(struct mtd_info *mtd, int maxchips)
{
	int i;
	struct onenand_chip *thisChip = (struct onenand_chip *)mtd->priv;

	if (!thisChip->read_word)
		thisChip->read_word = onenand_readw;
	if (!thisChip->write_word)
		thisChip->write_word = onenand_writew;

	if (!thisChip->command)
		thisChip->command = onenand_command;
	if (!thisChip->wait)
		thisChip->wait = onenand_wait;
	if (!thisChip->bbt_wait)
		thisChip->bbt_wait = onenand_bbt_wait;

	if (!thisChip->read_bufferram)
		thisChip->read_bufferram = onenand_read_bufferram;
	if (!thisChip->write_bufferram)
		thisChip->write_bufferram = onenand_write_bufferram;

	if (!thisChip->chip_probe)
		thisChip->chip_probe = onenand_chip_probe;

	if (!thisChip->block_markbad)
		thisChip->block_markbad = onenand_default_block_markbad;
	if (!thisChip->scan_bbt)
		thisChip->scan_bbt = onenand_default_bbt;

	if (onenand_probe(mtd))
		return -static_cast<sint>(ErrNo::ENXIO);

	if (thisChip->mmcontrol) {
		kdebugln("OneNAND Sync. Burst Read support");
		thisChip->read_bufferram = onenand_sync_read_bufferram;
	}

	if (!thisChip->page_buf) {
		thisChip->page_buf = (unsigned char*)kheap.allocate(mtd->writesize);
		if (!thisChip->page_buf) {
			kdebugln("onenand_scan(): Can't allocate page_buf");
			return -static_cast<sint>(ErrNo::ENOMEM);
		}
		zero_block(thisChip->page_buf, mtd->writesize);
		thisChip->options |= ONENAND_PAGEBUF_ALLOC;
	}
	if (!thisChip->oob_buf) {
		thisChip->oob_buf = (unsigned char*)kheap.allocate(mtd->oobsize);
		if (!thisChip->oob_buf) {
			kdebugln("onenand_scan: Can't allocate oob_buf");
			if (thisChip->options & ONENAND_PAGEBUF_ALLOC) {
				thisChip->options &= ~ONENAND_PAGEBUF_ALLOC;
				kheap.free(thisChip->page_buf);
			}
			return -static_cast<sint>(ErrNo::ENOMEM);
		}
		zero_block(thisChip->oob_buf, mtd->oobsize);
		thisChip->options |= ONENAND_OOBBUF_ALLOC;
	}

	thisChip->state = FCS_READY;

	switch (mtd->oobsize) {
	case 128:
		thisChip->ecclayout = &onenand_oob_128;
		mtd->subpage_sft = 0;
		break;

	case 64:
		thisChip->ecclayout = &onenand_oob_64;
		mtd->subpage_sft = 2;
		break;

	case 32:
		thisChip->ecclayout = &onenand_oob_32;
		mtd->subpage_sft = 1;
		break;

	default:
		kformatln("No OOB scheme defined for oobsize %d", mtd->oobsize);
		mtd->subpage_sft = 0;

		thisChip->ecclayout = &onenand_oob_32;
		break;
	}

	thisChip->subpagesize = mtd->writesize >> mtd->subpage_sft;

	thisChip->ecclayout->oobavail = 0;

	for (i = 0; i < MTD_MAX_OOBFREE_ENTRIES_LARGE &&
	    thisChip->ecclayout->oobfree[i].length; i++)
		thisChip->ecclayout->oobavail += thisChip->ecclayout->oobfree[i].length;
	mtd->oobavail = thisChip->ecclayout->oobavail;

	mtd->ecclayout = thisChip->ecclayout;

	onenand_unlock_all(mtd);

	return thisChip->scan_bbt(mtd);
}

void onenand_release(struct mtd_info *mtd)
{}


END_EXT_C

