
/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrResourceCache2.h"
#include "GrGpuResource.h"  

#include "SkGr.h"
#include "SkMessageBus.h"

DECLARE_SKMESSAGEBUS_MESSAGE(GrResourceInvalidatedMessage);

//////////////////////////////////////////////////////////////////////////////

GrResourceKey& GrResourceKey::NullScratchKey() {
    static const GrCacheID::Key kBogusKey = { { {0} } };
    static GrCacheID kBogusID(ScratchDomain(), kBogusKey);
    static GrResourceKey kNullScratchKey(kBogusID, NoneResourceType(), 0);
    return kNullScratchKey;
}

GrResourceKey::ResourceType GrResourceKey::NoneResourceType() {
    static const ResourceType gNoneResourceType = GenerateResourceType();
    return gNoneResourceType;
}

GrCacheID::Domain GrResourceKey::ScratchDomain() {
    static const GrCacheID::Domain gDomain = GrCacheID::GenerateDomain();
    return gDomain;
}

GrResourceKey::ResourceType GrResourceKey::GenerateResourceType() {
    static int32_t gNextType = 0;

    int32_t type = sk_atomic_inc(&gNextType);
    if (type >= (1 << 8 * sizeof(ResourceType))) {
        SkFAIL("Too many Resource Types");
    }

    return static_cast<ResourceType>(type);
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
#endif
    , fCount(0)
    , fBytes(0)
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
    AutoValidate av(this);

    SkASSERT(resource);
    SkASSERT(!resource->wasDestroyed());
    SkASSERT(!this->isInCache(resource));
    SkASSERT(!fPurging);
    fResources.addToHead(resource);

    ++fCount;
    SkDEBUGCODE(fHighWaterCount = SkTMax(fCount, fHighWaterCount));
    fBytes += resource->gpuMemorySize();
    SkDEBUGCODE(fHighWaterBytes = SkTMax(fBytes, fHighWaterBytes));
    if (!resource->cacheAccess().getScratchKey().isNullScratch()) {
        // TODO(bsalomon): Make this assertion possible.
        // SkASSERT(!resource->isWrapped());
        fScratchMap.insert(resource->cacheAccess().getScratchKey(), resource);
    }
    
    this->purgeAsNeeded();
}

void GrResourceCache2::removeResource(GrGpuResource* resource) {
    AutoValidate av(this);

    --fCount;
    fBytes -= resource->gpuMemorySize();
    SkASSERT(this->isInCache(resource));
    fResources.remove(resource);    
    if (!resource->cacheAccess().getScratchKey().isNullScratch()) {
        fScratchMap.remove(resource->cacheAccess().getScratchKey(), resource);
    }
    if (const GrResourceKey* contentKey = resource->cacheAccess().getContentKey()) {
        fContentHash.remove(*contentKey);
    }
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

GrGpuResource* GrResourceCache2::findAndRefScratchResource(const GrResourceKey& scratchKey,
                                                           uint32_t flags) {
    AutoValidate av(this);

    SkASSERT(!fPurging);
    SkASSERT(scratchKey.isScratch());

    GrGpuResource* resource;
    if (flags & (kPreferNoPendingIO_ScratchFlag | kRequireNoPendingIO_ScratchFlag)) {
        resource = fScratchMap.find(scratchKey, AvailableForScratchUse(true));
        if (resource) {
            this->makeResourceMRU(resource);
            return SkRef(resource);
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
    }
    return resource;
}

bool GrResourceCache2::didSetContentKey(GrGpuResource* resource) {
    SkASSERT(!fPurging);
    SkASSERT(resource);
    SkASSERT(this->isInCache(resource));
    SkASSERT(resource->cacheAccess().getContentKey());
    SkASSERT(!resource->cacheAccess().getContentKey()->isScratch());

    GrGpuResource* res = fContentHash.find(*resource->cacheAccess().getContentKey());
    if (NULL != res) {
        return false;
    }

    fContentHash.add(resource);
    this->validate();
    return true;
}

void GrResourceCache2::makeResourceMRU(GrGpuResource* resource) {
    AutoValidate av(this);

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
    bool overBudget = fCount > fMaxCount || fBytes > fMaxBytes;

    // Also purge if the resource has neither a valid scratch key nor a content key.
    bool noKey = !resource->cacheAccess().isScratch() &&
                 (NULL == resource->cacheAccess().getContentKey());

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

    fBytes += resource->gpuMemorySize() - oldSize;
    SkDEBUGCODE(fHighWaterBytes = SkTMax(fBytes, fHighWaterBytes));

    this->purgeAsNeeded();
    this->validate();
}

void GrResourceCache2::internalPurgeAsNeeded() {
    SkASSERT(!fPurging);
    SkASSERT(!fNewlyPurgableResourceWhilePurging);
    SkASSERT(fCount > fMaxCount || fBytes > fMaxBytes);

    fPurging = true;

    AutoValidate av(this); // Put this after setting fPurging so we're allowed to be over budget.

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
            if (fCount <= fMaxCount && fBytes <= fMaxBytes) {
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
}

void GrResourceCache2::purgeAllUnlocked() {
    SkASSERT(!fPurging);
    SkASSERT(!fNewlyPurgableResourceWhilePurging);

    fPurging = true;

    AutoValidate av(this); // Put this after setting fPurging so we're allowed to be over budget.

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
}

#ifdef SK_DEBUG
void GrResourceCache2::validate() const {
    size_t bytes = 0;
    int count = 0;
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
            SkASSERT(NULL == resource->cacheAccess().getContentKey());
            ++scratch;
            SkASSERT(fScratchMap.countForKey(resource->cacheAccess().getScratchKey()));
        } else if (!resource->cacheAccess().getScratchKey().isNullScratch()) {
            SkASSERT(NULL != resource->cacheAccess().getContentKey());
            ++couldBeScratch;
            SkASSERT(fScratchMap.countForKey(resource->cacheAccess().getScratchKey()));
        }

        if (const GrResourceKey* contentKey = resource->cacheAccess().getContentKey()) {
            ++content;
            SkASSERT(fContentHash.find(*contentKey) == resource);
        }
    }

    SkASSERT(bytes == fBytes);
    SkASSERT(count == fCount);
#if GR_CACHE_STATS
    SkASSERT(bytes <= fHighWaterBytes);
    SkASSERT(count <= fHighWaterCount);
#endif
    SkASSERT(content == fContentHash.count());
    SkASSERT(scratch + couldBeScratch == fScratchMap.count());

    // This assertion is not currently valid because we can be in recursive notifyIsPurgable()
    // calls. This will be fixed when subresource registration is explicit.
    // bool overBudget = bytes > fMaxBytes || count > fMaxCount;
    // SkASSERT(!overBudget || locked == count || fPurging);
}
#endif

#if GR_CACHE_STATS
void GrResourceCache2::printStats() const {
    this->validate();

    int locked = 0;
    int scratch = 0;

    ResourceList::Iter iter;
    GrGpuResource* resource = iter.init(fResources, ResourceList::Iter::kHead_IterStart);

    for ( ; resource; resource = iter.next()) {
        if (!resource->isPurgable()) {
            ++locked;
        }
        if (resource->cacheAccess().isScratch()) {
            ++scratch;
        }
    }

    float countUtilization = (100.f * fCount) / fMaxCount;
    float byteUtilization = (100.f * fBytes) / fMaxBytes;

    SkDebugf("Budget: %d items %d bytes\n", fMaxCount, fMaxBytes);
    SkDebugf("\t\tEntry Count: current %d (%d locked, %d scratch %.2g%% full), high %d\n",
                fCount, locked, scratch, countUtilization, fHighWaterCount);
    SkDebugf("\t\tEntry Bytes: current %d (%.2g%% full) high %d\n",
                fBytes, byteUtilization, fHighWaterBytes);
}

#endif
