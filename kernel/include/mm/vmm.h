/*
 *   File name: vmm.h
 *
 *  Created on: 2017年3月21日, 下午8:37:10
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __KERNEL_INCLUDE_VMM_H__
#define __KERNEL_INCLUDE_VMM_H__

#include <mk/oscfg.h>
#include <list.h>
#include <lockguard.h>
#include <mutex.h>
#include <errno.h>
#include <kconst.h>

namespace Kernel
{
	namespace Procedure
	{
		class GeneralProc;
	}
	namespace MM
	{
		typedef addr_t physical_addr ;
		typedef addr_t virtual_addr ;

		enum Wimg
		{
			GUARDED            =  0x01,
			MEMEMORY_COHERENCE =  0x02,
			CACHAE_INHIBITED   =  0x04,
			WRITE_THROUGH      =  0x08,

	

			GUARDED_COHERENCE = GUARDED | MEMEMORY_COHERENCE
		};
		enum PageProtection{
			NO_USERD0    =   0x00000000,
			NO_USERD1    =   0x00000001,
			READ_WRITE   =   0x00000002,
			READ_ONLY    =   0x00000003
		};

		struct PageEntry{

			union{
				word_t raw;
				struct{
					word_t rpn :20;
					word_t reserved1 :3;
					word_t r :1;
					word_t c :1;
					word_t wimg :4;
  				    word_t reserved2 :1;
					word_t pp :2;
				};
			};
			bool isValid(void)const{ return (raw != 0); }
			bool isWritable(void)const{ return (pp == READ_ONLY); }
			bool isDirty(void)const{ return c; }

			void setReadOnly(void){ pp = READ_ONLY; }
			void setWritable(void){ pp = READ_WRITE; }
			void setWimg(word_t val){ wimg = val; }
			void setRPN(physical_addr phyAddr){ raw |= (phyAddr & PageTableEntryRpnMask); }
			word_t getEntry(void)const{ return (raw); }
			physical_addr getRpn(void){ return raw & PageTableEntryRpnMask; }
		};

		struct PageTable
		{
			PageEntry pageEntries[PageTableEntryNumber];
		};

		enum PageDirectoryEntryFlag
		{
			PDEF_PRESENT    =    1,
			PDEF_WRITABLE   =    2,
			PDEF_USER	    =    4,
			PDEF_WRITETHOUT =    8,
			PDEF_NOT_CACHEABLE	= 0x10,
			PDEF_ACCESSED   = 0x20,
			PDEF_DIRTY	    = 0x40,
			PDEF_4MB	    = 0x80

		};

		struct PageTableEntry
		{
			union{
				u32 _pageDirectoryEntry;

				struct{
					word_t pt : 20 ;
					word_t __pad : 4 ;
					word_t ps : 1 ;
					word_t d : 1 ;
					word_t a : 1 ;
					word_t nc : 1 ;
					word_t wt : 1 ;
					word_t u : 1 ;
					word_t w : 1 ;
					word_t p : 1 ;
				}field;
			};

			void addFlag(PageDirectoryEntryFlag newFlags){ _pageDirectoryEntry |= newFlags; }
			void delFlag(PageDirectoryEntryFlag newFlags){ _pageDirectoryEntry &= ~newFlags; }
			bool isPresent(void){ return _pageDirectoryEntry & PageDirectoryEntryFlag::PDEF_PRESENT; }
			bool isUser(void){ return _pageDirectoryEntry & PageDirectoryEntryFlag::PDEF_USER; }
			bool isWritable(void){ return _pageDirectoryEntry & PageDirectoryEntryFlag::PDEF_WRITABLE; }
			bool is4MB(void){ return _pageDirectoryEntry & PageDirectoryEntryFlag::PDEF_4MB; }
			physical_addr pageTableAddr(void){ return _pageDirectoryEntry & PageDirectoryEntryMask; }
		};

		struct PageDirectoryTable
		{
			PageTableEntry pageTableEntries[PageDirectoryEntryNumber];
		};

		struct Page
		{
			addr_t _virAddr;
			addr_t _phyAddr;
			DoubleList _doubleList;
		};

		class Vmm
		{
		public:
			void firstInit(void);
			ErrNo secondInit(void);
			ErrNo init(addr_t *from , const addr_t to);
			u32 getFreePageNum(void)const{ return _freePageCounter; }
	

			void releaseProcAllPages(Procedure::GeneralProc* proc);

	

	

			ErrNo mapping4KB(Procedure::GeneralProc* const gProc, virtual_addr virAddr, word_t flags, bool isIdentityMapping = false);
			ErrNo unmapping4KB(Procedure::GeneralProc* const gProc, virtual_addr virAddr);
	

		private:
			Page* getFreePage(void);
			u32 _freePageCounter;
			Mutex _freePageLock;
			DoubleList _freePageList;
		};

		extern Vmm vmm;
	}

}

#if !defined(__KERNEL_MM_ALIAS__)
#define __KERNEL_MM_ALIAS__ 1
#define KMM  Kernel::MM
#endif

#if !defined(__KERNEL_VMM_ALIAS__)
#define __KERNEL_VMM_ALIAS__ 1
#define kvmm Kernel::MM::vmm
#endif

#endif

