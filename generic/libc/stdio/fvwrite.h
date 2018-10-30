/*
 *   File name: fvwrite.h
 *
 *  Created on: 2017年6月26日, 上午3:28:26
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __GENERIC_LIBC_STDIO_FVWRITE_H__
#define __GENERIC_LIBC_STDIO_FVWRITE_H__

struct __siov {
	void	*iov_base;
	size_t	iov_len;
};
struct __suio {
	struct	__siov *uio_iov;
	int	uio_iovcnt;
	int	uio_resid;
};

extern int __sfvwrite(FILE *, struct __suio *);

#endif

