/*
 *   File name: hardware.h
 *
 *  Created on: 2017年3月13日, 下午4:38:42
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __MK_HARDWARE_H__
#define __MK_HARDWARE_H__

#include <platform/platform.h>

#if !defined(ASSEMBLY)

namespace Hardware
{
	namespace CPU
	{
		const u32 CPUCount = 1;
	}

    namespace Ram
    {
        const addr_t BaseAddr = CONFIG_RAM_BASE_ADDR;
        const u32 Size = CONFIG_RAM_SIZE;
    }

    namespace Rom
	{
        const addr_t BaseAddr = CONFIG_ROM_BASE_ADDR;
        const u32 Size = CONFIG_ROM_SIZE;
    }

	namespace ISA
	{
    	const addr_t BaseAddr = CONFIG_ISA_REGS_BASE_ADDR ;
    	const u32 Size = CONFIG_ISA_REGS_SIZE;
    	const u32 BusFrequency = CONFIG_ISA_FREQ;
	}

#if defined(CONFIG_ISA_MEM)

    namespace IsaMem
	{
    	const addr_t BaseAddr = CONFIG_ISA_MEM_BASE_ADDR;
    	const u32 Size = CONFIG_ISA_MEM_SIZE;
	}

#endif

    namespace I8259A
	{
    	const addr_t MasterBaseAddr = ISA::BaseAddr + CONFIG_PIC_I8259A_MASTER_OFFSET_ADDR;
    	const s32 MasterVectorBase = CONFIG_PIC_I8259A_MASTER_INTVEC;

    	const addr_t SlaverBaseAddr = ISA::BaseAddr + CONFIG_PIC_I8259A_SLAVE_OFFSET_ADDR;
    	const s32 SlaverVectorBase = CONFIG_PIC_I8259A_SLAVE_INTVEC;

    	const u32 MasterCascadeSlavrNo = CONFIG_PIC_I8259A_MASTER_SLAVE_CASCADE_NO;
    	const u32 VectorNumber = 16;

		enum class I8259AEnum : word_t
		{
			Master = 0,
			Slaver,

			TotoalI8259Num
		};
	}

#if defined(CONFIG_PIC_MPC8260)

    namespace MPC8260
	{
		const addr_t MasterBaseAddr = ISA::BaseAddr + CONFIG_PIC_MPC8260_OFFSET_ADDR;
		const s32 MasterVectorBase = COMFIG_PID_MPC8260_INTVEC;

		enum class MPC8260Enum : word_t
		{
			Master = 0,
			TotoalPICNum
		};
	}

#endif

    namespace Uart
	{
    	const addr_t Uart0BaseAddr = ISA::BaseAddr + CONFIG_UART_COM0_OFFSET_ADDR;
    	const u32 Uart0FREQ     =      CONFIG_UART_COM0_FREQ;
    	const u32 Uart0BAUD     =      CONFIG_UART_COM0_BUAD;
    	const s32 Uart0InterruptVector = CONFIG_PIC_INTVEC_BASE + CONFIG_UART_COM0_INTVEC;

    	const addr_t Uart1BaseAddr = ISA::BaseAddr + CONFIG_UART_COM1_OFFSET_ADDR;
    	const u32 Uart1FREQ     =      CONFIG_UART_COM1_FREQ;
    	const u32 Uart1BAUD     =      CONFIG_UART_COM1_BUAD;
    	const s32 Uart1InterruptVector = CONFIG_PIC_INTVEC_BASE + CONFIG_UART_COM1_INTVEC;

    	enum class UartEnum : word_t
		{
    		Uart0,
			Uart1,
			TotalUartNum
		};

    	struct stUart
    	{
    		addr_t baseAddr;
    		u32 freq;
    		u32 baud;
    		s32 interruptVector;
    	};

    	const stUart uarts[static_cast<word_t>(UartEnum::TotalUartNum)] =
    	{
			{
				Uart0BaseAddr,
				Uart0FREQ,
				Uart0BAUD,
				Uart0InterruptVector
			},
			{
				Uart1BaseAddr,
				Uart1FREQ,
				Uart1BAUD,
				Uart1InterruptVector
			}
    	};
	}

#if defined(CONFIG_DISK_IDE)
    namespace IDE
	{
#if defined(CONFIG_DISK_IDE0_OFFSET_ADDR)
    	const addr_t Ide0BaseAddr = ISA::BaseAddr + CONFIG_DISK_IDE0_OFFSET_ADDR;
#if defined(CONFIG_DISK_IDE1_OFFSET_ADDR)
    	const u32 Ide1BaseAddr = ISA::BaseAddr + CONFIG_DISK_IDE1_OFFSET_ADDR;
#endif

#endif
	}

#endif

    namespace Timer8254
	{
    	const addr_t BaseAddr = ISA::BaseAddr + CONFIG_TIMER_I8254_OFFSET_ADDR;
    	const u32 Frequency = CONFIG_TIMER_I8254_FREQ;
    	const u32 InterruptVector = CONFIG_PIC_INTVEC_BASE + CONFIG_TIMER_I8254_INTVEC;
	}

    namespace MC146818
	{
    	const addr_t BaseAddr = ISA::BaseAddr + CONFIG_RTC_MC146818_OFFSET_ADDR;
    	const u32 TimeFormate = CONFIG_RTC_MC146818_TIMEFORMAT;
    	const u32 Frequency = CONFIG_RTC_MC146818_FREQ;
    	const u32 DividerRate = CONFIG_RTC_MC146818_DIVIDERRATE;
	}
}

#if !defined(_MK_HARDWARE_ALIAS_)

#define _MK_HARDWARE_ALIAS_ 1

#define HUart Hardware::Uart
#define HUartEnum Hardware::Uart::UartEnum

#define H8259A Hardware::I8259A
#define H8259AEnum Hardware::I8259A::I8259AEnum

#endif

#endif

#endif

