
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
    fEntryBytes          = 0;
    fClientDetachedCount = 0;
    fClientDetachedBytes = 0;

    fHead = fTail = NULL;
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

GrResourceEntry* GrResourceCache::findAndLock(const GrResourceKey& key) {
    GrAutoResourceCacheValidate atcv(this);

    GrResourceEntry* entry = fCache.find(key);
    if (entry) {
        this->internalDetach(entry, false);
        this->attachToHead(entry, false);
        // mark the entry as "busy" so it doesn't get purged
        entry->lock();
    }
    return entry;
}

GrResourceEntry* GrResourceCache::createAndLock(const GrResourceKey& key,
                                              GrResource* resource) {
    GrAutoResourceCacheValidate atcv(this);

    GrResourceEntry* entry = new GrResourceEntry(key, resource);

    this->attachToHead(entry, false);
    fCache.insert(key, entry);

#if GR_DUMP_TEXTURE_UPLOAD
    GrPrintf("--- add resource to cache %p, count=%d bytes= %d %d\n",
             entry, fEntryCount, resource->sizeInBytes(), fEntryBytes);
#endif

    // mark the entry as "busy" so it doesn't get purged
    entry->lock();
    this->purgeAsNeeded();
    return entry;
}

void GrResourceCache::detach(GrResourceEntry* entry) {
    internalDetach(entry, true);
    fCache.remove(entry->fKey, entry);
}

void GrResourceCache::reattachAndUnlock(GrResourceEntry* entry) {
    attachToHead(entry, true);
    fCache.insert(entry->key(), entry);
    unlock(entry);
}

void GrResourceCache::unlock(GrResourceEntry* entry) {
    GrAutoResourceCacheValidate atcv(this);

    GrAssert(entry);
    GrAssert(entry->isLocked());
    GrAssert(fCache.find(entry->key()));

    entry->unlock();
    this->purgeAsNeeded();
}

void GrResourceCache::purgeAsNeeded() {
    GrAutoResourceCacheValidate atcv(this);

    GrResourceEntry* entry = fTail;
    while (entry) {
        if (fEntryCount <= fMaxCount && fEntryBytes <= fMaxBytes) {
            break;
        }

        GrResourceEntry* prev = entry->fPrev;
        if (!entry->isLocked()) {
            // remove from our cache
            fCache.remove(entry->fKey, entry);

            // remove from our llist
            this->internalDetach(entry, false);

#if GR_DUMP_TEXTURE_UPLOAD
            GrPrintf("--- ~resource from cache %p [%d %d]\n", entry->resource(),
                     entry->resource()->width(),
                     entry->resource()->height());
#endif
            delete entry;
        }
        entry = prev;
    }
}

void GrResourceCache::removeAll() {
    GrAssert(!fClientDetachedCount);
    GrAssert(!fClientDetachedBytes);

    GrResourceEntry* entry = fHead;
    while (entry) {
        GrAssert(!entry->isLocked());

        GrResourceEntry* next = entry->fNext;
        delete entry;
        entry = next;
    }

    fCache.removeAll();
    fHead = fTail = NULL;
    fEntryCount = 0;
    fEntryBytes = 0;
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
    size_t bytes = 0;
    while (entry) {
        entry->validate();
        GrAssert(fCache.find(entry->key()));
        count += 1;
        bytes += entry->resource()->sizeInBytes();
        entry = entry->fNext;
    }
    GrAssert(count == fEntryCount - fClientDetachedCount);
    GrAssert(bytes == fEntryBytes  - fClientDetachedBytes);

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


