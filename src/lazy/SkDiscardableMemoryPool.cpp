/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkMalloc.h"
#include "include/private/SkMutex.h"
#include "include/private/SkTemplates.h"
#include "src/core/SkDiscardableMemory.h"
#include "src/core/SkMakeUnique.h"
#include "src/core/SkTInternalLList.h"
#include "src/lazy/SkDiscardableMemoryPool.h"

// Note:
// A PoolDiscardableMemory is memory that is counted in a pool.
// A DiscardableMemoryPool is a pool of PoolDiscardableMemorys.

namespace {

class PoolDiscardableMemory;

/**
 *  This non-global pool can be used for unit tests to verify that the
 *  pool works.
 */
class DiscardableMemoryPool : public SkDiscardableMemoryPool {
public:
    DiscardableMemoryPool(size_t budget);
    ~DiscardableMemoryPool() override;

    std::unique_ptr<SkDiscardableMemory> make(size_t bytes);
    SkDiscardableMemory* create(size_t bytes) override {
        return this->make(bytes).release();  // TODO: change API
    }

    size_t getRAMUsed() override;
    void setRAMBudget(size_t budget) override;
    size_t getRAMBudget() override { return fBudget; }

    /** purges all unlocked DMs */
    void dumpPool() override;

    #if SK_LAZY_CACHE_STATS  // Defined in SkDiscardableMemoryPool.h
    int getCacheHits() override { return fCacheHits; }
    int getCacheMisses() override { return fCacheMisses; }
    void resetCacheHitsAndMisses() override {
        fCacheHits = fCacheMisses = 0;
    }
    int          fCacheHits;
    int          fCacheMisses;
    #endif  // SK_LAZY_CACHE_STATS

private:
    SkMutex      fMutex;
    size_t       fBudget;
    size_t       fUsed;
    SkTInternalLList<PoolDiscardableMemory> fList;

    /** Function called to free memory if needed */
    void dumpDownTo(size_t budget);
    /** called by DiscardableMemoryPool upon destruction */
    void removeFromPool(PoolDiscardableMemory* dm);
    /** called by DiscardableMemoryPool::lock() */
    bool lock(PoolDiscardableMemory* dm);
    /** called by DiscardableMemoryPool::unlock() */
    void unlock(PoolDiscardableMemory* dm);

    friend class PoolDiscardableMemory;

    typedef SkDiscardableMemory::Factory INHERITED;
};

/**
 *  A PoolDiscardableMemory is a SkDiscardableMemory that relies on
 *  a DiscardableMemoryPool object to manage the memory.
 */
class PoolDiscardableMemory : public SkDiscardableMemory {
public:
    PoolDiscardableMemory(sk_sp<DiscardableMemoryPool> pool, SkAutoFree pointer, size_t bytes);
    ~PoolDiscardableMemory() override;
    bool lock() override;
    void* data() override;
    void unlock() override;
    friend class DiscardableMemoryPool;
private:
    SK_DECLARE_INTERNAL_LLIST_INTERFACE(PoolDiscardableMemory);
    sk_sp<DiscardableMemoryPool> fPool;
    bool                         fLocked;
    SkAutoFree                   fPointer;
    const size_t                 fBytes;
};

PoolDiscardableMemory::PoolDiscardableMemory(sk_sp<DiscardableMemoryPool> pool,
                                             SkAutoFree pointer,
                                             size_t bytes)
        : fPool(std::move(pool)), fLocked(true), fPointer(std::move(pointer)), fBytes(bytes) {
    SkASSERT(fPool != nullptr);
    SkASSERT(fPointer != nullptr);
    SkASSERT(fBytes > 0);
}

PoolDiscardableMemory::~PoolDiscardableMemory() {
    SkASSERT(!fLocked); // contract for SkDiscardableMemory
    fPool->removeFromPool(this);
}

bool PoolDiscardableMemory::lock() {
    SkASSERT(!fLocked); // contract for SkDiscardableMemory
    return fPool->lock(this);
}

void* PoolDiscardableMemory::data() {
    SkASSERT(fLocked); // contract for SkDiscardableMemory
    return fPointer.get();
}

void PoolDiscardableMemory::unlock() {
    SkASSERT(fLocked); // contract for SkDiscardableMemory
    fPool->unlock(this);
}

////////////////////////////////////////////////////////////////////////////////

DiscardableMemoryPool::DiscardableMemoryPool(size_t budget)
    : fBudget(budget)
    , fUsed(0) {
    #if SK_LAZY_CACHE_STATS
    fCacheHits = 0;
    fCacheMisses = 0;
    #endif  // SK_LAZY_CACHE_STATS
}
DiscardableMemoryPool::~DiscardableMemoryPool() {
    // PoolDiscardableMemory objects that belong to this pool are
    // always deleted before deleting this pool since each one has a
    // ref to the pool.
    SkASSERT(fList.isEmpty());
}

void DiscardableMemoryPool::dumpDownTo(size_t budget) {
    fMutex.assertHeld();
    if (fUsed <= budget) {
        return;
    }
    using Iter = SkTInternalLList<PoolDiscardableMemory>::Iter;
    Iter iter;
    PoolDiscardableMemory* cur = iter.init(fList, Iter::kTail_IterStart);
    while ((fUsed > budget) && (cur)) {
        if (!cur->fLocked) {
            PoolDiscardableMemory* dm = cur;
            SkASSERT(dm->fPointer != nullptr);
            dm->fPointer = nullptr;
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

std::unique_ptr<SkDiscardableMemory> DiscardableMemoryPool::make(size_t bytes) {
    SkAutoFree addr(sk_malloc_canfail(bytes));
    if (nullptr == addr) {
        return nullptr;
    }
    auto dm = skstd::make_unique<PoolDiscardableMemory>(sk_ref_sp(this), std::move(addr), bytes);
    SkAutoMutexExclusive autoMutexAcquire(fMutex);
    fList.addToHead(dm.get());
    fUsed += bytes;
    this->dumpDownTo(fBudget);
    return dm;
}

void DiscardableMemoryPool::removeFromPool(PoolDiscardableMemory* dm) {
    SkAutoMutexExclusive autoMutexAcquire(fMutex);
    // This is called by dm's destructor.
    if (dm->fPointer != nullptr) {
        SkASSERT(fUsed >= dm->fBytes);
        fUsed -= dm->fBytes;
        fList.remove(dm);
    } else {
        SkASSERT(!fList.isInList(dm));
    }
}

bool DiscardableMemoryPool::lock(PoolDiscardableMemory* dm) {
    SkASSERT(dm != nullptr);
    SkAutoMutexExclusive autoMutexAcquire(fMutex);
    if (nullptr == dm->fPointer) {
        // May have been purged while waiting for lock.
        #if SK_LAZY_CACHE_STATS
        ++fCacheMisses;
        #endif  // SK_LAZY_CACHE_STATS
        return false;
    }
    dm->fLocked = true;
    fList.remove(dm);
    fList.addToHead(dm);
    #if SK_LAZY_CACHE_STATS
    ++fCacheHits;
    #endif  // SK_LAZY_CACHE_STATS
    return true;
}

void DiscardableMemoryPool::unlock(PoolDiscardableMemory* dm) {
    SkASSERT(dm != nullptr);
    SkAutoMutexExclusive autoMutexAcquire(fMutex);
    dm->fLocked = false;
    this->dumpDownTo(fBudget);
}

size_t DiscardableMemoryPool::getRAMUsed() {
    return fUsed;
}
void DiscardableMemoryPool::setRAMBudget(size_t budget) {
    SkAutoMutexExclusive autoMutexAcquire(fMutex);
    fBudget = budget;
    this->dumpDownTo(fBudget);
}
void DiscardableMemoryPool::dumpPool() {
    SkAutoMutexExclusive autoMutexAcquire(fMutex);
    this->dumpDownTo(0);
}

}  // namespace

sk_sp<SkDiscardableMemoryPool> SkDiscardableMemoryPool::Make(size_t size) {
    return sk_make_sp<DiscardableMemoryPool>(size);
}

SkDiscardableMemoryPool* SkGetGlobalDiscardableMemoryPool() {
    // Intentionally leak this global pool.
    static SkDiscardableMemoryPool* global =
            new DiscardableMemoryPool(SK_DEFAULT_GLOBAL_DISCARDABLE_MEMORY_POOL_SIZE);
    return global;
}
