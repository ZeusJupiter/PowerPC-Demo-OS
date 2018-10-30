/*
 *   File name: error.cpp
 *
 *  Created on: 2017年6月25日, 下午8:15:21
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#include <macros.h>
#include <types.h>
#include <errno.h>

#include <kernel.h>
#include <task/schedule.h>

BEG_EXT_C

errno_t *__errno (void)
{
    errno_t *errPtr;

    return  (errPtr);
}

void __setError(ErrNo err)
{
    if (err == ErrNo::ENONE) {
        return;
    }

    u32          msr;
    KScheduler*   scheduler;

    msr = KCommKernel::closeInterrupt();

    scheduler = KCommKernel::curSheduler();
    if (scheduler->isNesting()) {
        scheduler->setErrorInNesting(err);

    } else {
    	KThread* thread = scheduler->getCurrentThread();
        if (thread) {
            thread->setLastError(err);
        } else {
            KCommKernel::setErrorWhenNotRun(err);
        }
    }

    KCommKernel::restoreInterrupt(msr);
}

END_EXT_C

