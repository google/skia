/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSurfaceProxy_DEFINED
#define GrSurfaceProxy_DEFINED

#include "GrGpuResource.h"
#include "GrSurface.h"

#include "SkRect.h"

class GrBackendTexture;
class GrCaps;
class GrOpList;
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

    int32_t getProxyRefCnt_TestOnly() const;
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
        // have forwarded on the unref call that got use here.
    }

    // This GrIORefProxy was deferred before but has just been instantiated. To
    // make all the reffing & unreffing work out we now need to transfer any deferred
    // refs & unrefs to the new GrSurface
    void transferRefs() {
        SkASSERT(fTarget);

        SkASSERT(fTarget->fRefCnt > 0);
        fTarget->fRefCnt += (fRefCnt-1); // don't xfer the proxy's creation ref
        fTarget->fPendingReads += fPendingReads;
        fTarget->fPendingWrites += fPendingWrites;
    }

    bool internalHasPendingIO() const {
        if (fTarget) {
            return fTarget->internalHasPendingIO();
        }

        return SkToBool(fPendingWrites | fPendingReads);
    }

    bool internalHasPendingWrite() const {
        if (fTarget) {
            return fTarget->internalHasPendingWrite();
        }

        return SkToBool(fPendingWrites);
    }

    // For deferred proxies this will be null. For wrapped proxies it will point to the
    // wrapped resource.
    GrSurface* fTarget;

private:
    // This class is used to manage conversion of refs to pending reads/writes.
    friend class GrSurfaceProxyRef;
    template <typename, GrIOType> friend class GrPendingIOResource;

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
    static sk_sp<GrSurfaceProxy> MakeWrapped(sk_sp<GrSurface>, GrSurfaceOrigin);
    static sk_sp<GrTextureProxy> MakeWrapped(sk_sp<GrTexture>, GrSurfaceOrigin);

    static sk_sp<GrTextureProxy> MakeDeferred(GrResourceProvider*,
                                              const GrSurfaceDesc&, SkBackingFit,
                                              SkBudgeted, uint32_t flags = 0);

    /**
     * Creates a proxy that will be mipmapped.
     *
     * @param desc          Description of the texture properties.
     * @param budgeted      Does the texture count against the resource cache budget?
     * @param texels        A contiguous array of mipmap levels
     * @param mipLevelCount The amount of elements in the texels array
     */
    static sk_sp<GrTextureProxy> MakeDeferredMipMap(GrResourceProvider*,
                                                    const GrSurfaceDesc& desc, SkBudgeted budgeted,
                                                    const GrMipLevel texels[], int mipLevelCount,
                                                    SkDestinationSurfaceColorMode mipColorMode =
                                                           SkDestinationSurfaceColorMode::kLegacy);

    /**
     * Like the call above but there are no texels to upload. A texture proxy is returned that
     * simply has space allocated for the mips. We will allocated the full amount of mip levels
     * based on the width and height in the GrSurfaceDesc.
     */
    static sk_sp<GrTextureProxy> MakeDeferredMipMap(GrResourceProvider*,
                                                    const GrSurfaceDesc& desc, SkBudgeted budgeted);


    // TODO: need to refine ownership semantics of 'srcData' if we're in completely
    // deferred mode
    static sk_sp<GrTextureProxy> MakeDeferred(GrResourceProvider*,
                                              const GrSurfaceDesc&, SkBudgeted,
                                              const void* srcData, size_t rowBytes);

    static sk_sp<GrTextureProxy> MakeWrappedBackend(GrContext*, GrBackendTexture&, GrSurfaceOrigin);

    using LazyInstantiateCallback = std::function<sk_sp<GrTexture>(GrResourceProvider*,
                                                                   GrSurfaceOrigin* outOrigin)>;

    enum class Renderable : bool {
        kNo = false,
        kYes = true
    };

    /**
     * Creates a texture proxy that will be instantiated by a user-supplied callback during flush.
     * (Mipmapping, MSAA, and stencil are not supported by this method.)
     */
    static sk_sp<GrTextureProxy> MakeLazy(LazyInstantiateCallback&&, Renderable, GrPixelConfig);

    GrPixelConfig config() const { return fConfig; }
    int width() const { SkASSERT(!this->isPendingLazyInstantiation()); return fWidth; }
    int height() const { SkASSERT(!this->isPendingLazyInstantiation()); return fHeight; }
    int worstCaseWidth() const;
    int worstCaseHeight() const;
    GrSurfaceOrigin origin() const {
        SkASSERT(!this->isPendingLazyInstantiation());
        SkASSERT(kTopLeft_GrSurfaceOrigin == fOrigin || kBottomLeft_GrSurfaceOrigin == fOrigin);
        return fOrigin;
    }

    // If the client gave us a LazyInstantiateCallback (via MakeLazy), then we will invoke that
    // callback during flush. fWidth, fHeight, and fOrigin will be undefined until that time.
    bool isPendingLazyInstantiation() const { return SkToBool(fLazyInstantiateCallback); }

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

    virtual bool instantiate(GrResourceProvider* resourceProvider) = 0;

    /**
     * Helper that gets the width and height of the surface as a bounding rectangle.
     */
    SkRect getBoundsRect() const {
        SkASSERT(!this->isPendingLazyInstantiation());
        return SkRect::MakeIWH(this->width(), this->height());
    }

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

    /**
     * Does the resource count against the resource budget?
     */
    SkBudgeted isBudgeted() const { return fBudgeted; }

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
        SkASSERT(!this->isPendingLazyInstantiation());
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
    // It always returns a kExact-backed proxy bc it is used when converting an SkSpecialImage
    // to an SkImage. The copy is is not a render target and not multisampled.
    static sk_sp<GrTextureProxy> Copy(GrContext*, GrSurfaceProxy* src, GrMipMapped,
                                      SkIRect srcRect, SkBudgeted);

    // Copy the entire 'src'
    // It always returns a kExact-backed proxy bc it is used in SkGpuDevice::snapSpecial
    static sk_sp<GrTextureProxy> Copy(GrContext* context, GrSurfaceProxy* src, GrMipMapped,
                                      SkBudgeted budgeted);

    // Test-only entry point - should decrease in use as proxies propagate
    static sk_sp<GrSurfaceContext> TestCopy(GrContext* context, const GrSurfaceDesc& dstDesc,
                                            GrSurfaceProxy* srcProxy);

    bool isWrapped_ForTesting() const;

    SkDEBUGCODE(void validate(GrContext*) const;)

    // Provides access to functions that aren't part of the public API.
    inline GrSurfaceProxyPriv priv();
    inline const GrSurfaceProxyPriv priv() const;

protected:
    // Deferred version
    GrSurfaceProxy(const GrSurfaceDesc& desc, SkBackingFit fit, SkBudgeted budgeted, uint32_t flags)
            : fConfig(desc.fConfig)
            , fWidth(desc.fWidth)
            , fHeight(desc.fHeight)
            , fOrigin(desc.fOrigin)
            , fFit(fit)
            , fBudgeted(budgeted)
            , fFlags(flags)
            , fNeedsClear(SkToBool(desc.fFlags & kPerformInitialClear_GrSurfaceFlag))
            , fGpuMemorySize(kInvalidGpuMemorySize)
            , fLastOpList(nullptr) {
        // Note: this ctor pulls a new uniqueID from the same pool at the GrGpuResources
    }

    // Lazy-callback version
    GrSurfaceProxy(LazyInstantiateCallback&& callback, GrPixelConfig config);

    // Wrapped version
    GrSurfaceProxy(sk_sp<GrSurface> surface, GrSurfaceOrigin origin, SkBackingFit fit);

    virtual ~GrSurfaceProxy();

    friend class GrSurfaceProxyPriv;

    // Methods made available via GrSurfaceProxyPriv
    bool hasPendingIO() const {
        return this->internalHasPendingIO();
    }

    bool hasPendingWrite() const {
        return this->internalHasPendingWrite();
    }

    void computeScratchKey(GrScratchKey*) const;

    virtual sk_sp<GrSurface> createSurface(GrResourceProvider*) const = 0;
    void assign(sk_sp<GrSurface> surface);

    sk_sp<GrSurface> createSurfaceImpl(GrResourceProvider*, int sampleCnt, bool needsStencil,
                                       GrSurfaceFlags flags, GrMipMapped mipMapped,
                                       SkDestinationSurfaceColorMode mipColorMode) const;

    bool instantiateImpl(GrResourceProvider* resourceProvider, int sampleCnt, bool needsStencil,
                         GrSurfaceFlags flags, GrMipMapped mipMapped,
                         SkDestinationSurfaceColorMode mipColorMode, const GrUniqueKey*);

private:
    // For wrapped resources, 'fConfig', 'fWidth', 'fHeight', and 'fOrigin; will always be filled in
    // from the wrapped resource.
    GrPixelConfig        fConfig;
    int                  fWidth;
    int                  fHeight;
    GrSurfaceOrigin      fOrigin;
    SkBackingFit         fFit;      // always kApprox for lazy-callback resources
                                    // always kExact for wrapped resources
    mutable SkBudgeted   fBudgeted; // always kYes for lazy-callback resources
                                    // set from the backing resource for wrapped resources
                                    // mutable bc of SkSurface/SkImage wishy-washiness
    const uint32_t       fFlags;

    const UniqueID       fUniqueID; // set from the backing resource for wrapped resources

    LazyInstantiateCallback fLazyInstantiateCallback;
    SkDEBUGCODE(virtual void validateLazyTexture(const GrTexture*) = 0;)

    static const size_t kInvalidGpuMemorySize = ~static_cast<size_t>(0);
    SkDEBUGCODE(size_t getRawGpuMemorySize_debugOnly() const { return fGpuMemorySize; })

    virtual size_t onUninstantiatedGpuMemorySize() const = 0;

    bool                 fNeedsClear;

    // This entry is lazily evaluated so, when the proxy wraps a resource, the resource
    // will be called but, when the proxy is deferred, it will compute the answer itself.
    // If the proxy computes its own answer that answer is checked (in debug mode) in
    // the instantiation method.
    mutable size_t      fGpuMemorySize;

    // The last opList that wrote to or is currently going to write to this surface
    // The opList can be closed (e.g., no surface context is currently bound
    // to this proxy).
    // This back-pointer is required so that we can add a dependancy between
    // the opList used to create the current contents of this surface
    // and the opList of a destination surface to which this one is being drawn or copied.
    // This pointer is unreffed. OpLists own a ref on their surface proxies.
    GrOpList* fLastOpList;

    typedef GrIORefProxy INHERITED;
};

#endif
