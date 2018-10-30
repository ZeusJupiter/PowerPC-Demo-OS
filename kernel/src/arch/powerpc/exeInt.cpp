/*
 *   File name: exeInt.c
 *
 *  Created on: 2016年11月15日, 下午10:01:57
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:

 */
#include <macros.h>
#include <types.h>
#include <stdio.h>

#include <interrupt.h>
#include <motherboard.h>

#include <arch/powerpc/context.h>
#include <arch/powerpc/mmu.h>
#include <arch/powerpc/msr.h>
#include <kernel.h>
#include <task/thread.h>

BEG_EXT_C

#define HandlerMsg(name) \
	do{ \
		printk("------Error!!!, " #name " is trigged-----\r\n"); \
		printContext(sp); \
		while(true); \
    }while(false)

void machineCheckIntHandler(Context* sp)
{
	printk("------Error!!!, Machine check Interrupt is trigged-----\r\n");
	printContext(sp);
	while(1);
}

void dataStorageIntHandler(word_t dar, word_t dsisr, Context* sp)
{
	printContext(sp);
	printk("----------In Data Storage Interrupt Handler-----------\r\n");
	printk("Effective Address=0x%x\r\n", dar);
	if(dsisr & (1 << 31))
	{
		printk("a load or store instruction results in a direct-store error interrupt\r\n");
	}
	else if(dsisr & (1 << 30))
	{
		printk("the translation of an attempted access is not found in the primary "
				"or secondary hash table entry group (HTEG), "
				"or in the range of a DBAT register (page fault condition);\r\n");

		printk("dsisr=0x%x\r\n", dsisr);
		phtab.showHashTable();
		PMMU::SR sr;
		sr.word = PMMU::getSR(dar);
		printk("sr=0x%x, vsid=0x%x \r\n", sr.word, sr.field.vsid);
	}
	else if(dsisr & (1 << 27))
	{
		printk("Page protection violation\r\n");
	}
	else
	{
		printk("dcbt/dcbtst Instruction\r\n");
	}

	printk("--------------------------------\r\n");
	while(1);
}

void intructionStorageIntHandler(word_t pc, word_t msr)
{
	printk("----------In Instruction Storage Interrupt Handler-----------\r\n");
	printk("current pc=0x%x, msr=0x%x\r\n", pc, msr);

	if(msr & (1 << 30))
	{
		printk("Set if the translation of an attempted access is not found in the primary "
				"or secondary hash table entry group (HTEG) or "
				"in the range of an IBAT register (page fault condition);\r\n");
	}
	else if(msr & (1 << 27))
	{
		printk("Page protection violation\r\n");
	}
	else if(msr & (1 << 28))
	{
		printk("the segment is designated as no-execute\r\n");
	}
	else
	{
		printk("msr=0x%x ", msr);
		printk("dcbt/dcbtst Instruction\r\n");
	}

	while(1);
}

void decrementIntHandler(Context* sp)
{
	Kernel::CommonKernel::handleSysTickInterrupt();
}

void systemCallIntHandler(Context* sp)
{
	HandlerMsg(systemCallIntHandler);
}

void externalIntHandler()
{
	register sint irqNo = HMB::peripheralIrqNo();

	if(irqNo < 0) return;

	KInterrupt::solveInterurpt(irqNo);
	HMB::peripheralAckIrq(irqNo);
}

void criticalIntHandler(Context* sp)
{
	HandlerMsg(criticalIntHandler);
}

void alignmentIntHandler(word_t dar, word_t dsisr, Context* sp)
{
	printk("Data Address Register=0x%x\r\n", dar);
	printk("DSISR=0x%x\r\n", dsisr);
	HandlerMsg(alignmentIntHandler);
}

void programIntHandler(Context* sp)
{
	HandlerMsg(programIntHandler);
}

void fpuUnaviableIntHandler(Context* sp)
{
	HandlerMsg(fpuUnaviableIntHandler);
}

void traceIntHandler(Context* sp)
{
	HandlerMsg(traceIntHandler);
}

void fpuAssistIntHandler(Context* sp)
{
	HandlerMsg(fpuAssistIntHandler);
}

void preformanceMonitorIntHandler(Context* sp)
{
	HandlerMsg(preformanceMonitorIntHandler);
}

void intructionTranslationMissIntHandler(Context* sp)
{
	HandlerMsg(intructionTranslationMissIntHandler);
}

void dataLoadTranslationMissIntHandler(Context* sp)
{
	HandlerMsg(dataLoadTranslationMissIntHandler);
}

void dataStoreTranslationMissIntHandler(Context* sp)
{
	HandlerMsg(dataStoreTranslationMissIntHandler);
}

void instructionAddressBreakpointIntHandler(Context* sp)
{
	HandlerMsg(instructionAddressBreakpointIntHandler);
}

void systemManagementIntHandler(Context* sp)
{
	HandlerMsg(systemManagementIntHandler);
}

void thermalManagementIntHandler(Context* sp)
{
	HandlerMsg(thermalManagementIntHandler);
}

END_EXT_C
