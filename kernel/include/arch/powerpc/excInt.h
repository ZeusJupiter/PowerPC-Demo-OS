/*
 *   File name: excInt.h
 *
 *  Created on: 2016年11月12日, 下午5:29:45
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description: exception and interrupt table.
 * 				the following definition is from 'e300 core reference manual'(E300),
 * 				'Programming Environments Manual for 32-Bit Implementations of
 * 				the PowerPC Architecture'(PEM), 'MPC750 RISC Microprocessor
 *              Family User’s Manual'(MPC750RMFUM), 'Advance Information
 *              PowerPC 603e RISC Microprocessor Technical Summary'(603ERMTS),
 *
 * 				EXC: exception
 * 				INT: interrupt
 * 			  INSTR: instruction
 * 			  TRANS: translation
 */

#ifndef __EXCINT_H__
#define __EXCINT_H__

#define   EXC_INT_BASE             0X00000
#define   EXC_INT_SYSTEM_RESET     (EXC_INT_BASE + 0x00100)
#define   EXC_INT_MACHINE_CHECK    (EXC_INT_BASE + 0x00200)
#define   EXC_INT_DATA_STORAGE_INT  (EXC_INT_BASE + 0x00300)
#define   EXC_INT_INST_STORAGE_INT  (EXC_INT_BASE + 0x00400)
#define   EXC_INT_EXTERNAL_INT     (EXC_INT_BASE + 0x00500)
#define   EXC_INT_ALIGNMENT        (EXC_INT_BASE + 0x00600)
#define   EXC_INT_PROGRAM          (EXC_INT_BASE + 0x00700)
#define   EXC_INT_FPU_UNAVAILABLE  (EXC_INT_BASE + 0x00800)
#define   EXC_INT_DECREMENTER      (EXC_INT_BASE + 0x00900)
#define   EXC_INT_CRITICAL_INT     (EXC_INT_BASE + 0x00A00)
#define   EXC_INT_SYSTEM_CALL      (EXC_INT_BASE + 0xC00)
#define   EXC_INT_TRACE            (EXC_INT_BASE + 0x00D00)
#define   EXC_INT_FPU_ASSIST      (EXC_INT_BASE + 0X00E00)
#define   EXC_INT_PERFORMANCE_MONITOR     (EXC_INT_BASE + 0x00F00)
#define   EXC_INT_INST_TRANS_MISS     (EXC_INT_BASE + 0x01000)
#define   EXC_INT_DATA_LOAD_TRANS_MISS     (EXC_INT_BASE + 0x01100)
#define   EXC_INT_DATA_STORE_TRANS_MISS     (EXC_INT_BASE + 0x01200)
#define   EXC_INT_INST_ADDRESS_BREAKPOINT     (EXC_INT_BASE + 0x01300)
#define   EXC_INT_SYSTEM_MANAGEMENT_INT     (EXC_INT_BASE + 0x01400)
#define   EXC_INT_THERMAL_MANAGEMENT_INT      (EXC_INT_BASE + 0x01700)
#define   EXC_INT_RESERVED_2F00               (EXC_INT_BASE + 0x02F00)

#endif

