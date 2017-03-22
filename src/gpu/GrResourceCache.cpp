/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrResourceCache.h"

#include "GrCaps.h"
#include "GrGpuResourceCacheAccess.h"
#include "GrTracing.h"
#include "SkGr.h"
#include "SkMessageBus.h"
#include "SkOpts.h"
#include "SkTSort.h"

DECLARE_SKMESSAGEBUS_MESSAGE(GrUniqueKeyInvalidatedMessage);

//////////////////////////////////////////////////////////////////////////////

GrScratchKey::ResourceType GrScratchKey::GenerateResourceType() {
    static int32_t gType = INHERITED::kInvalidDomain + 1;

    int32_t type = sk_atomic_inc(&gType);
    if (type > SK_MaxU16) {
        SkFAIL("Too many Resource Types");
    }

    return static_cast<ResourceType>(type);
}

GrUniqueKey::Domain GrUniqueKey::GenerateDomain() {
    static int32_t gDomain = INHERITED::kInvalidDomain + 1;

    int32_t domain = sk_atomic_inc(&gDomain);
    if (domain > SK_MaxU16) {
        SkFAIL("Too many GrUniqueKey Domains");
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


GrResourceCache::GrResourceCache(const GrCaps* caps)
    : fTimestamp(0)
    , fMaxCount(kDefaultMaxCount)
    , fMaxBytes(kDefaultMaxSize)
    , fMaxUnusedFlushes(kDefaultMaxUnusedFlushes)
#if GR_CACHE_STATS
    , fHighWaterCount(0)
    , fHighWaterBytes(0)
    , fBudgetedHighWaterCount(0)
    , fBudgetedHighWaterBytes(0)
#endif
    , fBytes(0)
    , fBudgetedCount(0)
    , fBudgetedBytes(0)
    , fRequestFlush(false)
    , fExternalFlushCnt(0)
    , fPreferVRAMUseOverFlushes(caps->preferVRAMUseOverFlushes()) {
    SkDEBUGCODE(fCount = 0;)
    SkDEBUGCODE(fNewlyPurgeableResourceForValidation = nullptr;)
}

GrResourceCache::~GrResourceCache() {
    this->releaseAll();
}

void GrResourceCache::setLimits(int count, size_t bytes, int maxUnusedFlushes) {
    fMaxCount = count;
    fMaxBytes = bytes;
    fMaxUnusedFlushes = maxUnusedFlushes;
    this->purgeAsNeeded();
}

void GrResourceCache::insertResource(GrGpuResource* resource) {
    SkASSERT(resource);
    SkASSERT(!this->isInCache(resource));
    SkASSERT(!resource->wasDestroyed());
    SkASSERT(!resource->isPurgeable());

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
    if (SkBudgeted::kYes == resource->resourcePriv().isBudgeted()) {
        ++fBudgetedCount;
        fBudgetedBytes += size;
        TRACE_COUNTER2(TRACE_DISABLED_BY_DEFAULT("skia.gpu.cache"), "skia budget", "used",
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
    if (SkBudgeted::kYes == resource->resourcePriv().isBudgeted()) {
        --fBudgetedCount;
        fBudgetedBytes -= size;
        TRACE_COUNTER2(TRACE_DISABLED_BY_DEFAULT("skia.gpu.cache"), "skia budget", "used",
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
    SkASSERT(!fUniqueHash.count());
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
        SkASSERT(!resource->getUniqueKey().isValid() &&
                 resource->resourcePriv().getScratchKey().isValid());
        if (resource->internalHasRef() || !resource->cacheAccess().isScratch()) {
            return false;
        }
        return !fRejectPendingIO || !resource->internalHasPendingIO();
    }

private:
    bool fRejectPendingIO;
};

GrGpuResource* GrResourceCache::findAndRefScratchResource(const GrScratchKey& scratchKey,
                                                          size_t resourceSize,
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
    resource = fScratchMap.find(scratchKey, AvailableForScratchUse(false));
    if (resource) {
        this->refAndMakeResourceMRU(resource);
        this->validate();
    }
    return resource;
}

void GrResourceCache::willRemoveScratchKey(const GrGpuResource* resource) {
    SkASSERT(resource->resourcePriv().getScratchKey().isValid());
    if (!resource->getUniqueKey().isValid()) {
        fScratchMap.remove(resource->resourcePriv().getScratchKey(), resource);
    }
}

void GrResourceCache::removeUniqueKey(GrGpuResource* resource) {
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

    this->validate();
}

void GrResourceCache::changeUniqueKey(GrGpuResource* resource, const GrUniqueKey& newKey) {
    SkASSERT(resource);
    SkASSERT(this->isInCache(resource));

    // If another resource has the new key, remove its key then install the key on this resource.
    if (newKey.isValid()) {
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

        if (GrGpuResource* old = fUniqueHash.find(newKey)) {
            // If the old resource using the key is purgeable and is unreachable, then remove it.
            if (!old->resourcePriv().getScratchKey().isValid() && old->isPurgeable()) {
                // release may call validate() which will assert that resource is in fUniqueHash
                // if it has a valid key. So in debug reset the key here before we assign it.
                SkDEBUGCODE(resource->cacheAccess().removeUniqueKey();)
                old->cacheAccess().release();
            } else {
                this->removeUniqueKey(old);
            }
        }
        SkASSERT(nullptr == fUniqueHash.find(newKey));
        resource->cacheAccess().setUniqueKey(newKey);
        fUniqueHash.add(resource);
    } else {
        this->removeUniqueKey(resource);
    }

    this->validate();
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

    resource->cacheAccess().setTimestamp(this->getNextTimestamp());
    this->validate();
}

void GrResourceCache::notifyCntReachedZero(GrGpuResource* resource, uint32_t flags) {
    SkASSERT(resource);
    SkASSERT(!resource->wasDestroyed());
    SkASSERT(flags);
    SkASSERT(this->isInCache(resource));
    // This resource should always be in the nonpurgeable array when this function is called. It
    // will be moved to the queue if it is newly purgeable.
    SkASSERT(fNonpurgeableResources[*resource->cacheAccess().accessCacheIndex()] == resource);

    if (SkToBool(ResourceAccess::kRefCntReachedZero_RefNotificationFlag & flags)) {
#ifdef SK_DEBUG
        // When the timestamp overflows validate() is called. validate() checks that resources in
        // the nonpurgeable array are indeed not purgeable. However, the movement from the array to
        // the purgeable queue happens just below in this function. So we mark it as an exception.
        if (resource->isPurgeable()) {
            fNewlyPurgeableResourceForValidation = resource;
        }
#endif
        resource->cacheAccess().setTimestamp(this->getNextTimestamp());
        SkDEBUGCODE(fNewlyPurgeableResourceForValidation = nullptr);
    }

    if (!SkToBool(ResourceAccess::kAllCntsReachedZero_RefNotificationFlag & flags)) {
        SkASSERT(!resource->isPurgeable());
        return;
    }

    SkASSERT(resource->isPurgeable());
    this->removeFromNonpurgeableArray(resource);
    fPurgeableQueue.insert(resource);
    resource->cacheAccess().setFlushCntWhenResourceBecamePurgeable(fExternalFlushCnt);
    resource->cacheAccess().setTimeWhenResourceBecomePurgeable();

    if (SkBudgeted::kNo == resource->resourcePriv().isBudgeted()) {
        // Check whether this resource could still be used as a scratch resource.
        if (!resource->resourcePriv().refsWrappedObjects() &&
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
        // Also purge if the resource has neither a valid scratch key nor a unique key.
        bool noKey = !resource->resourcePriv().getScratchKey().isValid() &&
                     !resource->getUniqueKey().isValid();
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
    if (SkBudgeted::kYes == resource->resourcePriv().isBudgeted()) {
        fBudgetedBytes += delta;
        TRACE_COUNTER2(TRACE_DISABLED_BY_DEFAULT("skia.gpu.cache"), "skia budget", "used",
                       fBudgetedBytes, "free", fMaxBytes - fBudgetedBytes);
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

    if (SkBudgeted::kYes == resource->resourcePriv().isBudgeted()) {
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
    TRACE_COUNTER2(TRACE_DISABLED_BY_DEFAULT("skia.gpu.cache"), "skia budget", "used",
                   fBudgetedBytes, "free", fMaxBytes - fBudgetedBytes);

    this->validate();
}

void GrResourceCache::purgeAsNeeded() {
    SkTArray<GrUniqueKeyInvalidatedMessage> invalidKeyMsgs;
    fInvalidUniqueKeyInbox.poll(&invalidKeyMsgs);
    if (invalidKeyMsgs.count()) {
        this->processInvalidUniqueKeys(invalidKeyMsgs);
    }

    if (fMaxUnusedFlushes > 0) {
        // We want to know how many complete flushes have occurred without the resource being used.
        // If the resource was tagged when fExternalFlushCnt was N then this means it became
        // purgeable during activity that became the N+1th flush. So when the flush count is N+2
        // it has sat in the purgeable queue for one entire flush.
        uint32_t oldestAllowedFlushCnt = fExternalFlushCnt - fMaxUnusedFlushes - 1;
        // check for underflow
        if (oldestAllowedFlushCnt < fExternalFlushCnt) {
            while (fPurgeableQueue.count()) {
                uint32_t flushWhenResourceBecamePurgeable =
                        fPurgeableQueue.peek()->cacheAccess().flushCntWhenResourceBecamePurgeable();
                if (oldestAllowedFlushCnt < flushWhenResourceBecamePurgeable) {
                    // Resources were given both LRU timestamps and tagged with a flush cnt when
                    // they first became purgeable. The LRU timestamp won't change again until the
                    // resource is made non-purgeable again. So, at this point all the remaining
                    // resources in the timestamp-sorted queue will have a flush count >= to this
                    // one.
                    break;
                }
                GrGpuResource* resource = fPurgeableQueue.peek();
                SkASSERT(resource->isPurgeable());
                resource->cacheAccess().release();
            }
        }
    }

    bool stillOverbudget = this->overBudget();
    while (stillOverbudget && fPurgeableQueue.count()) {
        GrGpuResource* resource = fPurgeableQueue.peek();
        SkASSERT(resource->isPurgeable());
        resource->cacheAccess().release();
        stillOverbudget = this->overBudget();
    }

    this->validate();

    if (stillOverbudget) {
        // Set this so that GrDrawingManager will issue a flush to free up resources with pending
        // IO that we were unable to purge in this pass.
        fRequestFlush = true;
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
        SkASSERT(resource->isPurgeable());
        resource->cacheAccess().release();
    }
}

void GrResourceCache::processInvalidUniqueKeys(
    const SkTArray<GrUniqueKeyInvalidatedMessage>& msgs) {
    for (int i = 0; i < msgs.count(); ++i) {
        GrGpuResource* resource = this->findAndRefUniqueResource(msgs[i].key());
        if (resource) {
            resource->resourcePriv().removeUniqueKey();
            resource->unref(); // If this resource is now purgeable, the cache will be notified.
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

void GrResourceCache::notifyFlushOccurred(FlushType type) {
    switch (type) {
        case FlushType::kImmediateMode:
            break;
        case FlushType::kCacheRequested:
            SkASSERT(fRequestFlush);
            fRequestFlush = false;
            break;
        case FlushType::kExternal:
            ++fExternalFlushCnt;
            if (0 == fExternalFlushCnt) {
                // When this wraps just reset all the purgeable resources' last used flush state.
                for (int i = 0; i < fPurgeableQueue.count(); ++i) {
                    fPurgeableQueue.at(i)->cacheAccess().setFlushCntWhenResourceBecamePurgeable(0);
                }
            }
            break;
    }
    this->purgeAsNeeded();
}

void GrResourceCache::dumpMemoryStatistics(SkTraceMemoryDump* traceMemoryDump) const {
    for (int i = 0; i < fNonpurgeableResources.count(); ++i) {
        fNonpurgeableResources[i]->dumpMemoryStatistics(traceMemoryDump);
    }
    for (int i = 0; i < fPurgeableQueue.count(); ++i) {
        fPurgeableQueue.at(i)->dumpMemoryStatistics(traceMemoryDump);
    }
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
        const UniqueHash* fUniqueHash;

        Stats(const GrResourceCache* cache) {
            memset(this, 0, sizeof(*this));
            fScratchMap = &cache->fScratchMap;
            fUniqueHash = &cache->fUniqueHash;
        }

        void update(GrGpuResource* resource) {
            fBytes += resource->gpuMemorySize();

            if (!resource->isPurgeable()) {
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
                SkASSERT(SkBudgeted::kNo == resource->resourcePriv().isBudgeted() ||
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
                SkASSERT(!resource->resourcePriv().refsWrappedObjects());
                SkASSERT(SkBudgeted::kYes == resource->resourcePriv().isBudgeted());

                if (scratchKey.isValid()) {
                    SkASSERT(!fScratchMap->has(resource, scratchKey));
                }
            }

            if (SkBudgeted::kYes == resource->resourcePriv().isBudgeted()) {
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

    for (int i = 0; i < fNonpurgeableResources.count(); ++i) {
        SkASSERT(!fNonpurgeableResources[i]->isPurgeable() ||
                 fNewlyPurgeableResourceForValidation == fNonpurgeableResources[i]);
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
