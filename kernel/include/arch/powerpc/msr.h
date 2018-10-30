/*
 *   File name: _msr_const.h
 *
 *  Created on: 2016年11月23日, 下午10:49:11
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __POWERPC_MSR_H__
#define __POWERPC_MSR_H__

#define PPC_MSR_POW_BIT       13

#define PPC_MSR_TGPR_BIT      14

#define PPC_MSR_ILE_BIT       15

#define PPC_MSR_EE_BIT        16

#define PPC_MSR_PR_BIT        17

#define PPC_MSR_FP_BIT        18

#define PPC_MSR_ME_BIT        19

#define PPC_MSR_FE0_BIT       20

#define PPC_MSR_SE_BIT        21

#define PPC_MSR_BE_BIT        22

#define PPC_MSR_FE1_BIT       23

#define PPC_MSR_IP_BIT        25

#define PPC_MSR_IR_BIT        26

#define PPC_MSR_DR_BIT        27

#define PPC_MSR_PMM_BIT       29

#define PPC_MSR_RI_BIT        30

#define PPC_MSR_PM_BIT        31

#define PPC_MSR_EE_ENABLED  1
#define PPC_MSR_EE_DISABLED 0

#define PPC_MSR_PR_KERNEL 0
#define PPC_MSR_PR_USER   1

#define PPC_MSR_FP_ENABLED  1
#define PPC_MSR_FP_DISABLED 0

#define PPC_MSR_ME_ENABLED  1
#define PPC_MSR_ME_DISABLED 0

#define PPC_MSR_SE_ENABLED	1
#define PPC_MSR_SE_DISABLED	0

#define PPC_MSR_BE_ENABLED	1
#define PPC_MSR_BE_DISABLED	0

#define PPC_MSR_IP_RAM    0
#define PPC_MSR_IP_ROM    1

#define PPC_MSR_IR_ENABLED  1
#define PPC_MSR_IR_DISABLED 0

#define PPC_MSR_DR_ENABLED  1
#define PPC_MSR_DR_DISABLED 0

#define _LEFT_SHIFT(x)       (31 - (x))

#define PPC_MSR_PR  ( 1 << _LEFT_SHIFT(PPC_MSR_PR_BIT) )
#define PPC_MSR_EE  ( 1 << _LEFT_SHIFT(PPC_MSR_EE_BIT) )

#define _MSR_TRANS  ( ( PPC_MSR_IR_ENABLED << _LEFT_SHIFT(PPC_MSR_IR_BIT) ) | \
		              ( PPC_MSR_DR_ENABLED << _LEFT_SHIFT(PPC_MSR_DR_BIT) ) | \
				      ( PPC_MSR_IP_RAM << _LEFT_SHIFT(PPC_MSR_IP_BIT) ) \
				    )

#define PPC_KERNEL_MSR  ( ( PPC_MSR_PR_KERNEL << _LEFT_SHIFT(PPC_MSR_PR_BIT) ) | \
		                  ( PPC_MSR_FP_DISABLED << _LEFT_SHIFT(PPC_MSR_FP_BIT) ) | \
						  ( PPC_MSR_ME_ENABLED << _LEFT_SHIFT(PPC_MSR_ME_BIT) ) | \
						  _MSR_TRANS \
                        )

#define PPC_EXECPTION_MSR ( ( PPC_MSR_PR_KERNEL << _LEFT_SHIFT(PPC_MSR_PR_BIT) ) | \
							( PPC_MSR_FP_DISABLED << _LEFT_SHIFT(PPC_MSR_FP_BIT) ) | \
							( PPC_MSR_ME_ENABLED << _LEFT_SHIFT(PPC_MSR_ME_BIT) ) | \
							_MSR_TRANS \
		                  )

#define PPC_USER_MSR   ( ( PPC_MSR_EE_ENABLED << _LEFT_SHIFT(PPC_MSR_EE_BIT) ) | \
						( PPC_MSR_PR_USER << _LEFT_SHIFT(PPC_MSR_PR_BIT) ) | \
						( PPC_MSR_FP_ENABLED << _LEFT_SHIFT(PPC_MSR_FP_BIT) ) | \
						( PPC_MSR_ME_ENABLED << _LEFT_SHIFT(PPC_MSR_ME_BIT) ) | \
						_MSR_TRANS \
                       )

#define PPC_REAL_MSR   ( ( PPC_MSR_PR_KERNEL << _LEFT_SHIFT(PPC_MSR_PR_BIT) ) | \
		                 ( PPC_MSR_FP_DISABLED << _LEFT_SHIFT(PPC_MSR_FP_BIT) ) | \
					     ( PPC_MSR_IP_RAM << _LEFT_SHIFT(PPC_MSR_IP_BIT) ) \
		               )

#if !defined( ASSEMBLY )

#define MSR_BIT(msr, bit)  ( ( (msr) >> _LEFT_SHIFT(bit) ) & 1)
#define MSR_SET(msr, bit)  (msr) |= (1 << _LEFT_SHIFT(bit) )
#define MSR_CLR(msr, bit)  ( (msr) & ~( 1 << _LEFT_SHIFT(bit) ) )

#ifdef __cplusplus
extern "C"{
#endif

InlineStatic u32 getMSR(void)
{
	u32 val;
	asm volatile("mfmsr %0 \n" : "=r"(val));
	return val;
}

InlineStatic void setMSR(u32 val)
{
	asm volatile("isync \n" \
			      "mtmsr %0 \n" \
			      "isync \n" \
				  : \
				  : "r"(val));
}

#ifdef __cplusplus
}
#endif

namespace Powerpc
{
	namespace MMU
	{
		InlineStatic void enableInstructionTrans(void)
		{
			u32 msr = getMSR();
			MSR_SET(msr, PPC_MSR_IR_BIT);
			setMSR(msr);
		}

		InlineStatic void disableInstructionTrans(void)
		{
			u32 msr = getMSR();
			MSR_CLR(msr, PPC_MSR_IR_BIT);
			setMSR(msr);
		}

		InlineStatic void enableDataTrans(void)
		{
			u32 msr = getMSR();
			MSR_SET(msr, PPC_MSR_DR_BIT);
			setMSR(msr);
		}

		InlineStatic void disableDataTrans(void)
		{
			u32 msr = getMSR();
			MSR_CLR(msr, PPC_MSR_DR_BIT);
			setMSR(msr);
		}

		InlineStatic void enableMMU(void)
		{
			u32 msr = getMSR();
			const u32 val = (1 << _LEFT_SHIFT(PPC_MSR_IR_BIT) ) | (1 << _LEFT_SHIFT(PPC_MSR_DR_BIT));
			msr |= val;
			setMSR(msr);
		}

		InlineStatic void disableMMU(void)
		{
			u32 msr = getMSR();
			msr &= ~( ( 1 << _LEFT_SHIFT(PPC_MSR_IR_BIT)) | (1 << _LEFT_SHIFT(PPC_MSR_DR_BIT)) );
			setMSR(msr);
		}

	}

	namespace Interrupt
	{
		InlineStatic void enableInterrupts(void)
		{
			u32 msr = getMSR();
			msr |= PPC_MSR_EE;
			setMSR(msr);
		}

		InlineStatic void disableInterrupts(void)
		{
			u32 msr = getMSR();
			msr = MSR_CLR(msr, PPC_MSR_EE_BIT);
			setMSR(msr);
		}

		InlineStatic u32 closeInterrupt(void)
		{
			u32 msr = getMSR();
			setMSR( MSR_CLR(msr, PPC_MSR_EE_BIT) );
			return msr;
		}

		InlineStatic void restoreInterrupt(u32 msr)
		{
			if( !(msr & PPC_MSR_EE) )

				return;
			msr = getMSR();
			msr |= PPC_MSR_EE;
			setMSR(msr);
		}
	}

}
#endif

#endif

