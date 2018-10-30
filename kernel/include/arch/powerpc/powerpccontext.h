/*
 *   File name: powerpc750context.h
 *
 *  Created on: 2017年6月25日, 上午1:42:33
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __ARCH_INCLUDE_POWERPC_POWERPCCONTEXT_H__
#define __ARCH_INCLUDE_POWERPC_POWERPCCONTEXT_H__

#if defined( ASSEMBLY )

#include <asm.h>

BEG_MACRO( saveContext )
	stwu     sp, -156(sp)
	stw      r0,   8(sp)
	mfcr     r0
	stw      r0,    4(sp)
	mflr     r0
	stw      r0,    148(sp)
	mfxer    r0
	stw      r0,   136(sp)
	mfctr    r0
	stw      r0,   132(sp)
	mfspr    r0,    srr0
	stw      r0,   140(sp)
	mfspr    r0,    srr1
	stw      r0,   144(sp)
	mfmsr    r0
	stw      r0,   152(sp)
	stw      r2,   12(sp)
	stw      r3,   16(sp)
	stw      r4,   20(sp)
	stw      r5,   24(sp)
	stw      r6,   28(sp)
	stw      r7,   32(sp)
	stw      r8,   36(sp)
	stw      r9,   40(sp)
	stw     r10,   44(sp)
	stw     r11,   48(sp)
	stw     r12,   52(sp)
	stw     r13,   56(sp)
	stw     r14,   60(sp)
	stw     r15,   64(sp)
	stw     r16,   68(sp)
	stw     r17,   72(sp)
	stw     r18,   76(sp)
	stw     r19,   80(sp)
	stw     r20,   84(sp)
	stw     r21,   88(sp)
	stw     r22,   92(sp)
	stw     r23,   96(sp)
	stw     r24,   100(sp)
	stw     r25,   104(sp)
	stw     r26,   108(sp)
	stw     r27,   112(sp)
	stw     r28,   116(sp)
	stw     r29,   120(sp)
	stw     r30,   124(sp)
	stw     r31,   128(sp)

END_MACRO( saveContext )

BEG_MACRO( restoreContext )
	lwz      r14 ,  60(sp)
	lwz      r15 ,  64(sp)
	lwz      r16 ,  68(sp)
	lwz      r17 ,  72(sp)
	lwz      r18 ,  76(sp)
	lwz      r19 ,  80(sp)
	lwz      r20 ,  84(sp)
	lwz      r21 ,  88(sp)
	lwz      r22 ,  92(sp)
	lwz      r23 ,  96(sp)
	lwz      r24 ,  100(sp)
	lwz      r25 ,  104(sp)
	lwz      r26 ,  108(sp)
	lwz      r27 ,  112(sp)
	lwz      r28 ,  116(sp)
	lwz      r29 ,  120(sp)
	lwz      r30 ,  124(sp)
	lwz      r31 ,  128(sp)
	lwz      r13 ,  56(sp)
	lwz      r12 ,  52(sp)
	lwz      r11 ,  48(sp)
	lwz      r10 ,  44(sp)
	lwz       r9 ,  40(sp)
	lwz       r8 ,  36(sp)
	lwz       r7 ,  32(sp)
	lwz       r6 ,  28(sp)
	lwz       r5 ,  24(sp)
	lwz       r4 ,  20(sp)
	lwz       r3 ,  16(sp)
	lwz       r2 ,  12(sp)

	lwz       r0 ,  148(sp)
	mtlr      r0

	lwz       r0 ,  132(sp)
	mtctr     r0

	lwz       r0 ,  136(sp)
	mtxer     r0

	lwz       r0 ,  140(sp)
	mtspr     srr0,    r0

	lwz       r0 ,  144(sp)
	mtspr     srr1,    r0

	lwz       r0 ,  4(sp)
	mtcr      r0

	lwz       r0 , 8(sp)
	addi      sp, sp, 156
	blr
END_MACRO( restoreContext )

#endif

#endif

