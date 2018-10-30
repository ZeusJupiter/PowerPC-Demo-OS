/*
 *   File name: cache.h
 *
 *  Created on: 2017年1月16日, 下午5:44:58
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */

#if POWERPC_FAMILY == POWERPC_MPC750

#include "60x/hid0.h"

#endif

#ifndef __CACHE_H__
#define __CACHE_H__

#if !defined(ASSEMBLY)

#if defined(__cplusplus)

namespace
{
		InlineStatic void ppcCacheCodeSync(word_t address)
		{ __asm__ __volatile__("dcbst 0, %0; sync; icbi 0, %0; isync;" : : "r"(address)); }

		InlineStatic void ppcCachePartialCodeSync(word_t address)
		{ __asm__ __volatile__("dcbst 0, %0; sync; icbi 0, %0;" : : "r"(address)); }

		InlineStatic void ppcCacheCompleteCodeSync( void )
		{ __asm__ __volatile__( "isync" ); }

		InlineStatic void ppcCacheZeroBlock(word_t address)
		{ __asm__ __volatile__("dcbz 0, %0" : : "r"(address)); }

		InlineStatic void ppcCacheAllocBlock(word_t address)
		{ __asm__ __volatile__("dcba 0, %0" : : "r"(address)); }

		InlineStatic void ppcCacheFlushBlock(word_t address)
		{ __asm__ __volatile__("dcbf 0, %0" : : "r"(address)); }

		InlineStatic void ppcCacheInvalidateBlock(word_t address)
		{ __asm__ __volatile__("dcbi 0, %0" : : "r"(address)); }

		InlineStatic void ppcCacheStoreBlock(word_t address)
		{ __asm__ __volatile__("dcbst 0, %0" : : "r" (address)); }

		InlineStatic void ppcCachePrefetch(word_t address)
		{ __asm__ __volatile__("dcbt 0, %0" : : "r"(address)); }

		InlineStatic void ppcCachePrefetchForStore(word_t address)
		{ __asm__ __volatile__("dcbtst 0,%0" : : "r" (address)); }
}

namespace Powerpc
{
	namespace Cache
	{
		const u32 POWERPC_CACHE_LINE_SIZE = 32;
		const VoidFuncPtrWord cacheCodeSync = ppcCacheCodeSync;
		const VoidFuncPtrWord cachePartialCodeSync = ppcCachePartialCodeSync;
		const VoidFuncPtrVoid cacheCompleteCodeSync= ppcCacheCompleteCodeSync;
		const VoidFuncPtrWord cacheZeroBlock = ppcCacheZeroBlock;
		const VoidFuncPtrWord allocCacheBlock = ppcCacheAllocBlock;
		const VoidFuncPtrWord flushCacheBlock = ppcCacheFlushBlock;
		const VoidFuncPtrWord invalidateCacheBlock = ppcCacheInvalidateBlock;
		const VoidFuncPtrWord storeCacheBlock = ppcCacheStoreBlock;
		const VoidFuncPtrWord prefetchCache = ppcCachePrefetch;
		const VoidFuncPtrWord prefetchCacheForStore = ppcCachePrefetchForStore;

		const VoidFuncPtrVoid init = initHid0;
		const VoidFuncPtrVoid invalidateICacheAll = ::invalidateICacheAll;
		const VoidFuncPtrVoid invalidateDCacheAll = ::invalidateDCacheAll;
		const VoidFuncPtrVoid invalidateCacheAll = ::invalidateCacheAll;

		const VoidFuncPtrVoid enableICache = ::enableICache;
		const VoidFuncPtrVoid enableDCache = ::enableDCache;
		const VoidFuncPtrVoid disableICache = ::disableICache;
		const VoidFuncPtrVoid disableDCache = ::disableDCache;

		const VoidFuncPtrVoid enableCache = ::enableCache;
		const VoidFuncPtrVoid disableCache = ::disableCache;
	}
}

#endif

#endif

#endif

