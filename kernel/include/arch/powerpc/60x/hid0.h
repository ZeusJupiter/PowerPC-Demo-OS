/*
 *   File name: hid0.h
 *
 *  Created on: 2017年8月1日, 下午9:15:56
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __KERNEL_INCLUDE_ARCH_POWERPC_60X_HID0_H__
#define __KERNEL_INCLUDE_ARCH_POWERPC_60X_HID0_H__

#define  PPC_HID0_EMCP_BIT_RD    (31)
#define  PPC_HID0_DBP_BIT_RD     (30)
#define  PPC_HID0_EBA_BIT_RD     (29)
#define  PPC_HID0_EBD_BIT_RD     (28)
#define  PPC_HID0_BCLK_BIT_RD    (27)
#define  PPC_HID0_ECLK_BIT_RD    (25)
#define  PPC_HID0_PAR_BIT_RD     (24)
#define  PPC_HID0_DOZE_BIT_RD    (23)
#define  PPC_HID0_NAP_BIT_RD     (22)
#define  PPC_HID0_SLEEP_BIT_RD   (21)
#define  PPC_HID0_DPM_BIT_RD     (20)
#define  PPC_HID0_NHR_BIT_RD     (16)
#define  PPC_HID0_ICE_BIT_RD     (15)

#define  PPC_HID0_DCE_BIT_RD     (14)

#define  PPC_HID0_ILOCK_BIT_RD   (13)
#define  PPC_HID0_DLOCK_BIT_RD   (12)
#define  PPC_HID0_ICFI_BIT_RD    (11)
#define  PPC_HID0_DCFI_BIT_RD    (10)
#define  PPC_HID0_SPD_BIT_RD     (9)
#define  PPC_HID0_IFEM_BIT_RD    (8)
#define  PPC_HID0_SGE_BIT_RD     (7)
#define  PPC_HID0_DCFA_BIT_RD    (6)
#define  PPC_HID0_BTIC_BIT_RD    (5)
#define  PPC_HID0_ABE_BIT_RD     (3)
#define  PPC_HID0_BHT_BIT_RD     (2)
#define  PPC_HID0_NOOPTI_BIT_RD  (0)

#if !defined( ASSEMBLY )

namespace
{
		InlineStatic u32 getHid0(void)
		{
			register u32 val;
			__asm__ __volatile__("mfspr %0, 1008 \n" : "=r"(val));
			return val;
		}

		InlineStatic void setHid0(u32 val)
		{
			__asm__ __volatile__("mtspr 1008, %0 \n" : : "r"(val));
		}
		class Hid0
		{
		public:
			void read(void) { raw = getHid0(); }
			void write(void) { __asm__ __volatile__("mtspr 1008, %0 \n" : : "r"(raw)); }
			bool iCacheIsEnabled(void){ return (this->raw & (1 << PPC_HID0_ICE_BIT_RD)); }
			bool dCacheIsEnabled(void){ return (this->raw & (1 << PPC_HID0_DCE_BIT_RD)); }
			bool iCacheIsDisabled(void){ return (!(this->raw & (1 << PPC_HID0_ICE_BIT_RD))); }
			bool dCacheIsDisabled(void){ return (!(this->raw & (1 << PPC_HID0_DCE_BIT_RD))); }
			union{
				u32 raw;
				struct {
					u32 emcp   : 1;
					u32 dbp    : 1;
					u32 eba    : 1;
					u32 ebd    : 1;
					u32 bclk   : 1;
					u32 unused : 1;
					u32 eclk   : 1;
					u32 par    : 1;
					u32 doze   : 1;
					u32 nap    : 1;
					u32 sleep  : 1;
					u32 dpm    : 1;
					u32 unused1 : 3;
					u32 nhr    : 1;
					u32 ice    : 1;
					u32 dce    : 1;
					u32 ilock  : 1;
					u32 dlock  : 1;
					u32 icfi   : 1;
					u32 dcfi   : 1;
					u32 spd    : 1;
					u32 ifem   : 1;
					u32 sge    : 1;
					u32 dcfa   : 1;
					u32 btic   : 1;
					u32 unused2 : 1;
					u32 abe    : 1;
					u32 bht    : 1;
					u32 unused3 : 1;
					u32 noopti : 1;
				} x;
			};
		};

		InlineStatic void initHid0(void)
		{
			Hid0 hid0;
			hid0.raw = 0;

			hid0.x.nhr = 1;

			hid0.x.doze = 1;

			hid0.x.dpm = 1;

			hid0.x.ice = 1;

			hid0.x.dce = 1;

			hid0.x.sge = 1;

			hid0.x.btic = 1;

			hid0.x.bht = 1;

			hid0.x.noopti = 1;

			hid0.write();
			isync();
		}

		InlineStatic void enableICache(void)
		{
			Hid0 hid0;
			hid0.read();
			const u32 val = (1 << PPC_HID0_ICE_BIT_RD);
			hid0.raw |= val;
			hid0.write();
			isync();
		}

		InlineStatic void enableDCache(void)
		{
			Hid0 hid0;
			hid0.read();
			const u32 val = (1 << PPC_HID0_DCE_BIT_RD);
			hid0.raw |= val;
			hid0.write();
			isync();
		}

		InlineStatic void enableCache(void)
		{
			Hid0 hid0;
			hid0.read();
			const u32 val = (1 << PPC_HID0_ICE_BIT_RD) | (1 << PPC_HID0_DCE_BIT_RD);
			hid0.raw |= val;
			hid0.write();
			isync();
		}

		InlineStatic void disableICache(void)
		{
			Hid0 hid0;
			hid0.read();
			const u32 val = ~(1 << PPC_HID0_ICE_BIT_RD);
			hid0.raw &= val;
			hid0.write();
			isync();
		}

		InlineStatic void disableCache(void)
		{
			Hid0 hid0;
			hid0.read();
			const u32 val = ~(1 << PPC_HID0_DCE_BIT_RD);
			hid0.raw &= val;
			hid0.write();
			isync();
		}

		InlineStatic void disableDCache(void)
		{
			Hid0 hid0;
			hid0.read();
			const u32 val = ~( (1 << PPC_HID0_ICE_BIT_RD) | (1 << PPC_HID0_DCE_BIT_RD) );
			hid0.raw &= val;
			hid0.write();
			isync();
		}

		InlineStatic void invalidateICacheAll(void)
		{
			Hid0 hid0;
			hid0.read();
			if( hid0.iCacheIsDisabled() )
				return;

			register u32 raw = hid0.raw;

			hid0.raw |= (1 << PPC_HID0_ICFI_BIT_RD);
			hid0.write();
			isync();

			hid0.raw = raw;
			hid0.write();
			isync();
		}

		InlineStatic void invalidateDCacheAll(void)
		{
			register Hid0 hid0;
			hid0.read();
			register u32 raw = hid0.raw;

			if( hid0.dCacheIsDisabled() )
				return;

			hid0.raw |= (1 << PPC_HID0_DCFI_BIT_RD);
			hid0.write();
			isync();

			hid0.raw = raw;
			isync();
		}

		InlineStatic void invalidateCacheAll(void)
		{
			register Hid0 hid0;
			hid0.read();
			register u32 raw = hid0.raw;

			const u32 val = (1 << PPC_HID0_ICFI_BIT_RD) | ( 1 << PPC_HID0_DCFI_BIT_RD);
			hid0.raw |= val;
			hid0.write();
			isync();

			hid0.raw = raw;
			hid0.write();
			isync();
		}
}

#endif

#endif

