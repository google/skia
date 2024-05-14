/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_Resource_DEFINED
#define skgpu_graphite_Resource_DEFINED

#include "include/gpu/GpuTypes.h"
#include "include/private/base/SkMutex.h"
#include "src/gpu/GpuTypesPriv.h"
#include "src/gpu/graphite/GraphiteResourceKey.h"
#include "src/gpu/graphite/ResourceTypes.h"

#include <atomic>
#include <functional>
#include <string>
#include <string_view>

class SkMutex;
class SkTraceMemoryDump;

namespace skgpu::graphite {

class ResourceCache;
class SharedContext;

#if defined(GRAPHITE_TEST_UTILS)
class Texture;
#endif

/**
 * Base class for objects that can be kept in the ResourceCache.
 */
class Resource {
public:
    Resource(const Resource&) = delete;
    Resource(Resource&&) = delete;
    Resource& operator=(const Resource&) = delete;
    Resource& operator=(Resource&&) = delete;

    // Adds a usage ref to the resource. Named ref so we can easily manage usage refs with sk_sp.
    void ref() const {
        // Only the cache should be able to add the first usage ref to a resource.
        SkASSERT(this->hasUsageRef());
        // No barrier required.
        (void)fUsageRefCnt.fetch_add(+1, std::memory_order_relaxed);
    }

    // Removes a usage ref from the resource
    void unref() const {
        bool shouldFree = false;
        {
            SkAutoMutexExclusive locked(fUnrefMutex);
            SkASSERT(this->hasUsageRef());
            // A release here acts in place of all releases we "should" have been doing in ref().
            if (1 == fUsageRefCnt.fetch_add(-1, std::memory_order_acq_rel)) {
                shouldFree = this->notifyARefIsZero(LastRemovedRef::kUsage);
            }
        }
        if (shouldFree) {
            Resource* mutableThis = const_cast<Resource*>(this);
            mutableThis->internalDispose();
        }
    }

    // Adds a command buffer ref to the resource
    void refCommandBuffer() const {
        if (fCommandBufferRefsAsUsageRefs) {
            return this->ref();
        }
        // No barrier required.
        (void)fCommandBufferRefCnt.fetch_add(+1, std::memory_order_relaxed);
    }

    // Removes a command buffer ref from the resource
    void unrefCommandBuffer() const {
        if (fCommandBufferRefsAsUsageRefs) {
            return this->unref();
        }
        bool shouldFree = false;
        {
            SkAutoMutexExclusive locked(fUnrefMutex);
            SkASSERT(this->hasCommandBufferRef());
            // A release here acts in place of all releases we "should" have been doing in ref().
            if (1 == fCommandBufferRefCnt.fetch_add(-1, std::memory_order_acq_rel)) {
                shouldFree = this->notifyARefIsZero(LastRemovedRef::kCommandBuffer);
            }
        }
        if (shouldFree) {
            Resource* mutableThis = const_cast<Resource*>(this);
            mutableThis->internalDispose();
        }
    }

    Ownership ownership() const { return fOwnership; }

    skgpu::Budgeted budgeted() const { return fBudgeted; }

    // Retrieves the amount of GPU memory used by this resource in bytes. It is approximate since we
    // aren't aware of additional padding or copies made by the driver.
    size_t gpuMemorySize() const { return fGpuMemorySize; }

    class UniqueID {
    public:
        UniqueID() = default;

        explicit UniqueID(uint32_t id) : fID(id) {}

        uint32_t asUInt() const { return fID; }

        bool operator==(const UniqueID& other) const { return fID == other.fID; }
        bool operator!=(const UniqueID& other) const { return !(*this == other); }

    private:
        uint32_t fID = SK_InvalidUniqueID;
    };

    // Gets an id that is unique for this Resource object. It is static in that it does not change
    // when the content of the Resource object changes. This will never return 0.
    UniqueID uniqueID() const { return fUniqueID; }

    // Describes the type of gpu resource that is represented by the implementing
    // class (e.g. texture, buffer, etc).  This data is used for diagnostic
    // purposes by dumpMemoryStatistics().
    //
    // The value returned is expected to be long lived and will not be copied by the caller.
    virtual const char* getResourceType() const = 0;

    std::string getLabel() const { return fLabel; }

    // We allow the label on a Resource to change when used for a different function. For example
    // when reusing a scratch Texture we can change the label to match callers current use.
    void setLabel(std::string_view label) {
        fLabel = label;

        if (!fLabel.empty()) {
            const std::string fullLabel = "Skia_" + fLabel;
            this->setBackendLabel(fullLabel.c_str());
        }
    }

    // Tests whether a object has been abandoned or released. All objects will be in this state
    // after their creating Context is destroyed or abandoned.
    //
    // @return true if the object has been released or abandoned,
    //         false otherwise.
    // TODO: As of now this function isn't really needed because in freeGpuData we are always
    // deleting this object. However, I want to implement all the purging logic first to make sure
    // we don't have a use case for calling internalDispose but not wanting to delete the actual
    // object yet.
    bool wasDestroyed() const { return fSharedContext == nullptr; }

    const GraphiteResourceKey& key() const { return fKey; }
    // This should only ever be called by the ResourceProvider
    void setKey(const GraphiteResourceKey& key) {
        SkASSERT(key.shareable() == Shareable::kNo || this->budgeted() == skgpu::Budgeted::kYes);
        fKey = key;
    }

    // Dumps memory usage information for this Resource to traceMemoryDump.
    void dumpMemoryStatistics(SkTraceMemoryDump* traceMemoryDump) const;

    /**
     * If the resource has a non-shareable key then this gives the resource subclass an opportunity
     * to prepare itself to re-enter the cache. The ResourceCache extends its privilege to take the
     * first UsageRef to this function via takeRef. If takeRef is called this resource will not
     * immediately enter the cache but will be re-reprocessed with the Usage Ref count again reaches
     * zero.
     */
    virtual void prepareForReturnToCache(const std::function<void()>& takeRef) {}

#if defined(GRAPHITE_TEST_UTILS)
    bool testingShouldDeleteASAP() const { return fDeleteASAP == DeleteASAP::kYes; }

    virtual const Texture* asTexture() const { return nullptr; }
#endif

protected:
    Resource(const SharedContext*,
             Ownership,
             skgpu::Budgeted,
             size_t gpuMemorySize,
             bool commandBufferRefsAsUsageRefs = false);
    virtual ~Resource();

    const SharedContext* sharedContext() const { return fSharedContext; }

    // Overridden to add extra information to the memory dump.
    virtual void onDumpMemoryStatistics(SkTraceMemoryDump* traceMemoryDump,
                                        const char* dumpName) const {}

#ifdef SK_DEBUG
    bool debugHasCommandBufferRef() const {
        return hasCommandBufferRef();
    }
#endif

    // Needed to be protected for DawnBuffer emscripten prepareForReturnToCache
    void setDeleteASAP() { fDeleteASAP = DeleteASAP::kYes; }

private:
    friend class ProxyCache; // for setDeleteASAP and updateAccessTime

    // Overridden to free GPU resources in the backend API.
    virtual void freeGpuData() = 0;

    // Overridden to call any release callbacks, if necessary
    virtual void invokeReleaseProc() {}

    enum class DeleteASAP : bool {
        kNo = false,
        kYes = true,
    };

    DeleteASAP shouldDeleteASAP() const { return fDeleteASAP; }

    // In the ResourceCache this is called whenever a Resource is moved into the purgeableQueue. It
    // may also be called by the ProxyCache to track the time on Resources it is holding on to.
    void updateAccessTime() {
        fLastAccess = skgpu::StdSteadyClock::now();
    }
    skgpu::StdSteadyClock::time_point lastAccessTime() const {
        return fLastAccess;
    }

    virtual void setBackendLabel(char const* label) {}

    ////////////////////////////////////////////////////////////////////////////
    // The following set of functions are only meant to be called by the ResourceCache. We don't
    // want them public general users of a Resource, but they also aren't purely internal calls.
    ////////////////////////////////////////////////////////////////////////////
    friend ResourceCache;

    void makeBudgeted() { fBudgeted = skgpu::Budgeted::kYes; }
    void makeUnbudgeted() { fBudgeted = skgpu::Budgeted::kNo; }

    // This version of ref allows adding a ref when the usage count is 0. This should only be called
    // from the ResourceCache.
    void initialUsageRef() const {
        // Only the cache should be able to add the first usage ref to a resource.
        SkASSERT(fUsageRefCnt >= 0);
        // No barrier required.
        (void)fUsageRefCnt.fetch_add(+1, std::memory_order_relaxed);
    }

    bool isPurgeable() const;
    int* accessReturnIndex()  const { return &fReturnIndex; }
    int* accessCacheIndex()  const { return &fCacheArrayIndex; }

    uint32_t timestamp() const { return fTimestamp; }
    void setTimestamp(uint32_t ts) { fTimestamp = ts; }

    void registerWithCache(sk_sp<ResourceCache>);

    // Adds a cache ref to the resource. This is only called by ResourceCache. A Resource will only
    // ever add a ref when the Resource is part of the cache (i.e. when insertResource is called)
    // and while the Resource is in the ResourceCache::ReturnQueue.
    void refCache() const {
        // No barrier required.
        (void)fCacheRefCnt.fetch_add(+1, std::memory_order_relaxed);
    }

    // Removes a cache ref from the resource. The unref here should only ever be called from the
    // ResourceCache and only in the Recorder thread the ResourceCache is part of.
    void unrefCache() const {
        bool shouldFree = false;
        {
            SkAutoMutexExclusive locked(fUnrefMutex);
            SkASSERT(this->hasCacheRef());
            // A release here acts in place of all releases we "should" have been doing in ref().
            if (1 == fCacheRefCnt.fetch_add(-1, std::memory_order_acq_rel)) {
                shouldFree = this->notifyARefIsZero(LastRemovedRef::kCache);
            }
        }
        if (shouldFree) {
            Resource* mutableThis = const_cast<Resource*>(this);
            mutableThis->internalDispose();
        }
    }

#ifdef SK_DEBUG
    bool isUsableAsScratch() const {
        return fKey.shareable() == Shareable::kNo && !this->hasUsageRef() && fNonShareableInCache;
    }
#endif

    ////////////////////////////////////////////////////////////////////////////
    // The remaining calls are meant to be truely private
    ////////////////////////////////////////////////////////////////////////////
    bool hasUsageRef() const {
        if (0 == fUsageRefCnt.load(std::memory_order_acquire)) {
            // The acquire barrier is only really needed if we return true.  It
            // prevents code conditioned on the result of hasUsageRef() from running until previous
            // owners are all totally done calling unref().
            return false;
        }
        return true;
    }

    bool hasCommandBufferRef() const {
        // Note that we don't check here for fCommandBufferRefsAsUsageRefs. This should always
        // report zero if that value is true.
        if (0 == fCommandBufferRefCnt.load(std::memory_order_acquire)) {
            // The acquire barrier is only really needed if we return true.  It
            // prevents code conditioned on the result of hasCommandBufferRef() from running
            // until previous owners are all totally done calling unrefCommandBuffer().
            return false;
        }
        SkASSERT(!fCommandBufferRefsAsUsageRefs);
        return true;
    }

    bool hasCacheRef() const {
        if (0 == fCacheRefCnt.load(std::memory_order_acquire)) {
            // The acquire barrier is only really needed if we return true. It
            // prevents code conditioned on the result of hasUsageRef() from running until previous
            // owners are all totally done calling unref().
            return false;
        }
        return true;
    }

    bool hasAnyRefs() const {
        return this->hasUsageRef() || this->hasCommandBufferRef() || this->hasCacheRef();
    }

    bool notifyARefIsZero(LastRemovedRef removedRef) const;

    // Frees the object in the underlying 3D API.
    void internalDispose();

    // We need to guard calling unref on the usage and command buffer refs since they each could be
    // unreffed on different threads. This can lead to calling notifyARefIsZero twice with each
    // instance thinking there are no more refs left and both trying to delete the object.
    mutable SkMutex fUnrefMutex;

    SkDEBUGCODE(mutable bool fCalledRemovedFromCache = false;)

    // This is not ref'ed but internalDispose() will be called before the Gpu object is destroyed.
    // That call will set this to nullptr.
    const SharedContext* fSharedContext;

    mutable std::atomic<int32_t> fUsageRefCnt;
    mutable std::atomic<int32_t> fCommandBufferRefCnt;
    mutable std::atomic<int32_t> fCacheRefCnt;
    // Indicates that CommandBufferRefs should be rerouted to UsageRefs.
    const bool fCommandBufferRefsAsUsageRefs = false;

    GraphiteResourceKey fKey;

    sk_sp<ResourceCache> fReturnCache;
    // An index into the return cache so we know whether or not the resource is already waiting to
    // be returned or not.
    mutable int fReturnIndex = -1;

    Ownership fOwnership;

    static const size_t kInvalidGpuMemorySize = ~static_cast<size_t>(0);
    mutable size_t fGpuMemorySize = kInvalidGpuMemorySize;

    // All resource created internally by Graphite and held in the ResourceCache as a shared
    // resource or available scratch resource are considered budgeted. Resources that back client
    // owned objects (e.g. SkSurface or SkImage) are not budgeted and do not count against cache
    // limits.
    skgpu::Budgeted fBudgeted;

    // This is only used by ProxyCache::purgeProxiesNotUsedSince which is called from
    // ResourceCache::purgeResourcesNotUsedSince. When kYes, this signals that the Resource
    // should've been purged based on its timestamp at some point regardless of what its
    // current timestamp may indicate (since the timestamp will be updated when the Resource
    // is returned to the ResourceCache).
    DeleteASAP fDeleteASAP = DeleteASAP::kNo;

    // An index into a heap when this resource is purgeable or an array when not. This is maintained
    // by the cache.
    mutable int fCacheArrayIndex = -1;
    // This value reflects how recently this resource was accessed in the cache. This is maintained
    // by the cache.
    uint32_t fTimestamp;
    skgpu::StdSteadyClock::time_point fLastAccess;

    const UniqueID fUniqueID;

    // String used to describe the current use of this Resource.
    std::string fLabel;

    // This is only used during validation checking. Lots of the validation code depends on a
    // resource being purgeable or not. However, purgeable itself just means having no refs. The
    // refs can be removed before a Resource is returned to the cache (or even added to the
    // ReturnQueue).
    SkDEBUGCODE(mutable bool fNonShareableInCache = false;)
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_Resource_DEFINED
