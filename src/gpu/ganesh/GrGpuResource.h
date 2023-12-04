/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGpuResource_DEFINED
#define GrGpuResource_DEFINED

#include "include/gpu/GpuTypes.h"
#include "include/private/base/SkNoncopyable.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/GpuTypesPriv.h"
#include "src/gpu/ResourceKey.h"

class GrDirectContext;
class GrGpu;
class GrResourceCache;
class SkTraceMemoryDump;

#if defined(GR_TEST_UTILS)
class GrSurface;
#endif

/**
 * Base class for GrGpuResource. Provides the hooks for resources to interact with the cache.
 * Separated out as a base class to isolate the ref-cnting behavior and provide friendship without
 * exposing all of GrGpuResource.
 *
 * PRIOR to the last ref being removed DERIVED::notifyARefCntWillBeZero() will be called
 * (static poly morphism using CRTP). It is legal for additional ref's to be added
 * during this time. AFTER the ref count reaches zero DERIVED::notifyARefCntIsZero() will be
 * called.
 */
template <typename DERIVED> class GrIORef : public SkNoncopyable {
public:
    bool unique() const { return fRefCnt == 1; }

    void ref() const {
        // Only the cache should be able to add the first ref to a resource.
        SkASSERT(this->getRefCnt() > 0);
        // No barrier required.
        (void)fRefCnt.fetch_add(+1, std::memory_order_relaxed);
    }

    // This enum is used to notify the GrResourceCache which type of ref just dropped to zero.
    enum class LastRemovedRef {
        kMainRef,            // This refers to fRefCnt
        kCommandBufferUsage, // This refers to fCommandBufferUsageCnt
    };

    void unref() const {
        SkASSERT(this->getRefCnt() > 0);
        if (1 == fRefCnt.fetch_add(-1, std::memory_order_acq_rel)) {
            this->notifyWillBeZero(LastRemovedRef::kMainRef);
        }
    }

    void refCommandBuffer() const {
        // No barrier required.
        (void)fCommandBufferUsageCnt.fetch_add(+1, std::memory_order_relaxed);
    }

    void unrefCommandBuffer() const {
        SkASSERT(!this->hasNoCommandBufferUsages());
        if (1 == fCommandBufferUsageCnt.fetch_add(-1, std::memory_order_acq_rel)) {
            this->notifyWillBeZero(LastRemovedRef::kCommandBufferUsage);
        }
    }

#if defined(GR_TEST_UTILS)
    int32_t testingOnly_getRefCnt() const { return this->getRefCnt(); }
#endif

protected:
    GrIORef() : fRefCnt(1), fCommandBufferUsageCnt(0) {}

    bool internalHasRef() const { return SkToBool(this->getRefCnt()); }
    bool internalHasNoCommandBufferUsages() const {
        return SkToBool(this->hasNoCommandBufferUsages());
    }

    // Privileged method that allows going from ref count = 0 to ref count = 1.
    void addInitialRef() const {
        SkASSERT(fRefCnt >= 0);
        // No barrier required.
        (void)fRefCnt.fetch_add(+1, std::memory_order_relaxed);
    }

private:
    void notifyWillBeZero(LastRemovedRef removedRef) const {
        static_cast<const DERIVED*>(this)->notifyARefCntIsZero(removedRef);
    }

    int32_t getRefCnt() const { return fRefCnt.load(std::memory_order_relaxed); }

    bool hasNoCommandBufferUsages() const {
        if (0 == fCommandBufferUsageCnt.load(std::memory_order_acquire)) {
            // The acquire barrier is only really needed if we return true.  It
            // prevents code conditioned on the result of hasNoCommandBufferUsages() from running
            // until previous owners are all totally done calling removeCommandBufferUsage().
            return true;
        }
        return false;
    }

    mutable std::atomic<int32_t> fRefCnt;
    mutable std::atomic<int32_t> fCommandBufferUsageCnt;

    using INHERITED = SkNoncopyable;
};

/**
 * Base class for objects that can be kept in the GrResourceCache.
 */
class GrGpuResource : public GrIORef<GrGpuResource> {
public:
    /**
     * Tests whether a object has been abandoned or released. All objects will
     * be in this state after their creating GrContext is destroyed or has
     * contextLost called. It's up to the client to test wasDestroyed() before
     * attempting to use an object if it holds refs on objects across
     * ~GrContext, freeResources with the force flag, or contextLost.
     *
     * @return true if the object has been released or abandoned,
     *         false otherwise.
     */
    bool wasDestroyed() const { return nullptr == fGpu; }

    /**
     * Retrieves the context that owns the object. Note that it is possible for
     * this to return NULL. When objects have been release()ed or abandon()ed
     * they no longer have an owning context. Destroying a GrDirectContext
     * automatically releases all its resources.
     */
    const GrDirectContext* getContext() const;
    GrDirectContext* getContext();

    /**
     * Retrieves the amount of GPU memory used by this resource in bytes. It is
     * approximate since we aren't aware of additional padding or copies made
     * by the driver.
     *
     * @return the amount of GPU memory used in bytes
     */
    size_t gpuMemorySize() const {
        if (kInvalidGpuMemorySize == fGpuMemorySize) {
            fGpuMemorySize = this->onGpuMemorySize();
            SkASSERT(kInvalidGpuMemorySize != fGpuMemorySize);
        }
        return fGpuMemorySize;
    }

    class UniqueID {
    public:
        UniqueID() = default;

        explicit UniqueID(uint32_t id) : fID(id) {}

        uint32_t asUInt() const { return fID; }

        bool operator==(const UniqueID& other) const { return fID == other.fID; }
        bool operator!=(const UniqueID& other) const { return !(*this == other); }

        void makeInvalid() { fID = SK_InvalidUniqueID; }
        bool isInvalid() const { return  fID == SK_InvalidUniqueID; }

    protected:
        uint32_t fID = SK_InvalidUniqueID;
    };

    /**
     * Gets an id that is unique for this GrGpuResource object. It is static in that it does
     * not change when the content of the GrGpuResource object changes. This will never return
     * 0.
     */
    UniqueID uniqueID() const { return fUniqueID; }

    /** Returns the current unique key for the resource. It will be invalid if the resource has no
        associated unique key. */
    const skgpu::UniqueKey& getUniqueKey() const { return fUniqueKey; }

    std::string getLabel() const { return fLabel; }

    void setLabel(std::string_view label) {
        fLabel = label;
        this->onSetLabel();
    }

    /**
     * Internal-only helper class used for manipulations of the resource by the cache.
     */
    class CacheAccess;
    inline CacheAccess cacheAccess();
    inline const CacheAccess cacheAccess() const;  // NOLINT(readability-const-return-type)

    /**
     * Internal-only helper class used for manipulations of the resource by GrSurfaceProxy.
     */
    class ProxyAccess;
    inline ProxyAccess proxyAccess();

    /**
     * Internal-only helper class used for manipulations of the resource by internal code.
     */
    class ResourcePriv;
    inline ResourcePriv resourcePriv();
    inline const ResourcePriv resourcePriv() const;  // NOLINT(readability-const-return-type)

    /**
     * Dumps memory usage information for this GrGpuResource to traceMemoryDump.
     * Typically, subclasses should not need to override this, and should only
     * need to override setMemoryBacking.
     **/
    virtual void dumpMemoryStatistics(SkTraceMemoryDump* traceMemoryDump) const;

    /**
     * Describes the type of gpu resource that is represented by the implementing
     * class (e.g. texture, buffer object, stencil).  This data is used for diagnostic
     * purposes by dumpMemoryStatistics().
     *
     * The value returned is expected to be long lived and will not be copied by the caller.
     */
    virtual const char* getResourceType() const = 0;

    static uint32_t CreateUniqueID();

#if defined(GR_TEST_UTILS)
    virtual const GrSurface* asSurface() const { return nullptr; }
#endif

protected:
    // This must be called by every non-wrapped GrGpuObject. It should be called once the object is
    // fully initialized (i.e. only from the constructors of the final class).
    void registerWithCache(skgpu::Budgeted);

    // This must be called by every GrGpuObject that references any wrapped backend objects. It
    // should be called once the object is fully initialized (i.e. only from the constructors of the
    // final class).
    void registerWithCacheWrapped(GrWrapCacheable);

    GrGpuResource(GrGpu*, std::string_view label);
    virtual ~GrGpuResource();

    GrGpu* getGpu() const { return fGpu; }

    /** Overridden to free GPU resources in the backend API. */
    virtual void onRelease() { }
    /** Overridden to abandon any internal handles, ptrs, etc to backend API resources.
        This may be called when the underlying 3D context is no longer valid and so no
        backend API calls should be made. */
    virtual void onAbandon() { }

    /**
     * Allows subclasses to add additional backing information to the SkTraceMemoryDump.
     **/
    virtual void setMemoryBacking(SkTraceMemoryDump*, const SkString&) const {}

    /**
     * Returns a string that uniquely identifies this resource.
     */
    SkString getResourceName() const;

    /**
     * A helper for subclasses that override dumpMemoryStatistics(). This method using a format
     * consistent with the default implementation of dumpMemoryStatistics() but allows the caller
     * to customize various inputs.
     */
    void dumpMemoryStatisticsPriv(SkTraceMemoryDump* traceMemoryDump, const SkString& resourceName,
                                  const char* type, size_t size) const;


private:
    bool isPurgeable() const;
    bool hasRef() const;
    bool hasNoCommandBufferUsages() const;

    /**
     * Called by the registerWithCache if the resource is available to be used as scratch.
     * Resource subclasses should override this if the instances should be recycled as scratch
     * resources and populate the scratchKey with the key.
     * By default resources are not recycled as scratch.
     **/
    virtual void computeScratchKey(skgpu::ScratchKey*) const {}

    /**
     * Removes references to objects in the underlying 3D API without freeing them.
     * Called by CacheAccess.
     */
    void abandon();

    /**
     * Frees the object in the underlying 3D API. Called by CacheAccess.
     */
    void release();

    virtual size_t onGpuMemorySize() const = 0;

    virtual void onSetLabel() = 0;

    // See comments in CacheAccess and ResourcePriv.
    void setUniqueKey(const skgpu::UniqueKey&);
    void removeUniqueKey();
    void notifyARefCntIsZero(LastRemovedRef removedRef) const;
    void removeScratchKey();
    void makeBudgeted();
    void makeUnbudgeted();

#ifdef SK_DEBUG
    friend class GrGpu;  // for assert in GrGpu to access getGpu
#endif

    // An index into a heap when this resource is purgeable or an array when not. This is maintained
    // by the cache.
    int fCacheArrayIndex;
    // This value reflects how recently this resource was accessed in the cache. This is maintained
    // by the cache.
    uint32_t fTimestamp;
    skgpu::StdSteadyClock::time_point fTimeWhenBecamePurgeable;

    static const size_t kInvalidGpuMemorySize = ~static_cast<size_t>(0);
    skgpu::ScratchKey fScratchKey;
    skgpu::UniqueKey fUniqueKey;

    // This is not ref'ed but abandon() or release() will be called before the GrGpu object
    // is destroyed. Those calls will set this to NULL.
    GrGpu* fGpu;
    mutable size_t fGpuMemorySize = kInvalidGpuMemorySize;

    GrBudgetedType fBudgetedType = GrBudgetedType::kUnbudgetedUncacheable;
    bool fRefsWrappedObjects = false;
    const UniqueID fUniqueID;
    std::string fLabel;

    using INHERITED = GrIORef<GrGpuResource>;
    friend class GrIORef<GrGpuResource>; // to access notifyRefCntWillBeZero and
                                         // notifyARefCntIsZero.
};

class GrGpuResource::ProxyAccess {
private:
    ProxyAccess(GrGpuResource* resource) : fResource(resource) {}

    /** Proxies are allowed to take a resource from no refs to one ref. */
    void ref(GrResourceCache* cache);

    // No taking addresses of this type.
    const CacheAccess* operator&() const = delete;
    CacheAccess* operator&() = delete;

    GrGpuResource* fResource;

    friend class GrGpuResource;
    friend class GrSurfaceProxy;
};

inline GrGpuResource::ProxyAccess GrGpuResource::proxyAccess() { return ProxyAccess(this); }

#endif
