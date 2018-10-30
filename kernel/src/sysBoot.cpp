/*
 *   File name: sysBoot.c
 *
 *  Created on: 2016年11月13日, 下午9:24:34
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:

 */
#include <macros.h>
#include <types.h>
#include <ctors.h>
#include <debug.h>
#include <string.h>

#include <mm/heap.h>
#include <mm/vmm.h>
#include <kernel.h>
#include <listenermanager.h>
#include <interrupt.h>
#include <kernel.h>
#include <task/procmanager.h>
#include <task/threadmanager.h>

#include <cpu.h>
#include <drivermanager.h>

#include <vfs/vfs.h>
#include <motherboard.h>

EXTERN_C void sysBoot(void)
{
	connectObjects();

	HMB::motherBoardInit();

	kdebugln("Mother Board has been initialized");

	CPU::init();
	CPU::reportSelf();

	FS::VFS::init();
	KProcMng::init();
	kthreadMng.init();
	KCommKernel::init();

	Driver::DriverManager::init();

	HMB::createAllVirtualDev();

	KCommKernel::curSheduler()->start();

}

