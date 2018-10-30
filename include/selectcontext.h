/*
 *   File name: selectcontext.h
 *
 *  Created on: 2017年5月17日, 下午10:12:58
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __INCLUDE_SELECTCONTEXT_H__
#define __INCLUDE_SELECTCONTEXT_H__

#include <semaphore.h>
#include <fdset.h>

struct SelectContext
{
    Semaphore	_wakeupSem;
    bool     	_pendedOnSelect;
    FdSet     *_readFds;
    FdSet     *_writeFds;
    FdSet     *_excFds;
    FdSet  *_origReadFds;
    FdSet  *_origWriteFds;
    FdSet  *_origExcFds;
    sint	_maxFd;		
    sint	_width;		
};

#endif

