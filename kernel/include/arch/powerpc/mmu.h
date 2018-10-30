/*
 *   File name: mmu.h
 *
 *  Created on: 2016年11月23日, 下午8:26:36
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __POWERPC_MMU_H__
#define __POWERPC_MMU_H__

#if defined(__cplusplus)

#include <debug.h>
#include <spinlock.h>

namespace Powerpc
{
	namespace MMU
	{
		const u32 POWERPC_PAGE_OFFSET_BITS = 12;

		InlineStatic void setSDR1(word_t val)
		{
			__asm__ __volatile__ ("mtsdr1 %0 \n" : : "r"(val));
		}

		InlineStatic word_t getSDR1(void)
		{
			word_t val;
			__asm__ __volatile__("mfsdr1 %0 \n" : "=r"(val));
			return val;
		}

		InlineStatic word_t getDSISR(void)
		{
			word_t val;
			__asm__ __volatile__("mfspr %0, 18 \n" : "=r"(val));
			return val;
		}

		InlineStatic void setSR(word_t sr, word_t val)
		{

			__asm__ __volatile__("mtsrin %0, %1"::"r"(val), "r"(sr << 28));
		}

		InlineStatic word_t getSR(word_t effAddr)
		{
			word_t ret;
			__asm__ __volatile__("mfsrin %0, %1": "=r"(ret): "r"(effAddr));
			return ret;
		}

		void init(void);

#if ( POWERPC_FAMILY == POWERPC_MPC750 )
		const u32 POWERPC_TLBIE_INC = (1 << 14);
		const u32 POWERPC_TLBIE_MAX = (1 << 20);
#endif

		InlineStatic void ppcInvalidTlbAll(void)
		{
			word_t page;
			for (page = 0; page < POWERPC_TLBIE_MAX; page += POWERPC_TLBIE_INC)
				__asm__ __volatile__("tlbie %0" : : "r"(page));
			__asm__ __volatile__("tlbsync");
		}

		InlineStatic void ppcInvalidTlbe(addr_t effAddr)
		{
			__asm__ __volatile__("tlbie %0" : : "r"(effAddr));
		}

		const u32 POWERPC_HTABORG_SHIFT = 16;
		class SDR1
		{
		public:
			union
			{
				struct
				{
					word_t htaborg :16;
					word_t reserved :7;
					word_t htabmask :9;
				} field;
				word_t word;
			};
			void buildSDR1(word_t base, word_t mask)
			{
				word = 0;

				field.htaborg = base >> POWERPC_HTABORG_SHIFT;
				field.htabmask = mask;
			}
		};

		class SR

		{
		public:
			SR(word_t d = 0): word(d){}
			union
			{
				struct
				{
					word_t t :1;
					word_t ks :1;
					word_t kp :1;
					word_t n :1;
					word_t reserved :4;
					word_t vsid :24;
				} field;
				word_t word;
			};
		};

		class PTE
		{
		public:
			struct{
				union
				{
					struct
					{
						word_t v :1;
						word_t vsid :24;
						word_t h :1;
						word_t api :6;
					};
					word_t raw;

				}virWord;
				union
				{
					struct
					{
						word_t rpn :20;
						word_t reserved1 :3;
						word_t r :1;
						word_t c :1;
						word_t wimg :4;
						word_t reserved2 :1;
						word_t pp :2;
					};
					word_t raw;

				}phyWord;
			};
			static word_t virtAddr2API(word_t virtAddr)
			{
				return ((virtAddr >> 22) & 0x3F);
			}
			static word_t api2VirtAddr(word_t api)
			{
				return (api << 22);
			}
			void buildPTE(addr_t virtAddr, addr_t phyAddr, word_t vsid, word_t isSecondaryHash, word_t wimg, word_t pp);
			void buildPTE(addr_t virtAddr, addr_t entry, word_t vsid, word_t isSecondaryHash);
			void invalidSelf(addr_t virtAddr);
		};

		const word_t PPC_HASH_TAB_MASK = ((1 << 19) - 1);
		const word_t PPC_PETG_PAGE_MASK = ((1 << 16) - 1);
		const word_t PPC_SMALL_HASH_TABLE_BITS = 17;
		const word_t PPC_SMALL_HASH_TABLE_SIZE = (1 << PPC_SMALL_HASH_TABLE_BITS);
		const word_t PPC_SMALL_HASH_TABLE_MASK = (~(PPC_SMALL_HASH_TABLE_SIZE - 1));
		const word_t PPC_PTEG_SIZE = 8;

		class HashTable
		{
		public:
			void init(pvoid physAddr, word_t size);
			word_t sdr1(void)const;
			PTE* locatePTE(addr_t virt);
			void insertPTE(addr_t virAddr, addr_t phyAddr);
			void invalidPTE(addr_t virAddr);
			static word_t optimalSize(addr_t totalPhyMemSize);
			void showHashTable(void)const;
		private:
			word_t primaryHash(addr_t virAddr, word_t vsid);
			word_t secondaryHash(word_t primaryHashVal);
			PTE* getPTEG(word_t hashVal);
			PTE* findPTE(addr_t virtAddr, word_t *isSecondaryHash);

			PTE* _baseAddr;
			word_t _size;
			word_t _hashMask;
			Spinlock _spinlock;
		};

		extern HashTable hashTab;
		const VoidFuncPtrVoid invalidateTlbAll = ppcInvalidTlbAll;
	}
}

#if !defined(_POWERPC_MMU_ALIAS_)
#define _POWERPC_MMU_ALIAS_ 1
#define PMMU Powerpc::MMU
#endif

#if !defined(_POWERPC_MMU_HASHTAB_ALIAS_)
#define _POWERPC_MMU_HASHTAB_ALIAS_ 1
#define PHTab Powerpc::MMU::HashTable
#define phtab Powerpc::MMU::hashTab
#endif

#endif

#endif

