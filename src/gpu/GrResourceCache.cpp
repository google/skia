
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#include "GrResourceCache.h"
#include "GrResource.h"


GrResourceKey::ResourceType GrResourceKey::GenerateResourceType() {
    static int32_t gNextType = 0;

    int32_t type = sk_atomic_inc(&gNextType);
    if (type >= (1 << 8 * sizeof(ResourceType))) {
        GrCrash("Too many Resource Types");
    }

    return static_cast<ResourceType>(type);
}

///////////////////////////////////////////////////////////////////////////////

GrResourceEntry::GrResourceEntry(const GrResourceKey& key, GrResource* resource)
        : fKey(key), fResource(resource) {
    // we assume ownership of the resource, and will unref it when we die
    GrAssert(resource);
    resource->ref();
}

GrResourceEntry::~GrResourceEntry() {
    fResource->setCacheEntry(NULL);
    fResource->unref();
}

#if GR_DEBUG
void GrResourceEntry::validate() const {
    GrAssert(fResource);
    GrAssert(fResource->getCacheEntry() == this);
    fResource->validate();
}
#endif

///////////////////////////////////////////////////////////////////////////////

GrResourceCache::GrResourceCache(int maxCount, size_t maxBytes) :
        fMaxCount(maxCount),
        fMaxBytes(maxBytes) {
#if GR_CACHE_STATS
    fHighWaterEntryCount          = 0;
    fHighWaterEntryBytes          = 0;
    fHighWaterClientDetachedCount = 0;
    fHighWaterClientDetachedBytes = 0;
#endif

    fEntryCount                   = 0;
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
        this->internalDetach(entry);

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
                                     BudgetBehaviors behavior) {
    fList.remove(entry);

    // update our stats
    if (kIgnore_BudgetBehavior == behavior) {
        fClientDetachedCount += 1;
        fClientDetachedBytes += entry->resource()->sizeInBytes();

#if GR_CACHE_STATS
        if (fHighWaterClientDetachedCount < fClientDetachedCount) {
            fHighWaterClientDetachedCount = fClientDetachedCount;
        }
        if (fHighWaterClientDetachedBytes < fClientDetachedBytes) {
            fHighWaterClientDetachedBytes = fClientDetachedBytes;
        }
#endif

    } else {
        GrAssert(kAccountFor_BudgetBehavior == behavior);

        fEntryCount -= 1;
        fEntryBytes -= entry->resource()->sizeInBytes();
    }
}

void GrResourceCache::attachToHead(GrResourceEntry* entry,
                                   BudgetBehaviors behavior) {
    fList.addToHead(entry);

    // update our stats
    if (kIgnore_BudgetBehavior == behavior) {
        fClientDetachedCount -= 1;
        fClientDetachedBytes -= entry->resource()->sizeInBytes();
    } else {
        GrAssert(kAccountFor_BudgetBehavior == behavior);

        fEntryCount += 1;
        fEntryBytes += entry->resource()->sizeInBytes();

#if GR_CACHE_STATS
        if (fHighWaterEntryCount < fEntryCount) {
            fHighWaterEntryCount = fEntryCount;
        }
        if (fHighWaterEntryBytes < fEntryBytes) {
            fHighWaterEntryBytes = fEntryBytes;
        }
#endif
    }
}

// This functor just searches for an entry with only a single ref (from
// the texture cache itself). Presumably in this situation no one else
// is relying on the texture.
class GrTFindUnreffedFunctor {
public:
    bool operator()(const GrResourceEntry* entry) const {
        return 1 == entry->resource()->getRefCnt();
    }
};

GrResource* GrResourceCache::find(const GrResourceKey& key, uint32_t ownershipFlags) {
    GrAutoResourceCacheValidate atcv(this);

    GrResourceEntry* entry = NULL;

    if (ownershipFlags & kNoOtherOwners_OwnershipFlag) {
        GrTFindUnreffedFunctor functor;

        entry = fCache.find<GrTFindUnreffedFunctor>(key, functor);
    } else {
        entry = fCache.find(key);
    }

    if (NULL == entry) {
        return NULL;
    }

    if (ownershipFlags & kHide_OwnershipFlag) {
        this->makeExclusive(entry);
    } else {
        // Make this resource MRU
        this->internalDetach(entry);
        this->attachToHead(entry);
    }

    return entry->fResource;
}

bool GrResourceCache::hasKey(const GrResourceKey& key) const {
    return NULL != fCache.find(key);
}

void GrResourceCache::addResource(const GrResourceKey& key,
                                  GrResource* resource,
                                  uint32_t ownershipFlags) {
    GrAssert(NULL == resource->getCacheEntry());
    // we don't expect to create new resources during a purge. In theory
    // this could cause purgeAsNeeded() into an infinite loop (e.g.
    // each resource destroyed creates and locks 2 resources and
    // unlocks 1 thereby causing a new purge).
    GrAssert(!fPurging);
    GrAutoResourceCacheValidate atcv(this);

    GrResourceEntry* entry = SkNEW_ARGS(GrResourceEntry, (key, resource));
    resource->setCacheEntry(entry);

    this->attachToHead(entry);
    fCache.insert(key, entry);

    if (ownershipFlags & kHide_OwnershipFlag) {
        this->makeExclusive(entry);
    }

}

void GrResourceCache::makeExclusive(GrResourceEntry* entry) {
    GrAutoResourceCacheValidate atcv(this);

    // When scratch textures are detached (to hide them from future finds) they
    // still count against the resource budget
    this->internalDetach(entry, kIgnore_BudgetBehavior);
    fCache.remove(entry->key(), entry);

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
        // Since scratch textures still count against the cache budget even
        // when they have been removed from the cache, re-adding them doesn't
        // alter the budget information.
        attachToHead(entry, kIgnore_BudgetBehavior);
        fCache.insert(entry->key(), entry);
    } else {
        this->removeInvalidResource(entry);
    }
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
        bool changed = false;

        // The purging process is repeated several times since one pass
        // may free up other resources
        do {
            EntryList::Iter iter;

            changed = false;

            // Note: the following code relies on the fact that the
            // doubly linked list doesn't invalidate its data/pointers
            // outside of the specific area where a deletion occurs (e.g.,
            // in internalDetach)
            GrResourceEntry* entry = iter.init(fList, EntryList::Iter::kTail_IterStart);

            while (NULL != entry) {
                GrAutoResourceCacheValidate atcv(this);

                if (fEntryCount <= fMaxCount && fEntryBytes <= fMaxBytes) {
                    withinBudget = true;
                    break;
                }

                GrResourceEntry* prev = iter.prev();
                if (1 == entry->fResource->getRefCnt()) {
                    changed = true;

                    // remove from our cache
                    fCache.remove(entry->key(), entry);

                    // remove from our llist
                    this->internalDetach(entry);
                    delete entry;
                }
                entry = prev;
            }
        } while (!withinBudget && changed);
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


    EntryList::Iter iter;

    // check that the exclusively held entries are okay
    const GrResourceEntry* entry = iter.init(const_cast<EntryList&>(fExclusiveList),
                                             EntryList::Iter::kHead_IterStart);

    for ( ; NULL != entry; entry = iter.next()) {
        entry->validate();
    }

    // check that the shareable entries are okay
    entry = iter.init(const_cast<EntryList&>(fList), EntryList::Iter::kHead_IterStart);

    int count = 0;
    for ( ; NULL != entry; entry = iter.next()) {
        entry->validate();
        GrAssert(fCache.find(entry->key()));
        count += 1;
    }
    GrAssert(count == fEntryCount - fClientDetachedCount);

    size_t bytes = countBytes(fList);
    GrAssert(bytes == fEntryBytes  - fClientDetachedBytes);

    bytes = countBytes(fExclusiveList);
    GrAssert(bytes == fClientDetachedBytes);

    GrAssert(fList.countEntries() == fEntryCount - fClientDetachedCount);

    GrAssert(fExclusiveList.countEntries() == fClientDetachedCount);
}
#endif // GR_DEBUG

#if GR_CACHE_STATS

void GrResourceCache::printStats() {
    int locked = 0;

    EntryList::Iter iter;

    GrResourceEntry* entry = iter.init(fList, EntryList::Iter::kTail_IterStart);

    for ( ; NULL != entry; entry = iter.prev()) {
        if (entry->fResource->getRefCnt() > 1) {
            ++locked;
        }
    }

    SkDebugf("Budget: %d items %d bytes\n", fMaxCount, fMaxBytes);
    SkDebugf("\t\tEntry Count: current %d (%d locked) high %d\n",
                fEntryCount, locked, fHighWaterEntryCount);
    SkDebugf("\t\tEntry Bytes: current %d high %d\n",
                fEntryBytes, fHighWaterEntryBytes);
    SkDebugf("\t\tDetached Entry Count: current %d high %d\n",
                fClientDetachedCount, fHighWaterClientDetachedCount);
    SkDebugf("\t\tDetached Bytes: current %d high %d\n",
                fClientDetachedBytes, fHighWaterClientDetachedBytes);
}

#endif

///////////////////////////////////////////////////////////////////////////////
