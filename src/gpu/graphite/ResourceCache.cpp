/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/ResourceCache.h"

#include "include/private/base/SingleOwner.h"
#include "src/base/SkNoDestructor.h"
#include "src/base/SkRandom.h"
#include "src/core/SkTMultiMap.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/graphite/GraphiteResourceKey.h"
#include "src/gpu/graphite/ProxyCache.h"
#include "src/gpu/graphite/Resource.h"

#if defined(GPU_TEST_UTILS)
#include "src/gpu/graphite/Texture.h"
#endif

namespace skgpu::graphite {

#define ASSERT_SINGLE_OWNER SKGPU_ASSERT_SINGLE_OWNER(fSingleOwner)

namespace {

static constexpr uint32_t kMaxUseToken = 0xFFFFFFFF;

// Singleton Resource subclass that provides a fixed address used to track the end of the return
// queue and when the resource cache is shutdown.
class Sentinel : public Resource {
public:
    static Resource* Get() {
        static SkNoDestructor<Sentinel> kSentinel{};
        return kSentinel.get();
    }

private:
    template <typename T>
    friend class ::SkNoDestructor;

    // We can pass in a null shared context here because the only instance that is ever created is
    // wrapped in SkNoDestructor, and we never actually use it as a Resource.
    Sentinel() : Resource(/*sharedContext=*/nullptr, Ownership::kOwned, /*gpuMemorySize=*/0) {}

    const char* getResourceType() const override { return "Sentinel"; }

    void freeGpuData() override {}
};

}  // anonymous namespace

sk_sp<ResourceCache> ResourceCache::Make(SingleOwner* singleOwner,
                                         uint32_t recorderID,
                                         size_t maxBytes) {
    return sk_sp<ResourceCache>(new ResourceCache(singleOwner, recorderID, maxBytes));
}

ResourceCache::ResourceCache(SingleOwner* singleOwner, uint32_t recorderID, size_t maxBytes)
        : fMaxBytes(maxBytes)
        , fSingleOwner(singleOwner) {
    if (recorderID != SK_InvalidGenID) {
        fProxyCache = std::make_unique<ProxyCache>(recorderID);
    }
    // TODO: Maybe when things start using ResourceCache, then like Ganesh the compiler won't
    // complain about not using fSingleOwner in Release builds and we can delete this.
#if !defined(SK_DEBUG)
    (void)fSingleOwner;
#endif
}

ResourceCache::~ResourceCache() {
    // The ResourceCache must have been shutdown by the ResourceProvider before it is destroyed.
    SkASSERT(fReturnQueue.load(std::memory_order_acquire) == Sentinel::Get());
}

void ResourceCache::shutdown() {
    ASSERT_SINGLE_OWNER

    // At this point no more changes will happen to fReturnQueue or the resources within that
    // linked list. We do need to finish processing them for a graceful shutdown.
    this->processReturnedResources(Sentinel::Get());

    if (fProxyCache) {
        fProxyCache->purgeAll();
        // NOTE: any resources that would become purgeable or reusable from purging the proxy cache
        // are not added to the return queue and remain in the nonpurgeable array. Below their
        // cache ref will be removed, causing them to be deleted immediately.
    }

    while (!fNonpurgeableResources.empty()) {
        Resource* back = *(fNonpurgeableResources.end() - 1);
        SkASSERT(!back->wasDestroyed());
        this->removeFromNonpurgeableArray(back);
        back->unrefCache();
    }

    while (fPurgeableQueue.count()) {
        Resource* top = fPurgeableQueue.peek();
        SkASSERT(!top->wasDestroyed());
        this->removeFromPurgeableQueue(top);
        top->unrefCache();
    }

    TRACE_EVENT_INSTANT0("skia.gpu.cache", TRACE_FUNC, TRACE_EVENT_SCOPE_THREAD);
}

void ResourceCache::insertResource(Resource* resource,
                                   const GraphiteResourceKey& key,
                                   Budgeted budgeted,
                                   Shareable shareable) {
    ASSERT_SINGLE_OWNER
    SkASSERT(resource);
    SkASSERT(key.isValid());
    SkASSERT(shareable == Shareable::kNo || budgeted == Budgeted::kYes);

    SkASSERT(!this->isInCache(resource));
    SkASSERT(!resource->wasDestroyed());
    SkASSERT(!resource->isPurgeable());
    SkASSERT(!resource->key().isValid());
    // All resources in the cache are owned. If we track wrapped resources in the cache we'll need
    // to update this check.
    SkASSERT(resource->ownership() == Ownership::kOwned);

    // Make sure we have the most accurate memory size for "memoryless" resources.
    resource->updateGpuMemorySize();

    // The reason to call processReturnedResources here is to get an accurate accounting of our
    // memory usage as some resources can go from unbudgeted to budgeted when they return. So we
    // want to have them all returned before adding the budget for the new resource in case we need
    // to purge things. However, if the new resource has a memory size of 0, then we just skip
    // returning resources (which has overhead for each call) since the new resource won't be
    // affecting whether we're over or under budget.
    if (resource->gpuMemorySize() > 0) {
        this->processReturnedResources();
    }

    resource->registerWithCache(sk_ref_sp(this), key, budgeted, shareable);

    // We must set the use token before adding to the array in case the token wraps and we wind
    // up iterating over all the resources that already have use tokens.
    this->setResourceUseToken(resource, this->getNextUseToken());
    resource->updateAccessTime();

    this->addToNonpurgeableArray(resource);

    SkDEBUGCODE(fCount++;)

    if (resource->shareable() != Shareable::kNo) {
        // Scratch and shareable resources are always available for reuse
        this->addToResourceMap(resource);
    }

    if (resource->budgeted() == Budgeted::kYes) {
        fBudgetedBytes += resource->gpuMemorySize();
    }

    this->purgeAsNeeded();
}

Resource* ResourceCache::findAndRefResource(const GraphiteResourceKey& key,
                                            Budgeted budgeted,
                                            Shareable shareable,
                                            const ScratchResourceSet* unavailable) {
    ASSERT_SINGLE_OWNER

    SkASSERT(key.isValid());
    SkASSERT(shareable == Shareable::kNo || budgeted == Budgeted::kYes);
    SkASSERT(shareable != Shareable::kScratch || SkToBool(unavailable));

    auto shareablePredicate = [shareable, unavailable](Resource* r) {
        // If the resource is in fResourceMap then it's available, so a non-shareable state means
        // it really has no outstanding uses and can be converted to any other shareable state.
        // Otherwise, if it's available, it can only be reused with the same mode. Additionally,
        // for kScratch resources, they cannot already be in the `unavailable` set passed in.
        return (r->shareable() == Shareable::kNo || r->shareable() == shareable) &&
               (shareable != Shareable::kScratch || !unavailable->contains(r));
    };

    Resource* resource = fResourceMap.find(key, shareablePredicate);
    if (!resource) {
        // The main reason to call processReturnedResources in this call is to see if there are any
        // resources that we could match with the key. However, there is overhead into calling it.
        // So we only call it if we first failed to find a matching resource.
        if (this->processReturnedResources()) {
            resource = fResourceMap.find(key, shareablePredicate);
        }
    }
    if (resource) {
        // All resources we pull out of the cache for use should be budgeted
        SkASSERT(resource->budgeted() == Budgeted::kYes);
        SkASSERT(resource->key() == key);

        if (shareable == Shareable::kNo) {
            // If the returned resource is no longer shareable then we remove it from the map so
            // that it isn't found again.
            SkASSERT(resource->shareable() == Shareable::kNo);
            this->removeFromResourceMap(resource);
            if (budgeted == Budgeted::kNo) {
                resource->setBudgeted(Budgeted::kNo);
                fBudgetedBytes -= resource->gpuMemorySize();
            }
        } else {
            // Shareable and scratch resources should never be requested as non-budgeted
            SkASSERT(budgeted == Budgeted::kYes);
            resource->setShareable(shareable);
        }
        this->refAndMakeResourceMRU(resource);
        this->validate();
    }

    // processReturnedResources may have added resources back into our budget if they were being
    // using in an SkImage or SkSurface previously. However, instead of calling purgeAsNeeded in
    // processReturnedResources, we delay calling it until now so we don't end up purging a resource
    // we're looking for in this function.
    //
    // We could avoid calling this if we didn't return any resources from processReturnedResources.
    // However, when not overbudget purgeAsNeeded is very cheap. When overbudget there may be some
    // really niche usage patterns that could cause us to never actually return resources to the
    // cache, but still be overbudget due to shared resources. So to be safe we just always call it
    // here.
    this->purgeAsNeeded();

    return resource;
}

void ResourceCache::refAndMakeResourceMRU(Resource* resource) {
    SkASSERT(resource);
    SkASSERT(this->isInCache(resource));

    if (this->inPurgeableQueue(resource)) {
        // It's about to become unpurgeable.
        this->removeFromPurgeableQueue(resource);
        this->addToNonpurgeableArray(resource);
    }
    resource->initialUsageRef();

    this->setResourceUseToken(resource, this->getNextUseToken());
    this->validate();
}

void ResourceCache::forceProcessReturnedResources() {
    ASSERT_SINGLE_OWNER
    this->processReturnedResources();
}

bool ResourceCache::returnResource(Resource* resource) {
    SkASSERT(resource && resource->cache() == this);
    // We only allow one instance of a Resource to be in the return queue at a time but it should
    // have already added a return queue ref.
    SkASSERT(!resource->inReturnQueue() && resource->hasReturnQueueRef());

    // Check once with a relaxed load to try and minimize the amount of wasted preparation work if
    // the cache is already shutdown.
    Resource* oldHeadPtr = fReturnQueue.load(std::memory_order_relaxed);
    if (oldHeadPtr == Sentinel::Get()) {
        return false;
    }

    // When a non-shareable resource's CB and Usage refs are both zero, give it a chance to prepare
    // itself to be reused. On Dawn/WebGPU we use this to remap kXferCpuToGpu buffers asynchronously
    // so that they are already mapped before they come out of the cache again.
    if (resource->shouldDeleteASAP() == Resource::DeleteASAP::kNo &&
        resource->shareable() == Shareable::kNo) {
        // If we get here, we know the usage ref count is 0, so the only way for that to increase
        // again is if the Resource triggers the initial usage ref in the callback.
        SkDEBUGCODE(bool takeRefActuallyCalled = false;)
        bool takeRefCalled = resource->prepareForReturnToCache([&] {
                // This adds a usage ref AND removes the return queue ref. When returnResource()
                // returns true, the cache takes responsibility for releasing the return queue ref.
                // If we returned false from returnResource() when the resource invokes the takeRef
                // function, there's a gap between when the resource can be used on another thread
                // and when this thread removes the return queue ref. If the resource's new usage
                // ref is removed on the other thread before this thread were to remove the return
                // queue ref, it would end up skipping the return.
                //
                // By immediately unreffing the return queue ref before the resource can be exposed
                // to another thread, the resource will always be able to be re-returned when the
                // async work completes.
                //
                // Since prepareForReturnToCache() can only be used with resources that require
                // purgeability for reusability, and it is non-shareable, the only ref that can
                // change off thread is the resource's cache ref if the cache is simultaneously
                // shutdown.
                //
                // Adding the usage ref first ensures the resource won't be disposed of early. When
                // the resource is prepared, it will come through returnResource() again but should
                // return false from prepareForReturnToCache() so that cache shutdown is detected.
                // This can add unnecessary preparation work for resources that won't ever be used,
                // but keeps the preparation logic relatively simple w/o needing a mutex.
                resource->initialUsageRef();
                resource->unrefReturnQueue();

                SkDEBUGCODE(takeRefActuallyCalled = true;
            )});

        SkASSERT(takeRefCalled == takeRefActuallyCalled);
        if (takeRefCalled) {
            // Return 'true' here because we've removed the return queue ref already and don't
            // want Resource to try and do that again. But since we added an initial ref, this
            // resource will be re-returned once the async prepare-for-return work has finished.
            return true;
        }
    }

    // Set the newly returned resource to be the head of the list, with its next pointer holding
    // the old head. If the head has changed between the assignment of the next pointer, we repeat
    // because it means there was a simultaneous return or the cache was shutdown.
    do {
        oldHeadPtr = fReturnQueue.load(std::memory_order_acquire);
        if (oldHeadPtr == Sentinel::Get()) {
            // Once the cache is shutdown, it can never be re-opened and we don't want to actually
            // return this resource.
            resource->setNextInReturnQueue(nullptr);
            return false;
        } else {
            // If oldHeadPtr is null, this resource will be the tail of the return queue as it grows
            // so set it's next pointer to the sentinel so that nullity can be used to test for
            // being in the queue or not.
            resource->setNextInReturnQueue(oldHeadPtr ? oldHeadPtr : Sentinel::Get());
        }
    } while(!fReturnQueue.compare_exchange_weak(oldHeadPtr, resource,
                                                std::memory_order_release,
                                                std::memory_order_relaxed));

    // Once we've got here, it means that `resource` has been atomically included in the return
    // queue. At this point, fReturnQueue's head value may or may not be `resource`, depending on if
    // another thread has added another resource or processed the return queue, but in either event,
    // `resource` will be visible to that thread.
    return true;
}

bool ResourceCache::processReturnedResources(Resource* queueHead) {
    SkASSERT(queueHead == nullptr || queueHead == Sentinel::Get());
    // We need to move the returned Resources off of the ReturnQueue before we start processing them
    // so that we can manipulate the resources without blocking subsequent returns on other threads.
    Resource* oldQueue = fReturnQueue.exchange(queueHead, std::memory_order_acq_rel);

    // Can't un-shutdown the cache
    SkASSERT(oldQueue != Sentinel::Get() || queueHead == Sentinel::Get());

    int returnCount = 0;
    // Stop if we encounter null or the sentinel address (either the list is empty, the cache is
    // shutdown, or we reached the tail returned resource that had next set to the sentinel).
    while (oldQueue && oldQueue != Sentinel::Get()) {
        returnCount++;
        oldQueue = this->processReturnedResource(oldQueue);
    }

    TRACE_EVENT_INSTANT1("skia.gpu.cache", TRACE_FUNC, TRACE_EVENT_SCOPE_THREAD,
                         "count", returnCount);
    return returnCount > 0;
}

Resource* ResourceCache::processReturnedResource(Resource* resource) {
    // A resource should not have been destroyed when placed into the return queue. Also before
    // purging any resources from the cache itself, it should always empty the queue first. When the
    // cache releases/abandons all of its resources, it first invalidates the return queue so no new
    // resources can be added. Thus we should not end up in a situation where a resource gets
    // destroyed after it was added to the return queue.
    SkASSERT(!resource->wasDestroyed());
    SkASSERT(this->isInCache(resource));

    const auto [isReusable, isPurgeable, next] = resource->unrefReturnQueue();

    if (resource->shareable() != Shareable::kNo) {
        // Shareable resources should still be discoverable in the resource map
        SkASSERT(fResourceMap.has(resource, resource->key()));
        SkASSERT(resource->isAvailableForReuse());

        // Reset the resource's sharing mode so that any shareable request can use it (e.g. now that
        // no more usages that required it to be scratch/shareable are held, the underlying resource
        // can be used in a non-shareable manner the next time it's fetched from the cache). We can
        // only change the shareable state when there are no outstanding usage refs. Because this
        // resource was shareable, it remained in fResourceMap and could have a new usage ref before
        // a prior return event was processed from the return queue. However, when a shareable ref
        // has no usage refs, this is the only thread that can add an initial usage ref so it is
        // safe to adjust its shareable type
        if (isReusable) {
            resource->setShareable(Shareable::kNo);
        }
    } else if (isReusable) {
        // Non-shareable resources are removed from the resource map when they are given out by the
        // cache. A resource is returned for either becoming reusable (needs to be added to the
        // resource map) or becoming purgeable (needs to be moved to the purgeable queue). Becoming
        // purgeable always implies becoming reusable, so as long as a previous return hasn't put it
        // into the resource map already, we do that now.
        if (!resource->isAvailableForReuse()) {
            SkASSERT(!fResourceMap.has(resource, resource->key()));
            this->addToResourceMap(resource);

            if (resource->budgeted() == Budgeted::kNo) {
                resource->setBudgeted(Budgeted::kYes);
                fBudgetedBytes += resource->gpuMemorySize();
            }
        }

        SkASSERT(fResourceMap.has(resource, resource->key()));
        SkASSERT(resource->isAvailableForReuse());
        // Since the resource should be non-shareable available as scratch, there are no outstanding
        // refs that would make this assert not thread safe.
        SkASSERT(resource->isUsableAsScratch());
    } else {
        // This was a stale entry in the return queue, which can arise when a Resource becomes
        // reusable while it has outstanding command buffer refs. If the timing is right, the
        // command buffer ref can be removed so the resource is purgeable (and goes back into the
        // queue to be processed from non-purgeable to purgeable), but immediately after, the cache
        // thread can add a usage ref. By the next time the return queue is processed, the resource
        // is neither purgeable nor reusable.
        SkASSERT(!fResourceMap.has(resource, resource->key()));
        SkASSERT(!resource->isAvailableForReuse());
        // At an instanteous moment, this resource should not be considered usable as scratch, but
        // we cannot assert !isUsableAsScratch() because the other threads that are holding the
        // extra refs described above can just as easily drop them between this assert and the last
        // call to unrefReturnQueue() that put us into this branch.
    }

    // Update GPU budget now that the budget policy is up to date. Some GPU resources may have their
    // actual memory amount change over time so update periodically.
    if (resource->budgeted() == Budgeted::kYes) {
        size_t oldSize = resource->gpuMemorySize();
        resource->updateGpuMemorySize();
        if (oldSize != resource->gpuMemorySize()) {
            fBudgetedBytes -= oldSize;
            fBudgetedBytes += resource->gpuMemorySize();
        }
    }

    this->setResourceUseToken(resource, this->getNextUseToken());

    // If the resource was not purgeable at the time the return queue ref was released, the
    // resource should still be in the non-purgeable array from when it was originally given
    // out. Another thread may have already removed the last refs keeping it non-purgeable by
    // the time this thread reachs this line but that will only have re-added it to the return
    // queue. The cache stores the resource based on its purgeability at the time of releasing
    // the return queue ref. Any subsequent return due to becoming purgeable will complete
    // moving the resource from the non-purgeable array to the purgeable queue.
    SkASSERT(this->inNonpurgeableArray(resource));
    if (!isPurgeable) {
        this->validate();
        return next;
    }

    // Since the resource is purgeable, there are no external refs that can add new refs to make
    // it non-purgeable at this point. Only the current cache thread has that ability so we can
    // safely continue moving the resource from non-purgeable to purgeable without worrying about
    // another state change.
    this->removeFromNonpurgeableArray(resource);

    if (resource->shouldDeleteASAP() == Resource::DeleteASAP::kYes) {
        this->purgeResource(resource);
    } else {
        // We don't purge this resource immediately even if we are overbudget. This allows later
        // purgeAsNeeded() calls to prioritize deleting less-recently-used Resources first.
        resource->updateAccessTime();
        fPurgeableQueue.insert(resource);
        fPurgeableBytes += resource->gpuMemorySize();
    }
    this->validate();

    return next;
}

void ResourceCache::addToResourceMap(Resource* resource) {
    SkASSERT(this->isInCache(resource));
    SkASSERT(!resource->isAvailableForReuse());
    SkASSERT(!fResourceMap.has(resource, resource->key()));
    fResourceMap.insert(resource->key(), resource);
    resource->setAvailableForReuse(true);
}

void ResourceCache::removeFromResourceMap(Resource* resource) {
    SkASSERT(this->isInCache(resource));
    SkASSERT(resource->isAvailableForReuse());
    SkASSERT(fResourceMap.has(resource, resource->key()));
    fResourceMap.remove(resource->key(), resource);
    resource->setAvailableForReuse(false);
}

void ResourceCache::addToNonpurgeableArray(Resource* resource) {
    SkASSERT(!this->inNonpurgeableArray(resource));

    int index = fNonpurgeableResources.size();
    *fNonpurgeableResources.append() = resource;
    *resource->accessCacheIndex() = index;
}

void ResourceCache::removeFromNonpurgeableArray(Resource* resource) {
    SkASSERT(this->inNonpurgeableArray(resource));

    int* index = resource->accessCacheIndex();
    // Fill the hole we will create in the array with the tail object, adjust its index, and
    // then pop the array
    Resource* tail = *(fNonpurgeableResources.end() - 1);
    SkASSERT(fNonpurgeableResources[*index] == resource);
    fNonpurgeableResources[*index] = tail;
    *tail->accessCacheIndex() = *index;
    fNonpurgeableResources.pop_back();
    *index = -1;
}

void ResourceCache::removeFromPurgeableQueue(Resource* resource) {
    SkASSERT(this->inPurgeableQueue(resource));

    fPurgeableQueue.remove(resource);
    fPurgeableBytes -= resource->gpuMemorySize();
    // SkTDPQueue will set the index back to -1 in debug builds, but we are using the index as a
    // flag for whether the Resource has been purged from the cache or not. So we need to make sure
    // it always gets set.
    *resource->accessCacheIndex() = -1;
}

bool ResourceCache::inPurgeableQueue(const Resource* resource) const {
    int index = *resource->accessCacheIndex();
    return index >= 0 && index < fPurgeableQueue.count() &&
           fPurgeableQueue.at(index) == resource;
}

#if defined(SK_DEBUG)

bool ResourceCache::inNonpurgeableArray(const Resource* resource) const {
    int index = *resource->accessCacheIndex();
    return index >= 0 && index < fNonpurgeableResources.size() &&
           fNonpurgeableResources[index] == resource;
}

bool ResourceCache::isInCache(const Resource* resource) const {
    if (this->inPurgeableQueue(resource) || this->inNonpurgeableArray(resource)) {
        SkASSERT(resource->cache() == this);
        SkASSERT(resource->hasCacheRef());
        return true;
    }
    // Resource index should have been set to -1 if the resource is not in the cache
    SkASSERT(*resource->accessCacheIndex() == -1);
    // Don't assert that the resource has no cache ref, as the ResourceCache asserts this is false
    // before it removes its cache ref in the event that that was the last resource keeping it alive
    return false;
}

#endif // SK_DEBUG

void ResourceCache::purgeResource(Resource* resource) {
    SkASSERT(resource->isPurgeable());

    TRACE_EVENT_INSTANT1("skia.gpu.cache", TRACE_FUNC, TRACE_EVENT_SCOPE_THREAD,
                         "size", resource->gpuMemorySize());

    this->removeFromResourceMap(resource);

    if (resource->shouldDeleteASAP() == Resource::DeleteASAP::kNo) {
        SkASSERT(this->inPurgeableQueue(resource));
        this->removeFromPurgeableQueue(resource);
    }

    SkASSERT(!this->isInCache(resource));

    fBudgetedBytes -= resource->gpuMemorySize();
    resource->unrefCache();
}

void ResourceCache::purgeAsNeeded() {
    ASSERT_SINGLE_OWNER

    if (this->overbudget() && fProxyCache) {
        fProxyCache->freeUniquelyHeld();

        // After the image cache frees resources we need to return those resources to the cache
        this->processReturnedResources();
    }
    while (this->overbudget() && fPurgeableQueue.count()) {
        Resource* resource = fPurgeableQueue.peek();
        SkASSERT(!resource->wasDestroyed());
        SkASSERT(fResourceMap.has(resource, resource->key()));

        if (resource->lastUseToken() == kMaxUseToken) {
            // If we hit a resource that is at kMaxUseToken, then we've hit the part of the
            // purgeable queue with all zero sized resources. We don't want to actually remove those
            // so we just break here.
            SkASSERT(resource->gpuMemorySize() == 0);
            break;
        }

        this->purgeResource(resource);
    }

    this->validate();
}

void ResourceCache::purgeResourcesNotUsedSince(StdSteadyClock::time_point purgeTime) {
    ASSERT_SINGLE_OWNER
    this->purgeResources(&purgeTime);
}

void ResourceCache::purgeResources() {
    ASSERT_SINGLE_OWNER
    this->purgeResources(nullptr);
}

void ResourceCache::purgeResources(const StdSteadyClock::time_point* purgeTime) {
    TRACE_EVENT0("skia.gpu.cache", TRACE_FUNC);
    if (fProxyCache) {
        fProxyCache->purgeProxiesNotUsedSince(purgeTime);
    }
    this->processReturnedResources();

    // Early out if the very first item is too new to purge to avoid sorting the queue when
    // nothing will be deleted.
    if (fPurgeableQueue.count() &&
        purgeTime &&
        fPurgeableQueue.peek()->lastAccessTime() >= *purgeTime) {
        return;
    }

    // Sort the queue
    fPurgeableQueue.sort();

    // Make a list of the scratch resources to delete
    SkTDArray<Resource*> resourcesToPurge;
    for (int i = 0; i < fPurgeableQueue.count(); i++) {
        Resource* resource = fPurgeableQueue.at(i);

        const skgpu::StdSteadyClock::time_point resourceTime = resource->lastAccessTime();
        if (purgeTime && resourceTime >= *purgeTime) {
            // scratch or not, all later iterations will be too recently used to purge.
            break;
        }
        SkASSERT(resource->isPurgeable());
        *resourcesToPurge.append() = resource;
    }

    // Delete the scratch resources. This must be done as a separate pass
    // to avoid messing up the sorted order of the queue
    for (int i = 0; i < resourcesToPurge.size(); i++) {
        this->purgeResource(resourcesToPurge[i]);
    }

    // Since we called process returned resources at the start of this call, we could still end up
    // over budget even after purging resources based on purgeTime. So we call purgeAsNeeded at the
    // end here.
    this->purgeAsNeeded();
}

uint32_t ResourceCache::getNextUseToken() {
    // If we wrap then all the existing resources will appear older than any resources that get
    // a token after the wrap. We wrap one value early when we reach kMaxUseToken so that we
    // can continue to use kMaxUseToken as a special case for zero sized resources.
    if (fUseToken == kMaxUseToken) {
        fUseToken = 0;
        int count = this->getResourceCount();
        if (count) {
            // Reset all the tokens. We sort the resources by their use token and then assign
            // sequential tokens beginning with 0. This is O(n*lg(n)) but it should be very rare.
            SkTDArray<Resource*> sortedPurgeableResources;
            sortedPurgeableResources.reserve(fPurgeableQueue.count());

            while (fPurgeableQueue.count()) {
                *sortedPurgeableResources.append() = fPurgeableQueue.peek();
                fPurgeableQueue.pop();
            }

            SkTQSort(fNonpurgeableResources.begin(), fNonpurgeableResources.end(), CompareUseToken);

            // Pick resources out of the purgeable and non-purgeable arrays based on lowest
            // use token and assign new tokens.
            int currP = 0;
            int currNP = 0;
            while (currP < sortedPurgeableResources.size() &&
                   currNP < fNonpurgeableResources.size()) {
                uint32_t tsP = sortedPurgeableResources[currP]->lastUseToken();
                uint32_t tsNP = fNonpurgeableResources[currNP]->lastUseToken();
                SkASSERT(tsP != tsNP);
                if (tsP < tsNP) {
                    this->setResourceUseToken(sortedPurgeableResources[currP++], fUseToken++);
                } else {
                    // Correct the index in the nonpurgeable array stored on the resource post-sort.
                    *fNonpurgeableResources[currNP]->accessCacheIndex() = currNP;
                    this->setResourceUseToken(fNonpurgeableResources[currNP++], fUseToken++);
                }
            }

            // The above loop ended when we hit the end of one array. Finish the other one.
            while (currP < sortedPurgeableResources.size()) {
                this->setResourceUseToken(sortedPurgeableResources[currP++], fUseToken++);
            }
            while (currNP < fNonpurgeableResources.size()) {
                *fNonpurgeableResources[currNP]->accessCacheIndex() = currNP;
                this->setResourceUseToken(fNonpurgeableResources[currNP++], fUseToken++);
            }

            // Rebuild the queue.
            for (int i = 0; i < sortedPurgeableResources.size(); ++i) {
                fPurgeableQueue.insert(sortedPurgeableResources[i]);
            }

            this->validate();
            SkASSERT(count == this->getResourceCount());

            // count should be the next use token we return.
            SkASSERT(fUseToken == SkToU32(count));
        }
    }
    return fUseToken++;
}

void ResourceCache::setResourceUseToken(Resource* resource, uint32_t token) {
    // We always set the use token for zero-sized resources to be kMaxUseToken
    if (resource->gpuMemorySize() == 0) {
        token = kMaxUseToken;
    }
    resource->setLastUseToken(token);
}

void ResourceCache::dumpMemoryStatistics(SkTraceMemoryDump* traceMemoryDump) const {
    ASSERT_SINGLE_OWNER

    // There is no need to process the return queue here. Resources in the queue are still in
    // either the purgeable queue or the nonpurgeable resources list (likely to be moved to the
    // purgeable queue). However, the Resource's own ref counts are used to report its purgeable
    // state to the memory dump, which is accurate without draining the return queue.

    for (int i = 0; i < fNonpurgeableResources.size(); ++i) {
        fNonpurgeableResources[i]->dumpMemoryStatistics(traceMemoryDump, false);
    }
    for (int i = 0; i < fPurgeableQueue.count(); ++i) {
        fPurgeableQueue.at(i)->dumpMemoryStatistics(traceMemoryDump, true);
    }
}

void ResourceCache::setMaxBudget(size_t bytes) {
    fMaxBytes = bytes;
    this->processReturnedResources();
    this->purgeAsNeeded();
}

////////////////////////////////////////////////////////////////////////////////

#if defined(SK_DEBUG)
void ResourceCache::validate() const {
    // Reduce the frequency of validations for large resource counts.
    static SkRandom gRandom;
    int mask = (SkNextPow2(fCount + 1) >> 5) - 1;
    if (~mask && (gRandom.nextU() & mask)) {
        return;
    }

    struct Stats {
        int fShareable;
        int fScratch;
        size_t fBudgetedBytes;
        size_t fPurgeableBytes;
        const ResourceMap* fResourceMap;
        const PurgeableQueue* fPurgeableQueue;

        Stats(const ResourceCache* cache) {
            memset(this, 0, sizeof(*this));
            fResourceMap = &cache->fResourceMap;
            fPurgeableQueue = &cache->fPurgeableQueue;
        }

        void update(Resource* resource) {
            const GraphiteResourceKey& key = resource->key();
            SkASSERT(key.isValid());

            // All resources in the cache are owned. If we track wrapped resources in the cache
            // we'll need to update this check.
            SkASSERT(resource->ownership() == Ownership::kOwned);

            if (resource->shareable() == Shareable::kYes) {
                SkASSERT(resource->isAvailableForReuse());
                SkASSERT(fResourceMap->has(resource, key));
                SkASSERT(resource->budgeted() == Budgeted::kYes);
                ++fShareable;
            } else if (resource->isAvailableForReuse()) {
                // We track scratch resources (either non-shareable with no refs that are returned,
                // or explicitly scratch shared) separately from fully shareable.
                SkASSERT(resource->isUsableAsScratch());
                SkASSERT(fResourceMap->has(resource, key));
                ++fScratch;
            } else {
                // This should be a non-shareable resource that isn't available for reuse.
                SkASSERT(resource->shareable() == Shareable::kNo);
                SkASSERT(!fResourceMap->has(resource, key));
            }

            if (resource->budgeted() == Budgeted::kYes) {
                fBudgetedBytes += resource->gpuMemorySize();
            }

            if (resource->gpuMemorySize() == 0) {
                SkASSERT(resource->lastUseToken() == kMaxUseToken);
            } else {
                SkASSERT(resource->lastUseToken() < kMaxUseToken);
            }

            int index = *resource->accessCacheIndex();
            if (index < fPurgeableQueue->count() && fPurgeableQueue->at(index) == resource) {
                SkASSERT(resource->isPurgeable());
                fPurgeableBytes += resource->gpuMemorySize();
            }
        }
    };

    {
        int count = 0;
        fResourceMap.foreach([&](const Resource& resource) {
            SkASSERT(resource.isUsableAsScratch() || resource.shareable() == Shareable::kYes);
            SkASSERT(resource.budgeted() == Budgeted::kYes);
            SkASSERT(resource.isAvailableForReuse());
            SkASSERT(this->isInCache(&resource));
            count++;
        });
        SkASSERT(count == fResourceMap.count());
    }

    // In the below checks we can assert that anything in the purgeable queue is purgeable because
    // we won't put a Resource into that queue unless all refs are zero. Thus there is no way for
    // that resource to be made non-purgeable without going through the cache (which will switch
    // queues back to non-purgeable).
    //
    // However, we can't say the same for things in the non-purgeable array. It is possible that
    // Resources have removed all their refs (thus technically become purgeable) but have not been
    // processed back into the cache yet. Thus we may not have moved resources to the purgeable
    // queue yet. Its also possible that Resource hasn't been added to the ReturnQueue yet (thread
    // paused between unref and adding to ReturnQueue) so we can't even make asserts like not
    // purgeable or is in ReturnQueue.
    Stats stats(this);
    for (int i = 0; i < fNonpurgeableResources.size(); ++i) {
        SkASSERT(this->isInCache(fNonpurgeableResources[i]));
        SkASSERT(*fNonpurgeableResources[i]->accessCacheIndex() == i);
        SkASSERT(!fNonpurgeableResources[i]->wasDestroyed());
        SkASSERT(!this->inPurgeableQueue(fNonpurgeableResources[i]));
        stats.update(fNonpurgeableResources[i]);
    }
    bool firstPurgeableIsSizeZero = false;
    for (int i = 0; i < fPurgeableQueue.count(); ++i) {
        if (i == 0) {
            firstPurgeableIsSizeZero = (fPurgeableQueue.at(0)->gpuMemorySize() == 0);
        }
        if (firstPurgeableIsSizeZero) {
            // If the first purgeable item (i.e. least recently used) is sized zero, then all other
            // purgeable resources must also be sized zero since they should all have a use token of
            // kMaxUseToken.
            SkASSERT(fPurgeableQueue.at(i)->gpuMemorySize() == 0);
        }
        SkASSERT(this->isInCache(fPurgeableQueue.at(i)));
        SkASSERT(fPurgeableQueue.at(i)->isPurgeable());
        SkASSERT(*fPurgeableQueue.at(i)->accessCacheIndex() == i);
        SkASSERT(!fPurgeableQueue.at(i)->wasDestroyed());
        stats.update(fPurgeableQueue.at(i));
    }

    SkASSERT((stats.fScratch + stats.fShareable) == fResourceMap.count());
    SkASSERT(stats.fBudgetedBytes == fBudgetedBytes);
    SkASSERT(stats.fPurgeableBytes == fPurgeableBytes);
}

#endif // SK_DEBUG

#if defined(GPU_TEST_UTILS)

int ResourceCache::numFindableResources() const {
    return fResourceMap.count();
}

Resource* ResourceCache::topOfPurgeableQueue() {
    if (!fPurgeableQueue.count()) {
        return nullptr;
    }
    return fPurgeableQueue.peek();
}

void ResourceCache::visitTextures(
        const std::function<void(const Texture*, bool purgeable)>& func) const {
    for (int i = 0; i < fNonpurgeableResources.size(); ++i) {
        if (const Texture* tex = fNonpurgeableResources[i]->asTexture()) {
            func(tex, /* purgeable= */ false);
        }
    }
    for (int i = 0; i < fPurgeableQueue.count(); ++i) {
        if (const Texture* tex = fPurgeableQueue.at(i)->asTexture()) {
            func(tex, /* purgeable= */ true);
        }
    }
}

#endif // defined(GPU_TEST_UTILS)

} // namespace skgpu::graphite
