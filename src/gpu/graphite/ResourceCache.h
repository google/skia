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
#include "src/gpu/graphite/Resource.h"
#include "src/gpu/graphite/ResourceTypes.h"

#if defined(GPU_TEST_UTILS)
#include <functional>
#endif

class SkTraceMemoryDump;

namespace skgpu {
class SingleOwner;
}

namespace skgpu::graphite {

class GraphiteResourceKey;
class ProxyCache;

#if defined(GPU_TEST_UTILS)
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

    using ScratchResourceSet = skia_private::THashSet<const Resource*>;
    // Find a resource that matches a key. If Shareable == kScratch, then `unavailable` must be
    // non-null and is used to filter the scratch resources that can fulfill this request.
    Resource* findAndRefResource(const GraphiteResourceKey& key,
                                 Budgeted, Shareable,
                                 const ScratchResourceSet* unavailable=nullptr);

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

    ProxyCache* proxyCache() { return fProxyCache.get(); }

    int getResourceCount() const { return fPurgeableQueue.count() + fNonpurgeableResources.size(); }

    size_t getMaxBudget() const { return fMaxBytes; }
    void setMaxBudget(size_t bytes);

    size_t currentBudgetedBytes() const { return fBudgetedBytes; }

    size_t currentPurgeableBytes() const { return fPurgeableBytes; }

    void dumpMemoryStatistics(SkTraceMemoryDump* traceMemoryDump) const;

    void forceProcessReturnedResources();

#if defined(GPU_TEST_UTILS)
    void forcePurgeAsNeeded() { this->purgeAsNeeded(); }

    // Returns the numbers of Resources that can currently be found in the cache. This includes all
    // shared Resources and all non-shareable resources that have been returned to the cache.
    int numFindableResources() const;

    Resource* topOfPurgeableQueue();

    bool testingInPurgeableQueue(Resource* resource) { return this->inPurgeableQueue(resource); }

    bool testingInReturnQueue(Resource* resource) { return resource->inReturnQueue(); }

    void visitTextures(const std::function<void(const Texture*, bool purgeable)>&) const;
#endif

    // This is a thread safe call and is a no-op if the cache has been shut down already. This
    // should only be called by Resource. Returns true if the resource was successfully added to
    // the return queue.
    bool returnResource(Resource*);

    // Registers the Resource with the cache; can only be called at the time of creation.
    void insertResource(Resource*, const GraphiteResourceKey&, Budgeted, Shareable);

private:
    ResourceCache(SingleOwner*, uint32_t recorderID, size_t maxBytes);

    // All these private functions are not meant to be thread safe. We don't check for is single
    // owner in them as we assume that has already been checked by the public api calls.
    void refAndMakeResourceMRU(Resource*);
    void addToNonpurgeableArray(Resource* resource);
    void removeFromNonpurgeableArray(Resource* resource);
    void removeFromPurgeableQueue(Resource* resource);
    // Resources in the resource map are reusable (can be returned from findAndRef), but are not
    // necessarily purgeable.
    void addToResourceMap(Resource* resource);
    void removeFromResourceMap(Resource* resource);

    // This will return true if any resources were actually returned to the cache
    bool processReturnedResources(Resource* queueHead=nullptr);
    // Returns the next resource in the linked list of the return queue
    Resource* processReturnedResource(Resource*);

    uint32_t getNextUseToken();
    void setResourceUseToken(Resource*, uint32_t token);


    bool overbudget() const { return fBudgetedBytes > fMaxBytes; }
    void purgeAsNeeded();
    void purgeResource(Resource*);
    // Passing in a nullptr for purgeTime will trigger us to try and free all unlocked resources.
    void purgeResources(const StdSteadyClock::time_point* purgeTime);

    bool inPurgeableQueue(const Resource*) const;

#if defined(SK_DEBUG)
    bool isInCache(const Resource* r) const;
    void validate() const;

    bool inNonpurgeableArray(const Resource*) const;
#else
    void validate() const {}
#endif

    struct MapTraits {
        static const GraphiteResourceKey& GetKey(const Resource& r) { return r.key(); }

        static uint32_t Hash(const GraphiteResourceKey& key) { return key.hash(); }
        static void OnFree(Resource*) {}
    };
    using ResourceMap = SkTMultiMap<Resource, GraphiteResourceKey, MapTraits>;

    static bool CompareUseToken(Resource* const& a, Resource* const& b) {
        return a->lastUseToken() < b->lastUseToken();
    }
    static int* AccessResourceIndex(Resource* const& res) { return res->accessCacheIndex(); }

    using PurgeableQueue = SkTDPQueue<Resource*, CompareUseToken, AccessResourceIndex>;
    using ResourceArray = SkTDArray<Resource*>;

    // NOTE: every Resource held by ResourceMap, ResourceArray, and PurgeableQueue will have a cache
    // ref keeping them alive until after their pointer has been removed.
    PurgeableQueue fPurgeableQueue;
    ResourceArray fNonpurgeableResources;
    ResourceMap fResourceMap;

    std::unique_ptr<ProxyCache> fProxyCache;

    // Our budget
    size_t fMaxBytes;
    size_t fBudgetedBytes = 0;
    size_t fPurgeableBytes = 0;

    // Whenever a resource is added to the cache or the result of a cache lookup, fUseToken is
    // assigned as the resource's last use token and then incremented. fPurgeableQueue orders the
    // purgeable resources by this value, and thus is used to purge resources in LRU order.
    // Resources with a size of zero are set to have max uint32_t value. This will also put them at
    // the end of the LRU priority queue. This will allow us to not purge these resources even when
    // we are over budget.
    uint32_t fUseToken = 0;

    // The head of the return queue if the pointer is non-null, or null if there are no returned
    // resources to process. A special sentinel address is used to encode when the cache is shut
    // down. Once set to that address, this will never change value.
    std::atomic<Resource*> fReturnQueue = nullptr;

    SingleOwner* fSingleOwner = nullptr;
    SkDEBUGCODE(int fCount = 0;)
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_ResourceCache_DEFINED
