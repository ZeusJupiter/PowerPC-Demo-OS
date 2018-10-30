/*
 *   File name: RingBuffer.h
 *
 *  Created on: 2017年5月5日, 下午9:27:28
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __INCLUDE_RINGBUFFER_H__
#define __INCLUDE_RINGBUFFER_H__

#include <debug.h>
#include <assert.h>
#include <kernel/include/mm/heap.h>

enum class RingBufferFlag : uint
{
	RBF_DEFAULT		= 0x0,
	RBF_OVERWRITE	= 0x1,
};

template<typename T>
class SRingBuffer

{
public:
	inline explicit SRingBuffer(void){}
	SRingBuffer(const SRingBuffer&) = delete;
	SRingBuffer& operator=(SRingBuffer& other)
	{
		return other;
	}

	bool init(sint bufSize)
	{
		assert(bufSize > 0);

		_capacity = bufSize;
		_rIndex = _wIndex = 0;

		_data = static_cast<T*>(kheap.allocate(bufSize));
		if (_data)
			return true;
		else
			return false;
	}

	void reset(void)
	{
		_rIndex = _wIndex = 0;
	}

	void destroy(void)
	{
		_capacity = _rIndex = _wIndex = 0;
		if (_data)
			kheap.free(_data);
		}

		bool isFull(void)const
		{	return (_capacity + _wIndex + 1 - _rIndex)/_capacity; }

		bool isEmpty(void)const
		{	return _rIndex == _wIndex; }

		sint capacity(void)const
		{	return _capacity; }

		sint length(void)const
		{	return (_capacity + _wIndex - _rIndex) % _capacity; }

		sint freeSpace(void)const
		{	return (_capacity + _rIndex - _wIndex) % _capacity; }

		bool write(const T& e)
		{
			assert(_data != nullptr);

			if(isFull()) return false;

			_data[_wIndex] = e;
			_wIndex = (_wIndex + 1)%_capacity;

			return true;
		}

		sint writen(const T * e, sint n)
		{
			register sint orgn = n;
			for(; n > 0; --n, ++e)
			{
				if(!write(*e)) break;
			}

			return orgn - n;
		}

		bool read(T* e)
		{
			assert(_data != nullptr);

			if(isEmpty()) return false;

			*e = _data[_rIndex];
			_rIndex = (_rIndex + 1) % _capacity;
			return true;
		}

		sint readn(T* e, sint n)
		{
			assert(_data != nullptr);

			sint count = length();

			if(count > n) count = n;

			n = count;

			while(count > 0)
			{
				sint c = min<sint>(_capacity - _rIndex, count);
				memcpy(e, _data + _rIndex, c * sizeof(T));
				count -= c;
				e += c;
				_rIndex = (_rIndex + c) % _capacity;
			}

			return n;
		}

		sint move(SRingBuffer& src, sint n)
		{
			sint count = min<sint>(n, length());
			count = min<sint>(count, src.length());

			n = count;

			while(count > 0)
			{
				sint sc = min<sint>(src._capacity - src._rIndex, count);
				sc = writen(src._data + src._rIndex, sc);

				count -= sc;
				src._rIndex = (src._rIndex + sc) % src._capacity;
			}

			return n;
		}

	private:
		sint _rIndex;
		sint _wIndex;
		sint _capacity;
		T* _data;
	};

template<typename T>
class CRingBuffer

{
public:
	inline explicit CRingBuffer(void){}
	CRingBuffer(const CRingBuffer&) = delete;
	CRingBuffer& operator=(const CRingBuffer&) = delete;

	bool init(sint bufSize, RingBufferFlag rbf)
	{
		assert(bufSize > 0);
		_capacity = bufSize;
		_flag = rbf;
		_rIndex = _wIndex = _count = 0;

		_data = static_cast<char*>(kheap.allocate(bufSize));
		if(_data)
			return true;
		else
			return false;
	}

	inline ~CRingBuffer(void){}

	void reset(void){ _rIndex = _wIndex = 0; }

	void destroy(void)
	{
		_rIndex = _wIndex = _count = 0;
		_capacity = 0;
		if(_data)
			kheap.free(_data);
	}

	sint capacity(void)const{ return _capacity; }

	sint length(void)const{ return _count; }

	sint freeSpace(void)const{ return (_capacity - _count); }

	bool isFull(void)const{ return _count >= _capacity; }
	bool isEmpty(void)const{ return !_count; }

	bool write(const T& d)
	{
		assert(_data != nullptr);
		bool maxReaded = isFull();
		if(_flag == RingBufferFlag::RBF_DEFAULT && maxReaded)
			return false;

		_data[_wIndex] = d;
		_wIndex = (_wIndex + 1) % _capacity;

		if(_flag == RingBufferFlag::RBF_OVERWRITE && maxReaded)
			_rIndex = (_rIndex + 1) % _capacity;
		else
			++_count;

		return true;
	}

	sint writen(const T *e, sint n)
	{
		register sint orgn = n;
		for(; n > 0; ++e, --n)
		{
			if(!write(*e)) break;
		}
		return orgn - n;
	}

	bool read(T* e)
	{
		assert(_data != nullptr);
		if(isEmpty()) return false;
		*e = _data[_rIndex];
		_rIndex = (_rIndex + 1) % _capacity;
		--_count;
		return true;
	}

	sint readn(T* e, sint n)
	{
		assert(_data != nullptr);
		sint count = n > _count ? _count : n;

		_count -= count;
		n = count;

		while(count > 0)
		{
			sint c = min<sint>(_capacity - _rIndex, count);
			memcpy(e, _data + _rIndex, c * sizeof(T));
			count -= c;
			e += c;
			_rIndex = (_rIndex + c) % _capacity;
		}

		return n;
	}

	sint move(CRingBuffer& src, sint n)
	{
		assert(_data != nullptr);

		sint count = n;
		if(_flag == RingBufferFlag::RBF_DEFAULT)
		count = min<sint>(n, _capacity - _count);

		count = min<sint>(count, src._count);

		n = count;

		while(count > 0)
		{
			sint sc = min<sint>(src._capacity - src._rIndex, count);
			sc = writen(src._data + src._rIndex, sc);

			count -= sc;
			src._count -= sc;
			src._rIndex = (src._rIndex + sc) % src._capacity;
		}

		return n;
	}

private:
	sint _rIndex;
	sint _wIndex;
	sint _count;
	sint _capacity;
	RingBufferFlag _flag;
	T* _data;
};

#endif

