/*
 *   File name: contextSwitch.S
 *
 *  Created on: 2017年4月14日, 下午11:18:26
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */
#ifndef ASSEMBLY
#define ASSEMBLY 1
#endif

#include <arch/powerpc/powerpccontext.h>

EXPORT_FUNC( contextSwitch )

BEG_SEC(".text", ARCH_TEXT_SEG_ALIGN )

BEG_FUNC( contextSwitch )
        saveContext

        lwz   r4,  0(r3)
        stw   sp,  0(r4)

        lwz   r4,  4(r3)
        stw   r4,  0(r3)
        lwz   sp,  0(r4)

        restoreContext
END_FUNC( contextSwitch )

END_SEC(".text")

