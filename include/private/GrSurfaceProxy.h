/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSurfaceProxy_DEFINED
#define GrSurfaceProxy_DEFINED

#include "include/core/SkRect.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrGpuResource.h"
#include "include/gpu/GrSurface.h"
#include "include/gpu/GrTexture.h"
#include "include/private/SkNoncopyable.h"

class GrCaps;
class GrContext_Base;
class GrOpList;
class GrRecordingContext;
class GrRenderTargetOpList;
class GrRenderTargetProxy;
class GrResourceProvider;
class GrSurfaceContext;
class GrSurfaceProxyPriv;
class GrTextureOpList;
class GrTextureProxy;

// This class replicates the functionality GrIORef<GrSurface> but tracks the
// utilitization for later resource allocation (for the deferred case) and
// forwards on the utilization in the wrapped case
class GrIORefProxy : public SkNoncopyable {
public:
    void ref() const {
        this->validate();

        ++fRefCnt;
        if (fTarget) {
            fTarget->ref();
        }
    }

    void unref() const {
        this->validate();

        if (fTarget) {
            fTarget->unref();
        }

        --fRefCnt;
        this->didRemoveRefOrPendingIO();
    }

#ifdef SK_DEBUG
    bool isUnique_debugOnly() const { // For asserts.
        SkASSERT(fRefCnt >= 0 && fPendingWrites >= 0 && fPendingReads >= 0);
        return 1 == fRefCnt + fPendingWrites + fPendingReads;
    }
#endif

    void release() {
        // The proxy itself may still have multiple refs. It can be owned by an SkImage and multiple
        // SkDeferredDisplayLists at the same time if we are using DDLs.
        SkASSERT(0 == fPendingReads);
        SkASSERT(0 == fPendingWrites);

        // In the current hybrid world, the proxy and backing surface are ref/unreffed in
        // synchrony. Each ref we've added or removed to the proxy was mirrored to the backing
        // surface. Though, that backing surface could be owned by other proxies as well. Remove
        // a ref from the backing surface for each ref the proxy has since we are about to remove
        // our pointer to the surface. If this proxy is reinstantiated then all the proxy's refs
        // get transferred to the (possibly new) backing surface.
        for (int refs = fRefCnt; refs; --refs) {
            fTarget->unref();
        }
        fTarget = nullptr;
    }

    void validate() const {
#ifdef SK_DEBUG
        SkASSERT(fRefCnt >= 0);
        SkASSERT(fPendingReads >= 0);
        SkASSERT(fPendingWrites >= 0);
        SkASSERT(fRefCnt + fPendingReads + fPendingWrites >= 1);

        if (fTarget) {
            // The backing GrSurface can have more refs than the proxy if the proxy
            // started off wrapping an external resource (that came in with refs).
            // The GrSurface should never have fewer refs than the proxy however.
            SkASSERT(fTarget->fRefCnt >= fRefCnt);
            SkASSERT(fTarget->fPendingReads >= fPendingReads);
            SkASSERT(fTarget->fPendingWrites >= fPendingWrites);
        }
#endif
    }

    int32_t getBackingRefCnt_TestOnly() const;
    int32_t getPendingReadCnt_TestOnly() const;
    int32_t getPendingWriteCnt_TestOnly() const;

    void addPendingRead() const {
        this->validate();

        ++fPendingReads;
        if (fTarget) {
            fTarget->addPendingRead();
        }
    }

    void completedRead() const {
        this->validate();

        if (fTarget) {
            fTarget->completedRead();
        }

        --fPendingReads;
        this->didRemoveRefOrPendingIO();
    }

    void addPendingWrite() const {
        this->validate();

        ++fPendingWrites;
        if (fTarget) {
            fTarget->addPendingWrite();
        }
    }

    void completedWrite() const {
        this->validate();

        if (fTarget) {
            fTarget->completedWrite();
        }

        --fPendingWrites;
        this->didRemoveRefOrPendingIO();
    }

protected:
    GrIORefProxy() : fTarget(nullptr), fRefCnt(1), fPendingReads(0), fPendingWrites(0) {}
    GrIORefProxy(sk_sp<GrSurface> surface) : fRefCnt(1), fPendingReads(0), fPendingWrites(0) {
        // Since we're manually forwarding on refs & unrefs we don't want sk_sp doing
        // anything extra.
        fTarget = surface.release();
    }
    virtual ~GrIORefProxy() {
        // We don't unref 'fTarget' here since the 'unref' method will already
        // have forwarded on the unref call that got us here.
    }

    // Privileged method that allows going from ref count = 0 to ref count = 1.
    void addInitialRef(GrResourceCache* cache) const {
        this->validate();
        ++fRefCnt;
        if (fTarget) {
            fTarget->proxyAccess().ref(cache);
        }
    }

    // This GrIORefProxy was deferred before but has just been instantiated. To
    // make all the reffing & unreffing work out we now need to transfer any deferred
    // refs & unrefs to the new GrSurface
    void transferRefs() {
        SkASSERT(fTarget);
        // Make sure we're going to take some ownership of our target.
        SkASSERT(fRefCnt > 0 || fPendingReads > 0 || fPendingWrites > 0);

        // Transfer pending read/writes first so that if we decrement the target's ref cnt we don't
        // cause a purge of the target.
        fTarget->fPendingReads += fPendingReads;
        fTarget->fPendingWrites += fPendingWrites;
        SkASSERT(fTarget->fRefCnt > 0);
        SkASSERT(fRefCnt >= 0);
        // Don't xfer the proxy's creation ref. If we're going to subtract a ref do it via unref()
        // so that proper cache notifications occur.
        if (!fRefCnt) {
            fTarget->unref();
        } else {
            fTarget->fRefCnt += (fRefCnt - 1);
        }
    }

    int32_t internalGetProxyRefCnt() const { return fRefCnt; }
    int32_t internalGetTotalRefs() const { return fRefCnt + fPendingReads + fPendingWrites; }

    // For deferred proxies this will be null. For wrapped proxies it will point to the
    // wrapped resource.
    GrSurface* fTarget;

private:
    // This class is used to manage conversion of refs to pending reads/writes.
    template <typename> friend class GrProxyRef;

    void didRemoveRefOrPendingIO() const {
        if (0 == fPendingReads && 0 == fPendingWrites && 0 == fRefCnt) {
            delete this;
        }
    }

    mutable int32_t fRefCnt;
    mutable int32_t fPendingReads;
    mutable int32_t fPendingWrites;
};

class GrSurfaceProxy : public GrIORefProxy {
public:
    /**
     * Some lazy proxy callbacks want to set their own (or no key) on the GrSurfaces they return.
     * Others want the GrSurface's key to be kept in sync with the proxy's key. This enum controls
     * the key relationship between proxies and their targets.
     */
    enum class LazyInstantiationKeyMode {
        /**
         * Don't key the GrSurface with the proxy's key. The lazy instantiation callback is free to
         * return a GrSurface that already has a unique key unrelated to the proxy's key.
         */
        kUnsynced,
        /**
         * Keep the GrSurface's unique key in sync with the proxy's unique key. The GrSurface
         * returned from the lazy instantiation callback must not have a unique key or have the same
         * same unique key as the proxy. If the proxy is later assigned a key it is in turn assigned
         * to the GrSurface.
         */
        kSynced
    };

    struct LazyInstantiationResult {
        LazyInstantiationResult() = default;
        LazyInstantiationResult(const LazyInstantiationResult&) = default;
        LazyInstantiationResult(LazyInstantiationResult&& that) = default;
        LazyInstantiationResult(sk_sp<GrSurface> surf,
                                LazyInstantiationKeyMode mode = LazyInstantiationKeyMode::kSynced)
                : fSurface(std::move(surf)), fKeyMode(mode) {}
        LazyInstantiationResult(sk_sp<GrTexture> tex)
                : LazyInstantiationResult(sk_sp<GrSurface>(std::move(tex))) {}

        LazyInstantiationResult& operator=(const LazyInstantiationResult&) = default;
        LazyInstantiationResult& operator=(LazyInstantiationResult&&) = default;

        sk_sp<GrSurface> fSurface;
        LazyInstantiationKeyMode fKeyMode = LazyInstantiationKeyMode::kSynced;
    };

    using LazyInstantiateCallback = std::function<LazyInstantiationResult(GrResourceProvider*)>;

    enum class LazyInstantiationType {
        kSingleUse,      // Instantiation callback is allowed to be called only once.
        kMultipleUse,    // Instantiation callback can be called multiple times.
        kDeinstantiate,  // Instantiation callback can be called multiple times,
                         // but we will deinstantiate the proxy after every flush.
    };

    enum class LazyState {
        kNot,       // The proxy is instantiated or does not have a lazy callback
        kPartially, // The proxy has a lazy callback but knows basic information about itself.
        kFully,     // The proxy has a lazy callback and also doesn't know its width, height, etc.
    };

    LazyState lazyInstantiationState() const {
        if (fTarget || !SkToBool(fLazyInstantiateCallback)) {
            return LazyState::kNot;
        } else {
            if (fWidth <= 0) {
                SkASSERT(fHeight <= 0);
                return LazyState::kFully;
            } else {
                SkASSERT(fHeight > 0);
                return LazyState::kPartially;
            }
        }
    }

    GrPixelConfig config() const { return fConfig; }
    int width() const {
        SkASSERT(LazyState::kFully != this->lazyInstantiationState());
        return fWidth;
    }
    int height() const {
        SkASSERT(LazyState::kFully != this->lazyInstantiationState());
        return fHeight;
    }

    SkISize isize() const { return {fWidth, fHeight}; }

    int worstCaseWidth() const;
    int worstCaseHeight() const;
    /**
     * Helper that gets the width and height of the surface as a bounding rectangle.
     */
    SkRect getBoundsRect() const {
        SkASSERT(LazyState::kFully != this->lazyInstantiationState());
        return SkRect::MakeIWH(this->width(), this->height());
    }
    /**
     * Helper that gets the worst case width and height of the surface as a bounding rectangle.
     */
    SkRect getWorstCaseBoundsRect() const {
        SkASSERT(LazyState::kFully != this->lazyInstantiationState());
        return SkRect::MakeIWH(this->worstCaseWidth(), this->worstCaseHeight());
    }

    GrSurfaceOrigin origin() const {
        SkASSERT(kTopLeft_GrSurfaceOrigin == fOrigin || kBottomLeft_GrSurfaceOrigin == fOrigin);
        return fOrigin;
    }

    const GrBackendFormat& backendFormat() const { return fFormat; }

    class UniqueID {
    public:
        static UniqueID InvalidID() {
            return UniqueID(uint32_t(SK_InvalidUniqueID));
        }

        // wrapped
        explicit UniqueID(const GrGpuResource::UniqueID& id) : fID(id.asUInt()) { }
        // deferred and lazy-callback
        UniqueID() : fID(GrGpuResource::CreateUniqueID()) { }

        uint32_t asUInt() const { return fID; }

        bool operator==(const UniqueID& other) const {
            return fID == other.fID;
        }
        bool operator!=(const UniqueID& other) const {
            return !(*this == other);
        }

        void makeInvalid() { fID = SK_InvalidUniqueID; }
        bool isInvalid() const { return SK_InvalidUniqueID == fID; }

    private:
        explicit UniqueID(uint32_t id) : fID(id) {}

        uint32_t fID;
    };

    /*
     * The contract for the uniqueID is:
     *   for wrapped resources:
     *      the uniqueID will match that of the wrapped resource
     *
     *   for deferred resources:
     *      the uniqueID will be different from the real resource, when it is allocated
     *      the proxy's uniqueID will not change across the instantiate call
     *
     *    the uniqueIDs of the proxies and the resources draw from the same pool
     *
     * What this boils down to is that the uniqueID of a proxy can be used to consistently
     * track/identify a proxy but should never be used to distinguish between
     * resources and proxies - beware!
     */
    UniqueID uniqueID() const { return fUniqueID; }

    UniqueID underlyingUniqueID() const {
        if (fTarget) {
            return UniqueID(fTarget->uniqueID());
        }

        return fUniqueID;
    }

    virtual bool instantiate(GrResourceProvider*) = 0;

    void deinstantiate();

    /**
     * Proxies that are already instantiated and whose backing surface cannot be recycled to
     * instantiate other proxies do not need to be considered by GrResourceAllocator.
     */
    bool canSkipResourceAllocator() const;

    /**
     * @return the texture proxy associated with the surface proxy, may be NULL.
     */
    virtual GrTextureProxy* asTextureProxy() { return nullptr; }
    virtual const GrTextureProxy* asTextureProxy() const { return nullptr; }

    /**
     * @return the render target proxy associated with the surface proxy, may be NULL.
     */
    virtual GrRenderTargetProxy* asRenderTargetProxy() { return nullptr; }
    virtual const GrRenderTargetProxy* asRenderTargetProxy() const { return nullptr; }

    bool isInstantiated() const { return SkToBool(fTarget); }

    // If the proxy is already instantiated, return its backing GrTexture; if not, return null.
    GrSurface* peekSurface() const { return fTarget; }

    // If this is a texture proxy and the proxy is already instantiated, return its backing
    // GrTexture; if not, return null.
    GrTexture* peekTexture() const { return fTarget ? fTarget->asTexture() : nullptr; }

    // If this is a render target proxy and the proxy is already instantiated, return its backing
    // GrRenderTarget; if not, return null.
    GrRenderTarget* peekRenderTarget() const {
        return fTarget ? fTarget->asRenderTarget() : nullptr;
    }

    /**
     * Does the resource count against the resource budget?
     */
    SkBudgeted isBudgeted() const { return fBudgeted; }

    /**
     * The pixel values of this proxy's surface cannot be modified (e.g. doesn't support write
     * pixels or MIP map level regen). Read-only proxies also bypass interval tracking and
     * assignment in GrResourceAllocator.
     */
    bool readOnly() const { return fSurfaceFlags & GrInternalSurfaceFlags::kReadOnly; }

    void setLastOpList(GrOpList* opList);
    GrOpList* getLastOpList() { return fLastOpList; }

    GrRenderTargetOpList* getLastRenderTargetOpList();
    GrTextureOpList* getLastTextureOpList();

    /**
     * Retrieves the amount of GPU memory that will be or currently is used by this resource
     * in bytes. It is approximate since we aren't aware of additional padding or copies made
     * by the driver.
     *
     * @return the amount of GPU memory used in bytes
     */
    size_t gpuMemorySize() const {
        SkASSERT(LazyState::kFully != this->lazyInstantiationState());
        if (fTarget) {
            return fTarget->gpuMemorySize();
        }
        if (kInvalidGpuMemorySize == fGpuMemorySize) {
            fGpuMemorySize = this->onUninstantiatedGpuMemorySize();
            SkASSERT(kInvalidGpuMemorySize != fGpuMemorySize);
        }
        return fGpuMemorySize;
    }

    // Helper function that creates a temporary SurfaceContext to perform the copy
    // The copy is is not a render target and not multisampled.
    static sk_sp<GrTextureProxy> Copy(GrRecordingContext*, GrSurfaceProxy* src, GrMipMapped,
                                      SkIRect srcRect, SkBackingFit, SkBudgeted);

    // Copy the entire 'src'
    static sk_sp<GrTextureProxy> Copy(GrRecordingContext*, GrSurfaceProxy* src, GrMipMapped,
                                      SkBackingFit, SkBudgeted);

    // Test-only entry point - should decrease in use as proxies propagate
    static sk_sp<GrSurfaceContext> TestCopy(GrRecordingContext* context,
                                            const GrSurfaceDesc& dstDesc,
                                            GrSurfaceOrigin, GrSurfaceProxy* srcProxy);

    bool isWrapped_ForTesting() const;

    SkDEBUGCODE(void validate(GrContext_Base*) const;)

    // Provides access to functions that aren't part of the public API.
    inline GrSurfaceProxyPriv priv();
    inline const GrSurfaceProxyPriv priv() const;

    /**
     * Provides privileged access to select callers to be able to add a ref to a GrSurfaceProxy
     * with zero refs.
     */
    class FirstRefAccess;
    inline FirstRefAccess firstRefAccess();

    GrInternalSurfaceFlags testingOnly_getFlags() const;

protected:
    // Deferred version
    GrSurfaceProxy(const GrBackendFormat& format, const GrSurfaceDesc& desc,
                   GrSurfaceOrigin origin, SkBackingFit fit,
                   SkBudgeted budgeted, GrInternalSurfaceFlags surfaceFlags)
            : GrSurfaceProxy(nullptr, LazyInstantiationType::kSingleUse, format, desc, origin, fit,
                             budgeted, surfaceFlags) {
        // Note: this ctor pulls a new uniqueID from the same pool at the GrGpuResources
    }

    // Lazy-callback version
    GrSurfaceProxy(LazyInstantiateCallback&&, LazyInstantiationType,
                   const GrBackendFormat& format, const GrSurfaceDesc&, GrSurfaceOrigin,
                   SkBackingFit, SkBudgeted, GrInternalSurfaceFlags);

    // Wrapped version.
    GrSurfaceProxy(sk_sp<GrSurface>, GrSurfaceOrigin, SkBackingFit);

    virtual ~GrSurfaceProxy();

    friend class GrSurfaceProxyPriv;

    // Methods made available via GrSurfaceProxyPriv
    bool ignoredByResourceAllocator() const { return fIgnoredByResourceAllocator; }
    void setIgnoredByResourceAllocator() { fIgnoredByResourceAllocator = true; }

    int32_t getProxyRefCnt() const { return this->internalGetProxyRefCnt(); }
    int32_t getTotalRefs() const { return this->internalGetTotalRefs(); }

    void computeScratchKey(GrScratchKey*) const;

    virtual sk_sp<GrSurface> createSurface(GrResourceProvider*) const = 0;
    void assign(sk_sp<GrSurface> surface);

    sk_sp<GrSurface> createSurfaceImpl(GrResourceProvider*, int sampleCnt, bool needsStencil,
                                       GrSurfaceDescFlags, GrMipMapped) const;

    // Once the size of a fully-lazy proxy is decided, and before it gets instantiated, the client
    // can use this optional method to specify the proxy's size. (A proxy's size can be less than
    // the GPU surface that backs it. e.g., SkBackingFit::kApprox.) Otherwise, the proxy's size will
    // be set to match the underlying GPU surface upon instantiation.
    void setLazySize(int width, int height) {
        SkASSERT(GrSurfaceProxy::LazyState::kFully == this->lazyInstantiationState());
        SkASSERT(width > 0 && height > 0);
        fWidth = width;
        fHeight = height;
    }

    bool instantiateImpl(GrResourceProvider* resourceProvider, int sampleCnt, bool needsStencil,
                         GrSurfaceDescFlags descFlags, GrMipMapped, const GrUniqueKey*);

    // In many cases these flags aren't actually known until the proxy has been instantiated.
    // However, Ganesh frequently needs to change its behavior based on these settings. For
    // internally create proxies we will know these properties ahead of time. For wrapped
    // proxies we will copy the properties off of the GrSurface. For lazy proxies we force the
    // call sites to provide the required information ahead of time. At instantiation time
    // we verify that the assumed properties match the actual properties.
    GrInternalSurfaceFlags fSurfaceFlags;

private:
    // For wrapped resources, 'fFormat', 'fConfig', 'fWidth', 'fHeight', and 'fOrigin; will always
    // be filled in from the wrapped resource.
    GrBackendFormat        fFormat;
    GrPixelConfig          fConfig;
    int                    fWidth;
    int                    fHeight;
    GrSurfaceOrigin        fOrigin;
    SkBackingFit           fFit;      // always kApprox for lazy-callback resources
                                      // always kExact for wrapped resources
    mutable SkBudgeted     fBudgeted; // always kYes for lazy-callback resources
                                      // set from the backing resource for wrapped resources
                                      // mutable bc of SkSurface/SkImage wishy-washiness

    const UniqueID         fUniqueID; // set from the backing resource for wrapped resources

    LazyInstantiateCallback fLazyInstantiateCallback;
    // If this is set to kSingleuse, then after one call to fLazyInstantiateCallback we will cleanup
    // the lazy callback and then delete it. This will allow for any refs and resources being held
    // by the standard function to be released. This is specifically useful in non-dll cases where
    // we make lazy proxies and instantiate them immediately.
    // Note: This is ignored if fLazyInstantiateCallback is null.
    LazyInstantiationType  fLazyInstantiationType;

    SkDEBUGCODE(void validateSurface(const GrSurface*);)
    SkDEBUGCODE(virtual void onValidateSurface(const GrSurface*) = 0;)

    static const size_t kInvalidGpuMemorySize = ~static_cast<size_t>(0);
    SkDEBUGCODE(size_t getRawGpuMemorySize_debugOnly() const { return fGpuMemorySize; })

    virtual size_t onUninstantiatedGpuMemorySize() const = 0;

    bool                   fNeedsClear;
    bool                   fIgnoredByResourceAllocator = false;

    // This entry is lazily evaluated so, when the proxy wraps a resource, the resource
    // will be called but, when the proxy is deferred, it will compute the answer itself.
    // If the proxy computes its own answer that answer is checked (in debug mode) in
    // the instantiation method.
    mutable size_t         fGpuMemorySize;

    // The last opList that wrote to or is currently going to write to this surface
    // The opList can be closed (e.g., no surface context is currently bound
    // to this proxy).
    // This back-pointer is required so that we can add a dependancy between
    // the opList used to create the current contents of this surface
    // and the opList of a destination surface to which this one is being drawn or copied.
    // This pointer is unreffed. OpLists own a ref on their surface proxies.
    GrOpList*              fLastOpList;

    typedef GrIORefProxy INHERITED;
};

class GrSurfaceProxy::FirstRefAccess {
private:
    void ref(GrResourceCache* cache) { fProxy->addInitialRef(cache); }

    FirstRefAccess(GrSurfaceProxy* proxy) : fProxy(proxy) {}

    // No taking addresses of this type.
    const FirstRefAccess* operator&() const = delete;
    FirstRefAccess* operator&() = delete;

    GrSurfaceProxy* fProxy;

    friend class GrSurfaceProxy;
    friend class GrProxyProvider;
    friend class GrDeinstantiateProxyTracker;
};

inline GrSurfaceProxy::FirstRefAccess GrSurfaceProxy::firstRefAccess() {
    return FirstRefAccess(this);
}

#endif
