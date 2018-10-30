/*
 *   File name: motherboard.cpp
 *
 *  Created on: 2017年4月13日, 下午8:34:13
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */
#include <macros.h>
#include <types.h>
#include <debug.h>
#include <io.h>

#include <time.h>

#include <motherboard.h>

#include <tty/serialtty.h>

namespace Hardware
{
	const VoidFuncPtrVoid MotherBoard::motherBoardInit = MotherBoardImp::init;
	const S32FuncPtrVoid  MotherBoard::peripheralIrqNo = MotherBoardImp::picIrqNo;
	const VoidFuncPtrU32  MotherBoard::peripheralEnableIrq = MotherBoardImp::picEnableIrq;
	const VoidFuncPtrU32  MotherBoard::peripheralDisableIrq = MotherBoardImp::picDisableIrq;
	const VoidFuncPtrU32  MotherBoard::peripheralAckIrq = MotherBoardImp::picAckIrq;

	Driver::Uart MotherBoardImp::serials[static_cast<word_t>(Hardware::Uart::UartEnum::TotalUartNum)];

	Driver::I8259 MotherBoardImp::pics[static_cast<word_t>(Hardware::I8259A::I8259AEnum::TotoalI8259Num)];
#if defined(CONFIG_PIC_MPC8260)
	Driver::MPC8260IntCtl MotherBoardImp::pics[static_cast<word_t>(Hardware::MPC8260::MPC8260Enum::TotoalPICNum)];
#endif

	Driver::MC146818 MotherBoardImp::rtc;
	Driver::I8254 MotherBoardImp::timer;


	void MotherBoardImp::init(void)
	{
		for(auto index = static_cast<word_t>(HUartEnum::Uart0);
			index < static_cast<word_t>(HUartEnum::TotalUartNum);
			++index)
		{
			serials[static_cast<word_t>(index)].init(HUart::uarts[index].baseAddr,
					HUart::uarts[index].freq,
					HUart::uarts[index].baud,
					HUart::uarts[index].interruptVector);
		}

		pics[static_cast<word_t>(H8259AEnum::Master)].init(H8259A::MasterBaseAddr,
				H8259A::MasterVectorBase, 1 << H8259A::MasterCascadeSlavrNo);
		pics[static_cast<word_t>(H8259AEnum::Slaver)].init(H8259A::SlaverBaseAddr,
				H8259A::SlaverVectorBase, H8259A::MasterCascadeSlavrNo);

		pics[static_cast<word_t>(H8259AEnum::Master)].unmask(H8259A::MasterCascadeSlavrNo);

#if defined(CONFIG_PIC_MPC8260)
		pics[static_cast<word_t>(Hardware::MPC8260::MPC8260Enum::Master)].init(Hardware::MPC8260::MasterBaseAddr);
#endif

		rtc.init(Hardware::MC146818::BaseAddr);
		timer.init(Hardware::Timer8254::BaseAddr, 1193182, Hardware::Timer8254::InterruptVector);
	}

	s32 MotherBoardImp::picIrqNo(void)
	{
		s32 masterIrq = pics[static_cast<word_t>(Hardware::I8259A::I8259AEnum::Master)].irr();

		masterIrq &= 0x07;

		s32 slaverIrq = pics[static_cast<word_t>(Hardware::I8259A::I8259AEnum::Slaver)].irr();

		slaverIrq &= 0x07;

		masterIrq |= (slaverIrq << 8) + H8259A::MasterVectorBase;

		if(masterIrq == (H8259A::MasterVectorBase + 7) && !( pics[static_cast<word_t>(Hardware::I8259A::I8259AEnum::Master)].isr() & (1 << 7) ))
		{
			return -1;
		}

		if(masterIrq == (H8259A::SlaverVectorBase + 7) && !( pics[static_cast<word_t>(Hardware::I8259A::I8259AEnum::Slaver)].isr() & (1 << 15) ))
		{
			pics[static_cast<word_t>(Hardware::I8259A::I8259AEnum::Master)].ackNormalEOI();
			return -1;
		}
		return (masterIrq);
	}

	void MotherBoardImp::picEnableIrq(u32 irq)
	{
		if(irq >= Hardware::I8259A::MasterVectorBase && irq < (Hardware::I8259A::MasterVectorBase + Hardware::I8259A::VectorNumber))
		{
			int index = (irq - Hardware::I8259A::MasterVectorBase) >> 3;
			pics[index].unmask(irq - ((index > 0) ? H8259A::SlaverVectorBase : H8259A::MasterVectorBase));
		}
	}
	void MotherBoardImp::picDisableIrq(u32 irq)
	{
		if(irq >= Hardware::I8259A::MasterVectorBase && irq < (Hardware::I8259A::MasterVectorBase + Hardware::I8259A::VectorNumber))
		{
			int index = (irq - Hardware::I8259A::MasterVectorBase) >> 3;
			pics[index].mask(irq - ((index > 0) ? H8259A::SlaverVectorBase : H8259A::MasterVectorBase));
		}
	}

	void MotherBoardImp::picAckIrq(u32 irq)
	{
		if(irq >= Hardware::I8259A::MasterVectorBase && irq < (Hardware::I8259A::MasterVectorBase + Hardware::I8259A::VectorNumber))
		{
			if( irq > Hardware::I8259A::MasterVectorBase + 7)
				pics[static_cast<word_t>(H8259AEnum::Slaver)].ackNormalEOI();
			pics[static_cast<word_t>(H8259AEnum::Master)].ackNormalEOI();
		}
	}

	void MotherBoard::createAllVirtualDev(void)
	{
		Driver::SerialTty::installDriver();

		for(auto index = static_cast<word_t>(OSDev::SerialTtyEnum::SerialTty0);
					index < static_cast<word_t>(OSDev::SerialTtyEnum::TotalSerialTtyCount);
					++index)
			Driver::SerialTty::createStty(OSDev::serialTTies[index].ttyEnum,
					enumUart(OSDev::serialTTies[index].comEnum),
					OSDev::serialTTies[index].ttyName,
					OSDev::serialTTies[index].readBufferSize,
					OSDev::serialTTies[index].writeBufferSize, false);
	}
}

