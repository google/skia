/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_ResourceCache_DEFINED
#define skgpu_graphite_ResourceCache_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/base/SkMutex.h"
#include "include/private/base/SkTArray.h"
#include "src/base/SkTDPQueue.h"
#include "src/core/SkTHash.h"
#include "src/core/SkTMultiMap.h"
#include "src/gpu/graphite/ResourceTypes.h"

#include <vector>

namespace skgpu {
class SingleOwner;
}

namespace skgpu::graphite {

class GraphiteResourceKey;
class Resource;

class ResourceCache : public SkRefCnt {
public:
    static sk_sp<ResourceCache> Make(SingleOwner*);
    ~ResourceCache() override;

    ResourceCache(const ResourceCache&) = delete;
    ResourceCache(ResourceCache&&) = delete;
    ResourceCache& operator=(const ResourceCache&) = delete;
    ResourceCache& operator=(ResourceCache&&) = delete;

    // Returns the number of resources.
    int getResourceCount() const {
        return fPurgeableQueue.count() + fNonpurgeableResources.size();
    }

    void insertResource(Resource*);

    // Find a resource that matches a key.
    Resource* findAndRefResource(const GraphiteResourceKey& key, skgpu::Budgeted);

    // This is a thread safe call. If it fails the ResourceCache is no longer valid and the
    // Resource should clean itself up if it is the last ref.
    bool returnResource(Resource*, LastRemovedRef);

    // Called by the ResourceProvider when it is dropping its ref to the ResourceCache. After this
    // is called no more Resources can be returned to the ResourceCache (besides those already in
    // the return queue). Also no new Resources can be retrieved from the ResourceCache.
    void shutdown();

#if GRAPHITE_TEST_UTILS
    void forceProcessReturnedResources() { this->processReturnedResources(); }

    // Returns the numbers of Resources that can currently be found in the cache. This includes all
    // shared Resources and all non-shareable resources that have been returned to the cache.
    int numFindableResources() const;
#endif

private:
    ResourceCache(SingleOwner*);

    // All these private functions are not meant to be thread safe. We don't check for is single
    // owner in them as we assume that has already been checked by the public api calls.
    void refAndMakeResourceMRU(Resource*);
    void addToNonpurgeableArray(Resource* resource);
    void removeFromNonpurgeableArray(Resource* resource);
    void removeFromPurgeableQueue(Resource* resource);

    void processReturnedResources();
    void returnResourceToCache(Resource*, LastRemovedRef);

    uint32_t getNextTimestamp();

    bool inPurgeableQueue(Resource*) const;

#ifdef SK_DEBUG
    bool isInCache(const Resource* r) const;
    void validate() const;
#else
    void validate() const {}
#endif

    struct MapTraits {
        static const GraphiteResourceKey& GetKey(const Resource& r);

        static uint32_t Hash(const GraphiteResourceKey& key);
        static void OnFree(Resource*) {}
    };
    typedef SkTMultiMap<Resource, GraphiteResourceKey, MapTraits> ResourceMap;

    static bool CompareTimestamp(Resource* const& a, Resource* const& b);
    static int* AccessResourceIndex(Resource* const& res);

    using PurgeableQueue = SkTDPQueue<Resource*, CompareTimestamp, AccessResourceIndex>;
    using ResourceArray = SkTDArray<Resource*>;

    // Whenever a resource is added to the cache or the result of a cache lookup, fTimestamp is
    // assigned as the resource's timestamp and then incremented. fPurgeableQueue orders the
    // purgeable resources by this value, and thus is used to purge resources in LRU order.
    uint32_t fTimestamp = 0;
    PurgeableQueue fPurgeableQueue;
    ResourceArray fNonpurgeableResources;

    SkDEBUGCODE(int fCount = 0;)

    ResourceMap fResourceMap;

    SingleOwner* fSingleOwner = nullptr;

    bool fIsShutdown = false;

    SkMutex fReturnMutex;
    using ReturnQueue = std::vector<std::pair<Resource*, LastRemovedRef>>;
    ReturnQueue fReturnQueue SK_GUARDED_BY(fReturnMutex);
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_ResourceCache_DEFINED
