/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrResourceCache.h"
#include <atomic>
#include <vector>
#include "include/gpu/GrDirectContext.h"
#include "include/private/GrSingleOwner.h"
#include "include/private/SkTo.h"
#include "include/utils/SkRandom.h"
#include "src/core/SkMessageBus.h"
#include "src/core/SkOpts.h"
#include "src/core/SkScopeExit.h"
#include "src/core/SkTSort.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrGpuResourceCacheAccess.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrTexture.h"
#include "src/gpu/GrTextureProxyCacheAccess.h"
#include "src/gpu/GrThreadSafeCache.h"
#include "src/gpu/GrTracing.h"
#include "src/gpu/SkGr.h"

DECLARE_SKMESSAGEBUS_MESSAGE(GrUniqueKeyInvalidatedMessage, uint32_t, true);

DECLARE_SKMESSAGEBUS_MESSAGE(GrTextureFreedMessage, GrDirectContext::DirectContextID, true);

#define ASSERT_SINGLE_OWNER GR_ASSERT_SINGLE_OWNER(fSingleOwner)

//////////////////////////////////////////////////////////////////////////////

GrScratchKey::ResourceType GrScratchKey::GenerateResourceType() {
    static std::atomic<int32_t> nextType{INHERITED::kInvalidDomain + 1};

    int32_t type = nextType.fetch_add(1, std::memory_order_relaxed);
    if (type > SkTo<int32_t>(UINT16_MAX)) {
        SK_ABORT("Too many Resource Types");
    }

    return static_cast<ResourceType>(type);
}

GrUniqueKey::Domain GrUniqueKey::GenerateDomain() {
    static std::atomic<int32_t> nextDomain{INHERITED::kInvalidDomain + 1};

    int32_t domain = nextDomain.fetch_add(1, std::memory_order_relaxed);
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

inline GrResourceCache::TextureAwaitingUnref::TextureAwaitingUnref() = default;

inline GrResourceCache::TextureAwaitingUnref::TextureAwaitingUnref(GrTexture* texture)
        : fTexture(texture), fNumUnrefs(1) {}

inline GrResourceCache::TextureAwaitingUnref::TextureAwaitingUnref(TextureAwaitingUnref&& that) {
    fTexture = std::exchange(that.fTexture, nullptr);
    fNumUnrefs = std::exchange(that.fNumUnrefs, 0);
}

inline GrResourceCache::TextureAwaitingUnref& GrResourceCache::TextureAwaitingUnref::operator=(
        TextureAwaitingUnref&& that) {
    fTexture = std::exchange(that.fTexture, nullptr);
    fNumUnrefs = std::exchange(that.fNumUnrefs, 0);
    return *this;
}

inline GrResourceCache::TextureAwaitingUnref::~TextureAwaitingUnref() {
    if (fTexture) {
        for (int i = 0; i < fNumUnrefs; ++i) {
            fTexture->unref();
        }
    }
}

inline void GrResourceCache::TextureAwaitingUnref::TextureAwaitingUnref::addRef() { ++fNumUnrefs; }

inline void GrResourceCache::TextureAwaitingUnref::unref() {
    SkASSERT(fNumUnrefs > 0);
    fTexture->unref();
    --fNumUnrefs;
}

inline bool GrResourceCache::TextureAwaitingUnref::finished() { return !fNumUnrefs; }

//////////////////////////////////////////////////////////////////////////////

GrResourceCache::GrResourceCache(GrSingleOwner* singleOwner,
                                 GrDirectContext::DirectContextID owningContextID,
                                 uint32_t familyID)
        : fInvalidUniqueKeyInbox(familyID)
        , fFreedTextureInbox(owningContextID)
        , fOwningContextID(owningContextID)
        , fContextUniqueID(familyID)
        , fSingleOwner(singleOwner) {
    SkASSERT(owningContextID.isValid());
    SkASSERT(familyID != SK_InvalidUniqueID);
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
    fHighWaterCount = std::max(this->getResourceCount(), fHighWaterCount);
    fHighWaterBytes = std::max(fBytes, fHighWaterBytes);
#endif
    if (GrBudgetedType::kBudgeted == resource->resourcePriv().budgetedType()) {
        ++fBudgetedCount;
        fBudgetedBytes += size;
        TRACE_COUNTER2("skia.gpu.cache", "skia budget", "used",
                       fBudgetedBytes, "free", fMaxBytes - fBudgetedBytes);
#if GR_CACHE_STATS
        fBudgetedHighWaterCount = std::max(fBudgetedCount, fBudgetedHighWaterCount);
        fBudgetedHighWaterBytes = std::max(fBudgetedBytes, fBudgetedHighWaterBytes);
#endif
    }
    SkASSERT(!resource->cacheAccess().isUsableAsScratch());
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

    if (resource->cacheAccess().isUsableAsScratch()) {
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
    fTexturesAwaitingUnref.reset();

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

    fThreadSafeCache->dropAllRefs();

    SkASSERT(!fScratchMap.count());
    SkASSERT(!fUniqueHash.count());
    SkASSERT(!fCount);
    SkASSERT(!this->getResourceCount());
    SkASSERT(!fBytes);
    SkASSERT(!fBudgetedCount);
    SkASSERT(!fBudgetedBytes);
    SkASSERT(!fPurgeableBytes);
    SkASSERT(!fTexturesAwaitingUnref.count());
}

void GrResourceCache::releaseAll() {
    AutoValidate av(this);

    fThreadSafeCache->dropAllRefs();

    this->processFreedGpuResources();

    // We need to make sure to free any resources that were waiting on a free message but never
    // received one.
    fTexturesAwaitingUnref.reset();

    SkASSERT(fProxyProvider); // better have called setProxyProvider
    SkASSERT(fThreadSafeCache); // better have called setThreadSafeCache too

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
    SkASSERT(!fTexturesAwaitingUnref.count());
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
        // Everything that is in the scratch map should be usable as a
        // scratch resource.
        return true;
    }
};

GrGpuResource* GrResourceCache::findAndRefScratchResource(const GrScratchKey& scratchKey) {
    SkASSERT(scratchKey.isValid());

    GrGpuResource* resource = fScratchMap.find(scratchKey, AvailableForScratchUse());
    if (resource) {
        fScratchMap.remove(scratchKey, resource);
        this->refAndMakeResourceMRU(resource);
        this->validate();
    }
    return resource;
}

void GrResourceCache::willRemoveScratchKey(const GrGpuResource* resource) {
    ASSERT_SINGLE_OWNER
    SkASSERT(resource->resourcePriv().getScratchKey().isValid());
    if (resource->cacheAccess().isUsableAsScratch()) {
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
    if (resource->cacheAccess().isUsableAsScratch()) {
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
            // from the ScratchMap. The isUsableAsScratch call depends on us not adding the new
            // unique key until after this check.
            if (resource->cacheAccess().isUsableAsScratch()) {
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
    } else if (!resource->cacheAccess().hasRefOrCommandBufferUsage() &&
               resource->resourcePriv().budgetedType() == GrBudgetedType::kBudgeted) {
        SkASSERT(fNumBudgetedResourcesFlushWillMakePurgeable > 0);
        fNumBudgetedResourcesFlushWillMakePurgeable--;
    }
    resource->cacheAccess().ref();

    resource->cacheAccess().setTimestamp(this->getNextTimestamp());
    this->validate();
}

void GrResourceCache::notifyARefCntReachedZero(GrGpuResource* resource,
                                               GrGpuResource::LastRemovedRef removedRef) {
    ASSERT_SINGLE_OWNER
    SkASSERT(resource);
    SkASSERT(!resource->wasDestroyed());
    SkASSERT(this->isInCache(resource));
    // This resource should always be in the nonpurgeable array when this function is called. It
    // will be moved to the queue if it is newly purgeable.
    SkASSERT(fNonpurgeableResources[*resource->cacheAccess().accessCacheIndex()] == resource);

    if (removedRef == GrGpuResource::LastRemovedRef::kMainRef) {
        if (resource->cacheAccess().isUsableAsScratch()) {
            fScratchMap.insert(resource->resourcePriv().getScratchKey(), resource);
        }
    }

    if (resource->cacheAccess().hasRefOrCommandBufferUsage()) {
        this->validate();
        return;
    }

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
        fBudgetedHighWaterBytes = std::max(fBudgetedBytes, fBudgetedHighWaterBytes);
        fBudgetedHighWaterCount = std::max(fBudgetedCount, fBudgetedHighWaterCount);
#endif
        if (!resource->resourcePriv().isPurgeable() &&
            !resource->cacheAccess().hasRefOrCommandBufferUsage()) {
            ++fNumBudgetedResourcesFlushWillMakePurgeable;
        }
        if (resource->cacheAccess().isUsableAsScratch()) {
            fScratchMap.insert(resource->resourcePriv().getScratchKey(), resource);
        }
        this->purgeAsNeeded();
    } else {
        SkASSERT(resource->resourcePriv().budgetedType() != GrBudgetedType::kUnbudgetedCacheable);
        --fBudgetedCount;
        fBudgetedBytes -= size;
        if (!resource->resourcePriv().isPurgeable() &&
            !resource->cacheAccess().hasRefOrCommandBufferUsage()) {
            --fNumBudgetedResourcesFlushWillMakePurgeable;
        }
        if (!resource->cacheAccess().hasRef() && !resource->getUniqueKey().isValid() &&
            resource->resourcePriv().getScratchKey().isValid()) {
            fScratchMap.remove(resource->resourcePriv().getScratchKey(), resource);
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
            if (invalidKeyMsgs[i].inThreadSafeCache()) {
                fThreadSafeCache->remove(invalidKeyMsgs[i].key());
                SkASSERT(!fThreadSafeCache->has(invalidKeyMsgs[i].key()));
            } else {
                fProxyProvider->processInvalidUniqueKey(
                                                    invalidKeyMsgs[i].key(), nullptr,
                                                    GrProxyProvider::InvalidateGPUResource::kYes);
                SkASSERT(!this->findAndRefUniqueResource(invalidKeyMsgs[i].key()));
            }
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

    if (stillOverbudget) {
        fThreadSafeCache->dropUniqueRefs(this);

        stillOverbudget = this->overBudget();
        while (stillOverbudget && fPurgeableQueue.count()) {
            GrGpuResource* resource = fPurgeableQueue.peek();
            SkASSERT(resource->resourcePriv().isPurgeable());
            resource->cacheAccess().release();
            stillOverbudget = this->overBudget();
        }
    }

    this->validate();
}

void GrResourceCache::purgeUnlockedResources(const GrStdSteadyClock::time_point* purgeTime,
                                             bool scratchResourcesOnly) {

    if (!scratchResourcesOnly) {
        if (purgeTime) {
            fThreadSafeCache->dropUniqueRefsOlderThan(*purgeTime);
        } else {
            fThreadSafeCache->dropUniqueRefs(nullptr);
        }

        // We could disable maintaining the heap property here, but it would add a lot of
        // complexity. Moreover, this is rarely called.
        while (fPurgeableQueue.count()) {
            GrGpuResource* resource = fPurgeableQueue.peek();

            const GrStdSteadyClock::time_point resourceTime =
                    resource->cacheAccess().timeWhenResourceBecamePurgeable();
            if (purgeTime && resourceTime >= *purgeTime) {
                // Resources were given both LRU timestamps and tagged with a frame number when
                // they first became purgeable. The LRU timestamp won't change again until the
                // resource is made non-purgeable again. So, at this point all the remaining
                // resources in the timestamp-sorted queue will have a frame number >= to this
                // one.
                break;
            }

            SkASSERT(resource->resourcePriv().isPurgeable());
            resource->cacheAccess().release();
        }
    } else {
        // Early out if the very first item is too new to purge to avoid sorting the queue when
        // nothing will be deleted.
        if (purgeTime && fPurgeableQueue.count() &&
            fPurgeableQueue.peek()->cacheAccess().timeWhenResourceBecamePurgeable() >= *purgeTime) {
            return;
        }

        // Sort the queue
        fPurgeableQueue.sort();

        // Make a list of the scratch resources to delete
        SkTDArray<GrGpuResource*> scratchResources;
        for (int i = 0; i < fPurgeableQueue.count(); i++) {
            GrGpuResource* resource = fPurgeableQueue.at(i);

            const GrStdSteadyClock::time_point resourceTime =
                    resource->cacheAccess().timeWhenResourceBecamePurgeable();
            if (purgeTime && resourceTime >= *purgeTime) {
                // scratch or not, all later iterations will be too recently used to purge.
                break;
            }
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

bool GrResourceCache::purgeToMakeHeadroom(size_t desiredHeadroomBytes) {
    AutoValidate av(this);
    if (desiredHeadroomBytes > fMaxBytes) {
        return false;
    }
    if (this->wouldFit(desiredHeadroomBytes)) {
        return true;
    }
    fPurgeableQueue.sort();

    size_t projectedBudget = fBudgetedBytes;
    int purgeCnt = 0;
    for (int i = 0; i < fPurgeableQueue.count(); i++) {
        GrGpuResource* resource = fPurgeableQueue.at(i);
        if (GrBudgetedType::kBudgeted == resource->resourcePriv().budgetedType()) {
            projectedBudget -= resource->gpuMemorySize();
        }
        if (projectedBudget + desiredHeadroomBytes <= fMaxBytes) {
            purgeCnt = i + 1;
            break;
        }
    }
    if (purgeCnt == 0) {
        return false;
    }

    // Success! Release the resources.
    // Copy to array first so we don't mess with the queue.
    std::vector<GrGpuResource*> resources;
    resources.reserve(purgeCnt);
    for (int i = 0; i < purgeCnt; i++) {
        resources.push_back(fPurgeableQueue.at(i));
    }
    for (GrGpuResource* resource : resources) {
        resource->cacheAccess().release();
    }
    return true;
}

void GrResourceCache::purgeUnlockedResources(size_t bytesToPurge, bool preferScratchResources) {

    const size_t tmpByteBudget = std::max((size_t)0, fBytes - bytesToPurge);
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

void GrResourceCache::insertDelayedTextureUnref(GrTexture* texture) {
    texture->ref();
    uint32_t id = texture->uniqueID().asUInt();
    if (auto* data = fTexturesAwaitingUnref.find(id)) {
        data->addRef();
    } else {
        fTexturesAwaitingUnref.set(id, {texture});
    }
}

void GrResourceCache::processFreedGpuResources() {
    if (!fTexturesAwaitingUnref.count()) {
        return;
    }

    SkTArray<GrTextureFreedMessage> msgs;
    fFreedTextureInbox.poll(&msgs);
    for (int i = 0; i < msgs.count(); ++i) {
        SkASSERT(msgs[i].fIntendedRecipient == fOwningContextID);
        uint32_t id = msgs[i].fTexture->uniqueID().asUInt();
        TextureAwaitingUnref* info = fTexturesAwaitingUnref.find(id);
        // If the GrContext was released or abandoned then fTexturesAwaitingUnref should have been
        // empty and we would have returned early above. Thus, any texture from a message should be
        // in the list of fTexturesAwaitingUnref.
        SkASSERT(info);
        info->unref();
        if (info->finished()) {
            fTexturesAwaitingUnref.remove(id);
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
    // Fill the hole we will create in the array with the tail object, adjust its index, and
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

            SkTQSort(fNonpurgeableResources.begin(), fNonpurgeableResources.end(),
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
#endif // GR_TEST_UTILS
#endif // GR_CACHE_STATS

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

            if (resource->cacheAccess().isUsableAsScratch()) {
                SkASSERT(!uniqueKey.isValid());
                SkASSERT(GrBudgetedType::kBudgeted == resource->resourcePriv().budgetedType());
                SkASSERT(!resource->cacheAccess().hasRef());
                ++fScratch;
                SkASSERT(fScratchMap->countForKey(scratchKey));
                SkASSERT(!resource->resourcePriv().refsWrappedObjects());
            } else if (scratchKey.isValid()) {
                SkASSERT(GrBudgetedType::kBudgeted != resource->resourcePriv().budgetedType() ||
                         uniqueKey.isValid() || resource->cacheAccess().hasRef());
                SkASSERT(!resource->resourcePriv().refsWrappedObjects());
                SkASSERT(!fScratchMap->has(resource, scratchKey));
            }
            if (uniqueKey.isValid()) {
                ++fContent;
                SkASSERT(fUniqueHash->find(uniqueKey) == resource);
                SkASSERT(GrBudgetedType::kBudgeted == resource->resourcePriv().budgetedType() ||
                         resource->resourcePriv().refsWrappedObjects());
            }

            if (GrBudgetedType::kBudgeted == resource->resourcePriv().budgetedType()) {
                ++fBudgetedCount;
                fBudgetedBytes += resource->gpuMemorySize();
            }
        }
    };

    {
        int count = 0;
        fScratchMap.foreach([&](const GrGpuResource& resource) {
            SkASSERT(resource.cacheAccess().isUsableAsScratch());
            count++;
        });
        SkASSERT(count == fScratchMap.count());
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
            !fNonpurgeableResources[i]->cacheAccess().hasRefOrCommandBufferUsage() &&
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
    SkASSERT(stats.fScratch == fScratchMap.count());

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

#endif // SK_DEBUG

#if GR_TEST_UTILS

int GrResourceCache::countUniqueKeysWithTag(const char* tag) const {
    int count = 0;
    fUniqueHash.foreach([&](const GrGpuResource& resource){
        if (0 == strcmp(tag, resource.getUniqueKey().tag())) {
            ++count;
        }
    });
    return count;
}

void GrResourceCache::changeTimestamp(uint32_t newTimestamp) {
    fTimestamp = newTimestamp;
}

#endif // GR_TEST_UTILS
