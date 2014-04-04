/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDiscardableMemoryPool_DEFINED
#define SkDiscardableMemoryPool_DEFINED

#include "SkDiscardableMemory.h"

#ifndef SK_LAZY_CACHE_STATS
    #ifdef SK_DEBUG
        #define SK_LAZY_CACHE_STATS 1
    #else
        #define SK_LAZY_CACHE_STATS 0
    #endif
#endif

/**
 *  An implementation of Discardable Memory that manages a fixed-size
 *  budget of memory.  When the allocated memory exceeds this size,
 *  unlocked blocks of memory are purged.  If all memory is locked, it
 *  can exceed the memory-use budget.
 */
class SkDiscardableMemoryPool : public SkDiscardableMemory::Factory {
public:
    virtual ~SkDiscardableMemoryPool() { }

    virtual size_t getRAMUsed() = 0;
    virtual void setRAMBudget(size_t budget) = 0;
    virtual size_t getRAMBudget() = 0;

    /** purges all unlocked DMs */
    virtual void dumpPool() = 0;

    #if SK_LAZY_CACHE_STATS
    /**
     * These two values are a count of the number of successful and
     * failed calls to SkDiscardableMemory::lock() for all DMs managed
     * by this pool.
     */
    virtual int getCacheHits() = 0;
    virtual int getCacheMisses() = 0;
    virtual void resetCacheHitsAndMisses() = 0;
    #endif

    /**
     *  This non-global pool can be used for unit tests to verify that
     *  the pool works.
     *  Without mutex, will be not be thread safe.
     */
    static SkDiscardableMemoryPool* Create(
            size_t size, SkBaseMutex* mutex = NULL);
};

/**
 *  Returns (and creates if needed) a threadsafe global
 *  SkDiscardableMemoryPool.
 */
SkDiscardableMemoryPool* SkGetGlobalDiscardableMemoryPool();

#if !defined(SK_DEFAULT_GLOBAL_DISCARDABLE_MEMORY_POOL_SIZE)
#define SK_DEFAULT_GLOBAL_DISCARDABLE_MEMORY_POOL_SIZE (128 * 1024 * 1024)
#endif

#endif  // SkDiscardableMemoryPool_DEFINED
