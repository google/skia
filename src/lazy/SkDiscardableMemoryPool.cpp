/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDiscardableMemoryPool.h"
#include "SkOnce.h"

// Note:
// A PoolDiscardableMemory is memory that is counted in a pool.
// A DiscardableMemoryPool is a pool of PoolDiscardableMemorys.

/**
 *  A SkPoolDiscardableMemory is a SkDiscardableMemory that relies on
 *  a SkDiscardableMemoryPool object to manage the memory.
 */
class SkPoolDiscardableMemory : public SkDiscardableMemory {
public:
    SkPoolDiscardableMemory(SkDiscardableMemoryPool* pool,
                            void* pointer, size_t bytes);
    virtual ~SkPoolDiscardableMemory();
    virtual bool lock() SK_OVERRIDE;
    virtual void* data() SK_OVERRIDE;
    virtual void unlock() SK_OVERRIDE;
    friend class SkDiscardableMemoryPool;
private:
    SK_DECLARE_INTERNAL_LLIST_INTERFACE(SkPoolDiscardableMemory);
    SkDiscardableMemoryPool* const fPool;
    bool                           fLocked;
    void*                          fPointer;
    const size_t                   fBytes;
};

SkPoolDiscardableMemory::SkPoolDiscardableMemory(SkDiscardableMemoryPool* pool,
                                                 void* pointer,
                                                 size_t bytes)
    : fPool(pool)
    , fLocked(true)
    , fPointer(pointer)
    , fBytes(bytes) {
    SkASSERT(fPool != NULL);
    SkASSERT(fPointer != NULL);
    SkASSERT(fBytes > 0);
    fPool->ref();
}

SkPoolDiscardableMemory::~SkPoolDiscardableMemory() {
    SkASSERT(!fLocked); // contract for SkDiscardableMemory
    fPool->free(this);
    fPool->unref();
}

bool SkPoolDiscardableMemory::lock() {
    SkASSERT(!fLocked); // contract for SkDiscardableMemory
    return fPool->lock(this);
}

void* SkPoolDiscardableMemory::data() {
    SkASSERT(fLocked); // contract for SkDiscardableMemory
    return fPointer;
}

void SkPoolDiscardableMemory::unlock() {
    SkASSERT(fLocked); // contract for SkDiscardableMemory
    fPool->unlock(this);
}

////////////////////////////////////////////////////////////////////////////////

SkDiscardableMemoryPool::SkDiscardableMemoryPool(size_t budget,
                                                 SkBaseMutex* mutex)
    : fMutex(mutex)
    , fBudget(budget)
    , fUsed(0) {
    #if LAZY_CACHE_STATS
    fCacheHits = 0;
    fCacheMisses = 0;
    #endif  // LAZY_CACHE_STATS
}
SkDiscardableMemoryPool::~SkDiscardableMemoryPool() {
    // SkPoolDiscardableMemory objects that belong to this pool are
    // always deleted before deleting this pool since each one has a
    // ref to the pool.
    SkASSERT(fList.isEmpty());
}

void SkDiscardableMemoryPool::dumpDownTo(size_t budget) {
    // assert((NULL = fMutex) || fMutex->isLocked());
    // TODO(halcanary) implement bool fMutex::isLocked().
    // WARNING: only call this function after aquiring lock.
    if (fUsed <= budget) {
        return;
    }
    typedef SkTInternalLList<SkPoolDiscardableMemory>::Iter Iter;
    Iter iter;
    SkPoolDiscardableMemory* cur = iter.init(fList, Iter::kTail_IterStart);
    while ((fUsed > budget) && (NULL != cur)) {
        if (!cur->fLocked) {
            SkPoolDiscardableMemory* dm = cur;
            SkASSERT(dm->fPointer != NULL);
            sk_free(dm->fPointer);
            dm->fPointer = NULL;
            SkASSERT(fUsed >= dm->fBytes);
            fUsed -= dm->fBytes;
            cur = iter.prev();
            // Purged DMs are taken out of the list.  This saves times
            // looking them up.  Purged DMs are NOT deleted.
            fList.remove(dm);
        } else {
            cur = iter.prev();
        }
    }
}

SkDiscardableMemory* SkDiscardableMemoryPool::create(size_t bytes) {
    void* addr = sk_malloc_flags(bytes, 0);
    if (NULL == addr) {
        return NULL;
    }
    SkPoolDiscardableMemory* dm = SkNEW_ARGS(SkPoolDiscardableMemory,
                                             (this, addr, bytes));
    SkAutoMutexAcquire autoMutexAcquire(fMutex);
    fList.addToHead(dm);
    fUsed += bytes;
    this->dumpDownTo(fBudget);
    return dm;
}

void SkDiscardableMemoryPool::free(SkPoolDiscardableMemory* dm) {
    // This is called by dm's destructor.
    if (dm->fPointer != NULL) {
        SkAutoMutexAcquire autoMutexAcquire(fMutex);
        sk_free(dm->fPointer);
        dm->fPointer = NULL;
        SkASSERT(fUsed >= dm->fBytes);
        fUsed -= dm->fBytes;
        fList.remove(dm);
    } else {
        SkASSERT(!fList.isInList(dm));
    }
}

bool SkDiscardableMemoryPool::lock(SkPoolDiscardableMemory* dm) {
    SkASSERT(dm != NULL);
    if (NULL == dm->fPointer) {
        #if LAZY_CACHE_STATS
        SkAutoMutexAcquire autoMutexAcquire(fMutex);
        ++fCacheMisses;
        #endif  // LAZY_CACHE_STATS
        return false;
    }
    SkAutoMutexAcquire autoMutexAcquire(fMutex);
    if (NULL == dm->fPointer) {
        // May have been purged while waiting for lock.
        #if LAZY_CACHE_STATS
        ++fCacheMisses;
        #endif  // LAZY_CACHE_STATS
        return false;
    }
    dm->fLocked = true;
    fList.remove(dm);
    fList.addToHead(dm);
    #if LAZY_CACHE_STATS
    ++fCacheHits;
    #endif  // LAZY_CACHE_STATS
    return true;
}

void SkDiscardableMemoryPool::unlock(SkPoolDiscardableMemory* dm) {
    SkASSERT(dm != NULL);
    SkAutoMutexAcquire autoMutexAcquire(fMutex);
    dm->fLocked = false;
    this->dumpDownTo(fBudget);
}

size_t SkDiscardableMemoryPool::getRAMUsed() {
    return fUsed;
}
void SkDiscardableMemoryPool::setRAMBudget(size_t budget) {
    SkAutoMutexAcquire autoMutexAcquire(fMutex);
    fBudget = budget;
    this->dumpDownTo(fBudget);
}
void SkDiscardableMemoryPool::dumpPool() {
    SkAutoMutexAcquire autoMutexAcquire(fMutex);
    this->dumpDownTo(0);
}

////////////////////////////////////////////////////////////////////////////////
SK_DECLARE_STATIC_MUTEX(gMutex);
static void create_pool(SkDiscardableMemoryPool** pool) {
    SkASSERT(NULL == *pool);
    *pool = SkNEW_ARGS(SkDiscardableMemoryPool,
                       (SK_DEFAULT_GLOBAL_DISCARDABLE_MEMORY_POOL_SIZE,
                        &gMutex));
}
SkDiscardableMemoryPool* SkGetGlobalDiscardableMemoryPool() {
    static SkDiscardableMemoryPool* gPool(NULL);
    SK_DECLARE_STATIC_ONCE(create_pool_once);
    SkOnce(&create_pool_once, create_pool, &gPool);
    SkASSERT(NULL != gPool);
    return gPool;
}

////////////////////////////////////////////////////////////////////////////////
