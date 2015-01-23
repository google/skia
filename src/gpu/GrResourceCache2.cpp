
/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrResourceCache2.h"
#include "GrGpuResource.h"  

#include "SkChecksum.h"
#include "SkGr.h"
#include "SkMessageBus.h"

DECLARE_SKMESSAGEBUS_MESSAGE(GrResourceInvalidatedMessage);

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
    if (kInvalidDomain == gDomain) {
        SkFAIL("Too many Content Key Domains");
    }

    return static_cast<Domain>(domain);
}
uint32_t GrResourceKeyHash(const uint32_t* data, size_t size) {
    return SkChecksum::Compute(data, size);
}

//////////////////////////////////////////////////////////////////////////////

class GrResourceCache2::AutoValidate : ::SkNoncopyable {
public:
    AutoValidate(GrResourceCache2* cache) : fCache(cache) { cache->validate(); }
    ~AutoValidate() { fCache->validate(); }
private:
    GrResourceCache2* fCache;
};

 //////////////////////////////////////////////////////////////////////////////

static const int kDefaultMaxCount = 2 * (1 << 10);
static const size_t kDefaultMaxSize = 96 * (1 << 20);

GrResourceCache2::GrResourceCache2()
    : fMaxCount(kDefaultMaxCount)
    , fMaxBytes(kDefaultMaxSize)
#if GR_CACHE_STATS
    , fHighWaterCount(0)
    , fHighWaterBytes(0)
    , fBudgetedHighWaterCount(0)
    , fBudgetedHighWaterBytes(0)
#endif
    , fCount(0)
    , fBytes(0)
    , fBudgetedCount(0)
    , fBudgetedBytes(0)
    , fPurging(false)
    , fNewlyPurgableResourceWhilePurging(false)
    , fOverBudgetCB(NULL)
    , fOverBudgetData(NULL) {
}

GrResourceCache2::~GrResourceCache2() {
    this->releaseAll();
}

void GrResourceCache2::setLimits(int count, size_t bytes) {
    fMaxCount = count;
    fMaxBytes = bytes;
    this->purgeAsNeeded();
}

void GrResourceCache2::insertResource(GrGpuResource* resource) {
    SkASSERT(resource);
    SkASSERT(!resource->wasDestroyed());
    SkASSERT(!this->isInCache(resource));
    SkASSERT(!fPurging);
    fResources.addToHead(resource);

    size_t size = resource->gpuMemorySize();
    ++fCount;
    fBytes += size;
#if GR_CACHE_STATS
    fHighWaterCount = SkTMax(fCount, fHighWaterCount);
    fHighWaterBytes = SkTMax(fBytes, fHighWaterBytes);
#endif
    if (resource->cacheAccess().isBudgeted()) {
        ++fBudgetedCount;
        fBudgetedBytes += size;
#if GR_CACHE_STATS
        fBudgetedHighWaterCount = SkTMax(fBudgetedCount, fBudgetedHighWaterCount);
        fBudgetedHighWaterBytes = SkTMax(fBudgetedBytes, fBudgetedHighWaterBytes);
#endif
    }
    if (resource->cacheAccess().getScratchKey().isValid()) {
        SkASSERT(!resource->cacheAccess().isWrapped());
        fScratchMap.insert(resource->cacheAccess().getScratchKey(), resource);
    }
    
    this->purgeAsNeeded();
}

void GrResourceCache2::removeResource(GrGpuResource* resource) {
    SkASSERT(this->isInCache(resource));

    size_t size = resource->gpuMemorySize();
    --fCount;
    fBytes -= size;
    if (resource->cacheAccess().isBudgeted()) {
        --fBudgetedCount;
        fBudgetedBytes -= size;
    }

    fResources.remove(resource);
    if (resource->cacheAccess().getScratchKey().isValid()) {
        fScratchMap.remove(resource->cacheAccess().getScratchKey(), resource);
    }
    if (resource->cacheAccess().getContentKey().isValid()) {
        fContentHash.remove(resource->cacheAccess().getContentKey());
    }
    this->validate();
}

void GrResourceCache2::abandonAll() {
    AutoValidate av(this);

    SkASSERT(!fPurging);
    while (GrGpuResource* head = fResources.head()) {
        SkASSERT(!head->wasDestroyed());
        head->cacheAccess().abandon();
        // abandon should have already removed this from the list.
        SkASSERT(head != fResources.head());
    }
    SkASSERT(!fScratchMap.count());
    SkASSERT(!fContentHash.count());
    SkASSERT(!fCount);
    SkASSERT(!fBytes);
    SkASSERT(!fBudgetedCount);
    SkASSERT(!fBudgetedBytes);
}

void GrResourceCache2::releaseAll() {
    AutoValidate av(this);

    SkASSERT(!fPurging);
    while (GrGpuResource* head = fResources.head()) {
        SkASSERT(!head->wasDestroyed());
        head->cacheAccess().release();
        // release should have already removed this from the list.
        SkASSERT(head != fResources.head());
    }
    SkASSERT(!fScratchMap.count());
    SkASSERT(!fCount);
    SkASSERT(!fBytes);
    SkASSERT(!fBudgetedCount);
    SkASSERT(!fBudgetedBytes);
}

class GrResourceCache2::AvailableForScratchUse {
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

GrGpuResource* GrResourceCache2::findAndRefScratchResource(const GrScratchKey& scratchKey,
                                                           uint32_t flags) {
    SkASSERT(!fPurging);
    SkASSERT(scratchKey.isValid());

    GrGpuResource* resource;
    if (flags & (kPreferNoPendingIO_ScratchFlag | kRequireNoPendingIO_ScratchFlag)) {
        resource = fScratchMap.find(scratchKey, AvailableForScratchUse(true));
        if (resource) {
            resource->ref();
            this->makeResourceMRU(resource);
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
        resource->ref();
        this->makeResourceMRU(resource);
        this->validate();
    }
    return resource;
}

void GrResourceCache2::willRemoveScratchKey(const GrGpuResource* resource) {
    SkASSERT(resource->cacheAccess().isScratch());
    fScratchMap.remove(resource->cacheAccess().getScratchKey(), resource);
}

bool GrResourceCache2::didSetContentKey(GrGpuResource* resource) {
    SkASSERT(!fPurging);
    SkASSERT(resource);
    SkASSERT(this->isInCache(resource));
    SkASSERT(resource->cacheAccess().getContentKey().isValid());

    GrGpuResource* res = fContentHash.find(resource->cacheAccess().getContentKey());
    if (NULL != res) {
        return false;
    }

    fContentHash.add(resource);
    this->validate();
    return true;
}

void GrResourceCache2::makeResourceMRU(GrGpuResource* resource) {
    SkASSERT(!fPurging);
    SkASSERT(resource);
    SkASSERT(this->isInCache(resource));
    fResources.remove(resource);    
    fResources.addToHead(resource);
}

void GrResourceCache2::notifyPurgable(GrGpuResource* resource) {
    SkASSERT(resource);
    SkASSERT(this->isInCache(resource));
    SkASSERT(resource->isPurgable());

    // We can't purge if in the middle of purging because purge is iterating. Instead record
    // that additional resources became purgable.
    if (fPurging) {
        fNewlyPurgableResourceWhilePurging = true;
        return;
    }

    // Purge the resource if we're over budget
    bool overBudget = fBudgetedCount > fMaxCount || fBudgetedBytes > fMaxBytes;

    // Also purge if the resource has neither a valid scratch key nor a content key.
    bool noKey = !resource->cacheAccess().getScratchKey().isValid() &&
                 !resource->cacheAccess().getContentKey().isValid();

    // Only cached resources should ever have a key.
    SkASSERT(noKey || resource->cacheAccess().isBudgeted());

    if (overBudget || noKey) {
        SkDEBUGCODE(int beforeCount = fCount;)
        resource->cacheAccess().release();
        // We should at least free this resource, perhaps dependent resources as well.
        SkASSERT(fCount < beforeCount);
    }

    this->validate();
}

void GrResourceCache2::didChangeGpuMemorySize(const GrGpuResource* resource, size_t oldSize) {
    // SkASSERT(!fPurging); GrPathRange increases size during flush. :(
    SkASSERT(resource);
    SkASSERT(this->isInCache(resource));

    ptrdiff_t delta = resource->gpuMemorySize() - oldSize;

    fBytes += delta;
#if GR_CACHE_STATS
    fHighWaterBytes = SkTMax(fBytes, fHighWaterBytes);
#endif
    if (resource->cacheAccess().isBudgeted()) {
        fBudgetedBytes += delta;
#if GR_CACHE_STATS
        fBudgetedHighWaterBytes = SkTMax(fBudgetedBytes, fBudgetedHighWaterBytes);
#endif
    }

    this->purgeAsNeeded();
    this->validate();
}

void GrResourceCache2::didChangeBudgetStatus(GrGpuResource* resource) {
    SkASSERT(!fPurging);
    SkASSERT(resource);
    SkASSERT(this->isInCache(resource));

    size_t size = resource->gpuMemorySize();

    if (resource->cacheAccess().isBudgeted()) {
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

void GrResourceCache2::internalPurgeAsNeeded() {
    SkASSERT(!fPurging);
    SkASSERT(!fNewlyPurgableResourceWhilePurging);
    SkASSERT(fBudgetedCount > fMaxCount || fBudgetedBytes > fMaxBytes);

    fPurging = true;

    bool overBudget = true;
    do {
        fNewlyPurgableResourceWhilePurging = false;
        ResourceList::Iter resourceIter;
        GrGpuResource* resource = resourceIter.init(fResources,
                                                    ResourceList::Iter::kTail_IterStart);

        while (resource) {
            GrGpuResource* prev = resourceIter.prev();
            if (resource->isPurgable()) {
                resource->cacheAccess().release();
            }
            resource = prev;
            if (fBudgetedCount <= fMaxCount && fBudgetedBytes <= fMaxBytes) {
                overBudget = false;
                resource = NULL;
            }
        }

        if (!fNewlyPurgableResourceWhilePurging && overBudget && fOverBudgetCB) {
            // Despite the purge we're still over budget. Call our over budget callback.
            (*fOverBudgetCB)(fOverBudgetData);
        }
    } while (overBudget && fNewlyPurgableResourceWhilePurging);

    fNewlyPurgableResourceWhilePurging = false;
    fPurging = false;
    this->validate();
}

void GrResourceCache2::purgeAllUnlocked() {
    SkASSERT(!fPurging);
    SkASSERT(!fNewlyPurgableResourceWhilePurging);

    fPurging = true;

    do {
        fNewlyPurgableResourceWhilePurging = false;
        ResourceList::Iter resourceIter;
        GrGpuResource* resource =
            resourceIter.init(fResources, ResourceList::Iter::kTail_IterStart);

        while (resource) {
            GrGpuResource* prev = resourceIter.prev();
            if (resource->isPurgable()) {
                resource->cacheAccess().release();
            }
            resource = prev;
        }

        if (!fNewlyPurgableResourceWhilePurging && fCount && fOverBudgetCB) {
            (*fOverBudgetCB)(fOverBudgetData);
        }
    } while (fNewlyPurgableResourceWhilePurging);
    fPurging = false;
    this->validate();
}

#ifdef SK_DEBUG
void GrResourceCache2::validate() const {
    size_t bytes = 0;
    int count = 0;
    int budgetedCount = 0;
    size_t budgetedBytes = 0;
    int locked = 0;
    int scratch = 0;
    int couldBeScratch = 0;
    int content = 0;

    ResourceList::Iter iter;
    GrGpuResource* resource = iter.init(fResources, ResourceList::Iter::kHead_IterStart);
    for ( ; resource; resource = iter.next()) {
        bytes += resource->gpuMemorySize();
        ++count;

        if (!resource->isPurgable()) {
            ++locked;
        }

        if (resource->cacheAccess().isScratch()) {
            SkASSERT(!resource->cacheAccess().getContentKey().isValid());
            ++scratch;
            SkASSERT(fScratchMap.countForKey(resource->cacheAccess().getScratchKey()));
            SkASSERT(!resource->cacheAccess().isWrapped());
        } else if (resource->cacheAccess().getScratchKey().isValid()) {
            SkASSERT(resource->cacheAccess().getContentKey().isValid());
            ++couldBeScratch;
            SkASSERT(fScratchMap.countForKey(resource->cacheAccess().getScratchKey()));
            SkASSERT(!resource->cacheAccess().isWrapped());
        }
        const GrContentKey& contentKey = resource->cacheAccess().getContentKey();
        if (contentKey.isValid()) {
            ++content;
            SkASSERT(fContentHash.find(contentKey) == resource);
            SkASSERT(!resource->cacheAccess().isWrapped());
        }

        if (resource->cacheAccess().isBudgeted()) {
            ++budgetedCount;
            budgetedBytes += resource->gpuMemorySize();
        }
    }

    SkASSERT(fBudgetedCount <= fCount);
    SkASSERT(fBudgetedBytes <= fBudgetedBytes);
    SkASSERT(bytes == fBytes);
    SkASSERT(count == fCount);
    SkASSERT(budgetedBytes == fBudgetedBytes);
    SkASSERT(budgetedCount == fBudgetedCount);
#if GR_CACHE_STATS
    SkASSERT(fBudgetedHighWaterCount <= fHighWaterCount);
    SkASSERT(fBudgetedHighWaterBytes <= fHighWaterBytes);
    SkASSERT(bytes <= fHighWaterBytes);
    SkASSERT(count <= fHighWaterCount);
    SkASSERT(budgetedBytes <= fBudgetedHighWaterBytes);
    SkASSERT(budgetedCount <= fBudgetedHighWaterCount);
#endif
    SkASSERT(content == fContentHash.count());
    SkASSERT(scratch + couldBeScratch == fScratchMap.count());

    // This assertion is not currently valid because we can be in recursive notifyIsPurgable()
    // calls. This will be fixed when subresource registration is explicit.
    // bool overBudget = budgetedBytes > fMaxBytes || budgetedCount > fMaxCount;
    // SkASSERT(!overBudget || locked == count || fPurging);
}
#endif

#if GR_CACHE_STATS
void GrResourceCache2::printStats() const {
    this->validate();

    int locked = 0;
    int scratch = 0;
    int wrapped = 0;
    size_t unbudgetedSize = 0;

    ResourceList::Iter iter;
    GrGpuResource* resource = iter.init(fResources, ResourceList::Iter::kHead_IterStart);

    for ( ; resource; resource = iter.next()) {
        if (!resource->isPurgable()) {
            ++locked;
        }
        if (resource->cacheAccess().isScratch()) {
            ++scratch;
        }
        if (resource->cacheAccess().isWrapped()) {
            ++wrapped;
        }
        if (!resource->cacheAccess().isBudgeted()) {
            unbudgetedSize += resource->gpuMemorySize();
        }
    }

    float countUtilization = (100.f * fBudgetedCount) / fMaxCount;
    float byteUtilization = (100.f * fBudgetedBytes) / fMaxBytes;

    SkDebugf("Budget: %d items %d bytes\n", fMaxCount, fMaxBytes);
    SkDebugf("\t\tEntry Count: current %d"
             " (%d budgeted, %d wrapped, %d locked, %d scratch %.2g%% full), high %d\n",
        fCount, fBudgetedCount, wrapped, locked, scratch, countUtilization, fHighWaterCount);
    SkDebugf("\t\tEntry Bytes: current %d (budgeted %d, %.2g%% full, %d unbudgeted) high %d\n",
                fBytes, fBudgetedBytes, byteUtilization, unbudgetedSize, fHighWaterBytes);
}

#endif
