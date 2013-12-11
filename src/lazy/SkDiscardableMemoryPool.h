/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDiscardableMemoryPool_DEFINED
#define SkDiscardableMemoryPool_DEFINED

#include "SkDiscardableMemory.h"
#include "SkTInternalLList.h"
#include "SkThread.h"

class SkPoolDiscardableMemory;

#ifdef SK_DEBUG
    #define LAZY_CACHE_STATS 1
#elif !defined(LAZY_CACHE_STATS)
    #define LAZY_CACHE_STATS 0
#endif

/**
 *  This non-global pool can be used for unit tests to verify that the
 *  pool works.
 */
class SkDiscardableMemoryPool : public SkDiscardableMemory::Factory {
public:
    /**
     *  Without mutex, will be not be thread safe.
     */
    SkDiscardableMemoryPool(size_t budget, SkBaseMutex* mutex = NULL);
    virtual ~SkDiscardableMemoryPool();

    virtual SkDiscardableMemory* create(size_t bytes) SK_OVERRIDE;

    size_t getRAMUsed();
    void setRAMBudget(size_t budget);

    /** purges all unlocked DMs */
    void dumpPool();

    #if LAZY_CACHE_STATS
    int          fCacheHits;
    int          fCacheMisses;
    #endif  // LAZY_CACHE_STATS

private:
    SkBaseMutex* fMutex;
    size_t       fBudget;
    size_t       fUsed;
    SkTInternalLList<SkPoolDiscardableMemory> fList;

    /** Function called to free memory if needed */
    void dumpDownTo(size_t budget);
    /** called by SkDiscardableMemoryPool upon destruction */
    void free(SkPoolDiscardableMemory* dm);
    /** called by SkDiscardableMemoryPool::lock() */
    bool lock(SkPoolDiscardableMemory* dm);
    /** called by SkDiscardableMemoryPool::unlock() */
    void unlock(SkPoolDiscardableMemory* dm);

    friend class SkPoolDiscardableMemory;

    typedef SkDiscardableMemory::Factory INHERITED;
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
