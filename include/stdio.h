/*
 *   File name: stdio.h
 *
 *  Created on: 2017年3月17日, 下午6:13:31
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#include "macros.h"
#include "stddef.h"
#include "stdarg.h"

#ifndef __INCLUDE_STDIO_H__
#define __INCLUDE_STDIO_H__

#if !defined(NULL)
#define NULL 0
#endif

struct __sbuf {
	unsigned char *_base;
	int	_size;
};

#if !defined(_ANSI_SOURCE) && !defined(__STRICT_ANSI__)
typedef off_t fpos_t;
typedef off64_t fpos64_t;
#else
typedef struct __sfpos {
	char	_pos[8];
} fpos_t;
typedef fpos_t fpos64_t;
#endif

#define	_FSTDIO	

typedef	struct __sFILE {
	unsigned char *_p;

	int _r;

	int _w;

	short _flags;

	short _file;

	struct __sbuf _bf;

	int _lbfsize;

	void *_cookie;

	int (*_close)(void *);
	int (*_read)(void *, char *, int);
	fpos_t (*_seek)(void *, fpos_t, int);
	int (*_write)(void *, const char *, int);

	struct __sbuf _ub;

	unsigned char *_up;

	int _ur;

	unsigned char _ubuf[3];

	unsigned char _nbuf[1];

	struct __sbuf _lb;

	int _blksize;

	fpos_t _offset;

} FILE;

#define	__SLBF	0x0001

#define	__SNBF	0x0002

#define	__SRD	0x0004

#define	__SWR	0x0008

#define	__SRW	0x0010

#define	__SEOF	0x0020

#define	__SERR	0x0040

#define	__SMBF	0x0080

#define	__SAPP	0x0100

#define	__SSTR	0x0200

#define	__SOPT	0x0400

#define	__SNPT	0x0800

#define	__SOFF	0x1000

#define	__SMOD	0x2000

#define	__SALC	0x4000

#define	_IOFBF	0

#define	_IOLBF	1

#define	_IONBF	2

#define	BUFSIZ	512

#define	EOF	    (-1)

#ifndef SEEK_SET
#define	SEEK_SET	0

#endif
#ifndef SEEK_CUR
#define	SEEK_CUR	1

#endif
#ifndef SEEK_END
#define	SEEK_END	2

#endif

#define	STDIN_FILENO	0
#define	STDOUT_FILENO	1
#define	STDERR_FILENO	2

		

#define	FOPEN_MAX	    200

#define	FILENAME_MAX	513

#define	P_tmpdir	"/tmp"
#define	L_tmpnam	513

#define	TMP_MAX		308915776

BEG_EXT_C

__DeclareFunc__(int, printk, (const char* RESTRICT fs, ...)) ATTRFORMATPRNT2(__printf__, 1, 2) ATTRNONULL(1);

__DeclareFunc__(int, printf, (const char* RESTRICT fs, ...)) ATTRFORMATPRNT2(__printf__, 1, 2) ATTRNONULL(1);

__DeclareFunc__(int, snprintf, (char * RESTRICT s, size_t n, const char * RESTRICT format, ...))
	ATTRFORMATPRNT2(__printf__, 3, 4) ATTRNONULL(3);

__DeclareFunc__(int, sprintf, (char * RESTRICT s, const char * RESTRICT, ...))
	ATTRFORMATPRNT2(__printf__, 2, 3) ATTRNONULL(2);

__DeclareFunc__(int, vfprintf, (FILE * RESTRICT, const char * RESTRICT, va_list))
	ATTRFORMATPRNT2(__printf__, 2, 0);

__DeclareFunc__(int, vsnprintf, (char * RESTRICT str, size_t n, const char * RESTRICT fmt, va_list ap))
		ATTRFORMATPRNT2(__printf__, 3, 0);

__DeclareFunc__(int, fflush, (FILE *));

__DeclareFunc__(int, fsync, (int));

__DeclareFunc__(int, rename, (const char *, const char *));

__DeclareFunc__(int, fprintf, (FILE *RESTRICT, const char *RESTRICT, ...))
ATTRFORMATPRNT2(__printf__, 2, 3);

__DeclareFunc__(FILE *, tmpfile, (void));

__DeclareFunc__(char *, tmpnam, (char * result));

__DeclareFunc__(char *, tempnam, (const char *, const char *));

__DeclareFunc__(int ,fclose, (FILE *));

__DeclareFunc__(FILE *,freopen, (const char *RESTRICT filename, const char *RESTRICT opentype, FILE *RESTRICT stream));

__DeclareFunc__(void,setbuf, (FILE *RESTRICT stream, char *RESTRICT buf));

__DeclareFunc__(int ,setvbuf, (FILE *RESTRICT, char *RESTRICT, int, size_t));

__DeclareFunc__(int ,fscanf, (FILE *RESTRICT stream, const char *RESTRICT fmt, ...))
ATTRFORMATPRNT2(__scanf__, 2, 3);

__DeclareFunc__(int ,scanf, (const char *RESTRICT fmt, ...))
ATTRFORMATPRNT2(__scanf__, 1, 2);

__DeclareFunc__(int , sscanf, (const char *RESTRICT, const char *RESTRICT, ...))
ATTRFORMATPRNT2(__scanf__, 2, 3);

__DeclareFunc__(int, ungetc, (int c, FILE *stream));

int	__srget (FILE *);
int	__svfscanf (FILE *, const char *, va_list);
int	__swbuf (int, FILE *);

#if defined(__GNUC__) && defined(__STDC__)

static inline int __sputc(int _c, FILE *_p) {
	if (--_p->_w >= 0 || (_p->_w >= _p->_lbfsize && (char)_c != '\n'))
		return (*_p->_p++ = _c);
	else
		return (__swbuf(_c, _p));
}

#else

#define	__sputc(c, p) \
	(--(p)->_w < 0 ? \
		(p)->_w >= (p)->_lbfsize ? \
			(*(p)->_p = (c)), *(p)->_p != '\n' ? \
				(int)*(p)->_p++ : \
				__swbuf('\n', p) : \
			__swbuf((int)(c), p) : \
		(*(p)->_p = (c), (int)*(p)->_p++))

#endif

__DeclareFunc__(FILE **, __stdin__,  (void));

__DeclareFunc__(FILE **, __stdout__, (void));

__DeclareFunc__(FILE **, __stderr__, (void));

#define stdin	(*__stdin__())
#define stdout	(*__stdout__())
#define stderr	(*__stderr__())

END_EXT_C

#define	__sfeof(p) ((int)(((p)->_flags & __SEOF) != 0))
#define	__sferror(p) ((int)(((p)->_flags & __SERR) != 0))
#define	__sclearerr(p) ((void)((p)->_flags &= ~(__SERR|__SEOF)))
#define	__sfileno(p) ((p)->_file)

#define	__sgetc(p) (--(p)->_r < 0 ? __srget(p) : (int)(*(p)->_p++))

#define	feof(p)		__sfeof(p)
#define	ferror(p)	__sferror(p)
#define	clearerr(p)	__sclearerr(p)

#endif

