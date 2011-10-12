
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#include "GrResourceCache.h"
#include "GrResource.h"

GrResourceEntry::GrResourceEntry(const GrResourceKey& key, GrResource* resource)
        : fKey(key), fResource(resource) {
    fLockCount = 0;
    fPrev = fNext = NULL;

    // we assume ownership of the resource, and will unref it when we die
    GrAssert(resource);
}

GrResourceEntry::~GrResourceEntry() {
    fResource->unref();
}

#if GR_DEBUG
void GrResourceEntry::validate() const {
    GrAssert(fLockCount >= 0);
    GrAssert(fResource);
    fResource->validate();
}
#endif

///////////////////////////////////////////////////////////////////////////////

GrResourceCache::GrResourceCache(int maxCount, size_t maxBytes) :
        fMaxCount(maxCount),
        fMaxBytes(maxBytes) {
    fEntryCount          = 0;
    fUnlockedEntryCount  = 0;
    fEntryBytes          = 0;
    fClientDetachedCount = 0;
    fClientDetachedBytes = 0;

    fHead = fTail = NULL;
    fPurging = false;
}

GrResourceCache::~GrResourceCache() {
    GrAutoResourceCacheValidate atcv(this);

    this->removeAll();
}

void GrResourceCache::getLimits(int* maxResources, size_t* maxResourceBytes) const{
    if (maxResources) {
        *maxResources = fMaxCount;
    }
    if (maxResourceBytes) {
        *maxResourceBytes = fMaxBytes;
    }
}

void GrResourceCache::setLimits(int maxResources, size_t maxResourceBytes) {
    bool smaller = (maxResources < fMaxCount) || (maxResourceBytes < fMaxBytes);

    fMaxCount = maxResources;
    fMaxBytes = maxResourceBytes;

    if (smaller) {
        this->purgeAsNeeded();
    }
}

void GrResourceCache::internalDetach(GrResourceEntry* entry,
                                    bool clientDetach) {
    GrResourceEntry* prev = entry->fPrev;
    GrResourceEntry* next = entry->fNext;

    if (prev) {
        prev->fNext = next;
    } else {
        fHead = next;
    }
    if (next) {
        next->fPrev = prev;
    } else {
        fTail = prev;
    }
    if (!entry->isLocked()) {
        --fUnlockedEntryCount;
    }

    // update our stats
    if (clientDetach) {
        fClientDetachedCount += 1;
        fClientDetachedBytes += entry->resource()->sizeInBytes();
    } else {
        fEntryCount -= 1;
        fEntryBytes -= entry->resource()->sizeInBytes();
    }
}

void GrResourceCache::attachToHead(GrResourceEntry* entry,
                                  bool clientReattach) {
    entry->fPrev = NULL;
    entry->fNext = fHead;
    if (fHead) {
        fHead->fPrev = entry;
    }
    fHead = entry;
    if (NULL == fTail) {
        fTail = entry;
    }
    if (!entry->isLocked()) {
        ++fUnlockedEntryCount;
    }

    // update our stats
    if (clientReattach) {
        fClientDetachedCount -= 1;
        fClientDetachedBytes -= entry->resource()->sizeInBytes();
    } else {
        fEntryCount += 1;
        fEntryBytes += entry->resource()->sizeInBytes();
    }
}

class GrResourceCache::Key {
    typedef GrResourceEntry T;

    const GrResourceKey& fKey;
public:
    Key(const GrResourceKey& key) : fKey(key) {}

    uint32_t getHash() const { return fKey.hashIndex(); }

    static bool LT(const T& entry, const Key& key) {
        return entry.key() < key.fKey;
    }
    static bool EQ(const T& entry, const Key& key) {
        return entry.key() == key.fKey;
    }
#if GR_DEBUG
    static uint32_t GetHash(const T& entry) {
        return entry.key().hashIndex();
    }
    static bool LT(const T& a, const T& b) {
        return a.key() < b.key();
    }
    static bool EQ(const T& a, const T& b) {
        return a.key() == b.key();
    }
#endif
};

GrResourceEntry* GrResourceCache::findAndLock(const GrResourceKey& key,
                                              LockType type) {
    GrAutoResourceCacheValidate atcv(this);

    GrResourceEntry* entry = fCache.find(key);
    if (entry) {
        this->internalDetach(entry, false);
        // mark the entry as "busy" so it doesn't get purged
        // do this between detach and attach for locked count tracking
        if (kNested_LockType == type || !entry->isLocked()) {
            entry->lock();
        }
        this->attachToHead(entry, false);
    }
    return entry;
}

GrResourceEntry* GrResourceCache::createAndLock(const GrResourceKey& key,
                                              GrResource* resource) {
    // we don't expect to create new resources during a purge. In theory
    // this could cause purgeAsNeeded() into an infinite loop (e.g.
    // each resource destroyed creates and locks 2 resources and
    // unlocks 1 thereby causing a new purge).
    GrAssert(!fPurging);
    GrAutoResourceCacheValidate atcv(this);

    GrResourceEntry* entry = new GrResourceEntry(key, resource);

    // mark the entry as "busy" so it doesn't get purged
    // do this before attach for locked count tracking
    entry->lock();

    this->attachToHead(entry, false);
    fCache.insert(key, entry);

#if GR_DUMP_TEXTURE_UPLOAD
    GrPrintf("--- add resource to cache %p, count=%d bytes= %d %d\n",
             entry, fEntryCount, resource->sizeInBytes(), fEntryBytes);
#endif

    this->purgeAsNeeded();
    return entry;
}

void GrResourceCache::detach(GrResourceEntry* entry) {
    GrAutoResourceCacheValidate atcv(this);
    internalDetach(entry, true);
    fCache.remove(entry->fKey, entry);
}

void GrResourceCache::reattachAndUnlock(GrResourceEntry* entry) {
    GrAutoResourceCacheValidate atcv(this);
    if (entry->resource()->isValid()) {
        attachToHead(entry, true);
        fCache.insert(entry->key(), entry);
    } else {
        // If the resource went invalid while it was detached then purge it
        // This can happen when a 3D context was lost,
        // the client called GrContext::contextDestroyed() to notify Gr,
        // and then later an SkGpuDevice's destructor releases its backing
        // texture (which was invalidated at contextDestroyed time).
        fClientDetachedCount -= 1;
        fEntryCount -= 1;
        size_t size = entry->resource()->sizeInBytes();
        fClientDetachedBytes -= size;
        fEntryBytes -= size;
    }
    this->unlock(entry);
}

void GrResourceCache::unlock(GrResourceEntry* entry) {
    GrAutoResourceCacheValidate atcv(this);

    GrAssert(entry);
    GrAssert(entry->isLocked());
    GrAssert(fCache.find(entry->key()));

    entry->unlock();
    if (!entry->isLocked()) {
        ++fUnlockedEntryCount;
    }
    this->purgeAsNeeded();
}

/**
 * Destroying a resource may potentially trigger the unlock of additional 
 * resources which in turn will trigger a nested purge. We block the nested
 * purge using the fPurging variable. However, the initial purge will keep
 * looping until either all resources in the cache are unlocked or we've met
 * the budget. There is an assertion in createAndLock to check against a
 * resource's destructor inserting new resources into the cache. If these
 * new resources were unlocked before purgeAsNeeded completed it could
 * potentially make purgeAsNeeded loop infinitely.
 */
void GrResourceCache::purgeAsNeeded() {
    if (!fPurging) {
        fPurging = true;
        bool withinBudget = false;
        do {
            GrResourceEntry* entry = fTail;
            while (entry && fUnlockedEntryCount) {
                GrAutoResourceCacheValidate atcv(this);
                if (fEntryCount <= fMaxCount && fEntryBytes <= fMaxBytes) {
                    withinBudget = true;
                    break;
                }

                GrResourceEntry* prev = entry->fPrev;
                if (!entry->isLocked()) {
                    // remove from our cache
                    fCache.remove(entry->fKey, entry);

                    // remove from our llist
                    this->internalDetach(entry, false);

        #if GR_DUMP_TEXTURE_UPLOAD
                    GrPrintf("--- ~resource from cache %p [%d %d]\n",
                             entry->resource(),
                             entry->resource()->width(),
                             entry->resource()->height());
        #endif
                    delete entry;
                }
                entry = prev;
            }
        } while (!withinBudget && fUnlockedEntryCount);
        fPurging = false;
    }
}

void GrResourceCache::removeAll() {
    GrAutoResourceCacheValidate atcv(this);

    GrResourceEntry* entry = fHead;

    // we can have one GrResource holding a lock on another
    // so we don't want to just do a simple loop kicking each
    // entry out. Instead change the budget and purge.

    int savedMaxBytes = fMaxBytes;
    int savedMaxCount = fMaxCount;
    fMaxBytes = -1;
    fMaxCount = 0;
    this->purgeAsNeeded();

    GrAssert(!fCache.count());
    GrAssert(!fUnlockedEntryCount);
    // Items may have been detached from the cache (such as the backing texture
    // for an SkGpuDevice). The above purge would not have removed them.
    GrAssert(fEntryCount == fClientDetachedCount);
    GrAssert(fEntryBytes == fClientDetachedBytes);
    GrAssert(NULL == fHead);
    GrAssert(NULL == fTail);

    fMaxBytes = savedMaxBytes;
    fMaxCount = savedMaxCount;
}

///////////////////////////////////////////////////////////////////////////////

#if GR_DEBUG
static int countMatches(const GrResourceEntry* head, const GrResourceEntry* target) {
    const GrResourceEntry* entry = head;
    int count = 0;
    while (entry) {
        if (target == entry) {
            count += 1;
        }
        entry = entry->next();
    }
    return count;
}

#if GR_DEBUG
static bool both_zero_or_nonzero(int count, size_t bytes) {
    return (count == 0 && bytes == 0) || (count > 0 && bytes > 0);
}
#endif

void GrResourceCache::validate() const {
    GrAssert(!fHead == !fTail);
    GrAssert(both_zero_or_nonzero(fEntryCount, fEntryBytes));
    GrAssert(both_zero_or_nonzero(fClientDetachedCount, fClientDetachedBytes));
    GrAssert(fClientDetachedBytes <= fEntryBytes);
    GrAssert(fClientDetachedCount <= fEntryCount);
    GrAssert((fEntryCount - fClientDetachedCount) == fCache.count());

    fCache.validate();

    GrResourceEntry* entry = fHead;
    int count = 0;
    int unlockCount = 0;
    size_t bytes = 0;
    while (entry) {
        entry->validate();
        GrAssert(fCache.find(entry->key()));
        count += 1;
        bytes += entry->resource()->sizeInBytes();
        if (!entry->isLocked()) {
            unlockCount += 1;
        }
        entry = entry->fNext;
    }
    GrAssert(count == fEntryCount - fClientDetachedCount);
    GrAssert(bytes == fEntryBytes  - fClientDetachedBytes);
    GrAssert(unlockCount == fUnlockedEntryCount);

    count = 0;
    for (entry = fTail; entry; entry = entry->fPrev) {
        count += 1;
    }
    GrAssert(count == fEntryCount - fClientDetachedCount);

    for (int i = 0; i < count; i++) {
        int matches = countMatches(fHead, fCache.getArray()[i]);
        GrAssert(1 == matches);
    }
}
#endif
