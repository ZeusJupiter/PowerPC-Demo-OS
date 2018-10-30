/*
 *   File name: asm.h
 *
 *  Created on: 2016年11月12日, 上午1:28:38
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 */

#ifndef __POWERPC_ASM_H__
#define __POWERPC_ASM_H__

#define   HIADJ(arg)  (arg)@ha
#define   HI(arg)     (arg)@h
#define   LO(arg) 	  (arg)@l

#define   LD_ADDR(reg , symAddr)  \
	lis reg, HI((symAddr)); \
	ori reg, reg, LO((symAddr));

#define   LD_VAL(reg, val)	    \
		lis reg, (((val) >> 16) & 0xFFFF); \
		ori reg, reg, ((val) & 0xFFFF);

#define   LD_LABEL(reg, label) LD_ADDR(reg, label)

#define  ARCH_TEXT_SEG_ALIGN      4
#define  ARCH_EXCE_SEG_ALIGN      4
#define  ARCH_DATA_SEG_ALIGN      4

#endif

