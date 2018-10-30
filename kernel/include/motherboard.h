/*
 *   File name: motherboard.h
 *
 *  Created on: 2017年4月13日, 下午8:25:58
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __DRIVER_MOTHERBOARD_H__
#define __DRIVER_MOTHERBOARD_H__

#include <mk/hardware.h>

#include <uart/uart.h>

#include <pic/i8259a/8259.h>
#if defined(CONFIG_PIC_MPC8260)
#include <pic/mpc8260/mpc8260intctl.h>
#endif

#include <rtc/mc146818/mc146818.h>
#include <timer/i8254/8254.h>

EXTERN_C void sysBoot(void);

namespace Hardware
{
	class MotherBoard;
	class MotherBoardImp
	{
		friend class MotherBoard;

		static Driver::Uart serials[];

		static Driver::I8259 pics[];
#if defined(CONFIG_PIC_MPC8260)
		static Driver::MPC8260IntCtl pics[];

#endif

		static Driver::MC146818 rtc;
		static Driver::I8254 timer;

		static void init(void);

		static s32 picIrqNo(void);
		static void picEnableIrq(u32 irq);
		static void picDisableIrq(u32 irq);
		static void picAckIrq(u32 irq);
#if defined(CONFIG_PIC_MPC8260)
		static s32 picIrqNo(void)
		{
			return pics[static_cast<word_t>(Hardware::MPC8260::MPC8260Enum::Master)].irqNo();
		}
		static void picEnableIrq(u32 irq)
		{
			return pics[static_cast<word_t>(Hardware::MPC8260::MPC8260Enum::Master)].unmask(irq);
		}
		static void picDisableIrq(u32 irq)
		{
			return pics[static_cast<word_t>(Hardware::MPC8260::MPC8260Enum::Master)].mask(irq);
		}
		static void picAckIrq(u32 UNUSED )
		{
			return pics[static_cast<word_t>(Hardware::MPC8260::MPC8260Enum::Master)].ack();
		}
#endif

	};

	class MotherBoard
	{
		friend void ::sysBoot(void);
		static void createAllVirtualDev(void);
		static const VoidFuncPtrVoid motherBoardInit;
	public:
		static const S32FuncPtrVoid  peripheralIrqNo;
		static const VoidFuncPtrU32  peripheralEnableIrq;
		static const VoidFuncPtrU32  peripheralDisableIrq;
		static const VoidFuncPtrU32  peripheralAckIrq;

		static Driver::Uart* enumUart(HUartEnum uartEnum){ return &MotherBoardImp::serials[static_cast<word_t>(uartEnum)]; }

		static Driver::MC146818* getRtc(void){ return &MotherBoardImp::rtc; }
	};
}

#if !defined(_DRIVER_MOTHERBOARD_ALIAS_)

#define _DRIVER_MOTHERBOARD_ALIAS_ 1
#define HMB Hardware::MotherBoard

#endif

#endif

