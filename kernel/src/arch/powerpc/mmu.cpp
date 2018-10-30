/*
 *   File name: mmu.cpp
 *
 *  Created on: 2017年3月28日, 上午10:10:26
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */
#include <macros.h>
#include <types.h>
#include <debug.h>
#include <lockguard.h>
#include <arch/powerpc/bat.h>
#include <arch/powerpc/mmu.h>

#define MMU_PTE_MIN_SIZE_8M     0x00010000
#define MMU_PTE_MIN_SIZE_16M    0x00020000
#define MMU_PTE_MIN_SIZE_32M    0x00040000
#define MMU_PTE_MIN_SIZE_64M    0x00080000
#define MMU_PTE_MIN_SIZE_128M   0x00100000
#define MMU_PTE_MIN_SIZE_256M   0x00200000
#define MMU_PTE_MIN_SIZE_512M   0x00400000
#define MMU_PTE_MIN_SIZE_1G     0x00800000
#define MMU_PTE_MIN_SIZE_2G     0x01000000
#define MMU_PTE_MIN_SIZE_4G     0x02000000

#define MMU_VSID_PRIM_HASH             0x000fffff
#define MMU_HASH_VALUE_LOW             0x000003ff
#define MMU_HASH_VALUE_HIGH            0x0007fc00
#define MMU_HASH_VALUE_HIGH_SHIFT      10
#define MMU_PTE_HASH_VALUE_HIGH_SHIFT  16
#define MMU_PTE_HASH_VALUE_LOW_SHIFT   6
#define MMU_PTES_IN_PTEG               8

#define GB(x)  (u32)((x) << 30)
#define MB(x)  (u32)((x) << 20)
#define KB(x)  (u32)((x) << 10)

#define MMU_SDR1_HTABORG_MASK   0xffff0000
#define MMU_SDR1_HTABORG_8M     0xffff0000
#define MMU_SDR1_HTABORG_16M    0xfffe0000
#define MMU_SDR1_HTABORG_32M    0xfffc0000
#define MMU_SDR1_HTABORG_64M    0xfff80000
#define MMU_SDR1_HTABORG_128M   0xfff00000
#define MMU_SDR1_HTABORG_256M   0xffe00000
#define MMU_SDR1_HTABORG_512M   0xffc00000
#define MMU_SDR1_HTABORG_1G     0xff800000
#define MMU_SDR1_HTABORG_2G     0xff000000
#define MMU_SDR1_HTABORG_4G     0xfe000000
#define MMU_SDR1_HTABMASK_8M    0x00000000
#define MMU_SDR1_HTABMASK_16M   0x00000001
#define MMU_SDR1_HTABMASK_32M   0x00000003
#define MMU_SDR1_HTABMASK_64M   0x00000007
#define MMU_SDR1_HTABMASK_128M  0x0000000f
#define MMU_SDR1_HTABMASK_256M  0x0000001f
#define MMU_SDR1_HTABMASK_512M  0x0000003f
#define MMU_SDR1_HTABMASK_1G    0x0000007f
#define MMU_SDR1_HTABMASK_2G    0x000000ff
#define MMU_SDR1_HTABMASK_4G    0x000001ff

namespace Powerpc
{
	namespace MMU
	{
		HashTable hashTab;

		void PTE::buildPTE(addr_t virtAddr, addr_t phyAddr, word_t vsid, word_t isSecondaryHash, word_t wimg, word_t pp)
		{
			this->virWord.raw = this->phyWord.raw = 0;
			this->phyWord.rpn = phyAddr >> POWERPC_PAGE_OFFSET_BITS;
			this->phyWord.r = 1;
			this->phyWord.c = 1;
			this->phyWord.wimg = wimg;
			this->phyWord.pp = pp;

			eieio();

			this->virWord.v = 1;
			this->virWord.vsid = vsid;
			this->virWord.h = isSecondaryHash;
			this->virWord.api = virtAddr2API(virtAddr);
		}

		void PTE::buildPTE(addr_t virtAddr, addr_t entry, word_t vsid, word_t isSecondaryHash)
		{
			this->virWord.raw = 0;
			this->phyWord.raw = entry;
			eieio();
			this->virWord.vsid = vsid;
			this->virWord.h = isSecondaryHash;
			this->virWord.api = virtAddr2API(virtAddr);
			this->virWord.v = 1;
			sync();
			kformat("virAddr=0x%x, entry=0x%x, vsid=0x%x, api=0x%x\r\n", virtAddr, entry, vsid, this->virWord.api);
		}

		void PTE::invalidSelf(addr_t virAddr)
		{
			this->phyWord.raw = 0;
			this->virWord.raw = 0;
			sync();
			ppcInvalidTlbe(virAddr);
			eieio();
			tlbsync();
			sync();
		}

		void HashTable::init(pvoid phyAddr, word_t size)
		{
			kdebugln("in HashTable::init");
			_size = size;
			addr_t tabOrgMask;

			if (size >= MMU_PTE_MIN_SIZE_4G) {
				tabOrgMask  = MMU_SDR1_HTABORG_4G;
				_hashMask = MMU_SDR1_HTABMASK_4G;

			} else if (size >= MMU_PTE_MIN_SIZE_2G) {
				tabOrgMask  = MMU_SDR1_HTABORG_2G;
				_hashMask = MMU_SDR1_HTABMASK_2G;

			} else if (size >= MMU_PTE_MIN_SIZE_1G) {
				tabOrgMask  = MMU_SDR1_HTABORG_1G;
				_hashMask = MMU_SDR1_HTABMASK_1G;

			} else if (size >= MMU_PTE_MIN_SIZE_512M) {
				tabOrgMask  = MMU_SDR1_HTABORG_512M;
				_hashMask = MMU_SDR1_HTABMASK_512M;

			} else if (size >= MMU_PTE_MIN_SIZE_256M) {
				tabOrgMask  = MMU_SDR1_HTABORG_256M;
				_hashMask = MMU_SDR1_HTABMASK_256M;

			} else if (size >= MMU_PTE_MIN_SIZE_128M) {
				tabOrgMask  = MMU_SDR1_HTABORG_128M;
				_hashMask = MMU_SDR1_HTABMASK_128M;

			} else if (size >= MMU_PTE_MIN_SIZE_64M) {
				tabOrgMask  = MMU_SDR1_HTABORG_64M;
				_hashMask = MMU_SDR1_HTABMASK_64M;

			} else if (size >= MMU_PTE_MIN_SIZE_32M) {
				tabOrgMask  = MMU_SDR1_HTABORG_32M;
				_hashMask = MMU_SDR1_HTABMASK_32M;

			} else if (size >= MMU_PTE_MIN_SIZE_16M) {
				tabOrgMask  = MMU_SDR1_HTABORG_16M;
				_hashMask = MMU_SDR1_HTABMASK_16M;

			} else {
				tabOrgMask  = MMU_SDR1_HTABORG_8M;
				_hashMask = MMU_SDR1_HTABMASK_8M;
			}

			_baseAddr = (PTE*)((addr_t)phyAddr & tabOrgMask);
			kformatln("tabOrgMask=0x%x, base=0x%x, _hashMask=0x%x, size=0x%x", tabOrgMask, _baseAddr, _hashMask, _size);
			kdebugln("Power Segment Page Address Translation has been initialized");
		}
		word_t HashTable::sdr1(void)const{ return ((word_t)_baseAddr | _hashMask); }

		static PTE * locateThePTE(PTE* pteg, word_t vsid, word_t api )
		{
			for(u32 i = 0; i < PPC_PTEG_SIZE; ++i, ++pteg)
				if( (pteg->virWord.v == 1) && (pteg->virWord.vsid == vsid) && (pteg->virWord.api == api) )
					return pteg;
			return NULL;
		}

		PTE* HashTable::locatePTE(addr_t virt)
		{
			word_t vsid = ((SR)(getSR(virt >> 28))).field.vsid;
			word_t hash = primaryHash( virt, vsid );
			word_t api = PTE::virtAddr2API( virt );
			PTE *pte = locateThePTE(getPTEG(hash), vsid, api);
			if(pte == NULL)
				pte = locateThePTE(getPTEG(secondaryHash(hash)), vsid, api);

			return pte;
		}

		static PTE* findInvalidPTE(PTE* pteg)
		{
		    u32  uiIndex;
		    for (uiIndex = 0; uiIndex < MMU_PTES_IN_PTEG; ++uiIndex) {
		        if (pteg[uiIndex].virWord.v == 0) {
		            return  (&pteg[uiIndex]);
		        }
		    }

		    return  (PTE*)0;
		}

		static PTE* findUnChangedPTE(PTE* pteg)
		{
			u32  uiIndex;
			for (uiIndex = 0; uiIndex < MMU_PTES_IN_PTEG; ++uiIndex) {
				if (pteg[uiIndex].phyWord.r == 0 && pteg[uiIndex].phyWord.c == 0) {
					return  (&pteg[uiIndex]);
				}
			}

			return  (PTE*)0;
		}

		PTE* HashTable::findPTE(addr_t virtAddr, word_t *isSecondaryHash)
		{
			word_t hash;
			word_t vsid = ((SR)(getSR(virtAddr))).field.vsid;
			hash = primaryHash(virtAddr, vsid);

			PTE* pteg = getPTEG(hash);

			PTE* retPte = findInvalidPTE(pteg);
			*isSecondaryHash = 0;
			if(!retPte)
			{
				hash = secondaryHash(hash);
				PTE* secPteg = getPTEG(hash);
				retPte = findInvalidPTE(secPteg);
				*isSecondaryHash = 1;

				if(!retPte)
				{
					retPte = findUnChangedPTE(pteg);
					*isSecondaryHash = 0;
				}
				if(!retPte)
				{
					retPte = findUnChangedPTE(secPteg);
					*isSecondaryHash = 1;
				}

				if(!retPte)
				{
					retPte = &secPteg[0];
					*isSecondaryHash = 1;
				}
			}

			kformatln("retPte=0x%x", (addr_t)retPte);
			return retPte;
		}

		word_t HashTable::primaryHash(addr_t virAddr, word_t vsid)
		{
			word_t pgi;
			pgi   = virAddr >> POWERPC_PAGE_OFFSET_BITS;
			pgi &= PPC_PETG_PAGE_MASK;

		    return (vsid & PPC_HASH_TAB_MASK) ^ pgi;
		}

		word_t HashTable::secondaryHash(word_t hashVal)
		{
			return (~hashVal & PPC_HASH_TAB_MASK);
		}

		PTE* HashTable::getPTEG(word_t hashVal)
		{
			word_t lower, higher;
		    lower = (hashVal & MMU_HASH_VALUE_LOW)  << MMU_PTE_HASH_VALUE_LOW_SHIFT;
		    higher = (hashVal & MMU_HASH_VALUE_HIGH) >> MMU_HASH_VALUE_HIGH_SHIFT;
		    higher = (higher & this->_hashMask) << MMU_PTE_HASH_VALUE_HIGH_SHIFT;
		    return (PTE *)((addr_t)this->_baseAddr | higher | lower);
		}

		word_t HashTable::optimalSize(addr_t totalPhyMemSize)
		{

			kformatln("totalSize=0x%x", totalPhyMemSize);
		    if (totalPhyMemSize < MB(8)) {
		        return  (0);
		    }

		    if (totalPhyMemSize >= 0xFFFFFFFF) {
		        return MMU_PTE_MIN_SIZE_4G;

		    } else if (totalPhyMemSize >= GB(2)) {
		        return MMU_PTE_MIN_SIZE_2G;

		    } else if (totalPhyMemSize >= GB(1)) {
		        return MMU_PTE_MIN_SIZE_1G;

		    } else if (totalPhyMemSize >= MB(512)) {
		        return MMU_PTE_MIN_SIZE_512M;

		    } else if (totalPhyMemSize >= MB(256)) {
		        return MMU_PTE_MIN_SIZE_256M;

		    } else if (totalPhyMemSize >= MB(128)) {
		        return MMU_PTE_MIN_SIZE_128M;

		    } else if (totalPhyMemSize >= MB(64)) {
		        return MMU_PTE_MIN_SIZE_64M;

		    } else if (totalPhyMemSize >= MB(32)) {
		        return MMU_PTE_MIN_SIZE_32M;

		    } else if (totalPhyMemSize >= MB(16)) {
		        return MMU_PTE_MIN_SIZE_16M;

		    } else {
		        return MMU_PTE_MIN_SIZE_8M;
		    }
		}

		void HashTable::insertPTE(addr_t virtAddr, addr_t phyAddr)
		{
			word_t isSecondaryHash;
			word_t vsid;
			PTE* pte;

			vsid = ((SR)(getSR(virtAddr))).field.vsid;

			LockGuard<Spinlock> g(_spinlock);
			pte = findPTE(virtAddr, &isSecondaryHash);
			if(pte->virWord.v)

			{
				sync();
				ppcInvalidTlbe(virtAddr);
				sync();
			}

			pte->buildPTE(virtAddr, phyAddr, vsid, isSecondaryHash);
		}

		void HashTable::invalidPTE(addr_t virAddr)
		{
			PTE* pte = locatePTE(virAddr);
			LockGuard<Spinlock> g(_spinlock);
			if(pte && pte->virWord.v)
				pte->invalidSelf(virAddr);
		}

		void init(void)
		{
			__asm__ __volatile__("isync");
			for (u32 i = 0; i < 16; ++i)
				setSR(i, i);
			__asm__ __volatile__("isync");

			initBAT();
			MMU::setSDR1(phtab.sdr1());

			kdebugln("PowerPC MMU and BAT has been initialized");
		}

		void HashTable::showHashTable(void)const
		{
			kdebug("----------showHashTable------------begin\r\n");
			for(u32 i = 0; i < _size/sizeof(PTE); ++i)
			{
				if(_baseAddr[i].virWord.raw != 0 || _baseAddr[i].phyWord.raw != 0)
				{
					kformat("pte=0x%x vir=0x%x phy=0x%x i=%d\r\n", (addr_t)&_baseAddr[i], _baseAddr[i].virWord.raw, _baseAddr[i].phyWord.raw, i);
				}
			}
			kdebug("----------showHashTable------------over\r\n");
		}
	}
}

