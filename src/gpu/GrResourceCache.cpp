
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

    // we assume ownership of the resource, and will unref it when we die
    GrAssert(resource);
}

GrResourceEntry::~GrResourceEntry() {
    fResource->setCacheEntry(NULL);
    fResource->unref();
}

#if GR_DEBUG
void GrResourceEntry::validate() const {
    GrAssert(fLockCount >= 0);
    GrAssert(fResource);
    GrAssert(fResource->getCacheEntry() == this);
    fResource->validate();
}
#endif

///////////////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////////////////

GrResourceCache::GrResourceCache(int maxCount, size_t maxBytes) :
        fMaxCount(maxCount),
        fMaxBytes(maxBytes) {
#if GR_DEBUG
    fHighWaterEntryCount          = 0;
    fHighWaterUnlockedEntryCount  = 0;
    fHighWaterEntryBytes          = 0;
    fHighWaterClientDetachedCount = 0;
    fHighWaterClientDetachedBytes = 0;
#endif

    fEntryCount                   = 0;
    fUnlockedEntryCount           = 0;
    fEntryBytes                   = 0;
    fClientDetachedCount          = 0;
    fClientDetachedBytes          = 0;

    fPurging = false;
}

GrResourceCache::~GrResourceCache() {
    GrAutoResourceCacheValidate atcv(this);

    EntryList::Iter iter;

    // Unlike the removeAll, here we really remove everything, including locked resources.
    while (GrResourceEntry* entry = fList.head()) {
        GrAutoResourceCacheValidate atcv(this);

        // remove from our cache
        fCache.remove(entry->fKey, entry);

        // remove from our llist
        this->internalDetach(entry, false);

        delete entry;
    }
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
    fList.remove(entry);

    if (!entry->isLocked()) {
        --fUnlockedEntryCount;
    }

    // update our stats
    if (clientDetach) {
        fClientDetachedCount += 1;
        fClientDetachedBytes += entry->resource()->sizeInBytes();

#if GR_DEBUG
        if (fHighWaterClientDetachedCount < fClientDetachedCount) {
            fHighWaterClientDetachedCount = fClientDetachedCount;
        }
        if (fHighWaterClientDetachedBytes < fClientDetachedBytes) {
            fHighWaterClientDetachedBytes = fClientDetachedBytes;
        }
#endif

    } else {
        fEntryCount -= 1;
        fEntryBytes -= entry->resource()->sizeInBytes();
    }
}

void GrResourceCache::attachToHead(GrResourceEntry* entry,
                                  bool clientReattach) {
    fList.addToHead(entry);

    if (!entry->isLocked()) {
        ++fUnlockedEntryCount;
#if GR_DEBUG
        if (fHighWaterUnlockedEntryCount < fUnlockedEntryCount) {
            fHighWaterUnlockedEntryCount = fUnlockedEntryCount;
        }
#endif
    }

    // update our stats
    if (clientReattach) {
        fClientDetachedCount -= 1;
        fClientDetachedBytes -= entry->resource()->sizeInBytes();
    } else {
        fEntryCount += 1;
        fEntryBytes += entry->resource()->sizeInBytes();

#if GR_DEBUG
        if (fHighWaterEntryCount < fEntryCount) {
            fHighWaterEntryCount = fEntryCount;
        }
        if (fHighWaterEntryBytes < fEntryBytes) {
            fHighWaterEntryBytes = fEntryBytes;
        }
#endif
    }
}

GrResource* GrResourceCache::findAndLock(const GrResourceKey& key,
                                         LockType type) {
    GrAutoResourceCacheValidate atcv(this);

    GrResourceEntry* entry = fCache.find(key);
    if (NULL == entry) {
        return NULL;
    }

    this->internalDetach(entry, false);
    // mark the entry as "busy" so it doesn't get purged
    // do this between detach and attach for locked count tracking
    if (kNested_LockType == type || !entry->isLocked()) {
        entry->lock();
    }
    this->attachToHead(entry, false);

    return entry->fResource;
}

bool GrResourceCache::hasKey(const GrResourceKey& key) const {
    return NULL != fCache.find(key);
}

GrResourceEntry* GrResourceCache::create(const GrResourceKey& key,
                                         GrResource* resource,
                                         bool lock,
                                         bool clientReattach) {
    // we don't expect to create new resources during a purge. In theory
    // this could cause purgeAsNeeded() into an infinite loop (e.g.
    // each resource destroyed creates and locks 2 resources and
    // unlocks 1 thereby causing a new purge).
    GrAssert(!fPurging);
    GrAutoResourceCacheValidate atcv(this);

    GrResourceEntry* entry = SkNEW_ARGS(GrResourceEntry, (key, resource));
    resource->setCacheEntry(entry);

    if (lock) {
        // mark the entry as "busy" so it doesn't get purged
        // do this before attach for locked count tracking
        entry->lock();
    }

    this->attachToHead(entry, clientReattach);
    fCache.insert(key, entry);

#if GR_DUMP_TEXTURE_UPLOAD
    GrPrintf("--- add resource to cache %p, count=%d bytes= %d %d\n",
             entry, fEntryCount, resource->sizeInBytes(), fEntryBytes);
#endif

    this->purgeAsNeeded();
    return entry;
}

void GrResourceCache::createAndLock(const GrResourceKey& key,
                                    GrResource* resource) {
    GrAssert(NULL == resource->getCacheEntry());
    this->create(key, resource, true, false);
}

void GrResourceCache::attach(const GrResourceKey& key,
                             GrResource* resource) {
    GrAssert(NULL == resource->getCacheEntry());
    this->create(key, resource, false, true);
}

void GrResourceCache::makeExclusive(GrResourceEntry* entry) {
    GrAutoResourceCacheValidate atcv(this);

    this->internalDetach(entry, true);
    fCache.remove(entry->fKey, entry);

#if GR_DEBUG
    fExclusiveList.addToHead(entry);
#endif
}

void GrResourceCache::removeInvalidResource(GrResourceEntry* entry) {
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

void GrResourceCache::makeNonExclusive(GrResourceEntry* entry) {
    GrAutoResourceCacheValidate atcv(this);

#if GR_DEBUG
    fExclusiveList.remove(entry);
#endif

    if (entry->resource()->isValid()) {
        attachToHead(entry, true);
        fCache.insert(entry->key(), entry);
    } else {
        this->removeInvalidResource(entry);
    }
}

void GrResourceCache::unlock(GrResourceEntry* entry) {
    GrAutoResourceCacheValidate atcv(this);

    GrAssert(entry);
    GrAssert(entry->isLocked());
    GrAssert(fCache.find(entry->key()));

    entry->unlock();
    if (!entry->isLocked()) {
        ++fUnlockedEntryCount;
#if GR_DEBUG
        if (fHighWaterUnlockedEntryCount < fUnlockedEntryCount) {
            fHighWaterUnlockedEntryCount = fUnlockedEntryCount;
        }
#endif
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
            EntryList::Iter iter;

            // Note: the following code relies on the fact that the
            // doubly linked list doesn't invalidate its data/pointers
            // outside of the specific area where a deletion occurs (e.g.,
            // in internalDetach)
            GrResourceEntry* entry = iter.init(fList, EntryList::Iter::kTail_IterStart);

            while (entry && fUnlockedEntryCount) {
                GrAutoResourceCacheValidate atcv(this);
                if (fEntryCount <= fMaxCount && fEntryBytes <= fMaxBytes) {
                    withinBudget = true;
                    break;
                }

                GrResourceEntry* prev = iter.prev();
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

void GrResourceCache::purgeAllUnlocked() {
    GrAutoResourceCacheValidate atcv(this);

    // we can have one GrResource holding a lock on another
    // so we don't want to just do a simple loop kicking each
    // entry out. Instead change the budget and purge.

    int savedMaxBytes = fMaxBytes;
    int savedMaxCount = fMaxCount;
    fMaxBytes = (size_t) -1;
    fMaxCount = 0;
    this->purgeAsNeeded();

#if GR_DEBUG
    GrAssert(fExclusiveList.countEntries() == fClientDetachedCount);
    GrAssert(countBytes(fExclusiveList) == fClientDetachedBytes);
    GrAssert(!fUnlockedEntryCount);
    if (!fCache.count()) {
        // Items may have been detached from the cache (such as the backing
        // texture for an SkGpuDevice). The above purge would not have removed
        // them.
        GrAssert(fEntryCount == fClientDetachedCount);
        GrAssert(fEntryBytes == fClientDetachedBytes);
        GrAssert(fList.isEmpty());
    }
#endif

    fMaxBytes = savedMaxBytes;
    fMaxCount = savedMaxCount;
}

///////////////////////////////////////////////////////////////////////////////

#if GR_DEBUG
size_t GrResourceCache::countBytes(const EntryList& list) {
    size_t bytes = 0;

    EntryList::Iter iter;

    const GrResourceEntry* entry = iter.init(const_cast<EntryList&>(list),
                                             EntryList::Iter::kTail_IterStart);

    for ( ; NULL != entry; entry = iter.prev()) {
        bytes += entry->resource()->sizeInBytes();
    }
    return bytes;
}

static bool both_zero_or_nonzero(int count, size_t bytes) {
    return (count == 0 && bytes == 0) || (count > 0 && bytes > 0);
}

void GrResourceCache::validate() const {
    fList.validate();
    fExclusiveList.validate();
    GrAssert(both_zero_or_nonzero(fEntryCount, fEntryBytes));
    GrAssert(both_zero_or_nonzero(fClientDetachedCount, fClientDetachedBytes));
    GrAssert(fClientDetachedBytes <= fEntryBytes);
    GrAssert(fClientDetachedCount <= fEntryCount);
    GrAssert((fEntryCount - fClientDetachedCount) == fCache.count());

    fCache.validate();

    int count = 0;
    int unlockCount = 0;

    EntryList::Iter iter;

    const GrResourceEntry* entry = iter.init(const_cast<EntryList&>(fList),
                                             EntryList::Iter::kHead_IterStart);

    for ( ; NULL != entry; entry = iter.next()) {
        entry->validate();
        GrAssert(fCache.find(entry->key()));
        count += 1;
        if (!entry->isLocked()) {
            unlockCount += 1;
        }
    }
    GrAssert(count == fEntryCount - fClientDetachedCount);

    size_t bytes = countBytes(fList);
    GrAssert(bytes == fEntryBytes  - fClientDetachedBytes);

    bytes = countBytes(fExclusiveList);
    GrAssert(bytes == fClientDetachedBytes);

    GrAssert(unlockCount == fUnlockedEntryCount);

    GrAssert(fList.countEntries() == fEntryCount - fClientDetachedCount);

    GrAssert(fExclusiveList.countEntries() == fClientDetachedCount);
}

void GrResourceCache::printStats() const {
    SkDebugf("Budget: %d items %d bytes\n", fMaxCount, fMaxBytes);
    SkDebugf("\t\tEntry Count: current %d high %d\n",
                fEntryCount, fHighWaterEntryCount);
    SkDebugf("\t\tUnlocked Entry Count: current %d high %d\n",
                fUnlockedEntryCount, fHighWaterUnlockedEntryCount);
    SkDebugf("\t\tEntry Bytes: current %d high %d\n",
                fEntryBytes, fHighWaterEntryBytes);
    SkDebugf("\t\tDetached Entry Count: current %d high %d\n",
                fClientDetachedCount, fHighWaterClientDetachedCount);
    SkDebugf("\t\tDetached Bytes: current %d high %d\n",
                fClientDetachedBytes, fHighWaterClientDetachedBytes);
}

#endif

///////////////////////////////////////////////////////////////////////////////
