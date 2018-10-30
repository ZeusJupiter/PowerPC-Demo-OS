/*
 *   File name: sigaction.cpp
 *
 *  Created on: 2017年6月25日, 下午7:53:45
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */
#include <macros.h>
#include <types.h>
#include <debug.h>
#include <assert.h>
#include <errno.h>

#include <signal.h>

#include <task/thread.h>

BEG_EXT_C
sint sigaction(sint sigNo, const sigaction_t * newSigAction, sigaction_t * oldSigAction)
{}

END_EXT_C

