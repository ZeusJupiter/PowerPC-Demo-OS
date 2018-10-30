/*
 *   File name: port.h
 *
 *  Created on: 2017年6月24日, 上午1:12:35
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#ifndef __INCLUDE_LINUX_PORT_H__
#define __INCLUDE_LINUX_PORT_H__

typedef uint16_t  __le16;
typedef uint16_t  __be16;
typedef uint32_t  __le32;
typedef uint32_t  __be32;
typedef uint64_t  __le64;
typedef uint64_t  __be64;

#ifndef __user
#define __user
#endif

#ifndef __iomem
#define __iomem
#endif

#ifndef __init
#define __init
#endif

#ifndef __exit
#define __exit
#endif

#ifndef __initdata
#define __initdata
#endif

#define min_t(type,x,y) \
	({ type __x = (x); type __y = (y); __x < __y ? __x: __y; })
#define max_t(type,x,y) \
	({ type __x = (x); type __y = (y); __x > __y ? __x: __y; })

BEG_EXT_C
extern char *skip_spaces(const char *str);
extern char *strim(char *s);
END_EXT_C

#endif

