/*
 *   File name: bat.S
 *
 *  Created on: 2016年11月14日, 下午3:24:17
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:

 */

#ifndef ASSEMBLY
#define ASSEMBLY 1
#endif

#include <asm.h>
#include <arch/powerpc/bat.h>
#include <arch/powerpc/batCfg.h>

EXPORT_FUNC( initBAT )

BEG_SEC(".text", ARCH_TEXT_SEG_ALIGN )

BEG_FUNC( initBAT )

    LD_VAL( p0 , IBAT0L_VAL )
    LD_VAL( p1 , IBAT0U_VAL )
    LD_VAL( p2 , IBAT1L_VAL )
    LD_VAL( p3 , IBAT1U_VAL )
    LD_VAL( p4 , IBAT2L_VAL )
    LD_VAL( p5 , IBAT2U_VAL )
    LD_VAL( p6 , IBAT3L_VAL )
    LD_VAL( p7 , IBAT3U_VAL )

    isync
    mtspr ibat0l , p0 ;
    mtspr ibat0u , p1 ;
    mtspr ibat1l , p2 ;
    mtspr ibat1u , p3 ;
    mtspr ibat2l , p4 ;
    mtspr ibat2u , p5 ;
    mtspr ibat3l , p6 ;
    mtspr ibat3u , p7 ;
    isync

    LD_VAL( p0 , DBAT0L_VAL )
    LD_VAL( p1 , DBAT0U_VAL )
    LD_VAL( p2 , DBAT1L_VAL )
    LD_VAL( p3 , DBAT1U_VAL )
    LD_VAL( p4 , DBAT2L_VAL )
    LD_VAL( p5 , DBAT2U_VAL )
    LD_VAL( p6 , DBAT3L_VAL )
    LD_VAL( p7 , DBAT3U_VAL )

    isync
    mtspr dbat0l , p0 ;
    mtspr dbat0u , p1 ;
    mtspr dbat1l , p2 ;
    mtspr dbat1u , p3 ;
    mtspr dbat2l , p4 ;
    mtspr dbat2u , p5 ;
    mtspr dbat3l , p6 ;
    mtspr dbat3u , p7 ;
    isync

    blr
 END_FUNC( initBAT )

END_SEC(".text")
