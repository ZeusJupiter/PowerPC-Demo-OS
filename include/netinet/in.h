/*
 *   File name: in.h
 *
 *  Created on: 2017年7月26日, 下午3:51:27
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#include <endian.h>

#ifndef __INCLUDE_NETINET_IN_H__
#define __INCLUDE_NETINET_IN_H__

#if defined(__cplusplus)
extern "C"{
#endif

InlineStatic uint32_t htonl(uint32_t x)
{
	return htobe32(x);
}

InlineStatic uint16_t htons(uint16_t x)
{
	return htobe16(x);
}

InlineStatic uint32_t ntohl(uint32_t x)
{
	return be32toh(x);
}

InlineStatic uint16_t ntohs(uint16_t x)
{
	return be16toh(x);
}

#if defined(__cplusplus)
}
#endif

#endif

