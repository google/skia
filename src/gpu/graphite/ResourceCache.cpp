/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/ResourceCache.h"

#include "include/private/base/SingleOwner.h"
#include "src/base/SkRandom.h"
#include "src/core/SkTMultiMap.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/graphite/GraphiteResourceKey.h"
#include "src/gpu/graphite/ProxyCache.h"
#include "src/gpu/graphite/Resource.h"

#if defined(GRAPHITE_TEST_UTILS)
#include "src/gpu/graphite/Texture.h"
#endif

namespace skgpu::graphite {

#define ASSERT_SINGLE_OWNER SKGPU_ASSERT_SINGLE_OWNER(fSingleOwner)

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
#ifndef SK_DEBUG
    (void)fSingleOwner;
#endif
}

ResourceCache::~ResourceCache() {
    // The ResourceCache must have been shutdown by the ResourceProvider before it is destroyed.
    SkASSERT(fIsShutdown);
}

void ResourceCache::shutdown() {
    ASSERT_SINGLE_OWNER

    SkASSERT(!fIsShutdown);

    {
        SkAutoMutexExclusive locked(fReturnMutex);
        fIsShutdown = true;
    }
    if (fProxyCache) {
        fProxyCache->purgeAll();
    }

    this->processReturnedResources();

    while (fNonpurgeableResources.size()) {
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

void ResourceCache::insertResource(Resource* resource) {
    ASSERT_SINGLE_OWNER
    SkASSERT(resource);
    SkASSERT(!this->isInCache(resource));
    SkASSERT(!resource->wasDestroyed());
    SkASSERT(!resource->isPurgeable());
    SkASSERT(resource->key().isValid());
    // All resources in the cache are owned. If we track wrapped resources in the cache we'll need
    // to update this check.
    SkASSERT(resource->ownership() == Ownership::kOwned);

    this->processReturnedResources();

    resource->registerWithCache(sk_ref_sp(this));
    resource->refCache();

    // We must set the timestamp before adding to the array in case the timestamp wraps and we wind
    // up iterating over all the resources that already have timestamps.
    this->setResourceTimestamp(resource, this->getNextTimestamp());
    resource->updateAccessTime();

    this->addToNonpurgeableArray(resource);

    SkDEBUGCODE(fCount++;)

    if (resource->key().shareable() == Shareable::kYes) {
        fResourceMap.insert(resource->key(), resource);
    }

    if (resource->budgeted() == skgpu::Budgeted::kYes) {
        fBudgetedBytes += resource->gpuMemorySize();
    }

    this->purgeAsNeeded();
}

Resource* ResourceCache::findAndRefResource(const GraphiteResourceKey& key,
                                            skgpu::Budgeted budgeted) {
    ASSERT_SINGLE_OWNER

    this->processReturnedResources();

    SkASSERT(key.isValid());

    Resource* resource = fResourceMap.find(key);
    if (resource) {
        // All resources we pull out of the cache for use should be budgeted
        SkASSERT(resource->budgeted() == skgpu::Budgeted::kYes);
        if (key.shareable() == Shareable::kNo) {
            // If a resource is not shareable (i.e. scratch resource) then we remove it from the map
            // so that it isn't found again.
            fResourceMap.remove(key, resource);
            if (budgeted == skgpu::Budgeted::kNo) {
                resource->makeUnbudgeted();
                fBudgetedBytes -= resource->gpuMemorySize();
            }
            SkDEBUGCODE(resource->fNonShareableInCache = false;)
        } else {
            // Shareable resources should never be requested as non budgeted
            SkASSERT(budgeted == skgpu::Budgeted::kYes);
        }
        this->refAndMakeResourceMRU(resource);
        this->validate();
    }

    // processReturnedResources may have added resources back into our budget if they were being
    // using in an SkImage or SkSurface previously. However, instead of calling purgeAsNeeded in
    // processReturnedResources, we delay calling it until now so we don't end up purging a resource
    // we're looking for in this function.
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

    this->setResourceTimestamp(resource, this->getNextTimestamp());
    this->validate();
}

bool ResourceCache::returnResource(Resource* resource, LastRemovedRef removedRef) {
    // We should never be trying to return a LastRemovedRef of kCache.
    SkASSERT(removedRef != LastRemovedRef::kCache);
    SkAutoMutexExclusive locked(fReturnMutex);
    if (fIsShutdown) {
        return false;
    }

    SkASSERT(resource);

    // We only allow one instance of a Resource to be in the return queue at a time. We do this so
    // that the ReturnQueue stays small and quick to process.
    //
    // Because we take CacheRefs to all Resources added to the ReturnQueue, we would be safe if we
    // decided to have multiple instances of a Resource. Even if an earlier returned instance of a
    // Resource triggers that Resource to get purged from the cache, the Resource itself wouldn't
    // get deleted until we drop all the CacheRefs in this ReturnQueue.
    if (*resource->accessReturnIndex() >= 0) {
        // If the resource is already in the return queue we promote the LastRemovedRef to be
        // kUsage if that is what is returned here.
        if (removedRef == LastRemovedRef::kUsage) {
            SkASSERT(*resource->accessReturnIndex() < (int)fReturnQueue.size());
            fReturnQueue[*resource->accessReturnIndex()].second = removedRef;
        }
        return true;
    }
#ifdef SK_DEBUG
    for (auto& nextResource : fReturnQueue) {
        SkASSERT(nextResource.first != resource);
    }
#endif
    fReturnQueue.push_back(std::make_pair(resource, removedRef));
    *resource->accessReturnIndex() = fReturnQueue.size() - 1;
    resource->refCache();
    return true;
}

void ResourceCache::processReturnedResources() {
    // We need to move the returned Resources off of the ReturnQueue before we start processing them
    // so that we can drop the fReturnMutex. When we process a Resource we may need to grab its
    // UnrefMutex. This could cause a deadlock if on another thread the Resource has the UnrefMutex
    // and is waiting on the ReturnMutex to be free.
    ReturnQueue tempQueue;
    {
        SkAutoMutexExclusive locked(fReturnMutex);
        // TODO: Instead of doing a copy of the vector, we may be able to improve the performance
        // here by storing some form of linked list, then just move the pointer the first element
        // and reset the ReturnQueue's top element to nullptr.
        tempQueue = fReturnQueue;
        fReturnQueue.clear();
        for (auto& nextResource : tempQueue) {
            auto [resource, ref] = nextResource;
            SkASSERT(*resource->accessReturnIndex() >= 0);
            *resource->accessReturnIndex() = -1;
        }
    }

    if (tempQueue.empty()) {
        return;
    }

    // Trace after the lock has been released so we can simply record the tempQueue size.
    TRACE_EVENT1("skia.gpu.cache", TRACE_FUNC, "count", tempQueue.size());

    for (auto& nextResource : tempQueue) {
        auto [resource, ref] = nextResource;
        // We need this check here to handle the following scenario. A Resource is sitting in the
        // ReturnQueue (say from kUsage last ref) and the Resource still has a command buffer ref
        // out in the wild. When the ResourceCache calls processReturnedResources it locks the
        // ReturnMutex. Immediately after this, the command buffer ref is released on another
        // thread. The Resource cannot be added to the ReturnQueue since the lock is held. Back in
        // the ResourceCache (we'll drop the ReturnMutex) and when we try to return the Resource we
        // will see that it is purgeable. If we are overbudget it is possible that the Resource gets
        // purged from the ResourceCache at this time setting its cache index to -1. The unrefCache
        // call will actually block here on the Resource's UnrefMutex which is held from the command
        // buffer ref. Eventually the command bufer ref thread will get to run again and with the
        // ReturnMutex lock dropped it will get added to the ReturnQueue. At this point the first
        // unrefCache call will continue on the main ResourceCache thread. When we call
        // processReturnedResources the next time, we don't want this Resource added back into the
        // cache, thus we have the check here. The Resource will then get deleted when we call
        // unrefCache below to remove the cache ref added from the ReturnQueue.
        if (*resource->accessCacheIndex() != -1) {
            this->returnResourceToCache(resource, ref);
        }
        // Remove cache ref held by ReturnQueue
        resource->unrefCache();
    }
}

void ResourceCache::returnResourceToCache(Resource* resource, LastRemovedRef removedRef) {
    // A resource should not have been destroyed when placed into the return queue. Also before
    // purging any resources from the cache itself, it should always empty the queue first. When the
    // cache releases/abandons all of its resources, it first invalidates the return queue so no new
    // resources can be added. Thus we should not end up in a situation where a resource gets
    // destroyed after it was added to the return queue.
    SkASSERT(!resource->wasDestroyed());

    SkASSERT(this->isInCache(resource));
    if (removedRef == LastRemovedRef::kUsage) {
        if (resource->key().shareable() == Shareable::kYes) {
            // Shareable resources should still be in the cache
            SkASSERT(fResourceMap.find(resource->key()));
        } else {
            SkDEBUGCODE(resource->fNonShareableInCache = true;)
            fResourceMap.insert(resource->key(), resource);
            if (resource->budgeted() == skgpu::Budgeted::kNo) {
                resource->makeBudgeted();
                fBudgetedBytes += resource->gpuMemorySize();
            }
        }
    }

    // If we weren't using multiple threads, it is ok to assume a resource that isn't purgeable must
    // be in the non purgeable array. However, since resources can be unreffed from multiple
    // threads, it is possible that a resource became purgeable while we are in the middle of
    // returning resources. For example, a resource could have 1 usage and 1 command buffer ref. We
    // then unref the usage which puts the resource in the return queue. Then the ResourceCache
    // thread locks the ReturnQueue as it returns the Resource. At this same time another thread
    // unrefs the command buffer usage but can't add the Resource to the ReturnQueue as it is
    // locked (but the command buffer ref has been reduced to zero). When we are processing the
    // Resource (from the kUsage ref) to return it to the cache it will look like it is purgeable
    // since all refs are zero. Thus we will move the Resource from the non purgeable to purgeable
    // queue. Then later when we return the command buffer ref, the Resource will have already been
    // moved to purgeable queue and we don't need to do it again.
    if (!resource->isPurgeable() || this->inPurgeableQueue(resource)) {
        this->validate();
        return;
    }

    this->setResourceTimestamp(resource, this->getNextTimestamp());

    this->removeFromNonpurgeableArray(resource);

    if (resource->shouldDeleteASAP() == Resource::DeleteASAP::kYes) {
        this->purgeResource(resource);
    } else {
        resource->updateAccessTime();
        fPurgeableQueue.insert(resource);
    }
    this->validate();
}

void ResourceCache::addToNonpurgeableArray(Resource* resource) {
    int index = fNonpurgeableResources.size();
    *fNonpurgeableResources.append() = resource;
    *resource->accessCacheIndex() = index;
}

void ResourceCache::removeFromNonpurgeableArray(Resource* resource) {
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
    fPurgeableQueue.remove(resource);
    // SkTDPQueue will set the index back to -1 in debug builds, but we are using the index as a
    // flag for whether the Resource has been purged from the cache or not. So we need to make sure
    // it always gets set.
    *resource->accessCacheIndex() = -1;
}

bool ResourceCache::inPurgeableQueue(Resource* resource) const {
    SkASSERT(this->isInCache(resource));
    int index = *resource->accessCacheIndex();
    if (index < fPurgeableQueue.count() && fPurgeableQueue.at(index) == resource) {
        return true;
    }
    return false;
}

void ResourceCache::purgeResource(Resource* resource) {
    SkASSERT(resource->isPurgeable());

    TRACE_EVENT_INSTANT1("skia.gpu.cache", TRACE_FUNC, TRACE_EVENT_SCOPE_THREAD,
                         "size", resource->gpuMemorySize());

    fResourceMap.remove(resource->key(), resource);

    if (resource->shouldDeleteASAP() == Resource::DeleteASAP::kNo) {
        SkASSERT(this->inPurgeableQueue(resource));
        this->removeFromPurgeableQueue(resource);
    } else {
        SkASSERT(!this->isInCache(resource));
    }

    fBudgetedBytes -= resource->gpuMemorySize();
    resource->unrefCache();
}

void ResourceCache::purgeAsNeeded() {
    ASSERT_SINGLE_OWNER

    this->processReturnedResources();

    if (this->overbudget() && fProxyCache) {
        fProxyCache->freeUniquelyHeld();

        // After the image cache frees resources we need to return those resources to the cache
        this->processReturnedResources();
    }
    while (this->overbudget() && fPurgeableQueue.count()) {
        Resource* resource = fPurgeableQueue.peek();
        SkASSERT(!resource->wasDestroyed());
        SkASSERT(fResourceMap.find(resource->key()));

        if (resource->timestamp() == kMaxTimestamp) {
            // If we hit a resource that is at kMaxTimestamp, then we've hit the part of the
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
    SkTDArray<Resource*> nonZeroSizedResources;
    for (int i = 0; i < fPurgeableQueue.count(); i++) {
        Resource* resource = fPurgeableQueue.at(i);

        const skgpu::StdSteadyClock::time_point resourceTime = resource->lastAccessTime();
        if (purgeTime && resourceTime >= *purgeTime) {
            // scratch or not, all later iterations will be too recently used to purge.
            break;
        }
        SkASSERT(resource->isPurgeable());
        if (resource->gpuMemorySize() > 0) {
            *nonZeroSizedResources.append() = resource;
        }
    }

    // Delete the scratch resources. This must be done as a separate pass
    // to avoid messing up the sorted order of the queue
    for (int i = 0; i < nonZeroSizedResources.size(); i++) {
        this->purgeResource(nonZeroSizedResources[i]);
    }

    // Since we called process returned resources at the start of this call, we could still end up
    // over budget even after purging resources based on purgeTime. So we call purgeAsNeeded at the
    // end here.
    this->purgeAsNeeded();
}

uint32_t ResourceCache::getNextTimestamp() {
    // If we wrap then all the existing resources will appear older than any resources that get
    // a timestamp after the wrap. We wrap one value early when we reach kMaxTimestamp so that we
    // can continue to use kMaxTimestamp as a special case for zero sized resources.
    if (fTimestamp == kMaxTimestamp) {
        fTimestamp = 0;
        int count = this->getResourceCount();
        if (count) {
            // Reset all the timestamps. We sort the resources by timestamp and then assign
            // sequential timestamps beginning with 0. This is O(n*lg(n)) but it should be extremely
            // rare.
            SkTDArray<Resource*> sortedPurgeableResources;
            sortedPurgeableResources.reserve(fPurgeableQueue.count());

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
            while (currP < sortedPurgeableResources.size() &&
                   currNP < fNonpurgeableResources.size()) {
                uint32_t tsP = sortedPurgeableResources[currP]->timestamp();
                uint32_t tsNP = fNonpurgeableResources[currNP]->timestamp();
                SkASSERT(tsP != tsNP);
                if (tsP < tsNP) {
                    this->setResourceTimestamp(sortedPurgeableResources[currP++], fTimestamp++);
                } else {
                    // Correct the index in the nonpurgeable array stored on the resource post-sort.
                    *fNonpurgeableResources[currNP]->accessCacheIndex() = currNP;
                    this->setResourceTimestamp(fNonpurgeableResources[currNP++], fTimestamp++);
                }
            }

            // The above loop ended when we hit the end of one array. Finish the other one.
            while (currP < sortedPurgeableResources.size()) {
                this->setResourceTimestamp(sortedPurgeableResources[currP++], fTimestamp++);
            }
            while (currNP < fNonpurgeableResources.size()) {
                *fNonpurgeableResources[currNP]->accessCacheIndex() = currNP;
                this->setResourceTimestamp(fNonpurgeableResources[currNP++], fTimestamp++);
            }

            // Rebuild the queue.
            for (int i = 0; i < sortedPurgeableResources.size(); ++i) {
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

void ResourceCache::setResourceTimestamp(Resource* resource, uint32_t timestamp) {
    // We always set the timestamp for zero sized resources to be kMaxTimestamp
    if (resource->gpuMemorySize() == 0) {
        timestamp = kMaxTimestamp;
    }
    resource->setTimestamp(timestamp);
}

void ResourceCache::dumpMemoryStatistics(SkTraceMemoryDump* traceMemoryDump) const {
    for (int i = 0; i < fNonpurgeableResources.size(); ++i) {
        fNonpurgeableResources[i]->dumpMemoryStatistics(traceMemoryDump);
    }
    for (int i = 0; i < fPurgeableQueue.count(); ++i) {
        fPurgeableQueue.at(i)->dumpMemoryStatistics(traceMemoryDump);
    }
}

////////////////////////////////////////////////////////////////////////////////

const GraphiteResourceKey& ResourceCache::MapTraits::GetKey(const Resource& r) {
    return r.key();
}

uint32_t ResourceCache::MapTraits::Hash(const GraphiteResourceKey& key) {
    return key.hash();
}

bool ResourceCache::CompareTimestamp(Resource* const& a, Resource* const& b) {
    return a->timestamp() < b->timestamp();
}

int* ResourceCache::AccessResourceIndex(Resource* const& res) {
    return res->accessCacheIndex();
}

#ifdef SK_DEBUG
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
        const ResourceMap* fResourceMap;

        Stats(const ResourceCache* cache) {
            memset(this, 0, sizeof(*this));
            fResourceMap = &cache->fResourceMap;
        }

        void update(Resource* resource) {
            const GraphiteResourceKey& key = resource->key();
            SkASSERT(key.isValid());

            // We should always have at least 1 cache ref
            SkASSERT(resource->hasCacheRef());

            // All resources in the cache are owned. If we track wrapped resources in the cache
            // we'll need to update this check.
            SkASSERT(resource->ownership() == Ownership::kOwned);

            // We track scratch (non-shareable, no usage refs, has been returned to cache) and
            // shareable resources here as those should be the only things in the fResourceMap. A
            // non-shareable resources that does meet the scratch criteria will not be able to be
            // given back out from a cache requests. After processing all the resources we assert
            // that the fScratch + fShareable equals the count in the fResourceMap.
            if (resource->isUsableAsScratch()) {
                SkASSERT(key.shareable() == Shareable::kNo);
                SkASSERT(!resource->hasUsageRef());
                ++fScratch;
                SkASSERT(fResourceMap->has(resource, key));
                SkASSERT(resource->budgeted() == skgpu::Budgeted::kYes);
            } else if (key.shareable() == Shareable::kNo) {
                SkASSERT(!fResourceMap->has(resource, key));
            } else {
                SkASSERT(key.shareable() == Shareable::kYes);
                ++fShareable;
                SkASSERT(fResourceMap->has(resource, key));
                SkASSERT(resource->budgeted() == skgpu::Budgeted::kYes);
            }

            if (resource->budgeted() == skgpu::Budgeted::kYes) {
                fBudgetedBytes += resource->gpuMemorySize();
            }

            if (resource->gpuMemorySize() == 0) {
                SkASSERT(resource->timestamp() == kMaxTimestamp);
            } else {
                SkASSERT(resource->timestamp() < kMaxTimestamp);
            }
        }
    };

    {
        int count = 0;
        fResourceMap.foreach([&](const Resource& resource) {
            SkASSERT(resource.isUsableAsScratch() || resource.key().shareable() == Shareable::kYes);
            SkASSERT(resource.budgeted() == skgpu::Budgeted::kYes);
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
            // purgeable resources must also be sized zero since they should all have a timestamp of
            // kMaxTimestamp.
            SkASSERT(fPurgeableQueue.at(i)->gpuMemorySize() == 0);
        }
        SkASSERT(fPurgeableQueue.at(i)->isPurgeable());
        SkASSERT(*fPurgeableQueue.at(i)->accessCacheIndex() == i);
        SkASSERT(!fPurgeableQueue.at(i)->wasDestroyed());
        stats.update(fPurgeableQueue.at(i));
    }

    SkASSERT((stats.fScratch + stats.fShareable) == fResourceMap.count());
    SkASSERT(stats.fBudgetedBytes == fBudgetedBytes);
}

bool ResourceCache::isInCache(const Resource* resource) const {
    int index = *resource->accessCacheIndex();
    if (index < 0) {
        return false;
    }
    if (index < fPurgeableQueue.count() && fPurgeableQueue.at(index) == resource) {
        return true;
    }
    if (index < fNonpurgeableResources.size() && fNonpurgeableResources[index] == resource) {
        return true;
    }
    SkDEBUGFAIL("Resource index should be -1 or the resource should be in the cache.");
    return false;
}

#endif // SK_DEBUG

#if defined(GRAPHITE_TEST_UTILS)

int ResourceCache::numFindableResources() const {
    return fResourceMap.count();
}

void ResourceCache::setMaxBudget(size_t bytes) {
    fMaxBytes = bytes;
    this->purgeAsNeeded();
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

#endif // defined(GRAPHITE_TEST_UTILS)

} // namespace skgpu::graphite
