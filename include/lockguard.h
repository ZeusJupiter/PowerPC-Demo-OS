/*
 *   File name: lockguard.h
 *
 *  Created on: 2017年4月10日, 上午1:57:59
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __INCLUDE_LOCKGUARD_H__
#define __INCLUDE_LOCKGUARD_H__

template <typename T>
class LockGuard
{
    private:
        T &_lock;
        LockGuard(const LockGuard&) = delete;
        LockGuard& operator=(const LockGuard&) = delete;
    public:
        inline explicit LockGuard (T &l) : _lock (l)
        {
            _lock.lock();
        }

        inline ~LockGuard()
        {
            _lock.unlock();
        }
};

#endif

