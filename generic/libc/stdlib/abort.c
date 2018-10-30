/*
 *   File name: abort.c
 *
 *  Created on: 2017年6月26日, 上午5:54:15
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */
#include <macros.h>
#include <types.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

void abort(void)
{
    raise(SIGABRT);
    _exit(EXIT_FAILURE);
}

