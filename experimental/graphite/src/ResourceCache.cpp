/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/ResourceCache.h"

#include "experimental/graphite/src/GraphiteResourceKey.h"
#include "experimental/graphite/src/Resource.h"
#include "include/private/SingleOwner.h"
#include "src/core/SkTMultiMap.h"

namespace skgpu {

#define ASSERT_SINGLE_OWNER SKGPU_ASSERT_SINGLE_OWNER(fSingleOwner)

sk_sp<ResourceCache> ResourceCache::Make(SingleOwner* singleOwner) {
    return sk_sp<ResourceCache>(new ResourceCache(singleOwner));
}

ResourceCache::ResourceCache(SingleOwner* singleOwner) : fSingleOwner(singleOwner) {
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
    this->processReturnedResources();

    while (fNonpurgeableResources.count()) {
        Resource* back = *(fNonpurgeableResources.end() - 1);
        SkASSERT(!back->wasDestroyed());
        back->removedFromCache();
        this->removeFromNonpurgeableArray(back);
    }

    while (fPurgeableQueue.count()) {
        Resource* top = fPurgeableQueue.peek();
        SkASSERT(!top->wasDestroyed());
        top->removedFromCache();
        fPurgeableQueue.remove(top);
    }
}

void ResourceCache::insertResource(Resource* resource) {
    ASSERT_SINGLE_OWNER
    SkASSERT(resource);
    SkASSERT(!this->isInCache(resource));
    SkASSERT(!resource->wasDestroyed());
    SkASSERT(!resource->isPurgeable());
    SkASSERT(resource->key().isValid());

    this->processReturnedResources();

    resource->registerWithCache(sk_ref_sp(this));

    // We must set the timestamp before adding to the array in case the timestamp wraps and we wind
    // up iterating over all the resources that already have timestamps.
    resource->setTimestamp(this->getNextTimestamp());

    this->addToNonpurgeableArray(resource);

    if (resource->key().shareable() == Shareable::kYes) {
        fResourceMap.insert(resource->key(), resource);
    }

    // TODO: purge resources if adding this one put us over budget (when we actually have a budget).
}

Resource* ResourceCache::findAndRefResource(const skgpu::GraphiteResourceKey& key) {
    ASSERT_SINGLE_OWNER

    this->processReturnedResources();

    SkASSERT(key.isValid());

    Resource* resource = fResourceMap.find(key);
    if (resource) {
        if (key.shareable() == Shareable::kNo) {
            // If a resource is not shareable (i.e. scratch resource) then we remove it from the map
            // so that it isn't found again.
            fResourceMap.remove(key, resource);
        }
        this->refAndMakeResourceMRU(resource);
        this->validate();
    }
    return resource;
}

void ResourceCache::refAndMakeResourceMRU(Resource* resource) {
    SkASSERT(resource);
    SkASSERT(this->isInCache(resource));

    if (resource->isPurgeable()) {
        // It's about to become unpurgeable.
        fPurgeableQueue.remove(resource);
        this->addToNonpurgeableArray(resource);
    }
    resource->refCacheOnly();

    resource->setTimestamp(this->getNextTimestamp());
    this->validate();
}

bool ResourceCache::returnResource(Resource* resource, LastRemovedRef removedRef) {
    SkAutoMutexExclusive locked(fReturnMutex);
    if (fIsShutdown) {
        return false;
    }

    SkASSERT(resource);
    fReturnQueue.push_back(std::tie(resource, removedRef));
    return true;
}

void ResourceCache::processReturnedResources() {
    // TODO: There is currently some non-trivial amount of work that happens in
    // ResourceCache::returnResource for each returned Resource. During this time we are keeping the
    // fReturnMutex locked. It would ideal if instead we could quickly move the Resource's off of
    // fReturnQueue, drop the lock, and then send them back to the ResourceCache.
    SkAutoMutexExclusive locked(fReturnMutex);

    for (auto& nextResource : fReturnQueue) {
        auto [resource, ref] = nextResource;
        this->returnResourceToCache(resource, ref);
    }
    fReturnQueue.clear();
}

void ResourceCache::returnResourceToCache(Resource* resource, LastRemovedRef removedRef) {
    // A resource should not have been destroyed when placed into the return queue. Also before
    // purging any resources from the cache itself, it should always empty the queue first. When the
    // cache releases/abandons all of its resources, it first invalidates the return queue so no new
    // resources can be added. Thus we should not end up in a situation where a resource gets
    // destroyed after it was added to the return queue.
    SkASSERT(!resource->wasDestroyed());

    SkASSERT(this->isInCache(resource));
    if (removedRef == LastRemovedRef::kUsageRef) {
        if (resource->key().shareable() == Shareable::kYes) {
            // Shareable resources should still be in the cache
            SkASSERT(fResourceMap.find(resource->key()));
        } else {
            fResourceMap.insert(resource->key(), resource);
        }
    }

    if (!resource->isPurgeable()) {
        this->validate();
        return;
    }

    this->removeFromNonpurgeableArray(resource);
    fPurgeableQueue.insert(resource);
}

void ResourceCache::addToNonpurgeableArray(Resource* resource) {
    int index = fNonpurgeableResources.count();
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
    fNonpurgeableResources.pop();
    SkDEBUGCODE(*index = -1);
}

uint32_t ResourceCache::getNextTimestamp() {
    // If we wrap then all the existing resources will appear older than any resources that get
    // a timestamp after the wrap.
    if (0 == fTimestamp) {
        int count = this->getResourceCount();
        if (count) {
            // Reset all the timestamps. We sort the resources by timestamp and then assign
            // sequential timestamps beginning with 0. This is O(n*lg(n)) but it should be extremely
            // rare.
            SkTDArray<Resource*> sortedPurgeableResources;
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
                uint32_t tsP = sortedPurgeableResources[currP]->timestamp();
                uint32_t tsNP = fNonpurgeableResources[currNP]->timestamp();
                SkASSERT(tsP != tsNP);
                if (tsP < tsNP) {
                    sortedPurgeableResources[currP++]->setTimestamp(fTimestamp++);
                } else {
                    // Correct the index in the nonpurgeable array stored on the resource post-sort.
                    *fNonpurgeableResources[currNP]->accessCacheIndex() = currNP;
                    fNonpurgeableResources[currNP++]->setTimestamp(fTimestamp++);
                }
            }

            // The above loop ended when we hit the end of one array. Finish the other one.
            while (currP < sortedPurgeableResources.count()) {
                sortedPurgeableResources[currP++]->setTimestamp(fTimestamp++);
            }
            while (currNP < fNonpurgeableResources.count()) {
                *fNonpurgeableResources[currNP]->accessCacheIndex() = currNP;
                fNonpurgeableResources[currNP++]->setTimestamp(fTimestamp++);
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
    // TODO: fill this out
}

bool ResourceCache::isInCache(const Resource* resource) const {
    int index = *resource->accessCacheIndex();
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

} // namespace skgpu
