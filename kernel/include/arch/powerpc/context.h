/*
 *   File name: context.h
 *
 *  Created on: 2017年4月5日, 下午6:19:32
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __ARCH_INCLUDE_POWERPC_CONTEXT_H__
#define __ARCH_INCLUDE_POWERPC_CONTEXT_H__

#include <stdio.h>

#if defined(__cplusplus)
extern "C"{
#endif

typedef struct stContext
{
	word_t sp;
	word_t cr;
	word_t r0;
	word_t r2;
	word_t r3;
	word_t r4;
	word_t r5;
	word_t r6;
	word_t r7;
	word_t r8;
	word_t r9;
	word_t r10;
	word_t r11;
	word_t r12;
	word_t r13;
	word_t r14;
	word_t r15;
	word_t r16;
	word_t r17;
	word_t r18;
	word_t r19;
	word_t r20;
	word_t r21;
	word_t r22;
	word_t r23;
	word_t r24;
	word_t r25;
	word_t r26;
	word_t r27;
	word_t r28;
	word_t r29;
	word_t r30;
	word_t r31;
	word_t ctr;
	word_t xer;
	word_t srr0;
	word_t srr1;
	word_t lr;
	word_t msr;
}Context;

typedef struct stFpContext
{
	double fpRegs[32];
	u32    fpCtrlStatusReg;
	u32    fpCtlStatusCpyReg;
}FpContext;

InlineStatic void printContext(Context* c)
{
	printk("--------------Context begin-----------\r\n");
	printk("sp = 0x%x\r\n", c->sp);
	printk("cr = 0x%x\r\n", c->cr);
	printk("r0 = 0x%x\r\n", c->r0);
	printk("r2 = 0x%x\r\n", c->r2);

	printk("r3 = 0x%x\r\n", c->r3);
	printk("r4 = 0x%x\r\n", c->r4);
	printk("r5 = 0x%x\r\n", c->r5);
	printk("r6 = 0x%x\r\n", c->r6);
	printk("r7 = 0x%x\r\n", c->r7);
	printk("r8 = 0x%x\r\n", c->r8);
	printk("r9 = 0x%x\r\n", c->r9);
	printk("r10 = 0x%x\r\n", c->r10);
	printk("r11 = 0x%x\r\n", c->r11);
	printk("r12 = 0x%x\r\n", c->r12);

	printk("r13 = 0x%x\r\n", c->r13);
	printk("r14 = 0x%x\r\n", c->r14);
	printk("r15 = 0x%x\r\n", c->r15);
	printk("r16 = 0x%x\r\n", c->r16);
	printk("r17 = 0x%x\r\n", c->r17);
	printk("r18 = 0x%x\r\n", c->r18);
	printk("r19 = 0x%x\r\n", c->r19);
	printk("r20 = 0x%x\r\n", c->r20);
	printk("r21 = 0x%x\r\n", c->r21);
	printk("r22 = 0x%x\r\n", c->r22);
	printk("r23 = 0x%x\r\n", c->r23);
	printk("r24 = 0x%x\r\n", c->r24);
	printk("r25 = 0x%x\r\n", c->r25);
	printk("r26 = 0x%x\r\n", c->r26);
	printk("r27 = 0x%x\r\n", c->r27);
	printk("r28 = 0x%x\r\n", c->r28);
	printk("r29 = 0x%x\r\n", c->r29);
	printk("r30 = 0x%x\r\n", c->r30);
	printk("r31 = 0x%x\r\n", c->r31);
	printk("srr0 = 0x%x\r\n", c->srr0);
	printk("srr1 = 0x%x\r\n", c->srr1);

	printk("--------------Context over-----------\r\n");
}

#if defined(__cplusplus)
}
#endif

#endif

