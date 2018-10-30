/*
 *   File name: oscfg.h
 *
 *  Created on: 2017年3月13日, 下午5:51:04
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __MK_OSCFG_H__
#define __MK_OSCFG_H__

#include "hardware.h"

#define TTY_NOT_7_BIT 1

namespace OSCfg
{
	namespace MemLayout
	{
		struct MemBlock
		{
			addr_t baseAddr;
			word_t size;
		};

		const MemBlock dmaSpaces[] =
		{
			{
				CONFIG_RAM_BASE_ADDR + CONFIG_OS_SIZE,
				CONFIG_DMA_SIZE
			}
		};
		const u32 DmaSegmentCount = sizeof(dmaSpaces) / sizeof(MemBlock);

		const MemBlock VmmSpaces[] =
		{
				{
					CONFIG_RAM_BASE_ADDR + CONFIG_OS_SIZE + CONFIG_DMA_SIZE,
					Hardware::Ram::Size - (CONFIG_OS_SIZE + CONFIG_DMA_SIZE)
				}
		};
		const u32 VmmSegmentCount = sizeof(VmmSpaces) / sizeof(MemBlock);
	}

	namespace Dev
	{
		const u32 TTY_NAME_LEN = 16;
		const u32 MAX_DEV_NUM = 64;

		enum class SerialTtyEnum : word_t
		{
				SerialTty0,
				SerialTty1,
				TotalSerialTtyCount
		};

		struct stSerialTty
		{
			SerialTtyEnum ttyEnum;
			HUartEnum comEnum;
			const c8* ttyName;
			uint readBufferSize;
			uint writeBufferSize;
		};

		const stSerialTty serialTTies[static_cast<word_t>(SerialTtyEnum::TotalSerialTtyCount)] =
		{
			{
				SerialTtyEnum::SerialTty0,
				HUartEnum::Uart0,
				"ttys0",
				30,
				50
			},
			{
				SerialTtyEnum::SerialTty1,
				HUartEnum::Uart1,
				"ttys1",
				30,
				50
			}
		};
	}

	namespace Clock
	{
		const u32 SystemTicksPreSecond = 100;
	}
}

#if !defined(_MK_OSCFG_ALIAS_)

#define _MK_OSCFG_ALIAS_ 1
#define OMLayout OSCfg::MemLayout
#define OSDev    OSCfg::Dev
#endif

#endif

