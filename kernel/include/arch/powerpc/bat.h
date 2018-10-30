/*
 *   File name: bat.h
 *
 *  Created on: 2016年11月23日, 下午10:45:23
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __POWERPC_BAT_H__
#define __POWERPC_BAT_H__

#define PPC_BAT_BLOCK_MASK       0xFFFE0000

#define PPC_UPPER_BAT_BEPI_MASK  PPC_BAT_BLOCK_MASK
#define PPC_UPPER_BAT_BL_MASK    0x00001FFC
#define PPC_UPPER_BAT_BL_128K    0x00000000
#define PPC_UPPER_BAT_BL_256K    0x00000004
#define PPC_UPPER_BAT_BL_512K    0x0000000C
#define PPC_UPPER_BAT_BL_1M      0x0000001C
#define PPC_UPPER_BAT_BL_2M      0x0000003C
#define PPC_UPPER_BAT_BL_4M      0x0000007C
#define PPC_UPPER_BAT_BL_8M      0x000000FC
#define PPC_UPPER_BAT_BL_16M     0x000001FC
#define PPC_UPPER_BAT_BL_32M     0x000003FC
#define PPC_UPPER_BAT_BL_64M     0x000007FC
#define PPC_UPPER_BAT_BL_128M    0x00000FFC
#define PPC_UPPER_BAT_BL_256M    0x00001FFC

#define PPC_UPPER_BAT_VALID_SUPERVISOR    (1 << 1)

#define PPC_UPPER_BAT_VALID_USER          0x00000001

#define PPC_UPPER_BAT_INVALID             0x00000000

#define PPC_LOWER_BAT_BRPN_MASK  PPC_BAT_BLOCK_MASK

#define PPC_LOWER_BAT_WIMG_MASK  0x00000078

#define PPC_LOWER_BAT_WRITE_THROUGH      0x00000040

#define PPC_LOWER_BAT_CACHAE_INHIBITED   0x00000020

#define PPC_LOWER_BAT_MEMEMORY_COHERENCE 0x00000010

#define PPC_LOWER_BAT_GUARDED            0x00000008

#define PPC_LOWER_BAT_PP_MASK            0x00000003
#define PPC_LOWER_BAT_PP_NO_ACCESS       0x00000000
#define PPC_LOWER_BAT_PP_READ_ONLY       0x00000001
#define PPC_LOWER_BAT_PP_READ_WRITE      0x00000002

#if !defined( ASSEMBLY )

#ifdef __cplusplus
extern "C"{
#endif

void initBAT(void);

#ifdef __cplusplus
}
#endif

#endif

#endif

