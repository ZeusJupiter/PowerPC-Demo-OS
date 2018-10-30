/*
 *   File name: inc.c
 *
 *  Created on: 2017年7月13日, 上午7:31:03
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */
#include <macros.h>
#include <types.h>
#include <stdio.h>

#include "../private.h"

int __stdIOIsInitilized__;

void _cleanup()
{

	(void) _fwalk(__sflush);
}

void __sinit(void)
{

	__stdIOIsInitilized__ = 1;
}

