/*
 *   File name: _fp_const.h
 *
 *  Created on: 2016年11月23日, 下午10:47:50
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __FP_CONST_H__
#define __FP_CONST_H__

#define PPC_FPSCR_FX           0x80000000
#define PPC_FPSCR_FEX          0x40000000
#define PPC_FPSCR_VX           0x20000000
#define PPC_FPSCR_OX           0x10000000
#define PPC_FPSCR_UX           0x08000000
#define PPC_FPSCR_ZX           0x04000000
#define PPC_FPSCR_XX           0x02000000
#define PPC_FPSCR_VXSNAN       0x01000000
#define PPC_FPSCR_VXISI        0x00800000
#define PPC_FPSCR_VXIDI        0x00400000
#define PPC_FPSCR_VXZDZ        0x00200000
#define PPC_FPSCR_VXIMZ        0x00100000
#define PPC_FPSCR_VXVC         0x00080000
#define PPC_FPSCR_FR           0x00040000
#define PPC_FPSCR_FI           0x00020000
#define PPC_FPSCR_FPRF         0x0001F000
#define PPC_FPSCR_VXSOFT       0x00000400
#define PPC_FPSCR_VXSQRT       0x00000200
#define PPC_FPSCR_VXCVI        0x00000100
#define PPC_FPSCR_VE           0x00000080
#define PPC_FPSCR_OE           0x00000040
#define PPC_FPSCR_UE           0x00000020
#define PPC_FPSCR_ZE           0x00000010
#define PPC_FPSCR_XE           0x00000008
#define PPC_FPSCR_NI           0x00000004
#define PPC_FPSCR_RN(n)        (n)
#define PPC_FPSCR_RN_MASK      0x00000003
#define PPC_FPSCR_EXC_MASK     0x1ff80700
#define PPC_FPSCR_CTRL_MASK    0x000000ff

#define PPC_FPSCR_INIT         (ARCH_PPC_FPSCR_OE | ARCH_PPC_FPSCR_UE | ARCH_PPC_FPSCR_ZE \
                                     | ARCH_PPC_FPSCR_RN(0))

#endif

