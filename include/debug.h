/*
 *   File name: kdebug.h
 *
 *  Created on: 2016年11月23日, 下午4:09:36
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __INCLUDE_DEBUG_H__
#define __INCLUDE_DEBUG_H__

#if defined(_DEBUG) || defined(_RELEASE_WITH_MSG)
	#include <stdio.h>
    #define kdebug(msg)  printk(msg)
	#define kformat(fmt, msgs...) printk(fmt, msgs)
	#define kformatinfo(fmt, msgs...)  printk("File=%s, Line=%d -- " fmt, __FILE__, __LINE__, msgs)

	#define kdebugln(msg)  printk(msg "\r\n")
	#define kformatln(fmt, msgs...) printk(fmt "\r\n", msgs)
	#define kformatinfoln(fmt, msgs...)  printk("File=%s, Line=%d -- " fmt "\r\n", __FILE__, __LINE__, msgs)
#else

#define kdebug(msg)             do{}while(0)
#define kformat(fmt, ...)       do{}while(0)
#define kformatinfo(fmt, ...)   do{}while(0)

#define kdebugln(msg)           do{}while(false)
#define kformatln(fmt, msgs...) do{}while(0)
#define kformatinfoln(fmt, ...)   do{}while(0)

#endif

#endif

