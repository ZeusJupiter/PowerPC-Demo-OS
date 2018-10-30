/*
 *   File name: message.h
 *
 *  Created on: 2017年6月2日, 上午1:38:26
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __KERNEL_INCLUDE_MESSAGE_H__
#define __KERNEL_INCLUDE_MESSAGE_H__

template<uint msgSize>
struct TMessage
{
	uint _len;
	char _body[msgSize];
};

typedef TMessage<0> GMessage;

typedef GMessage*  GMessagePtr;

InlineStatic uint msgSize(const GMessagePtr gm)
{ return sizeof(gm->_len) + gm->_len; }

#endif

