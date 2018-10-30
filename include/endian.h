/*
 *   File name: endian.h
 *
 *  Created on: 2017年7月26日, 下午3:45:36
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#include <platform/cpucfg.h>

#ifndef __INCLUDE_ENDIAN_H__
#define __INCLUDE_ENDIAN_H__

InlineStatic uint16_t bswap16(uint16_t x)
{
    return  (uint16_t)((((x) & 0x00ff) << 8) |
                      (((x) & 0xff00) >> 8));
}

InlineStatic uint32_t bswap32(uint32_t x)
{
    return  (uint32_t)((((x) & 0x000000ff) << 24) |
                       (((x) & 0x0000ff00) <<  8) |
                       (((x) & 0x00ff0000) >>  8) |
                       (((x) & 0xff000000) >> 24));
}

InlineStatic uint64_t bswap64(uint64_t x)
{
    uint32_t tl, th;
    th = bswap32((uint32_t)(x & 0x00000000ffffffffULL));
	tl = bswap32((uint32_t)((x >> 32) & 0x00000000ffffffffULL));
    return ((uint64_t)th << 32) | tl;
}

#define htobe16(x)	(x)
#define htobe32(x)	(x)
#define htobe64(x)	(x)
#define htole16(x)	bswap16(x)
#define htole32(x)	bswap32(x)
#define htole64(x)	bswap64(x)

#define be16toh(x)	(x)
#define be32toh(x)	(x)
#define be64toh(x)	(x)
#define le16toh(x)	bswap16(x)
#define le32toh(x)	bswap32(x)
#define le64toh(x)	bswap64(x)

#define HTOBE16(x)	htobe16(x)
#define HTOBE32(x)	htobe32(x)
#define HTOBE64(x)	htobe64(x)
#define HTOLE16(x)	htole16(x)
#define HTOLE32(x)	htole32(x)
#define HTOLE64(x)	htole64(x)

#define BE16TOH(x)	be16toh(x)
#define BE32TOH(x)	be32toh(x)
#define BE64TOH(x)	be64toh(x)
#define LE16TOH(x)	le16toh(x)
#define LE32TOH(x)	le32toh(x)
#define LE64TOH(x)	le64toh(x)

InlineStatic void UNUSED be16enc(void *buf, uint16_t u)
{
	uint8_t *p = (uint8_t *)buf;

	p[0] = (uint8_t)(((unsigned)u >> 8) & 0xff);
	p[1] = (uint8_t)(u & 0xff);
}

InlineStatic void UNUSED le16enc(void *buf, uint16_t u)
{
	uint8_t *p = (uint8_t *)buf;

	p[0] = (uint8_t)(u & 0xff);
	p[1] = (uint8_t)(((unsigned)u >> 8) & 0xff);
}

InlineStatic uint16_t UNUSED be16dec(const void *buf)
{
	const uint8_t *p = (const uint8_t *)buf;

	return (uint16_t)((p[0] << 8) | p[1]);
}

InlineStatic uint16_t UNUSED le16dec(const void *buf)
{
	const uint8_t *p = (const uint8_t *)buf;

	return (uint16_t)((p[1] << 8) | p[0]);
}

InlineStatic void UNUSED be32enc(void *buf, uint32_t u)
{
	uint8_t *p = (uint8_t *)buf;

	p[0] = (uint8_t)((u >> 24) & 0xff);
	p[1] = (uint8_t)((u >> 16) & 0xff);
	p[2] = (uint8_t)((u >> 8) & 0xff);
	p[3] = (uint8_t)(u & 0xff);
}

InlineStatic void UNUSED le32enc(void *buf, uint32_t u)
{
	uint8_t *p = (uint8_t *)buf;

	p[0] = (uint8_t)(u & 0xff);
	p[1] = (uint8_t)((u >> 8) & 0xff);
	p[2] = (uint8_t)((u >> 16) & 0xff);
	p[3] = (uint8_t)((u >> 24) & 0xff);
}

InlineStatic uint32_t UNUSED be32dec(const void *buf)
{
	const uint8_t *p = (const uint8_t *)buf;

	return ((p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3]);
}

InlineStatic uint32_t UNUSED le32dec(const void *buf)
{
	const uint8_t *p = (const uint8_t *)buf;

	return ((p[3] << 24) | (p[2] << 16) | (p[1] << 8) | p[0]);
}

InlineStatic void UNUSED be64enc(void *buf, uint64_t u)
{
	uint8_t *p = (uint8_t *)buf;

	be32enc(p, (uint32_t)(u >> 32));
	be32enc(p + 4, (uint32_t)(u & 0xffffffffULL));
}

InlineStatic void UNUSED le64enc(void *buf, uint64_t u)
{
	uint8_t *p = (uint8_t *)buf;

	le32enc(p, (uint32_t)(u & 0xffffffffULL));
	le32enc(p + 4, (uint32_t)(u >> 32));
}

InlineStatic uint64_t UNUSED be64dec(const void *buf)
{
	const uint8_t *p = (const uint8_t *)buf;

	return (((uint64_t)be32dec(p) << 32) | be32dec(p + 4));
}

InlineStatic uint64_t UNUSED le64dec(const void *buf)
{
	const uint8_t *p = (const uint8_t *)buf;

	return (le32dec(p) | ((uint64_t)le32dec(p + 4) << 32));
}

#endif

