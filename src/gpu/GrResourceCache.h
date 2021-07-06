/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrResourceCache_DEFINED
#define GrResourceCache_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/gpu/GrDirectContext.h"
#include "include/private/GrResourceKey.h"
#include "include/private/SkTArray.h"
#include "include/private/SkTHash.h"
#include "src/core/SkMessageBus.h"
#include "src/core/SkTDPQueue.h"
#include "src/core/SkTInternalLList.h"
#include "src/core/SkTMultiMap.h"
#include "src/gpu/GrGpuResource.h"
#include "src/gpu/GrGpuResourceCacheAccess.h"
#include "src/gpu/GrGpuResourcePriv.h"

class GrCaps;
class GrProxyProvider;
class SkString;
class SkTraceMemoryDump;
class GrSingleOwner;
class GrTexture;
class GrThreadSafeCache;

struct GrTextureFreedMessage {
    GrTexture* fTexture;
    GrDirectContext::DirectContextID fIntendedRecipient;
};

static inline bool SkShouldPostMessageToBus(
        const GrTextureFreedMessage& msg, GrDirectContext::DirectContextID potentialRecipient) {
    return potentialRecipient == msg.fIntendedRecipient;
}

/**
 * Manages the lifetime of all GrGpuResource instances.
 *
 * Resources may have optionally have two types of keys:
 *      1) A scratch key. This is for resources whose allocations are cached but not their contents.
 *         Multiple resources can share the same scratch key. This is so a caller can have two
 *         resource instances with the same properties (e.g. multipass rendering that ping-pongs
 *         between two temporary surfaces). The scratch key is set at resource creation time and
 *         should never change. Resources need not have a scratch key.
 *      2) A unique key. This key's meaning is specific to the domain that created the key. Only one
 *         resource may have a given unique key. The unique key can be set, cleared, or changed
 *         anytime after resource creation.
 *
 * A unique key always takes precedence over a scratch key when a resource has both types of keys.
 * If a resource has neither key type then it will be deleted as soon as the last reference to it
 * is dropped.
 */
class GrResourceCache {
public:
    GrResourceCache(GrSingleOwner* owner,
                    GrDirectContext::DirectContextID owningContextID,
                    uint32_t familyID);
    ~GrResourceCache();

    // Default maximum number of bytes of gpu memory of budgeted resources in the cache.
    static const size_t kDefaultMaxSize             = 256 * (1 << 20);

    /** Used to access functionality needed by GrGpuResource for lifetime management. */
    class ResourceAccess;
    ResourceAccess resourceAccess();

    /** Unique ID of the owning GrContext. */
    uint32_t contextUniqueID() const { return fContextUniqueID; }

    /** Sets the max gpu memory byte size of the cache. */
    void setLimit(size_t bytes);

    /**
     * Returns the number of resources.
     */
    int getResourceCount() const {
        return fPurgeableQueue.count() + fNonpurgeableResources.count();
    }

    /**
     * Returns the number of resources that count against the budget.
     */
    int getBudgetedResourceCount() const { return fBudgetedCount; }

    /**
     * Returns the number of bytes consumed by resources.
     */
    size_t getResourceBytes() const { return fBytes; }

    /**
     * Returns the number of bytes held by unlocked resources which are available for purging.
     */
    size_t getPurgeableBytes() const { return fPurgeableBytes; }

    /**
     * Returns the number of bytes consumed by budgeted resources.
     */
    size_t getBudgetedResourceBytes() const { return fBudgetedBytes; }

    /**
     * Returns the number of bytes consumed by cached resources.
     */
    size_t getMaxResourceBytes() const { return fMaxBytes; }

    /**
     * Abandons the backend API resources owned by all GrGpuResource objects and removes them from
     * the cache.
     */
    void abandonAll();

    /**
     * Releases the backend API resources owned by all GrGpuResource objects and removes them from
     * the cache.
     */
    void releaseAll();

    /**
     * Find a resource that matches a scratch key.
     */
    GrGpuResource* findAndRefScratchResource(const GrScratchKey& scratchKey);

#ifdef SK_DEBUG
    // This is not particularly fast and only used for validation, so debug only.
    int countScratchEntriesForKey(const GrScratchKey& scratchKey) const {
        return fScratchMap.countForKey(scratchKey);
    }
#endif

    /**
     * Find a resource that matches a unique key.
     */
    GrGpuResource* findAndRefUniqueResource(const GrUniqueKey& key) {
        GrGpuResource* resource = fUniqueHash.find(key);
        if (resource) {
            this->refAndMakeResourceMRU(resource);
        }
        return resource;
    }

    /**
     * Query whether a unique key exists in the cache.
     */
    bool hasUniqueKey(const GrUniqueKey& key) const {
        return SkToBool(fUniqueHash.find(key));
    }

    /** Purges resources to become under budget and processes resources with invalidated unique
        keys. */
    void purgeAsNeeded();

    // Purge unlocked resources. If 'scratchResourcesOnly' is true the purgeable resources
    // containing persistent data are spared. If it is false then all purgeable resources will
    // be deleted.
    void purgeUnlockedResources(bool scratchResourcesOnly=false) {
        this->purgeUnlockedResources(/*purgeTime=*/nullptr, scratchResourcesOnly);
    }

    // Purge unlocked resources not used since the passed point in time. If 'scratchResourcesOnly'
    // is true the purgeable resources containing persistent data are spared. If it is false then
    // all purgeable resources older than 'purgeTime' will be deleted.
    void purgeResourcesNotUsedSince(GrStdSteadyClock::time_point purgeTime,
                                    bool scratchResourcesOnly=false) {
        this->purgeUnlockedResources(&purgeTime, scratchResourcesOnly);
    }

    /** If it's possible to purge enough resources to get the provided amount of budget
        headroom, do so and return true. If it's not possible, do nothing and return false.
     */
    bool purgeToMakeHeadroom(size_t desiredHeadroomBytes);

    bool overBudget() const { return fBudgetedBytes > fMaxBytes; }

    /**
     * Purge unlocked resources from the cache until the the provided byte count has been reached
     * or we have purged all unlocked resources. The default policy is to purge in LRU order, but
     * can be overridden to prefer purging scratch resources (in LRU order) prior to purging other
     * resource types.
     *
     * @param maxBytesToPurge the desired number of bytes to be purged.
     * @param preferScratchResources If true scratch resources will be purged prior to other
     *                               resource types.
     */
    void purgeUnlockedResources(size_t bytesToPurge, bool preferScratchResources);

    /** Returns true if the cache would like a flush to occur in order to make more resources
        purgeable. */
    bool requestsFlush() const;

    /** Maintain a ref to this texture until we receive a GrTextureFreedMessage. */
    void insertDelayedTextureUnref(GrTexture*);

#if GR_CACHE_STATS
    struct Stats {
        int fTotal;
        int fNumPurgeable;
        int fNumNonPurgeable;

        int fScratch;
        int fWrapped;
        size_t fUnbudgetedSize;

        Stats() { this->reset(); }

        void reset() {
            fTotal = 0;
            fNumPurgeable = 0;
            fNumNonPurgeable = 0;
            fScratch = 0;
            fWrapped = 0;
            fUnbudgetedSize = 0;
        }

        void update(GrGpuResource* resource) {
            if (resource->cacheAccess().isScratch()) {
                ++fScratch;
            }
            if (resource->resourcePriv().refsWrappedObjects()) {
                ++fWrapped;
            }
            if (GrBudgetedType::kBudgeted != resource->resourcePriv().budgetedType()) {
                fUnbudgetedSize += resource->gpuMemorySize();
            }
        }
    };

    void getStats(Stats*) const;

#if GR_TEST_UTILS
    void dumpStats(SkString*) const;

    void dumpStatsKeyValuePairs(SkTArray<SkString>* keys, SkTArray<double>* value) const;
#endif

#endif // GR_CACHE_STATS

#if GR_TEST_UTILS
    int countUniqueKeysWithTag(const char* tag) const;

    // This function is for unit testing and is only defined in test tools.
    void changeTimestamp(uint32_t newTimestamp);
#endif

    // Enumerates all cached resources and dumps their details to traceMemoryDump.
    void dumpMemoryStatistics(SkTraceMemoryDump* traceMemoryDump) const;

    void setProxyProvider(GrProxyProvider* proxyProvider) { fProxyProvider = proxyProvider; }
    void setThreadSafeCache(GrThreadSafeCache* threadSafeCache) {
        fThreadSafeCache = threadSafeCache;
    }

private:
    ///////////////////////////////////////////////////////////////////////////
    /// @name Methods accessible via ResourceAccess
    ////
    void insertResource(GrGpuResource*);
    void removeResource(GrGpuResource*);
    void notifyARefCntReachedZero(GrGpuResource*, GrGpuResource::LastRemovedRef);
    void changeUniqueKey(GrGpuResource*, const GrUniqueKey&);
    void removeUniqueKey(GrGpuResource*);
    void willRemoveScratchKey(const GrGpuResource*);
    void didChangeBudgetStatus(GrGpuResource*);
    void refResource(GrGpuResource* resource);
    /// @}

    void refAndMakeResourceMRU(GrGpuResource*);
    void processFreedGpuResources();
    void addToNonpurgeableArray(GrGpuResource*);
    void removeFromNonpurgeableArray(GrGpuResource*);

    bool wouldFit(size_t bytes) const { return fBudgetedBytes+bytes <= fMaxBytes; }

    uint32_t getNextTimestamp();

    void purgeUnlockedResources(const GrStdSteadyClock::time_point* purgeTime,
                                bool scratchResourcesOnly);

#ifdef SK_DEBUG
    bool isInCache(const GrGpuResource* r) const;
    void validate() const;
#else
    void validate() const {}
#endif

    class AutoValidate;

    class AvailableForScratchUse;

    struct ScratchMapTraits {
        static const GrScratchKey& GetKey(const GrGpuResource& r) {
            return r.resourcePriv().getScratchKey();
        }

        static uint32_t Hash(const GrScratchKey& key) { return key.hash(); }
        static void OnFree(GrGpuResource*) { }
    };
    typedef SkTMultiMap<GrGpuResource, GrScratchKey, ScratchMapTraits> ScratchMap;

    struct UniqueHashTraits {
        static const GrUniqueKey& GetKey(const GrGpuResource& r) { return r.getUniqueKey(); }

        static uint32_t Hash(const GrUniqueKey& key) { return key.hash(); }
    };
    typedef SkTDynamicHash<GrGpuResource, GrUniqueKey, UniqueHashTraits> UniqueHash;

    class TextureAwaitingUnref {
    public:
        TextureAwaitingUnref();
        TextureAwaitingUnref(GrTexture* texture);
        TextureAwaitingUnref(const TextureAwaitingUnref&) = delete;
        TextureAwaitingUnref& operator=(const TextureAwaitingUnref&) = delete;
        TextureAwaitingUnref(TextureAwaitingUnref&&);
        TextureAwaitingUnref& operator=(TextureAwaitingUnref&&);
        ~TextureAwaitingUnref();
        void addRef();
        void unref();
        bool finished();

    private:
        GrTexture* fTexture = nullptr;
        int fNumUnrefs = 0;
    };
    using TexturesAwaitingUnref = SkTHashMap<uint32_t, TextureAwaitingUnref>;

    static bool CompareTimestamp(GrGpuResource* const& a, GrGpuResource* const& b) {
        return a->cacheAccess().timestamp() < b->cacheAccess().timestamp();
    }

    static int* AccessResourceIndex(GrGpuResource* const& res) {
        return res->cacheAccess().accessCacheIndex();
    }

    using TextureFreedMessageBus = SkMessageBus<GrTextureFreedMessage,
                                                GrDirectContext::DirectContextID>;

    typedef SkMessageBus<GrUniqueKeyInvalidatedMessage, uint32_t>::Inbox InvalidUniqueKeyInbox;
    typedef SkTDPQueue<GrGpuResource*, CompareTimestamp, AccessResourceIndex> PurgeableQueue;
    typedef SkTDArray<GrGpuResource*> ResourceArray;

    GrProxyProvider*                    fProxyProvider = nullptr;
    GrThreadSafeCache*                  fThreadSafeCache = nullptr;

    // Whenever a resource is added to the cache or the result of a cache lookup, fTimestamp is
    // assigned as the resource's timestamp and then incremented. fPurgeableQueue orders the
    // purgeable resources by this value, and thus is used to purge resources in LRU order.
    uint32_t                            fTimestamp = 0;
    PurgeableQueue                      fPurgeableQueue;
    ResourceArray                       fNonpurgeableResources;

    // This map holds all resources that can be used as scratch resources.
    ScratchMap                          fScratchMap;
    // This holds all resources that have unique keys.
    UniqueHash                          fUniqueHash;

    // our budget, used in purgeAsNeeded()
    size_t                              fMaxBytes = kDefaultMaxSize;

#if GR_CACHE_STATS
    int                                 fHighWaterCount = 0;
    size_t                              fHighWaterBytes = 0;
    int                                 fBudgetedHighWaterCount = 0;
    size_t                              fBudgetedHighWaterBytes = 0;
#endif

    // our current stats for all resources
    SkDEBUGCODE(int                     fCount = 0;)
    size_t                              fBytes = 0;

    // our current stats for resources that count against the budget
    int                                 fBudgetedCount = 0;
    size_t                              fBudgetedBytes = 0;
    size_t                              fPurgeableBytes = 0;
    int                                 fNumBudgetedResourcesFlushWillMakePurgeable = 0;

    InvalidUniqueKeyInbox               fInvalidUniqueKeyInbox;
    TextureFreedMessageBus::Inbox       fFreedTextureInbox;
    TexturesAwaitingUnref               fTexturesAwaitingUnref;

    GrDirectContext::DirectContextID    fOwningContextID;
    uint32_t                            fContextUniqueID = SK_InvalidUniqueID;
    GrSingleOwner*                      fSingleOwner = nullptr;

    // This resource is allowed to be in the nonpurgeable array for the sake of validate() because
    // we're in the midst of converting it to purgeable status.
    SkDEBUGCODE(GrGpuResource*          fNewlyPurgeableResourceForValidation = nullptr;)
};

class GrResourceCache::ResourceAccess {
private:
    ResourceAccess(GrResourceCache* cache) : fCache(cache) { }
    ResourceAccess(const ResourceAccess& that) : fCache(that.fCache) { }
    ResourceAccess& operator=(const ResourceAccess&) = delete;

    /**
     * Insert a resource into the cache.
     */
    void insertResource(GrGpuResource* resource) { fCache->insertResource(resource); }

    /**
     * Removes a resource from the cache.
     */
    void removeResource(GrGpuResource* resource) { fCache->removeResource(resource); }

    /**
     * Adds a ref to a resource with proper tracking if the resource has 0 refs prior to
     * adding the ref.
     */
    void refResource(GrGpuResource* resource) { fCache->refResource(resource); }

    /**
     * Notifications that should be sent to the cache when the ref/io cnt status of resources
     * changes.
     */
    enum RefNotificationFlags {
        /** All types of refs on the resource have reached zero. */
        kAllCntsReachedZero_RefNotificationFlag = 0x1,
        /** The normal (not pending IO type) ref cnt has reached zero. */
        kRefCntReachedZero_RefNotificationFlag  = 0x2,
    };
    /**
     * Called by GrGpuResources when they detect one of their ref cnts have reached zero. This may
     * either be the main ref or the command buffer usage ref.
     */
    void notifyARefCntReachedZero(GrGpuResource* resource,
                                  GrGpuResource::LastRemovedRef removedRef) {
        fCache->notifyARefCntReachedZero(resource, removedRef);
    }

    /**
     * Called by GrGpuResources to change their unique keys.
     */
    void changeUniqueKey(GrGpuResource* resource, const GrUniqueKey& newKey) {
         fCache->changeUniqueKey(resource, newKey);
    }

    /**
     * Called by a GrGpuResource to remove its unique key.
     */
    void removeUniqueKey(GrGpuResource* resource) { fCache->removeUniqueKey(resource); }

    /**
     * Called by a GrGpuResource when it removes its scratch key.
     */
    void willRemoveScratchKey(const GrGpuResource* resource) {
        fCache->willRemoveScratchKey(resource);
    }

    /**
     * Called by GrGpuResources when they change from budgeted to unbudgeted or vice versa.
     */
    void didChangeBudgetStatus(GrGpuResource* resource) { fCache->didChangeBudgetStatus(resource); }

    // No taking addresses of this type.
    const ResourceAccess* operator&() const;
    ResourceAccess* operator&();

    GrResourceCache* fCache;

    friend class GrGpuResource; // To access all the proxy inline methods.
    friend class GrResourceCache; // To create this type.
};

inline GrResourceCache::ResourceAccess GrResourceCache::resourceAccess() {
    return ResourceAccess(this);
}

#endif
