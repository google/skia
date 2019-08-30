/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrResourceCache.h"
#include <atomic>
#include "include/gpu/GrContext.h"
#include "include/gpu/GrTexture.h"
#include "include/private/GrSingleOwner.h"
#include "include/private/SkTo.h"
#include "include/utils/SkRandom.h"
#include "src/core/SkExchange.h"
#include "src/core/SkMessageBus.h"
#include "src/core/SkOpts.h"
#include "src/core/SkScopeExit.h"
#include "src/core/SkTSort.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrGpuResourceCacheAccess.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrTextureProxyCacheAccess.h"
#include "src/gpu/GrTracing.h"
#include "src/gpu/SkGr.h"

DECLARE_SKMESSAGEBUS_MESSAGE(GrUniqueKeyInvalidatedMessage);

DECLARE_SKMESSAGEBUS_MESSAGE(GrGpuResourceFreedMessage);

#define ASSERT_SINGLE_OWNER \
    SkDEBUGCODE(GrSingleOwner::AutoEnforce debug_SingleOwner(fSingleOwner);)

//////////////////////////////////////////////////////////////////////////////

GrScratchKey::ResourceType GrScratchKey::GenerateResourceType() {
    static std::atomic<int32_t> nextType{INHERITED::kInvalidDomain + 1};

    int32_t type = nextType++;
    if (type > SkTo<int32_t>(UINT16_MAX)) {
        SK_ABORT("Too many Resource Types");
    }

    return static_cast<ResourceType>(type);
}

GrUniqueKey::Domain GrUniqueKey::GenerateDomain() {
    static std::atomic<int32_t> nextDomain{INHERITED::kInvalidDomain + 1};

    int32_t domain = nextDomain++;
    if (domain > SkTo<int32_t>(UINT16_MAX)) {
        SK_ABORT("Too many GrUniqueKey Domains");
    }

    return static_cast<Domain>(domain);
}

uint32_t GrResourceKeyHash(const uint32_t* data, size_t size) {
    return SkOpts::hash(data, size);
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

inline GrResourceCache::ResourceAwaitingUnref::ResourceAwaitingUnref() = default;

inline GrResourceCache::ResourceAwaitingUnref::ResourceAwaitingUnref(GrGpuResource* resource)
        : fResource(resource), fNumUnrefs(1) {}

inline GrResourceCache::ResourceAwaitingUnref::ResourceAwaitingUnref(ResourceAwaitingUnref&& that) {
    fResource = skstd::exchange(that.fResource, nullptr);
    fNumUnrefs = skstd::exchange(that.fNumUnrefs, 0);
}

inline GrResourceCache::ResourceAwaitingUnref& GrResourceCache::ResourceAwaitingUnref::operator=(
        ResourceAwaitingUnref&& that) {
    fResource = skstd::exchange(that.fResource, nullptr);
    fNumUnrefs = skstd::exchange(that.fNumUnrefs, 0);
    return *this;
}

inline GrResourceCache::ResourceAwaitingUnref::~ResourceAwaitingUnref() {
    if (fResource) {
        for (int i = 0; i < fNumUnrefs; ++i) {
            fResource->unref();
        }
    }
}

inline void GrResourceCache::ResourceAwaitingUnref::addRef() { ++fNumUnrefs; }

inline void GrResourceCache::ResourceAwaitingUnref::unref() {
    SkASSERT(fNumUnrefs > 0);
    fResource->unref();
    --fNumUnrefs;
}

inline bool GrResourceCache::ResourceAwaitingUnref::finished() { return !fNumUnrefs; }

//////////////////////////////////////////////////////////////////////////////

GrResourceCache::GrResourceCache(const GrCaps* caps, GrSingleOwner* singleOwner,
                                 uint32_t contextUniqueID)
        : fInvalidUniqueKeyInbox(contextUniqueID)
        , fFreedGpuResourceInbox(contextUniqueID)
        , fContextUniqueID(contextUniqueID)
        , fSingleOwner(singleOwner)
        , fPreferVRAMUseOverFlushes(caps->preferVRAMUseOverFlushes()) {
    SkASSERT(contextUniqueID != SK_InvalidUniqueID);
}

GrResourceCache::~GrResourceCache() {
    this->releaseAll();
}

void GrResourceCache::setLimit(size_t bytes) {
    fMaxBytes = bytes;
    this->purgeAsNeeded();
}

void GrResourceCache::insertResource(GrGpuResource* resource) {
    ASSERT_SINGLE_OWNER
    SkASSERT(resource);
    SkASSERT(!this->isInCache(resource));
    SkASSERT(!resource->wasDestroyed());
    SkASSERT(!resource->resourcePriv().isPurgeable());

    // We must set the timestamp before adding to the array in case the timestamp wraps and we wind
    // up iterating over all the resources that already have timestamps.
    resource->cacheAccess().setTimestamp(this->getNextTimestamp());

    this->addToNonpurgeableArray(resource);

    size_t size = resource->gpuMemorySize();
    SkDEBUGCODE(++fCount;)
    fBytes += size;
#if GR_CACHE_STATS
    fHighWaterCount = SkTMax(this->getResourceCount(), fHighWaterCount);
    fHighWaterBytes = SkTMax(fBytes, fHighWaterBytes);
#endif
    if (GrBudgetedType::kBudgeted == resource->resourcePriv().budgetedType()) {
        ++fBudgetedCount;
        fBudgetedBytes += size;
        TRACE_COUNTER2("skia.gpu.cache", "skia budget", "used",
                       fBudgetedBytes, "free", fMaxBytes - fBudgetedBytes);
#if GR_CACHE_STATS
        fBudgetedHighWaterCount = SkTMax(fBudgetedCount, fBudgetedHighWaterCount);
        fBudgetedHighWaterBytes = SkTMax(fBudgetedBytes, fBudgetedHighWaterBytes);
#endif
    }
    if (resource->resourcePriv().getScratchKey().isValid() &&
        !resource->getUniqueKey().isValid()) {
        SkASSERT(!resource->resourcePriv().refsWrappedObjects());
        fScratchMap.insert(resource->resourcePriv().getScratchKey(), resource);
    }

    this->purgeAsNeeded();
}

void GrResourceCache::removeResource(GrGpuResource* resource) {
    ASSERT_SINGLE_OWNER
    this->validate();
    SkASSERT(this->isInCache(resource));

    size_t size = resource->gpuMemorySize();
    if (resource->resourcePriv().isPurgeable()) {
        fPurgeableQueue.remove(resource);
        fPurgeableBytes -= size;
    } else {
        this->removeFromNonpurgeableArray(resource);
    }

    SkDEBUGCODE(--fCount;)
    fBytes -= size;
    if (GrBudgetedType::kBudgeted == resource->resourcePriv().budgetedType()) {
        --fBudgetedCount;
        fBudgetedBytes -= size;
        TRACE_COUNTER2("skia.gpu.cache", "skia budget", "used",
                       fBudgetedBytes, "free", fMaxBytes - fBudgetedBytes);
    }

    if (resource->resourcePriv().getScratchKey().isValid() &&
        !resource->getUniqueKey().isValid()) {
        fScratchMap.remove(resource->resourcePriv().getScratchKey(), resource);
    }
    if (resource->getUniqueKey().isValid()) {
        fUniqueHash.remove(resource->getUniqueKey());
    }
    this->validate();
}

void GrResourceCache::abandonAll() {
    AutoValidate av(this);

    // We need to make sure to free any resources that were waiting on a free message but never
    // received one.
    fResourcesAwaitingUnref.reset();

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
    SkASSERT(!fUniqueHash.count());
    SkASSERT(!fCount);
    SkASSERT(!this->getResourceCount());
    SkASSERT(!fBytes);
    SkASSERT(!fBudgetedCount);
    SkASSERT(!fBudgetedBytes);
    SkASSERT(!fPurgeableBytes);
    SkASSERT(!fResourcesAwaitingUnref.count());
}

void GrResourceCache::releaseAll() {
    AutoValidate av(this);

    this->processFreedGpuResources();

    // We need to make sure to free any resources that were waiting on a free message but never
    // received one.
    fResourcesAwaitingUnref.reset();

    SkASSERT(fProxyProvider); // better have called setProxyProvider
    // We must remove the uniqueKeys from the proxies here. While they possess a uniqueKey
    // they also have a raw pointer back to this class (which is presumably going away)!
    fProxyProvider->removeAllUniqueKeys();

    while (fNonpurgeableResources.count()) {
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
    SkASSERT(!fUniqueHash.count());
    SkASSERT(!fCount);
    SkASSERT(!this->getResourceCount());
    SkASSERT(!fBytes);
    SkASSERT(!fBudgetedCount);
    SkASSERT(!fBudgetedBytes);
    SkASSERT(!fPurgeableBytes);
    SkASSERT(!fResourcesAwaitingUnref.count());
}

void GrResourceCache::refResource(GrGpuResource* resource) {
    SkASSERT(resource);
    SkASSERT(resource->getContext()->priv().getResourceCache() == this);
    if (resource->cacheAccess().hasRef()) {
        resource->ref();
    } else {
        this->refAndMakeResourceMRU(resource);
    }
    this->validate();
}

class GrResourceCache::AvailableForScratchUse {
public:
    AvailableForScratchUse() { }

    bool operator()(const GrGpuResource* resource) const {
        SkASSERT(!resource->getUniqueKey().isValid() &&
                 resource->resourcePriv().getScratchKey().isValid());

        // isScratch() also tests that the resource is budgeted. TODO: why are unbudgeted
        // scratch resouces in the scratchMap?
        if (resource->internalHasRef() || !resource->cacheAccess().isScratch()) {
            return false;
        }
        return true;
    }
};

GrGpuResource* GrResourceCache::findAndRefScratchResource(const GrScratchKey& scratchKey,
                                                          size_t resourceSize,
                                                          ScratchFlags flags) {
    SkASSERT(scratchKey.isValid());

    GrGpuResource* resource;
    // TODO: remove these conditions and fuse the two code paths!
    if (flags & (ScratchFlags::kPreferNoPendingIO | ScratchFlags::kRequireNoPendingIO)) {
        resource = fScratchMap.find(scratchKey, AvailableForScratchUse());
        if (resource) {
            this->refAndMakeResourceMRU(resource);
            this->validate();
            return resource;
        } else if (flags & ScratchFlags::kRequireNoPendingIO) {
            return nullptr;
        }
        // We would prefer to consume more available VRAM rather than flushing
        // immediately, but on ANGLE this can lead to starving of the GPU.
        if (fPreferVRAMUseOverFlushes && this->wouldFit(resourceSize)) {
            // kPrefer is specified, we didn't find a resource without pending io,
            // but there is still space in our budget for the resource so force
            // the caller to allocate a new resource.
            return nullptr;
        }
    }
    resource = fScratchMap.find(scratchKey, AvailableForScratchUse());
    if (resource) {
        this->refAndMakeResourceMRU(resource);
        this->validate();
    }
    return resource;
}

void GrResourceCache::willRemoveScratchKey(const GrGpuResource* resource) {
    ASSERT_SINGLE_OWNER
    SkASSERT(resource->resourcePriv().getScratchKey().isValid());
    if (!resource->getUniqueKey().isValid()) {
        fScratchMap.remove(resource->resourcePriv().getScratchKey(), resource);
    }
}

void GrResourceCache::removeUniqueKey(GrGpuResource* resource) {
    ASSERT_SINGLE_OWNER
    // Someone has a ref to this resource in order to have removed the key. When the ref count
    // reaches zero we will get a ref cnt notification and figure out what to do with it.
    if (resource->getUniqueKey().isValid()) {
        SkASSERT(resource == fUniqueHash.find(resource->getUniqueKey()));
        fUniqueHash.remove(resource->getUniqueKey());
    }
    resource->cacheAccess().removeUniqueKey();
    if (resource->resourcePriv().getScratchKey().isValid()) {
        fScratchMap.insert(resource->resourcePriv().getScratchKey(), resource);
    }

    // Removing a unique key from a kUnbudgetedCacheable resource would make the resource
    // require purging. However, the resource must be ref'ed to get here and therefore can't
    // be purgeable. We'll purge it when the refs reach zero.
    SkASSERT(!resource->resourcePriv().isPurgeable());
    this->validate();
}

void GrResourceCache::changeUniqueKey(GrGpuResource* resource, const GrUniqueKey& newKey) {
    ASSERT_SINGLE_OWNER
    SkASSERT(resource);
    SkASSERT(this->isInCache(resource));

    // If another resource has the new key, remove its key then install the key on this resource.
    if (newKey.isValid()) {
        if (GrGpuResource* old = fUniqueHash.find(newKey)) {
            // If the old resource using the key is purgeable and is unreachable, then remove it.
            if (!old->resourcePriv().getScratchKey().isValid() &&
                old->resourcePriv().isPurgeable()) {
                old->cacheAccess().release();
            } else {
                // removeUniqueKey expects an external owner of the resource.
                this->removeUniqueKey(sk_ref_sp(old).get());
            }
        }
        SkASSERT(nullptr == fUniqueHash.find(newKey));

        // Remove the entry for this resource if it already has a unique key.
        if (resource->getUniqueKey().isValid()) {
            SkASSERT(resource == fUniqueHash.find(resource->getUniqueKey()));
            fUniqueHash.remove(resource->getUniqueKey());
            SkASSERT(nullptr == fUniqueHash.find(resource->getUniqueKey()));
        } else {
            // 'resource' didn't have a valid unique key before so it is switching sides. Remove it
            // from the ScratchMap
            if (resource->resourcePriv().getScratchKey().isValid()) {
                fScratchMap.remove(resource->resourcePriv().getScratchKey(), resource);
            }
        }

        resource->cacheAccess().setUniqueKey(newKey);
        fUniqueHash.add(resource);
    } else {
        this->removeUniqueKey(resource);
    }

    this->validate();
}

void GrResourceCache::refAndMakeResourceMRU(GrGpuResource* resource) {
    ASSERT_SINGLE_OWNER
    SkASSERT(resource);
    SkASSERT(this->isInCache(resource));

    if (resource->resourcePriv().isPurgeable()) {
        // It's about to become unpurgeable.
        fPurgeableBytes -= resource->gpuMemorySize();
        fPurgeableQueue.remove(resource);
        this->addToNonpurgeableArray(resource);
    } else if (!resource->cacheAccess().hasRef() &&
               resource->resourcePriv().budgetedType() == GrBudgetedType::kBudgeted) {
        SkASSERT(fNumBudgetedResourcesFlushWillMakePurgeable > 0);
        fNumBudgetedResourcesFlushWillMakePurgeable--;
    }
    resource->cacheAccess().ref();

    resource->cacheAccess().setTimestamp(this->getNextTimestamp());
    this->validate();
}

void GrResourceCache::notifyRefCntReachedZero(GrGpuResource* resource) {
    ASSERT_SINGLE_OWNER
    SkASSERT(resource);
    SkASSERT(!resource->wasDestroyed());
    SkASSERT(this->isInCache(resource));
    // This resource should always be in the nonpurgeable array when this function is called. It
    // will be moved to the queue if it is newly purgeable.
    SkASSERT(fNonpurgeableResources[*resource->cacheAccess().accessCacheIndex()] == resource);

#ifdef SK_DEBUG
    // When the timestamp overflows validate() is called. validate() checks that resources in
    // the nonpurgeable array are indeed not purgeable. However, the movement from the array to
    // the purgeable queue happens just below in this function. So we mark it as an exception.
    if (resource->resourcePriv().isPurgeable()) {
        fNewlyPurgeableResourceForValidation = resource;
    }
#endif
    resource->cacheAccess().setTimestamp(this->getNextTimestamp());
    SkDEBUGCODE(fNewlyPurgeableResourceForValidation = nullptr);

    if (!resource->resourcePriv().isPurgeable() &&
        resource->resourcePriv().budgetedType() == GrBudgetedType::kBudgeted) {
        ++fNumBudgetedResourcesFlushWillMakePurgeable;
    }

    if (!resource->resourcePriv().isPurgeable()) {
        this->validate();
        return;
    }

    this->removeFromNonpurgeableArray(resource);
    fPurgeableQueue.insert(resource);
    resource->cacheAccess().setTimeWhenResourceBecomePurgeable();
    fPurgeableBytes += resource->gpuMemorySize();

    bool hasUniqueKey = resource->getUniqueKey().isValid();

    GrBudgetedType budgetedType = resource->resourcePriv().budgetedType();

    if (budgetedType == GrBudgetedType::kBudgeted) {
        // Purge the resource immediately if we're over budget
        // Also purge if the resource has neither a valid scratch key nor a unique key.
        bool hasKey = resource->resourcePriv().getScratchKey().isValid() || hasUniqueKey;
        if (!this->overBudget() && hasKey) {
            return;
        }
    } else {
        // We keep unbudgeted resources with a unique key in the purgeable queue of the cache so
        // they can be reused again by the image connected to the unique key.
        if (hasUniqueKey && budgetedType == GrBudgetedType::kUnbudgetedCacheable) {
            return;
        }
        // Check whether this resource could still be used as a scratch resource.
        if (!resource->resourcePriv().refsWrappedObjects() &&
            resource->resourcePriv().getScratchKey().isValid()) {
            // We won't purge an existing resource to make room for this one.
            if (this->wouldFit(resource->gpuMemorySize())) {
                resource->resourcePriv().makeBudgeted();
                return;
            }
        }
    }

    SkDEBUGCODE(int beforeCount = this->getResourceCount();)
    resource->cacheAccess().release();
    // We should at least free this resource, perhaps dependent resources as well.
    SkASSERT(this->getResourceCount() < beforeCount);
    this->validate();
}

void GrResourceCache::didChangeBudgetStatus(GrGpuResource* resource) {
    ASSERT_SINGLE_OWNER
    SkASSERT(resource);
    SkASSERT(this->isInCache(resource));

    size_t size = resource->gpuMemorySize();
    // Changing from BudgetedType::kUnbudgetedCacheable to another budgeted type could make
    // resource become purgeable. However, we should never allow that transition. Wrapped
    // resources are the only resources that can be in that state and they aren't allowed to
    // transition from one budgeted state to another.
    SkDEBUGCODE(bool wasPurgeable = resource->resourcePriv().isPurgeable());
    if (resource->resourcePriv().budgetedType() == GrBudgetedType::kBudgeted) {
        ++fBudgetedCount;
        fBudgetedBytes += size;
#if GR_CACHE_STATS
        fBudgetedHighWaterBytes = SkTMax(fBudgetedBytes, fBudgetedHighWaterBytes);
        fBudgetedHighWaterCount = SkTMax(fBudgetedCount, fBudgetedHighWaterCount);
#endif
        if (!resource->resourcePriv().isPurgeable() && !resource->cacheAccess().hasRef()) {
            ++fNumBudgetedResourcesFlushWillMakePurgeable;
        }
        this->purgeAsNeeded();
    } else {
        SkASSERT(resource->resourcePriv().budgetedType() != GrBudgetedType::kUnbudgetedCacheable);
        --fBudgetedCount;
        fBudgetedBytes -= size;
        if (!resource->resourcePriv().isPurgeable() && !resource->cacheAccess().hasRef()) {
            --fNumBudgetedResourcesFlushWillMakePurgeable;
        }
    }
    SkASSERT(wasPurgeable == resource->resourcePriv().isPurgeable());
    TRACE_COUNTER2("skia.gpu.cache", "skia budget", "used",
                   fBudgetedBytes, "free", fMaxBytes - fBudgetedBytes);

    this->validate();
}

void GrResourceCache::purgeAsNeeded() {
    SkTArray<GrUniqueKeyInvalidatedMessage> invalidKeyMsgs;
    fInvalidUniqueKeyInbox.poll(&invalidKeyMsgs);
    if (invalidKeyMsgs.count()) {
        SkASSERT(fProxyProvider);

        for (int i = 0; i < invalidKeyMsgs.count(); ++i) {
            fProxyProvider->processInvalidUniqueKey(invalidKeyMsgs[i].key(), nullptr,
                                                    GrProxyProvider::InvalidateGPUResource::kYes);
            SkASSERT(!this->findAndRefUniqueResource(invalidKeyMsgs[i].key()));
        }
    }

    this->processFreedGpuResources();

    bool stillOverbudget = this->overBudget();
    while (stillOverbudget && fPurgeableQueue.count()) {
        GrGpuResource* resource = fPurgeableQueue.peek();
        SkASSERT(resource->resourcePriv().isPurgeable());
        resource->cacheAccess().release();
        stillOverbudget = this->overBudget();
    }

    this->validate();
}

void GrResourceCache::purgeUnlockedResources(bool scratchResourcesOnly) {
    if (!scratchResourcesOnly) {
        // We could disable maintaining the heap property here, but it would add a lot of
        // complexity. Moreover, this is rarely called.
        while (fPurgeableQueue.count()) {
            GrGpuResource* resource = fPurgeableQueue.peek();
            SkASSERT(resource->resourcePriv().isPurgeable());
            resource->cacheAccess().release();
        }
    } else {
        // Sort the queue
        fPurgeableQueue.sort();

        // Make a list of the scratch resources to delete
        SkTDArray<GrGpuResource*> scratchResources;
        for (int i = 0; i < fPurgeableQueue.count(); i++) {
            GrGpuResource* resource = fPurgeableQueue.at(i);
            SkASSERT(resource->resourcePriv().isPurgeable());
            if (!resource->getUniqueKey().isValid()) {
                *scratchResources.append() = resource;
            }
        }

        // Delete the scratch resources. This must be done as a separate pass
        // to avoid messing up the sorted order of the queue
        for (int i = 0; i < scratchResources.count(); i++) {
            scratchResources.getAt(i)->cacheAccess().release();
        }
    }

    this->validate();
}

void GrResourceCache::purgeResourcesNotUsedSince(GrStdSteadyClock::time_point purgeTime) {
    while (fPurgeableQueue.count()) {
        const GrStdSteadyClock::time_point resourceTime =
                fPurgeableQueue.peek()->cacheAccess().timeWhenResourceBecamePurgeable();
        if (resourceTime >= purgeTime) {
            // Resources were given both LRU timestamps and tagged with a frame number when
            // they first became purgeable. The LRU timestamp won't change again until the
            // resource is made non-purgeable again. So, at this point all the remaining
            // resources in the timestamp-sorted queue will have a frame number >= to this
            // one.
            break;
        }
        GrGpuResource* resource = fPurgeableQueue.peek();
        SkASSERT(resource->resourcePriv().isPurgeable());
        resource->cacheAccess().release();
    }
}

void GrResourceCache::purgeUnlockedResources(size_t bytesToPurge, bool preferScratchResources) {

    const size_t tmpByteBudget = SkTMax((size_t)0, fBytes - bytesToPurge);
    bool stillOverbudget = tmpByteBudget < fBytes;

    if (preferScratchResources && bytesToPurge < fPurgeableBytes) {
        // Sort the queue
        fPurgeableQueue.sort();

        // Make a list of the scratch resources to delete
        SkTDArray<GrGpuResource*> scratchResources;
        size_t scratchByteCount = 0;
        for (int i = 0; i < fPurgeableQueue.count() && stillOverbudget; i++) {
            GrGpuResource* resource = fPurgeableQueue.at(i);
            SkASSERT(resource->resourcePriv().isPurgeable());
            if (!resource->getUniqueKey().isValid()) {
                *scratchResources.append() = resource;
                scratchByteCount += resource->gpuMemorySize();
                stillOverbudget = tmpByteBudget < fBytes - scratchByteCount;
            }
        }

        // Delete the scratch resources. This must be done as a separate pass
        // to avoid messing up the sorted order of the queue
        for (int i = 0; i < scratchResources.count(); i++) {
            scratchResources.getAt(i)->cacheAccess().release();
        }
        stillOverbudget = tmpByteBudget < fBytes;

        this->validate();
    }

    // Purge any remaining resources in LRU order
    if (stillOverbudget) {
        const size_t cachedByteCount = fMaxBytes;
        fMaxBytes = tmpByteBudget;
        this->purgeAsNeeded();
        fMaxBytes = cachedByteCount;
    }
}
bool GrResourceCache::requestsFlush() const {
    return this->overBudget() && !fPurgeableQueue.count() &&
           fNumBudgetedResourcesFlushWillMakePurgeable > 0;
}


void GrResourceCache::insertDelayedResourceUnref(GrGpuResource* resource) {
    resource->ref();
    uint32_t id = resource->uniqueID().asUInt();
    if (auto* data = fResourcesAwaitingUnref.find(id)) {
        data->addRef();
    } else {
        fResourcesAwaitingUnref.set(id, {resource});
    }
}

void GrResourceCache::processFreedGpuResources() {
    SkTArray<GrGpuResourceFreedMessage> msgs;
    fFreedGpuResourceInbox.poll(&msgs);
    for (int i = 0; i < msgs.count(); ++i) {
        SkASSERT(msgs[i].fOwningUniqueID == fContextUniqueID);
        uint32_t id = msgs[i].fResource->uniqueID().asUInt();
        ResourceAwaitingUnref* info = fResourcesAwaitingUnref.find(id);
        // If we called release or abandon on the GrContext we will have already released our ref on
        // the GrGpuResource. If then the message arrives before the actual GrContext gets destroyed
        // we will try to process the message when we destroy the GrContext. This protects us from
        // trying to unref the resource twice.
        if (info) {
            info->unref();
            if (info->finished()) {
                fResourcesAwaitingUnref.remove(id);
            }
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

uint32_t GrResourceCache::getNextTimestamp() {
    // If we wrap then all the existing resources will appear older than any resources that get
    // a timestamp after the wrap.
    if (0 == fTimestamp) {
        int count = this->getResourceCount();
        if (count) {
            // Reset all the timestamps. We sort the resources by timestamp and then assign
            // sequential timestamps beginning with 0. This is O(n*lg(n)) but it should be extremely
            // rare.
            SkTDArray<GrGpuResource*> sortedPurgeableResources;
            sortedPurgeableResources.setReserve(fPurgeableQueue.count());

            while (fPurgeableQueue.count()) {
                *sortedPurgeableResources.append() = fPurgeableQueue.peek();
                fPurgeableQueue.pop();
            }

            SkTQSort(fNonpurgeableResources.begin(), fNonpurgeableResources.end() - 1,
                     CompareTimestamp);

            // Pick resources out of the purgeable and non-purgeable arrays based on lowest
            // timestamp and assign new timestamps.
            int currP = 0;
            int currNP = 0;
            while (currP < sortedPurgeableResources.count() &&
                   currNP < fNonpurgeableResources.count()) {
                uint32_t tsP = sortedPurgeableResources[currP]->cacheAccess().timestamp();
                uint32_t tsNP = fNonpurgeableResources[currNP]->cacheAccess().timestamp();
                SkASSERT(tsP != tsNP);
                if (tsP < tsNP) {
                    sortedPurgeableResources[currP++]->cacheAccess().setTimestamp(fTimestamp++);
                } else {
                    // Correct the index in the nonpurgeable array stored on the resource post-sort.
                    *fNonpurgeableResources[currNP]->cacheAccess().accessCacheIndex() = currNP;
                    fNonpurgeableResources[currNP++]->cacheAccess().setTimestamp(fTimestamp++);
                }
            }

            // The above loop ended when we hit the end of one array. Finish the other one.
            while (currP < sortedPurgeableResources.count()) {
                sortedPurgeableResources[currP++]->cacheAccess().setTimestamp(fTimestamp++);
            }
            while (currNP < fNonpurgeableResources.count()) {
                *fNonpurgeableResources[currNP]->cacheAccess().accessCacheIndex() = currNP;
                fNonpurgeableResources[currNP++]->cacheAccess().setTimestamp(fTimestamp++);
            }

            // Rebuild the queue.
            for (int i = 0; i < sortedPurgeableResources.count(); ++i) {
                fPurgeableQueue.insert(sortedPurgeableResources[i]);
            }

            this->validate();
            SkASSERT(count == this->getResourceCount());

            // count should be the next timestamp we return.
            SkASSERT(fTimestamp == SkToU32(count));
        }
    }
    return fTimestamp++;
}

void GrResourceCache::dumpMemoryStatistics(SkTraceMemoryDump* traceMemoryDump) const {
    for (int i = 0; i < fNonpurgeableResources.count(); ++i) {
        fNonpurgeableResources[i]->dumpMemoryStatistics(traceMemoryDump);
    }
    for (int i = 0; i < fPurgeableQueue.count(); ++i) {
        fPurgeableQueue.at(i)->dumpMemoryStatistics(traceMemoryDump);
    }
}

#if GR_CACHE_STATS
void GrResourceCache::getStats(Stats* stats) const {
    stats->reset();

    stats->fTotal = this->getResourceCount();
    stats->fNumNonPurgeable = fNonpurgeableResources.count();
    stats->fNumPurgeable = fPurgeableQueue.count();

    for (int i = 0; i < fNonpurgeableResources.count(); ++i) {
        stats->update(fNonpurgeableResources[i]);
    }
    for (int i = 0; i < fPurgeableQueue.count(); ++i) {
        stats->update(fPurgeableQueue.at(i));
    }
}

#if GR_TEST_UTILS
void GrResourceCache::dumpStats(SkString* out) const {
    this->validate();

    Stats stats;

    this->getStats(&stats);

    float byteUtilization = (100.f * fBudgetedBytes) / fMaxBytes;

    out->appendf("Budget: %d bytes\n", (int)fMaxBytes);
    out->appendf("\t\tEntry Count: current %d"
                 " (%d budgeted, %d wrapped, %d locked, %d scratch), high %d\n",
                 stats.fTotal, fBudgetedCount, stats.fWrapped, stats.fNumNonPurgeable,
                 stats.fScratch, fHighWaterCount);
    out->appendf("\t\tEntry Bytes: current %d (budgeted %d, %.2g%% full, %d unbudgeted) high %d\n",
                 SkToInt(fBytes), SkToInt(fBudgetedBytes), byteUtilization,
                 SkToInt(stats.fUnbudgetedSize), SkToInt(fHighWaterBytes));
}

void GrResourceCache::dumpStatsKeyValuePairs(SkTArray<SkString>* keys,
                                             SkTArray<double>* values) const {
    this->validate();

    Stats stats;
    this->getStats(&stats);

    keys->push_back(SkString("gpu_cache_purgable_entries")); values->push_back(stats.fNumPurgeable);
}
#endif

#endif

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
        const UniqueHash* fUniqueHash;

        Stats(const GrResourceCache* cache) {
            memset(this, 0, sizeof(*this));
            fScratchMap = &cache->fScratchMap;
            fUniqueHash = &cache->fUniqueHash;
        }

        void update(GrGpuResource* resource) {
            fBytes += resource->gpuMemorySize();

            if (!resource->resourcePriv().isPurgeable()) {
                ++fLocked;
            }

            const GrScratchKey& scratchKey = resource->resourcePriv().getScratchKey();
            const GrUniqueKey& uniqueKey = resource->getUniqueKey();

            if (resource->cacheAccess().isScratch()) {
                SkASSERT(!uniqueKey.isValid());
                ++fScratch;
                SkASSERT(fScratchMap->countForKey(scratchKey));
                SkASSERT(!resource->resourcePriv().refsWrappedObjects());
            } else if (scratchKey.isValid()) {
                SkASSERT(GrBudgetedType::kBudgeted != resource->resourcePriv().budgetedType() ||
                         uniqueKey.isValid());
                if (!uniqueKey.isValid()) {
                    ++fCouldBeScratch;
                    SkASSERT(fScratchMap->countForKey(scratchKey));
                }
                SkASSERT(!resource->resourcePriv().refsWrappedObjects());
            }
            if (uniqueKey.isValid()) {
                ++fContent;
                SkASSERT(fUniqueHash->find(uniqueKey) == resource);
                SkASSERT(GrBudgetedType::kBudgeted == resource->resourcePriv().budgetedType() ||
                         resource->resourcePriv().refsWrappedObjects());

                if (scratchKey.isValid()) {
                    SkASSERT(!fScratchMap->has(resource, scratchKey));
                }
            }

            if (GrBudgetedType::kBudgeted == resource->resourcePriv().budgetedType()) {
                ++fBudgetedCount;
                fBudgetedBytes += resource->gpuMemorySize();
            }
        }
    };

    {
        ScratchMap::ConstIter iter(&fScratchMap);

        int count = 0;
        for ( ; !iter.done(); ++iter) {
            const GrGpuResource* resource = *iter;
            SkASSERT(resource->resourcePriv().getScratchKey().isValid());
            SkASSERT(!resource->getUniqueKey().isValid());
            count++;
        }
        SkASSERT(count == fScratchMap.count()); // ensure the iterator is working correctly
    }

    Stats stats(this);
    size_t purgeableBytes = 0;
    int numBudgetedResourcesFlushWillMakePurgeable = 0;

    for (int i = 0; i < fNonpurgeableResources.count(); ++i) {
        SkASSERT(!fNonpurgeableResources[i]->resourcePriv().isPurgeable() ||
                 fNewlyPurgeableResourceForValidation == fNonpurgeableResources[i]);
        SkASSERT(*fNonpurgeableResources[i]->cacheAccess().accessCacheIndex() == i);
        SkASSERT(!fNonpurgeableResources[i]->wasDestroyed());
        if (fNonpurgeableResources[i]->resourcePriv().budgetedType() == GrBudgetedType::kBudgeted &&
            !fNonpurgeableResources[i]->cacheAccess().hasRef() &&
            fNewlyPurgeableResourceForValidation != fNonpurgeableResources[i]) {
            ++numBudgetedResourcesFlushWillMakePurgeable;
        }
        stats.update(fNonpurgeableResources[i]);
    }
    for (int i = 0; i < fPurgeableQueue.count(); ++i) {
        SkASSERT(fPurgeableQueue.at(i)->resourcePriv().isPurgeable());
        SkASSERT(*fPurgeableQueue.at(i)->cacheAccess().accessCacheIndex() == i);
        SkASSERT(!fPurgeableQueue.at(i)->wasDestroyed());
        stats.update(fPurgeableQueue.at(i));
        purgeableBytes += fPurgeableQueue.at(i)->gpuMemorySize();
    }

    SkASSERT(fCount == this->getResourceCount());
    SkASSERT(fBudgetedCount <= fCount);
    SkASSERT(fBudgetedBytes <= fBytes);
    SkASSERT(stats.fBytes == fBytes);
    SkASSERT(fNumBudgetedResourcesFlushWillMakePurgeable ==
             numBudgetedResourcesFlushWillMakePurgeable);
    SkASSERT(stats.fBudgetedBytes == fBudgetedBytes);
    SkASSERT(stats.fBudgetedCount == fBudgetedCount);
    SkASSERT(purgeableBytes == fPurgeableBytes);
#if GR_CACHE_STATS
    SkASSERT(fBudgetedHighWaterCount <= fHighWaterCount);
    SkASSERT(fBudgetedHighWaterBytes <= fHighWaterBytes);
    SkASSERT(fBytes <= fHighWaterBytes);
    SkASSERT(fCount <= fHighWaterCount);
    SkASSERT(fBudgetedBytes <= fBudgetedHighWaterBytes);
    SkASSERT(fBudgetedCount <= fBudgetedHighWaterCount);
#endif
    SkASSERT(stats.fContent == fUniqueHash.count());
    SkASSERT(stats.fScratch + stats.fCouldBeScratch == fScratchMap.count());

    // This assertion is not currently valid because we can be in recursive notifyCntReachedZero()
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
