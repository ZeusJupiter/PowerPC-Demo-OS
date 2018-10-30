/*
 *   File name: uart.c
 *
 *  Created on: 2016年11月16日, 下午4:14:30
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#include <platform/platform.h>

#include <macros.h>
#include <types.h>
#include "uart.h"

namespace Driver
{
	void Uart::init(word_t baseAddr, word_t freq, word_t baud, sint interNum, word_t fifolen)
	{
		_base = baseAddr;
		_frequency = freq;
		_baud = baud;
		_vecNum = interNum;

		word_t division = (_frequency + (baud << 3)) >> 4;
		division /= baud;
		_fifoLength = fifolen;

		initCacheRegs();

		out8(_base + UART_IER, 0);
		out8(_base + UART_LCR, UART_LCR_DLAB);

		out8(_base + UART_DLL, (division & 0xFF));

		out8(_base + UART_DLH, (division >> 8) & 0xFF);

		out8(_base + UART_LCR, _lcr);
		out8(_base + UART_FCR, _fcr);
		out8(_base + UART_MCR, _mcr);
	}

	void Uart::startup(void)
	{
		if(_mode == ModeSet::MODE_INTERRUPT)
		{
			if(_options & static_cast<u32>( OptionSet::CLOCAL))

			{
				_ier |= UART_IER_ETHREI;
				out8(_base + UART_IER, _ier);
			}
			else
			{
				c8 mask = in8(_base + UART_MSR) & UART_MSR_CTS;
				if(mask & UART_MSR_CTS)
				{
					_ier |= UART_IER_ETHREI;
				}
				else
				{
					_ier &= (~UART_IER_ETHREI);
					out8(_base + UART_IER, _ier);
				}
			}
		}
	}

	void Uart::setBaud(u32 baud)
	{
		word_t division = (_frequency + (baud << 3)) >> 4;
		division /= baud;

		_baud = baud;

		out8(_base + UART_IER, 0);
		out8(_base + UART_LCR, (UART_LCR_DLAB | _lcr));

		out8(_base + UART_DLL, (division & 0xFF));

		out8(_base + UART_DLH, (division >> 8) & 0xFF);

		out8(_base + UART_LCR, _lcr);

		out8(_base + UART_IER, _ier);
	}

	ErrNo Uart::ioctl(IoctlSet ioctlset, u32 arg)
	{
		ErrNo ret = ErrNo::ENONE;
		switch(ioctlset)
		{
		case IoctlSet::BAUD_SET:
			setBaud(arg);
			break;
		case IoctlSet::BAUD_GET:
			*(u32*)arg = _baud;
			break;
		case IoctlSet::MODE_GET:
			*(u32*)arg = _ier & (2 << 6);
			break;
		case IoctlSet::MODE_SET:
			if(arg == static_cast<u32>(ModeSet::MODE_INTERRUPT))
			{
				enableInterruptMode();
			}
			else if(arg == static_cast<u32>(ModeSet::MODE_POLL))
			{
				enablePollingMode();
			}
			break;
		case IoctlSet::HUP:
			hangUp();
			break;
		case IoctlSet::OPEN:
			open(arg);
			break;
		default:
			ret = ErrNo::EINVAL;
		}

		return ret;
	}

	void Uart::uartIsr(Uart* uart)
	{
		u8 iir;
		u8 maskIIR;

		iir = in8(uart->_base + UART_IIR);
		maskIIR = iir & UART_IIR_INTID_MASK;

		if(maskIIR == UART_IIR_RDAI)
			while( in8(uart->_base + UART_LSR) & UART_LSR_DATA_READY )
			{
				u8 rxd;
				rxd = in8(uart->_base + UART_RBR);
				uart->_outputRxCharFunc(uart->_outputRxCharArg, rxd);
			}
		else if( maskIIR == UART_IIR_THREI && (uart->_ier & UART_IER_ETHREI) &&
		   (in8(uart->_base + UART_LSR) & UART_LSR_THRE))
		{
			u8 txd;
			for(u32 i = 0; i < uart->_fifoLength; ++i)
			{
				if(uart->_getTxCharFunc(uart->_getTxCharArg, &txd) != ErrNo::ENONE)
				{
					uart->_ier &= (~UART_IER_ETHREI);
					break;
				}
				else
					out8(uart->_base + UART_THR, txd);
			}
		}
		else if(iir == 0)
		{
			u8 msr;
			msr = in8(uart->_base + UART_MSR);
			if(msr & UART_MSR_DCTS)
			{
				if(msr & UART_MSR_CTS)
					uart->_ier |= UART_IER_ETHREI;
				else
					uart->_ier &= (~UART_IER_ETHREI);
			}
		}

		out8(uart->_base + UART_IER, uart->_ier);
	}
}
