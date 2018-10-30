/*
 *   File name: findfirstBit.cpp
 *
 *  Created on: 2017年6月22日, 下午3:41:38
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description: find first bit set library
 */
#include <macros.h>
#include <types.h>

static const u8 mostSignificantBitTable [256] =
{
    0, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
};

static const u8 leastSignificantBitTable [256] =
{
    0, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
};

union MultiValue32
{
	u32 _u32Val;
	u16 _u16Val[2];
	u8 _u8Val[4];
};

union MultiValue64
{
	u64 qword;
	u32 dwords[2];
};

BEG_EXT_C

sint findMostSignificantBit32(u32 val)
{
	MultiValue32 uVal;
	uVal._u32Val = val;
	if (uVal._u16Val[0])
	{
		if (uVal._u8Val[0])
			return (mostSignificantBitTable[uVal._u8Val[0]] + 24 + 1);
		else
			return (mostSignificantBitTable[uVal._u8Val[1]] + 16 + 1);
	}
	else
	{
		if (uVal._u8Val[2])
			return (mostSignificantBitTable[uVal._u8Val[2]] + 8 + 1);
		else
			return (mostSignificantBitTable[uVal._u8Val[3]] + (val ? 1 : 0));
	}
}

sint findLeastSignificantBit32(u32 val)
{
	MultiValue32 uVal;
	uVal._u32Val = val;

	if (uVal._u16Val[1])
	{
		if (uVal._u8Val[3])
			return (leastSignificantBitTable[uVal._u8Val[3]] + 1);
		else
			return (leastSignificantBitTable[uVal._u8Val[2]] + 8 + 1);
	}
	else
	{
		if (uVal._u8Val[1])
			return (leastSignificantBitTable[uVal._u8Val[1]] + 16 + 1);
		else
			return (leastSignificantBitTable[uVal._u8Val[0]] + (val ? 24+1 : 0));
	}
}

sint UNUSED findMostSignificantBit64(u64 val)
{
	MultiValue64 uVal;
	uVal.qword = val;

	if (uVal.dwords[0])
	{
		return findMostSignificantBit32(uVal.dwords[0]) + 32;
	}
	else
	{
		return findMostSignificantBit32(uVal.dwords[1]);
	}
}

sint findLeastSignificantBit64(u64 val)
{
	MultiValue64 uVal;

	uVal.qword = val;

	if (uVal.dwords[1])
	{
		return findLeastSignificantBit32(uVal.dwords[1]);
	}
	else
	{
		return findLeastSignificantBit32(uVal.dwords[0]) + (val ? 32 : 0);
	}
}

END_EXT_C
