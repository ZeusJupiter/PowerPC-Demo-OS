/*
 *   File name: listener.h
 *
 *  Created on: 2017年5月5日, 下午5:12:03
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __INCLUDE_LISTENER_H__
#define __INCLUDE_LISTENER_H__

namespace Kernel
{
	class ListenerManagerImp;
	class ListenerManager;
}

class Listener
{
	friend class Kernel::ListenerManagerImp;
	friend class Kernel::ListenerManager;
public:

private:
	Listener(const Listener&) = delete;
	Listener operator=(const Listener&) = delete;
	Listener(void){}

	u32 _tid;
	bool _block;
	Listener * _next;
	time_t _time;
};

#endif

