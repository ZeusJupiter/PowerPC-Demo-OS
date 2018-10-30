/*
 *   File name: uart_Config.h
 *
 *  Created on: 2016年11月16日, 下午4:09:07
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:

 */

#ifndef __UART_CONFIG_H__
#define __UART_CONFIG_H__

#define UART_RBR    		( 0x0 )

#define UART_THR    		( 0x0 )
#define UART_DLL    		( 0x0 )
#define UART_DLH    		( 0x1 )

#define UART_IER           ( 0x01 )

#define UART_IER_ELPM      (1 << 5)
#define UART_IER_ESM       (1 << 4)
#define UART_IER_EMSI      (1 << 3)
#define UART_IER_ERLSI     (1 << 2)
#define UART_IER_ETHREI    (1 << 1)
#define UARR_IER_ERDAI     (1)

#define UART_IIR           ( 0x02 )

#define UART_IIR_FIFO_EN     ( 3 << 6)

#define UART_IIR_FIFO_64B   (1 << 5)
#define UART_IIR_INTID_MASK      (7 << 1)
#define UART_IIR_THREI           (1 << 1)
#define UART_IIR_RDAI            (2 << 1)
#define UART_IIR_RLSI            (3 << 1)
#define UART_IIR_CTII            (6 << 1)
#define UART_IIR_IPEND     (1)
#define UART_FCR          (0x02 )
#define UART_FCR_RX_FIFO_TRG_LEVEL_1B       (0 << 6)
#define UART_FCR_RX_FIFO_TRG_LEVEL_4B       (1 << 6)
#define UART_FCR_RX_FIFO_TRG_LEVEL_8B       (2 << 6)
#define UART_FCR_RX_FIFO_TRG_LEVEL_14B      (3 << 6)
#define UART_FCR_DMA_MODE1_DIS     (0 << 3)
#define UART_FCR_DMA_MODE1_EN      (1 << 3)
#define UART_FCR_TXCLR         (1 << 2)
#define UART_FCR_RXCLR         (1 << 1)
#define UART_FCR_FIFO_EN       (1)
#define UART_LCR    		( 0x3 )
#define UART_LCR_DLAB    	( 1 << 7 )
#define UART_LCR_BC	        ( 1 << 6 )
#define UART_LCR_SP         ( 1 << 5 )
#define UART_LCR_EPS        ( 1 << 4 )
#define UART_LCR_PEN        ( 1 << 3 )
#define UART_LCR_STB_ONE_BIT       (0 << 2)
#define UART_LCR_STB_BY_WLS        (1 << 2)
#define UART_LCR_WLS_5b     	(0x0)
#define UART_LCR_WLS_6b     	(0x1)
#define UART_LCR_WLS_7b     	(0x2)
#define UART_LCR_WLS_8b     	(0x3)
#define UART_MCR           ( 0x04 )
#define UART_MCR_AUTOFLOW_DIS      0
#define UART_MCR_AUTOFLOW_EN      (1 << 5)
#define UART_MCR_LOOP_DIS    (0 << 4)
#define UART_MCR_LOOP_EN     (1 << 4)
#define UART_MCR_OUT2     (1 << 3)
#define UART_MCR_OUT1     (1 << 2)
#define UART_MCR_RTS     (1 << 1)
#define UART_MCR_DTR     (1)
#define UART_LSR    		( 0x05 )
#define UART_LSR_RXFIFOE    (1 << 7)
#define UART_LSR_TEMT       (1 << 6)
#define UART_LSR_THRE    	(1 << 5)
#define UART_LSR_BI         (1 << 4)
#define UART_LSR_FRAMING_ERR         (1 << 3)
#define UART_LSR_PARITY_ERR         (1 << 2)
#define UART_LSR_OVERRUN_ERR         (1 << 1)
#define UART_LSR_DATA_READY      	(1)

#define UART_MSR           ( 0x06 )
#define UART_MSR_DCD       (1 << 7)
#define UART_MSR_RI        (1 << 6)
#define UART_MSR_DSR       (1 << 5)
#define UART_MSR_CTS       (1 << 4)
#define UART_MSR_DDCD      (1 << 3)

#define UART_MSR_TERI      (1 << 2)

#define UART_MSR_DDSR      (1 << 1)

#define UART_MSR_DCTS      (1 << 0)

#define UART_SCR           ( 0x07 )

#endif

