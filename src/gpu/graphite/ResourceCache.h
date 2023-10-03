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
#include "src/gpu/GpuTypesPriv.h"
#include "src/gpu/graphite/ResourceTypes.h"

#if defined(GRAPHITE_TEST_UTILS)
#include <functional>
#endif
#include <vector>

namespace skgpu {
class SingleOwner;
}

namespace skgpu::graphite {

class GraphiteResourceKey;
class ProxyCache;
class Resource;

#if defined(GRAPHITE_TEST_UTILS)
class Texture;
#endif

class ResourceCache : public SkRefCnt {
public:
    static sk_sp<ResourceCache> Make(SingleOwner*, uint32_t recorderID, size_t maxBytes);
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

    // Purge resources not used since the passed point in time. Resources that have a gpu memory
    // size of zero will not be purged.
    // TODO: Should we add an optional flag to also allow purging of zero sized resources? Would we
    // want to be able to differentiate between things like Pipelines (probably never want to purge)
    // and things like descriptor sets.
    void purgeResourcesNotUsedSince(StdSteadyClock::time_point purgeTime);

    // Purge any unlocked resources. Resources that have a gpu memory size of zero will not be
    // purged.
    void purgeResources();

    // Called by the ResourceProvider when it is dropping its ref to the ResourceCache. After this
    // is called no more Resources can be returned to the ResourceCache (besides those already in
    // the return queue). Also no new Resources can be retrieved from the ResourceCache.
    void shutdown();

    size_t getMaxBudget() const { return fMaxBytes; }

    size_t currentBudgetedBytes() const { return fBudgetedBytes; }

#if defined(GRAPHITE_TEST_UTILS)
    void forceProcessReturnedResources() { this->processReturnedResources(); }

    void forcePurgeAsNeeded() { this->purgeAsNeeded(); }

    // Returns the numbers of Resources that can currently be found in the cache. This includes all
    // shared Resources and all non-shareable resources that have been returned to the cache.
    int numFindableResources() const;

    // This will probably end up being a public function to change the current budget size, but for
    // now just making this a testing only function.
    void setMaxBudget(size_t bytes);

    Resource* topOfPurgeableQueue();

    bool testingInPurgeableQueue(Resource* resource) { return this->inPurgeableQueue(resource); }

    void visitTextures(const std::function<void(const Texture*, bool purgeable)>&) const;
#endif

    ProxyCache* proxyCache() { return fProxyCache.get(); }

private:
    ResourceCache(SingleOwner*, uint32_t recorderID, size_t maxBytes);

    // All these private functions are not meant to be thread safe. We don't check for is single
    // owner in them as we assume that has already been checked by the public api calls.
    void refAndMakeResourceMRU(Resource*);
    void addToNonpurgeableArray(Resource* resource);
    void removeFromNonpurgeableArray(Resource* resource);
    void removeFromPurgeableQueue(Resource* resource);

    void processReturnedResources();
    void returnResourceToCache(Resource*, LastRemovedRef);

    uint32_t getNextTimestamp();
    void setResourceTimestamp(Resource*, uint32_t timestamp);

    bool inPurgeableQueue(Resource*) const;

    bool overbudget() const { return fBudgetedBytes > fMaxBytes; }
    void purgeAsNeeded();
    void purgeResource(Resource*);
    // Passing in a nullptr for purgeTime will trigger us to try and free all unlocked resources.
    void purgeResources(const StdSteadyClock::time_point* purgeTime);

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
    // Resources with a size of zero are set to have max uint32_t value. This will also put them at
    // the end of the LRU priority queue. This will allow us to not purge these resources even when
    // we are over budget.
    uint32_t fTimestamp = 0;
    static const uint32_t kMaxTimestamp = 0xFFFFFFFF;

    PurgeableQueue fPurgeableQueue;
    ResourceArray fNonpurgeableResources;
    std::unique_ptr<ProxyCache> fProxyCache;

    SkDEBUGCODE(int fCount = 0;)

    ResourceMap fResourceMap;

    // Our budget
    size_t fMaxBytes;
    size_t fBudgetedBytes = 0;

    SingleOwner* fSingleOwner = nullptr;

    bool fIsShutdown = false;

    SkMutex fReturnMutex;
    using ReturnQueue = std::vector<std::pair<Resource*, LastRemovedRef>>;
    ReturnQueue fReturnQueue SK_GUARDED_BY(fReturnMutex);
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_ResourceCache_DEFINED
