/*
 *   File name: private.h
 *
 *  Created on: 2017年6月26日, 上午4:20:13
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __GENERIC_LIBC_STDIO_PRIVATE_H__
#define __GENERIC_LIBC_STDIO_PRIVATE_H__

BEG_EXT_C

__DeclareFunc__(int, __sflush,(FILE *));
__DeclareFunc__(FILE *,__sfp ,(void));
__DeclareFunc__(int,__srefill,(FILE *));
__DeclareFunc__(int,__sread, (void *, char *, int));
__DeclareFunc__(int,__swrite, (void *, char const *, int));
__DeclareFunc__(fpos_t, __sseek,(void *, fpos_t, int));
__DeclareFunc__(int, __sclose,(void *));
__DeclareFunc__(void, __sinit,(void));
__DeclareFunc__(void, cleanup,(void));
extern void (*__cleanup)(void);
__DeclareFunc__(void, __smakebuf,(FILE *));
__DeclareFunc__(int, __swhatbuf,(FILE *, size_t *, int *));
__DeclareFunc__(int, _fwalk,(int (*)(FILE *)));
__DeclareFunc__(int, __swsetup,(FILE *));
__DeclareFunc__(int, __sflags,(const char *, int *));

extern int	__stdIOIsInitilized__;

END_EXT_C

#define	cantwrite(fp) \
	((((fp)->_flags & __SWR) == 0 || (fp)->_bf._base == NULL) && \
	 __swsetup(fp))

#define	HASUB(fp) ((fp)->_ub._base != NULL)
#define	FREEUB(fp) { \
	if ((fp)->_ub._base != (fp)->_ubuf) \
		free((char *)(fp)->_ub._base); \
	(fp)->_ub._base = NULL; \
}

#define	HASLB(fp) ((fp)->_lb._base != NULL)
#define	FREELB(fp) { \
	free((char *)(fp)->_lb._base); \
	(fp)->_lb._base = NULL; \
}

#endif

