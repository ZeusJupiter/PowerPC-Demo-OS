/*
 *   File name: div64.S
 *
 *  Created on: 2017年7月29日, 下午2:52:35
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#include <asm.h>

EXPORT_FUNC( __lshrdi3 )

BEG_SEC(".text", ARCH_TEXT_SEG_ALIGN )

BEG_FUNC(__lshrdi3)
    subfic  r6,r5,32
    srw r4,r4,r5
    addi    r7,r5,32
    slw r6,r3,r6
    srw r7,r3,r7
    or  r4,r4,r6
    srw r3,r3,r5
    or  r4,r4,r7
    blr
END_FUNC(__lshrdi3)

END_SEC(".text")
