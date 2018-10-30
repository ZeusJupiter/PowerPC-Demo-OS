/*
 *   File name: timer.h
 *
 *  Created on: 2017年3月4日, 下午9:46:55
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __INCLUDE_TIMER_H__
#define __INCLUDE_TIMER_H__

#if defined(__cplusplus)
extern "C" {
#endif

extern int timer_create(clockid_t clock_id, sigevent_t *evp, timer_t *ptimer);
extern int timer_delete(timer_t timerid);
extern int timer_gettime(timer_t timerid, struct itimerspec *value);
extern int timer_settime(timer_t timerid, int flags,
		const struct itimerspec *value, struct itimerspec *ovalue);
extern int timer_getoverrun(timer_t timerid);
extern int timer_connect(timer_t timerid,
		void (*routine)(timer_t timerId, sint arg), sint arg);
extern int timer_cancel(timer_t timerid);

extern timer_t timer_open(const char * name, int mode, clockid_t clockId,
		sigevent_t * evp, void * context);
extern sint timer_close(timer_t timerId);
extern sint timer_unlink(const char * name);
extern void timerWdHandler(timer_t timerid);

#if defined(__cplusplus)
}
#endif

#endif

