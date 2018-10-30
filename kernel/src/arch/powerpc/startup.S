/*
 *   File name: startup.S
 *
 *  Created on: 2016年11月12日, 上午2:25:38
 *      Author: victor 
 *   Toolchain: 
 *    Language: Powerpc Assembly
 */

#include <asm.h>
#include <arch/powerpc/msr.h>
#include <arch/powerpc/excInt.h>

#define FRAME_SIZE 160

IMPORT_FUNC( machineCheckIntHandler )
IMPORT_FUNC( dataStorageIntHandler )
IMPORT_FUNC( intructionStorageIntHandler )
IMPORT_FUNC( decrementIntHandler )
IMPORT_FUNC( systemCallIntHandler )
IMPORT_FUNC( externalIntHandler )
IMPORT_FUNC( criticalIntHandler )
IMPORT_FUNC( alignmentIntHandler )
IMPORT_FUNC( programIntHandler )
IMPORT_FUNC( fpuUnaviableIntHandler )
IMPORT_FUNC( traceIntHandler )
IMPORT_FUNC( fpuAssistIntHandler )
IMPORT_FUNC( preformanceMonitorIntHandler )
IMPORT_FUNC( intructionTranslationMissIntHandler )
IMPORT_FUNC( dataLoadTranslationMissIntHandler )
IMPORT_FUNC( dataStoreTranslationMissIntHandler )
IMPORT_FUNC( instructionAddressBreakpointIntHandler )
IMPORT_FUNC( systemManagementIntHandler )
IMPORT_FUNC( thermalManagementIntHandler )

IMPORT_FUNC( sysBoot )

EXPORT_FUNC( _exc_int_start_ )
EXPORT_FUNC( _systemReset )
EXPORT_FUNC( _eabi )
EXPORT_FUNC( archPpcIRet )

 #define DEF_EXC(offset, name) \
 	. = offset;                \
 _##name##:

BEG_MACRO( SAVE_REGS )
    stwu     sp, -156(sp)
    stw      r3,   16(sp)
    stw      r4,   20(sp)
    mfcr     r3
    mflr     r4
	stw      r3,    4(sp)
	stw      r4,    148(sp)
	mfxer    r3
	mfctr    r4
	stw      r3,   136(sp)
    stw      r4,   132(sp)
    stw      r0,   8(sp)
    stw      r2,   12(sp)
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
    mfspr    r3,    srr0
    mfspr    r4,    srr1
    stw      r3,   140(sp)
    stw      r4,   144(sp)
    subi     sp, sp, 32
END_MACRO( SAVE_REGS )

BEG_MACRO( ENABLE_MMU )
    mtspr sprg3 , r3
    mfmsr r3
    ori   r3, r3, PPC_EXECPTION_MSR
    mtmsr r3
    isync
    mfspr r3, sprg3
END_MACRO( ENABLE_MMU )

BEG_SEC(".except", ARCH_EXCE_SEG_ALIGN)

BEG_FUNC(_exc_int_start_)

DEF_EXC(EXC_INT_BASE, base)
	b _systemReset
	.long 0

BEG_FUNC(archPpcIRet)
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

        rfi

failed: b failed ;

DEF_EXC( EXC_INT_SYSTEM_RESET , systemReset )

    LD_ADDR( r3, _img_text_start )
    LD_ADDR( r4, _text_start )

    cmplw r3, r4
    beq   copyData

    subi r3, r3, 4
    subi r4, r4, 4

    LD_ADDR( r5, _img_text_end )
cont:
    lwzu  r6, 4(r3)
    stwu  r6, 4(r4)
    cmplw r3, r5
    blt   cont

copyData:
	LD_ADDR( r3 , _img_data_start )
	LD_ADDR( r4 , _data_start )

	cmplw  r3 , r4
	beq    clearBss

	subi   r3, r3, 4
	subi   r4, r4, 4

	LD_ADDR( r5 , _img_data_end )
1:  lwzu  r6, 4(r3)
	stwu  r6, 4(r4)
	cmplw r3, r5
	blt 1b

clearBss:

	LD_ADDR( r3 , _bss_start - 4 )
	LD_ADDR( r5 , _bss_end )

	xor    r4 , r4 , r4
2:  stwu   r4 , 4(r3)
	cmplw  r3 , r5
	blt    2b

    xor     r3, r3, r3
    xor     r0, r0, r0

	LD_ADDR( r7, sysBoot )
	mtspr  srr0, r7

    LD_VAL(r4, PPC_REAL_MSR)

    mtspr  srr1, r4

    LD_ADDR( sp, __boot_stack_end - 8)
    stw sp, 0(sp)
    stw r0, 4(sp)

	rfi

DEF_EXC( EXC_INT_MACHINE_CHECK , machineCheckInt )

    SAVE_REGS

    mr r3, sp
    bl machineCheckIntHandler
    addi     sp, sp, 32
    b       archPpcIRet

DEF_EXC( EXC_INT_DATA_STORAGE_INT , dataStorageInt )

    ENABLE_MMU
	SAVE_REGS

	mfspr   p0 , dar
	mfspr   p1 , dsisr
	mr      p2 , sp
	bl      dataStorageIntHandler
	addi     sp, sp, 32
	b       archPpcIRet

DEF_EXC( EXC_INT_INST_STORAGE_INT , intructionStorageInt )

    ENABLE_MMU
	SAVE_REGS

    mfspr   p0 , dar
    mfspr   p1 , dsisr
	bl      intructionStorageIntHandler
	addi     sp, sp, 32
	b       archPpcIRet

DEF_EXC( EXC_INT_EXTERNAL_INT , externalInt )

    ENABLE_MMU
    SAVE_REGS

    mr r3, sp
    bl externalIntHandler
    addi     sp, sp, 32
    b       archPpcIRet

DEF_EXC( EXC_INT_ALIGNMENT , alignmentInt )

    ENABLE_MMU
    SAVE_REGS

    mfspr   p0 , dar
    mfspr   p1 , dsisr
    mr      p2 , sp
    bl alignmentIntHandler
    addi     sp, sp, 32
    b       archPpcIRet

DEF_EXC( EXC_INT_PROGRAM , programInt )

    ENABLE_MMU
    SAVE_REGS

    mr r3, sp
    bl programIntHandler
    addi     sp, sp, 32
    b       archPpcIRet

DEF_EXC( EXC_INT_FPU_UNAVAILABLE , fpuUnaviableInt )

    ENABLE_MMU
    SAVE_REGS

    mr r3, sp
    bl fpuUnaviableIntHandler
    addi     sp, sp, 32
    b       archPpcIRet

DEF_EXC( EXC_INT_DECREMENTER , decrementInt )

    ENABLE_MMU
    SAVE_REGS

    mr      r3, sp
    bl      decrementIntHandler
	addi     sp, sp, 32
    b       archPpcIRet

DEF_EXC( EXC_INT_CRITICAL_INT , criticalInt )

    ENABLE_MMU
    SAVE_REGS

    mr      r3, sp
    bl      decrementIntHandler
	addi     sp, sp, 32
    b       archPpcIRet

DEF_EXC( EXC_INT_SYSTEM_CALL , systemCallInt )

    ENABLE_MMU
    SAVE_REGS

    lwz     p0 , 20(sp)
    lwz     p1 , 24(sp)
    lwz     p2 , 28(sp)
    lwz     p3 , 32(sp)
    lwz     p4 , 36(sp)
    lwz     p5 , 40(sp)

    mfmsr   r0 ;
    ori     r0 , r0, PPC_MSR_EE ;
    mtmsr   r0 ;

    bl      systemCallIntHandler
    stw     r3 , 20(r1)
	addi     sp, sp, 32
    b       archPpcIRet

DEF_EXC( EXC_INT_TRACE , traceInt )

    ENABLE_MMU
    SAVE_REGS

    mr r3, sp
    bl traceIntHandler
    addi     sp, sp, 32
    b       archPpcIRet

DEF_EXC( EXC_INT_FPU_ASSIST , fpuAssistInt )

    ENABLE_MMU
    SAVE_REGS

    mr r3, sp
    bl fpuAssistIntHandler
    addi     sp, sp, 32
    b       archPpcIRet

DEF_EXC( EXC_INT_PERFORMANCE_MONITOR , preformanceMonitorInt )

    ENABLE_MMU
    SAVE_REGS

    mr r3, sp
    bl preformanceMonitorIntHandler
    addi     sp, sp, 32
    b       archPpcIRet

DEF_EXC( EXC_INT_INST_TRANS_MISS , intructionTranslationMissInt )

    ENABLE_MMU
    SAVE_REGS

    mr r3, sp
    bl intructionTranslationMissIntHandler
    addi     sp, sp, 32
    b       archPpcIRet

DEF_EXC( EXC_INT_DATA_LOAD_TRANS_MISS , dataLoadTranslationMissInt )

    ENABLE_MMU
    SAVE_REGS

    mr r3, sp
    bl dataLoadTranslationMissIntHandler
    addi     sp, sp, 32
    b       archPpcIRet

DEF_EXC( EXC_INT_DATA_STORE_TRANS_MISS , dataStoreTranslationMissInt )

    ENABLE_MMU
    SAVE_REGS

    mr r3, sp
    bl dataStoreTranslationMissIntHandler
    addi     sp, sp, 32
    b       archPpcIRet

DEF_EXC( EXC_INT_INST_ADDRESS_BREAKPOINT , instructionAddressBreakpointInt )

    ENABLE_MMU
    SAVE_REGS

    mr r3, sp
    bl instructionAddressBreakpointIntHandler
    addi     sp, sp, 32
    b       archPpcIRet

DEF_EXC( EXC_INT_SYSTEM_MANAGEMENT_INT , systemManagementInt )

    ENABLE_MMU
    SAVE_REGS

    mr r3, sp
    bl systemManagementIntHandler
    addi     sp, sp, 32
    b       archPpcIRet

DEF_EXC( EXC_INT_THERMAL_MANAGEMENT_INT , thermalManagementInt )

    ENABLE_MMU
    SAVE_REGS

    mr r3, sp
    bl thermalManagementIntHandler
    addi     sp, sp, 32
    b       archPpcIRet

DEF_EXC( EXC_INT_RESERVED_2F00 , reserved2F00 )
	rfi;

END_FUNC(_exc_int_start_)

_eabi:
	blr

END_SEC(".except")

