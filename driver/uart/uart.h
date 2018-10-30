/*
 *   File name: uart.h
 *
 *  Created on: 2016年11月16日, 下午4:10:19
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:

 */

#ifndef __UART_H__
#define __UART_H__


#include <io.h>
#include <spinlock.h>
#include <errno.h>

#include "uart_config.h"

namespace Hardware
{
	class MotherBoardImp;
}

typedef ErrNo (*ErrorNoFuncPtrPvoidVar)(pvoid,...);

namespace Driver
{
	class SerialTty;
	class Uart
	{
	public:

		static void uartIsr(Uart* uart);

		enum class IoctlSet : word_t
		{
			BAUD_SET		= 0x1003,
			BAUD_GET		= 0x1004,
			HW_OPTS_SET		= 0x1005,
			HW_OPTS_GET		= 0x1006,
			MODE_SET		= 0x1007,
			MODE_GET		= 0x1008,
			AVAIL_MODES_GET	= 0x1009,

			OPEN = 0x100A,
			HUP	 = 0x100B,
			MODEM_STAT_GET        = 0x100C,
			MODEM_CTRL_BITS_SET   = 0x100D,
			MODEM_CTRL_BITS_CLR   = 0x100E,
			MODEM_CTRL_ISIG_MASK  = 0x100F,
			MODEM_CTRL_OSIG_MASK  = 0x1010,

			KYBD_MODE_SET  = 0x1011,
			KYBD_MODE_GET  = 0x1012,

			KYBD_LED_SET  = 0x1013,
			KYBD_LED_GET  = 0x1014,

			DEV_LOCK    = 0x1015,
			DEV_UNLOCK  = 0x1016,

		};

		enum class ModeSet : ushort
		{
			MODE_POLL = 1,
		    MODE_INTERRUPT = 2
		};

		enum class OptionSet : word_t
		{
			CLOCAL = 0x1,
			CREAD  = 0x2,

			CSIZE  = 0xc,
			CS5	   = 0x0,
			CS6	   = 0x4,
			CS7	   = 0x8,
			CS8	   = 0xc,

			HUPCL	= 0x10,
			STOPB	= 0x20,
			PARENB	= 0x40,
			PARODD	= 0x80,
		};

		enum class ModemSignalsSet : u8
		{
			MODEM_DTR = 0x01,
			MODEM_RTS = 0x02,
			MODEM_CTS = 0x04,
			MODEM_CD  = 0x08,
			MODEM_RI  = 0x10,
			MODEM_DSR = 0x20,
		};

		enum class KeyboardModeSet : u8
		{
			KYBD_MODE_RAW     = 1,
			KYBD_MODE_ASCII   = 2,
			KYBD_MODE_UNICODE = 3,
		};

		enum class KeyboardLedSet : u8
		{
			KYBD_LED_NUM = 1,
			KYBD_LED_CAP = 2,
			KYBD_LED_SCR = 4,
		};

		enum class CallbackType : word_t
		{
			CALLBACK_GET_TX_CHAR,
			CALLBACK_PUT_RX_CHAR
		};
#define   __isEmptyNowByLSR     ( read8(UART_LSR + this->_base) & UART_LSR_THRE )
		void putcByPolling(const char ch)
		{
			while(! __isEmptyNowByLSR ) ;
			out8(UART_THR + this->_base, ch);
		}

		void putsByPolling(const char* msgPtr)
		{
			if( NULL == msgPtr) return ;
			while( '\0' != *msgPtr )
			{
				while(! __isEmptyNowByLSR ) ;
				out8(UART_THR + this->_base, *msgPtr++);
			}
		}
#undef __isEmptyNowByLSR
		char getcByPolling(bool isBlock)
		{
			if( !(read8(UART_LSR + this->_base) & UART_LSR_DATA_READY) )
			{
				if(!isBlock) return (char)-1u;
				while( !(read8(UART_LSR + this->_base) & UART_LSR_DATA_READY) ) ;
			}
			return read8(UART_RBR + this->_base);
		}

	private:
		explicit Uart(void){}
		Uart(const Uart&) = delete;
		Uart& operator=(const Uart&) = delete;

		void setIER(u8 ier){ _ier = ier; }

		void enableInterruptMode(void)const{ out8(_base + UART_IER, _ier); }
		void enablePollingMode(void)const{ out8(_base + UART_IER, 0); }
		void disableInterruptMode(void)const{ enablePollingMode(); }

		void setBaud(u32 baud);

		void setMCR(u8 mcr){ _mcr = mcr; }
		void setLCR(u8 lcr){ _lcr = lcr; }
		void setFCR(u8 fcr){ _fcr = fcr; }

		void open(u32 )
		{
			out8(_base + UART_MCR, _mcr);
			out8(_base + UART_FCR, _fcr);
			out8(_base + UART_LCR, _lcr);
		}

		void reset(void)const
		{
			out8(_base + UART_IER, 0);
			out8(_base + UART_MCR, _mcr);
			out8(_base + UART_FCR, _fcr);
			out8(_base + UART_LCR, _lcr);
			out8(_base + UART_IER, _ier);
		}

		void hangUp(void)
		{
			out8(_base + UART_IER, 0);
			out8(_base + UART_MCR, _mcr | (~(UART_MCR_RTS | UART_MCR_DTR)));
			out8(_base + UART_FCR, UART_FCR_TXCLR | UART_FCR_RXCLR);
			out8(_base + UART_IER, _ier);
		}

		void close(void){ hangUp(); }

		ErrNo ioctl(IoctlSet ioctlset, u32 arg);
		void init(word_t baseAddr, word_t freq, word_t buad, sint interNum, word_t fifolen = 16);

		void initCacheRegs(void)
		{
			_mode = ModeSet::MODE_POLL;
			_ier = UART_IER_ETHREI | UARR_IER_ERDAI;
			_lcr = UART_LCR_WLS_8b| UART_LCR_STB_BY_WLS;
			_mcr = UART_MCR_DTR | UART_MCR_RTS | UART_MCR_OUT2;
			_fcr = UART_FCR_RX_FIFO_TRG_LEVEL_14B | UART_FCR_TXCLR | UART_FCR_RXCLR | UART_FCR_FIFO_EN;
		}

		void startup(void);

		word_t _base;
		word_t _frequency;
		word_t _baud;
		sint   _vecNum;

		word_t _fifoLength;

		ErrorNoFuncPtrPvoidVar _getTxCharFunc;
		pvoid _getTxCharArg;

		ErrorNoFuncPtrPvoidVar _outputRxCharFunc;
		pvoid _outputRxCharArg;

		u8 _ier;
		u8 _mcr;
		u8 _lcr;
		u8 _fcr;

		u16 _regDelta;
		ModeSet _mode;

		u32 _options;

		friend class Hardware::MotherBoardImp;
		friend class SerialTty;
	};
}

#endif

