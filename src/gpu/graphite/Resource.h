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

class GlobalCache;
class ResourceCache;
class SharedContext;
class Texture;

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

    Budgeted budgeted() const { return fBudgeted; }
    Shareable shareable() const { return fShareable; }
    const GraphiteResourceKey& key() const { return fKey; }

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

    // Describes the type of gpu resource that is represented by the implementing
    // class (e.g. texture, buffer, etc).  This data is used for diagnostic
    // purposes by dumpMemoryStatistics().
    //
    // The value returned is expected to be long lived and will not be copied by the caller.
    virtual const char* getResourceType() const = 0;

    virtual const Texture* asTexture() const { return nullptr; }

#if defined(GPU_TEST_UTILS)
    bool testingShouldDeleteASAP() const { return fDeleteASAP == DeleteASAP::kYes; }
#endif

protected:
    Resource(const SharedContext*,
             Ownership,
             size_t gpuMemorySize,
             bool commandBufferRefsAsUsageRefs = false);
    virtual ~Resource();

    const SharedContext* sharedContext() const { return fSharedContext; }

#ifdef SK_DEBUG
    bool debugHasCommandBufferRef() const {
        return hasCommandBufferRef();
    }
#endif

    // Needed to be protected for DawnBuffer's emscripten prepareForReturnToCache()
    void setDeleteASAP() { fDeleteASAP = DeleteASAP::kYes; }

private:
    ///////////////////////////////////////////////////////////////////////////////////////////////
    // The following set of functions are only meant to be called by the [Global|Proxy]Cache. We
    // don't want them public general users of a Resource, but they also aren't purely internal.
    ///////////////////////////////////////////////////////////////////////////////////////////////
    friend class ProxyCache; // for setDeleteASAP and updateAccessTime
    friend GlobalCache; // for lastAccessTime and updateAccessTime

    enum class DeleteASAP : bool {
        kNo = false,
        kYes = true,
    };

    DeleteASAP shouldDeleteASAP() const { return fDeleteASAP; }

    // In the ResourceCache this is called whenever a Resource is moved into the purgeableQueue. It
    // may also be called by the ProxyCache and GlobalCache to track the time on Resources they are
    // holding on to.
    void updateAccessTime() {
        fLastAccess = skgpu::StdSteadyClock::now();
    }
    skgpu::StdSteadyClock::time_point lastAccessTime() const {
        return fLastAccess;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // The following set of functions are only meant to be called by the ResourceCache. We don't
    // want them public general users of a Resource, but they also aren't purely internal calls.
    ///////////////////////////////////////////////////////////////////////////////////////////////
    friend ResourceCache;

    void setBudgeted(Budgeted budgeted) {
        SkASSERT(budgeted == Budgeted::kNo || fOwnership == Ownership::kOwned);
        fBudgeted = budgeted;
    }
    void setShareable(Shareable shareable) {
        SkASSERT(shareable == Shareable::kNo || fBudgeted == Budgeted::kYes);
        fShareable = shareable;
    }

    void setAvailableForReuse(bool avail) { fAvailableForReuse = avail; }
    bool isAvailableForReuse() const { return fAvailableForReuse; }

    int* accessReturnIndex()  const { return &fReturnIndex; }
    int* accessCacheIndex()  const { return &fCacheArrayIndex; }

    uint32_t lastUseToken() const { return fLastUseToken; }
    void setLastUseToken(uint32_t token) { fLastUseToken = token; }

    // If possible, queries the backend API to check the current allocation size of the gpu
    // resource and updates the tracked value. This is specifically useful for Vulkan backends which
    // use lazy allocated memory for "memoryless" resources. Ideally that memory should stay zero
    // throughout its usage, but certain usage patterns can trigger the device to commit real memory
    // to the resource. So this will allow us to have a more accurate tracking of our memory usage.
    void updateGpuMemorySize() { fGpuMemorySize = this->onUpdateGpuMemorySize(); }

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

    // This version of ref allows adding a ref when the usage count is 0. This should only be called
    // from the ResourceCache.
    void initialUsageRef() const {
        // Only the cache should be able to add the first usage ref to a resource.
        SkASSERT(fUsageRefCnt >= 0);
        // No barrier required.
        (void)fUsageRefCnt.fetch_add(+1, std::memory_order_relaxed);
    }

    // May only be called once.
    void registerWithCache(sk_sp<ResourceCache>, const GraphiteResourceKey&, Budgeted, Shareable);

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

    bool isPurgeable() const;

#ifdef SK_DEBUG
    bool isUsableAsScratch() const {
        return fShareable == Shareable::kScratch ||
               (fShareable == Shareable::kNo && !this->hasUsageRef() && fAvailableForReuse);
    }
#endif

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // The remaining calls are meant to be truely private.
    ///////////////////////////////////////////////////////////////////////////////////////////////

    // Overridden to free GPU resources in the backend API.
    virtual void freeGpuData() = 0;

    // Overridden to call any release callbacks, if necessary
    virtual void invokeReleaseProc() {}

    // Overridden to set the label on the underlying GPU resource
    virtual void setBackendLabel(char const* label) {}

    // Overridden to add extra information to the memory dump.
    virtual void onDumpMemoryStatistics(SkTraceMemoryDump* traceMemoryDump,
                                        const char* dumpName) const {}

    // Overridden to calculate a more up-to-date size in bytes.
    virtual size_t onUpdateGpuMemorySize() { return fGpuMemorySize; }

    bool hasUsageRef() const {
        if (0 == fUsageRefCnt.load(std::memory_order_acquire)) {
            // The acquire barrier is only really needed if we return true.  It prevents code
            // conditioned on the result of hasUsageRef() from running until previous owners are all
            // totally done calling unref().
            return false;
        }
        return true;
    }

    bool hasCommandBufferRef() const {
        // Note that we don't check here for fCommandBufferRefsAsUsageRefs. This should always
        // report zero if that value is true.
        if (0 == fCommandBufferRefCnt.load(std::memory_order_acquire)) {
            // The acquire barrier is only really needed if we return true.  It prevents code
            // conditioned on the result of hasCommandBufferRef() from running until previous owners
            // are all totally done calling unrefCommandBuffer().
            return false;
        }
        SkASSERT(!fCommandBufferRefsAsUsageRefs);
        return true;
    }

    bool hasCacheRef() const {
        if (0 == fCacheRefCnt.load(std::memory_order_acquire)) {
            // The acquire barrier is only really needed if we return true. It prevents code
            // conditioned on the result of hasUsageRef() from running until previous owners are all
            // totally done calling unref().
            return false;
        }
        return true;
    }

    bool hasAnyRefs() const {
        return this->hasUsageRef() || this->hasCommandBufferRef() || this->hasCacheRef();
    }

    bool notifyARefIsZero(LastRemovedRef removedRef) const;

    // Frees the object in the underlying 3D API *and* deletes the object itself.
    void internalDispose();

    static const size_t kInvalidGpuMemorySize = ~static_cast<size_t>(0);

    // We need to guard calling unref on the usage and command buffer refs since they each could be
    // unreffed on different threads. This can lead to calling notifyARefIsZero twice with each
    // instance thinking there are no more refs left and both trying to delete the object.
    mutable SkMutex fUnrefMutex;
    mutable std::atomic<int32_t> fUsageRefCnt;
    mutable std::atomic<int32_t> fCommandBufferRefCnt;
    mutable std::atomic<int32_t> fCacheRefCnt;

    // Indicates that CommandBufferRefs should be rerouted to UsageRefs.
    const bool fCommandBufferRefsAsUsageRefs = false;

    // This is not ref'ed but internalDispose() will be called before the Gpu object is destroyed.
    // That call will set this to nullptr.
    const SharedContext* fSharedContext;

    const Ownership fOwnership;
    const UniqueID fUniqueID;

    // The resource key and return cache are both set at most once, during registerWithCache().
    /*const*/ GraphiteResourceKey  fKey;
    /*const*/ sk_sp<ResourceCache> fReturnCache;

    // An index into the return cache so we know whether or not the resource is already waiting to
    // be returned or not.
    mutable int fReturnIndex = -1;

    // The remaining fields are mutable state that is only modified by the ResourceCache on the
    // cache's thread, guarded by `fReturnCache::fSingleOwner`.

    size_t fGpuMemorySize = kInvalidGpuMemorySize;

    // All resources created internally by Graphite that are held in the ResourceCache as shared or
    // available scratch resources are considered budgeted. Resources that back client-owned objects
    // (e.g. SkSurface or SkImage) and wrapper objects (e.g. BackendTexture) do not count against
    // cache limits and therefore should never be budgeted.
    Budgeted fBudgeted = Budgeted::kNo;
    // All resources start out as non-shareable (the strictest mode) and revert to non-shareable
    // when they are returned to the cache and have no more usage refs. An available resource can
    // be returned if its shareable type matches the request, or if it was non-shareable at which
    // point the resource is upgraded to the more permissive mode (until all shared usages are
    // dropped at which point it can be used for any purpose again).
    Shareable fShareable = Shareable::kNo;

    // This is only used by ProxyCache::purgeProxiesNotUsedSince which is called from
    // ResourceCache::purgeResourcesNotUsedSince. When kYes, this signals that the Resource
    // should've been purged based on its timestamp at some point regardless of what its
    // current timestamp may indicate (since the timestamp will be updated when the Resource
    // is returned to the ResourceCache).
    DeleteASAP fDeleteASAP = DeleteASAP::kNo;

    // Set to true when the resource is contained in its cache's `fResourceMap`, which allows it to
    // be returned from findAndRefResource().
    bool fAvailableForReuse = false;

    // An index into a heap when this resource is purgeable or an array when not. This is maintained
    // by the cache. Must be mutable to fit SkTDPQueue's access API.
    mutable int fCacheArrayIndex = -1;

    // This value reflects how recently this resource was accessed in the cache. This is maintained
    // by the cache. It defines a total order over resources, even if their fLastAccess times are
    // the same (i.e. returned at time points less than the system's granularity).
    uint32_t fLastUseToken;
    skgpu::StdSteadyClock::time_point fLastAccess;

    // String used to describe the current use of this Resource.
    std::string fLabel;

    SkDEBUGCODE(mutable bool fCalledRemovedFromCache = false;)
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_Resource_DEFINED
