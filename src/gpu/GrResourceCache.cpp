
/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrResourceCache.h"
#include "GrGpuResourceCacheAccess.h"
#include "SkChecksum.h"
#include "SkGr.h"
#include "SkMessageBus.h"

DECLARE_SKMESSAGEBUS_MESSAGE(GrContentKeyInvalidatedMessage);

//////////////////////////////////////////////////////////////////////////////

GrScratchKey::ResourceType GrScratchKey::GenerateResourceType() {
    static int32_t gType = INHERITED::kInvalidDomain + 1;

    int32_t type = sk_atomic_inc(&gType);
    if (type > SK_MaxU16) {
        SkFAIL("Too many Resource Types");
    }

    return static_cast<ResourceType>(type);
}

GrContentKey::Domain GrContentKey::GenerateDomain() {
    static int32_t gDomain = INHERITED::kInvalidDomain + 1;

    int32_t domain = sk_atomic_inc(&gDomain);
    if (domain > SK_MaxU16) {
        SkFAIL("Too many Content Key Domains");
    }

    return static_cast<Domain>(domain);
}
uint32_t GrResourceKeyHash(const uint32_t* data, size_t size) {
    return SkChecksum::Compute(data, size);
}

//////////////////////////////////////////////////////////////////////////////

class GrResourceCache::AutoValidate : ::SkNoncopyable {
public:
    AutoValidate(GrResourceCache* cache) : fCache(cache) { cache->validate(); }
    ~AutoValidate() { fCache->validate(); }
private:
    GrResourceCache* fCache;
};

 //////////////////////////////////////////////////////////////////////////////

static const int kDefaultMaxCount = 2 * (1 << 10);
static const size_t kDefaultMaxSize = 96 * (1 << 20);

GrResourceCache::GrResourceCache()
    : fTimestamp(0)
    , fMaxCount(kDefaultMaxCount)
    , fMaxBytes(kDefaultMaxSize)
#if GR_CACHE_STATS
    , fHighWaterCount(0)
    , fHighWaterBytes(0)
    , fBudgetedHighWaterCount(0)
    , fBudgetedHighWaterBytes(0)
#endif
    , fBytes(0)
    , fBudgetedCount(0)
    , fBudgetedBytes(0)
    , fOverBudgetCB(NULL)
    , fOverBudgetData(NULL) {
    SkDEBUGCODE(fCount = 0;)
}

GrResourceCache::~GrResourceCache() {
    this->releaseAll();
}

void GrResourceCache::setLimits(int count, size_t bytes) {
    fMaxCount = count;
    fMaxBytes = bytes;
    this->purgeAsNeeded();
}

void GrResourceCache::insertResource(GrGpuResource* resource) {
    SkASSERT(resource);
    SkASSERT(!this->isInCache(resource));
    SkASSERT(!resource->wasDestroyed());
    SkASSERT(!resource->isPurgeable());
    this->addToNonpurgeableArray(resource);

    size_t size = resource->gpuMemorySize();
    SkDEBUGCODE(++fCount;)
    fBytes += size;
#if GR_CACHE_STATS
    fHighWaterCount = SkTMax(this->getResourceCount(), fHighWaterCount);
    fHighWaterBytes = SkTMax(fBytes, fHighWaterBytes);
#endif
    if (resource->resourcePriv().isBudgeted()) {
        ++fBudgetedCount;
        fBudgetedBytes += size;
#if GR_CACHE_STATS
        fBudgetedHighWaterCount = SkTMax(fBudgetedCount, fBudgetedHighWaterCount);
        fBudgetedHighWaterBytes = SkTMax(fBudgetedBytes, fBudgetedHighWaterBytes);
#endif
    }
    if (resource->resourcePriv().getScratchKey().isValid()) {
        SkASSERT(!resource->cacheAccess().isWrapped());
        fScratchMap.insert(resource->resourcePriv().getScratchKey(), resource);
    }

    resource->cacheAccess().setTimestamp(fTimestamp++);

    this->purgeAsNeeded();
}

void GrResourceCache::removeResource(GrGpuResource* resource) {
    this->validate();
    SkASSERT(this->isInCache(resource));

    if (resource->isPurgeable()) {
        fPurgeableQueue.remove(resource);
    } else {
        this->removeFromNonpurgeableArray(resource);
    }

    size_t size = resource->gpuMemorySize();
    SkDEBUGCODE(--fCount;)
    fBytes -= size;
    if (resource->resourcePriv().isBudgeted()) {
        --fBudgetedCount;
        fBudgetedBytes -= size;
    }

    if (resource->resourcePriv().getScratchKey().isValid()) {
        fScratchMap.remove(resource->resourcePriv().getScratchKey(), resource);
    }
    if (resource->getContentKey().isValid()) {
        fContentHash.remove(resource->getContentKey());
    }
    this->validate();
}

void GrResourceCache::abandonAll() {
    AutoValidate av(this);

    while (fNonpurgeableResources.count()) {
        GrGpuResource* back = *(fNonpurgeableResources.end() - 1);
        SkASSERT(!back->wasDestroyed());
        back->cacheAccess().abandon();
    }

    while (fPurgeableQueue.count()) {
        GrGpuResource* top = fPurgeableQueue.peek();
        SkASSERT(!top->wasDestroyed());
        top->cacheAccess().abandon();
    }

    SkASSERT(!fScratchMap.count());
    SkASSERT(!fContentHash.count());
    SkASSERT(!fCount);
    SkASSERT(!this->getResourceCount());
    SkASSERT(!fBytes);
    SkASSERT(!fBudgetedCount);
    SkASSERT(!fBudgetedBytes);
}

void GrResourceCache::releaseAll() {
    AutoValidate av(this);

    while(fNonpurgeableResources.count()) {
        GrGpuResource* back = *(fNonpurgeableResources.end() - 1);
        SkASSERT(!back->wasDestroyed());
        back->cacheAccess().release();
    }

    while (fPurgeableQueue.count()) {
        GrGpuResource* top = fPurgeableQueue.peek();
        SkASSERT(!top->wasDestroyed());
        top->cacheAccess().release();
    }

    SkASSERT(!fScratchMap.count());
    SkASSERT(!fContentHash.count());
    SkASSERT(!fCount);
    SkASSERT(!this->getResourceCount());
    SkASSERT(!fBytes);
    SkASSERT(!fBudgetedCount);
    SkASSERT(!fBudgetedBytes);
}

class GrResourceCache::AvailableForScratchUse {
public:
    AvailableForScratchUse(bool rejectPendingIO) : fRejectPendingIO(rejectPendingIO) { }

    bool operator()(const GrGpuResource* resource) const {
        if (resource->internalHasRef() || !resource->cacheAccess().isScratch()) {
            return false;
        }
        return !fRejectPendingIO || !resource->internalHasPendingIO();
    }

private:
    bool fRejectPendingIO;
};

GrGpuResource* GrResourceCache::findAndRefScratchResource(const GrScratchKey& scratchKey,
                                                          uint32_t flags) {
    SkASSERT(scratchKey.isValid());

    GrGpuResource* resource;
    if (flags & (kPreferNoPendingIO_ScratchFlag | kRequireNoPendingIO_ScratchFlag)) {
        resource = fScratchMap.find(scratchKey, AvailableForScratchUse(true));
        if (resource) {
            this->refAndMakeResourceMRU(resource);
            this->validate();
            return resource;
        } else if (flags & kRequireNoPendingIO_ScratchFlag) {
            return NULL;
        }
        // TODO: fail here when kPrefer is specified, we didn't find a resource without pending io,
        // but there is still space in our budget for the resource.
    }
    resource = fScratchMap.find(scratchKey, AvailableForScratchUse(false));
    if (resource) {
        this->refAndMakeResourceMRU(resource);
        this->validate();
    }
    return resource;
}

void GrResourceCache::willRemoveScratchKey(const GrGpuResource* resource) {
    SkASSERT(resource->resourcePriv().getScratchKey().isValid());
    fScratchMap.remove(resource->resourcePriv().getScratchKey(), resource);
}

void GrResourceCache::willRemoveContentKey(const GrGpuResource* resource) {
    // Someone has a ref to this resource in order to invalidate it. When the ref count reaches
    // zero we will get a notifyPurgable() and figure out what to do with it.
    SkASSERT(resource->getContentKey().isValid());
    fContentHash.remove(resource->getContentKey());
}

bool GrResourceCache::didSetContentKey(GrGpuResource* resource) {
    SkASSERT(resource);
    SkASSERT(this->isInCache(resource));
    SkASSERT(resource->getContentKey().isValid());

    GrGpuResource* res = fContentHash.find(resource->getContentKey());
    if (NULL != res) {
        return false;
    }

    fContentHash.add(resource);
    this->validate();
    return true;
}

void GrResourceCache::refAndMakeResourceMRU(GrGpuResource* resource) {
    SkASSERT(resource);
    SkASSERT(this->isInCache(resource));
    if (resource->isPurgeable()) {
        // It's about to become unpurgeable.
        fPurgeableQueue.remove(resource);
        this->addToNonpurgeableArray(resource);
    }
    resource->ref();
    resource->cacheAccess().setTimestamp(fTimestamp++);
    this->validate();
}

void GrResourceCache::notifyPurgeable(GrGpuResource* resource) {
    SkASSERT(resource);
    SkASSERT(this->isInCache(resource));
    SkASSERT(resource->isPurgeable());

    this->removeFromNonpurgeableArray(resource);
    fPurgeableQueue.insert(resource);

    if (!resource->resourcePriv().isBudgeted()) {
        // Check whether this resource could still be used as a scratch resource.
        if (!resource->cacheAccess().isWrapped() &&
            resource->resourcePriv().getScratchKey().isValid()) {
            // We won't purge an existing resource to make room for this one.
            if (fBudgetedCount < fMaxCount &&
                fBudgetedBytes + resource->gpuMemorySize() <= fMaxBytes) {
                resource->resourcePriv().makeBudgeted();
                return;
            }
        }
    } else {
        // Purge the resource immediately if we're over budget
        // Also purge if the resource has neither a valid scratch key nor a content key.
        bool noKey = !resource->resourcePriv().getScratchKey().isValid() &&
                     !resource->getContentKey().isValid();
        if (!this->overBudget() && !noKey) {
            return;
        }
    }

    SkDEBUGCODE(int beforeCount = this->getResourceCount();)
    resource->cacheAccess().release();
    // We should at least free this resource, perhaps dependent resources as well.
    SkASSERT(this->getResourceCount() < beforeCount);
    this->validate();
}

void GrResourceCache::didChangeGpuMemorySize(const GrGpuResource* resource, size_t oldSize) {
    // SkASSERT(!fPurging); GrPathRange increases size during flush. :(
    SkASSERT(resource);
    SkASSERT(this->isInCache(resource));

    ptrdiff_t delta = resource->gpuMemorySize() - oldSize;

    fBytes += delta;
#if GR_CACHE_STATS
    fHighWaterBytes = SkTMax(fBytes, fHighWaterBytes);
#endif
    if (resource->resourcePriv().isBudgeted()) {
        fBudgetedBytes += delta;
#if GR_CACHE_STATS
        fBudgetedHighWaterBytes = SkTMax(fBudgetedBytes, fBudgetedHighWaterBytes);
#endif
    }

    this->purgeAsNeeded();
    this->validate();
}

void GrResourceCache::didChangeBudgetStatus(GrGpuResource* resource) {
    SkASSERT(resource);
    SkASSERT(this->isInCache(resource));

    size_t size = resource->gpuMemorySize();

    if (resource->resourcePriv().isBudgeted()) {
        ++fBudgetedCount;
        fBudgetedBytes += size;
#if GR_CACHE_STATS
        fBudgetedHighWaterBytes = SkTMax(fBudgetedBytes, fBudgetedHighWaterBytes);
        fBudgetedHighWaterCount = SkTMax(fBudgetedCount, fBudgetedHighWaterCount);
#endif
        this->purgeAsNeeded();
    } else {
        --fBudgetedCount;
        fBudgetedBytes -= size;
    }

    this->validate();
}

void GrResourceCache::internalPurgeAsNeeded() {
    SkASSERT(this->overBudget());

    bool stillOverbudget = true;
    while (fPurgeableQueue.count()) {
        GrGpuResource* resource = fPurgeableQueue.peek();
        SkASSERT(resource->isPurgeable());
        resource->cacheAccess().release();
        if (!this->overBudget()) {
            stillOverbudget = false;
            break;
        }
    }

    this->validate();

    if (stillOverbudget) {
        // Despite the purge we're still over budget. Call our over budget callback. If this frees
        // any resources then we'll get notifyPurgeable() calls and take appropriate action.
        (*fOverBudgetCB)(fOverBudgetData);
        this->validate();
    }
}

void GrResourceCache::purgeAllUnlocked() {
    // We could disable maintaining the heap property here, but it would add a lot of complexity.
    // Moreover, this is rarely called.
    while (fPurgeableQueue.count()) {
        GrGpuResource* resource = fPurgeableQueue.peek();
        SkASSERT(resource->isPurgeable());
        resource->cacheAccess().release();
    }

    this->validate();
}

void GrResourceCache::processInvalidContentKeys(
    const SkTArray<GrContentKeyInvalidatedMessage>& msgs) {
    for (int i = 0; i < msgs.count(); ++i) {
        GrGpuResource* resource = this->findAndRefContentResource(msgs[i].key());
        if (resource) {
            resource->resourcePriv().removeContentKey();
            resource->unref(); // will call notifyPurgeable, if it is indeed now purgeable.
        }
    }
}

void GrResourceCache::addToNonpurgeableArray(GrGpuResource* resource) {
    int index = fNonpurgeableResources.count();
    *fNonpurgeableResources.append() = resource;
    *resource->cacheAccess().accessCacheIndex() = index;
}

void GrResourceCache::removeFromNonpurgeableArray(GrGpuResource* resource) {
    int* index = resource->cacheAccess().accessCacheIndex();
    // Fill the whole we will create in the array with the tail object, adjust its index, and
    // then pop the array
    GrGpuResource* tail = *(fNonpurgeableResources.end() - 1);
    SkASSERT(fNonpurgeableResources[*index] == resource);
    fNonpurgeableResources[*index] = tail;
    *tail->cacheAccess().accessCacheIndex() = *index;
    fNonpurgeableResources.pop();
    SkDEBUGCODE(*index = -1);
}

#ifdef SK_DEBUG
void GrResourceCache::validate() const {
    // Reduce the frequency of validations for large resource counts.
    static SkRandom gRandom;
    int mask = (SkNextPow2(fCount + 1) >> 5) - 1;
    if (~mask && (gRandom.nextU() & mask)) {
        return;
    }

    struct Stats {
        size_t fBytes;
        int fBudgetedCount;
        size_t fBudgetedBytes;
        int fLocked;
        int fScratch;
        int fCouldBeScratch;
        int fContent;
        const ScratchMap* fScratchMap;
        const ContentHash* fContentHash;

        Stats(const GrResourceCache* cache) {
            memset(this, 0, sizeof(*this));
            fScratchMap = &cache->fScratchMap;
            fContentHash = &cache->fContentHash;
        }

        void update(GrGpuResource* resource) {
            fBytes += resource->gpuMemorySize();

            if (!resource->isPurgeable()) {
                ++fLocked;
            }

            if (resource->cacheAccess().isScratch()) {
                SkASSERT(!resource->getContentKey().isValid());
                ++fScratch;
                SkASSERT(fScratchMap->countForKey(resource->resourcePriv().getScratchKey()));
                SkASSERT(!resource->cacheAccess().isWrapped());
            } else if (resource->resourcePriv().getScratchKey().isValid()) {
                SkASSERT(!resource->resourcePriv().isBudgeted() ||
                         resource->getContentKey().isValid());
                ++fCouldBeScratch;
                SkASSERT(fScratchMap->countForKey(resource->resourcePriv().getScratchKey()));
                SkASSERT(!resource->cacheAccess().isWrapped());
            }
            const GrContentKey& contentKey = resource->getContentKey();
            if (contentKey.isValid()) {
                ++fContent;
                SkASSERT(fContentHash->find(contentKey) == resource);
                SkASSERT(!resource->cacheAccess().isWrapped());
                SkASSERT(resource->resourcePriv().isBudgeted());
            }

            if (resource->resourcePriv().isBudgeted()) {
                ++fBudgetedCount;
                fBudgetedBytes += resource->gpuMemorySize();
            }
        }
    };

    Stats stats(this);

    for (int i = 0; i < fNonpurgeableResources.count(); ++i) {
        SkASSERT(!fNonpurgeableResources[i]->isPurgeable());
        SkASSERT(*fNonpurgeableResources[i]->cacheAccess().accessCacheIndex() == i);
        SkASSERT(!fNonpurgeableResources[i]->wasDestroyed());
        stats.update(fNonpurgeableResources[i]);
    }
    for (int i = 0; i < fPurgeableQueue.count(); ++i) {
        SkASSERT(fPurgeableQueue.at(i)->isPurgeable());
        SkASSERT(*fPurgeableQueue.at(i)->cacheAccess().accessCacheIndex() == i);
        SkASSERT(!fPurgeableQueue.at(i)->wasDestroyed());
        stats.update(fPurgeableQueue.at(i));
    }

    SkASSERT(fCount == this->getResourceCount());
    SkASSERT(fBudgetedCount <= fCount);
    SkASSERT(fBudgetedBytes <= fBytes);
    SkASSERT(stats.fBytes == fBytes);
    SkASSERT(stats.fBudgetedBytes == fBudgetedBytes);
    SkASSERT(stats.fBudgetedCount == fBudgetedCount);
#if GR_CACHE_STATS
    SkASSERT(fBudgetedHighWaterCount <= fHighWaterCount);
    SkASSERT(fBudgetedHighWaterBytes <= fHighWaterBytes);
    SkASSERT(fBytes <= fHighWaterBytes);
    SkASSERT(fCount <= fHighWaterCount);
    SkASSERT(fBudgetedBytes <= fBudgetedHighWaterBytes);
    SkASSERT(fBudgetedCount <= fBudgetedHighWaterCount);
#endif
    SkASSERT(stats.fContent == fContentHash.count());
    SkASSERT(stats.fScratch + stats.fCouldBeScratch == fScratchMap.count());

    // This assertion is not currently valid because we can be in recursive notifyIsPurgeable()
    // calls. This will be fixed when subresource registration is explicit.
    // bool overBudget = budgetedBytes > fMaxBytes || budgetedCount > fMaxCount;
    // SkASSERT(!overBudget || locked == count || fPurging);
}

bool GrResourceCache::isInCache(const GrGpuResource* resource) const {
    int index = *resource->cacheAccess().accessCacheIndex();
    if (index < 0) {
        return false;
    }
    if (index < fPurgeableQueue.count() && fPurgeableQueue.at(index) == resource) {
        return true;
    }
    if (index < fNonpurgeableResources.count() && fNonpurgeableResources[index] == resource) {
        return true;
    }
    SkDEBUGFAIL("Resource index should be -1 or the resource should be in the cache.");
    return false;
}

#endif
