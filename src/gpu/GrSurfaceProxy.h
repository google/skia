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
#include "src/gpu/GrNonAtomicRef.h"
#include "src/gpu/GrSwizzle.h"

class GrCaps;
class GrContext_Base;
class GrOpsTask;
class GrRecordingContext;
class GrRenderTargetProxy;
class GrRenderTask;
class GrResourceProvider;
class GrSurfaceContext;
class GrSurfaceProxyPriv;
class GrTextureProxy;

class GrSurfaceProxy : public GrNonAtomicRef<GrSurfaceProxy> {
public:
    virtual ~GrSurfaceProxy();

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
        if (this->isInstantiated() || !SkToBool(fLazyInstantiateCallback)) {
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

    const GrSwizzle& textureSwizzle() const { return fTextureSwizzle; }

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
    GrSurface* peekSurface() const { return fTarget.get(); }

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

    /**
     * This means surface is a multisampled render target, and internally holds a non-msaa texture
     * for resolving into. The render target resolves itself by blitting into this internal texture.
     * (asTexture() might or might not return the internal texture, but if it does, we always
     * resolve the render target before accessing this texture's data.)
     */
    bool requiresManualMSAAResolve() const {
        return fSurfaceFlags & GrInternalSurfaceFlags::kRequiresManualMSAAResolve;
    }

    void setLastRenderTask(GrRenderTask*);
    GrRenderTask* getLastRenderTask() { return fLastRenderTask; }

    GrOpsTask* getLastOpsTask();

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

    enum class RectsMustMatch : bool {
        kNo = false,
        kYes = true
    };

    // Helper function that creates a temporary SurfaceContext to perform the copy
    // The copy is is not a render target and not multisampled.
    static sk_sp<GrTextureProxy> Copy(GrRecordingContext*, GrSurfaceProxy* src, GrMipMapped,
                                      SkIRect srcRect, SkBackingFit, SkBudgeted,
                                      RectsMustMatch = RectsMustMatch::kNo);

    // Copy the entire 'src'
    static sk_sp<GrTextureProxy> Copy(GrRecordingContext*, GrSurfaceProxy* src, GrMipMapped,
                                      SkBackingFit, SkBudgeted);

#if GR_TEST_UTILS
    int32_t testingOnly_getBackingRefCnt() const;
    GrInternalSurfaceFlags testingOnly_getFlags() const;
#endif

    SkDEBUGCODE(void validate(GrContext_Base*) const;)

    // Provides access to functions that aren't part of the public API.
    inline GrSurfaceProxyPriv priv();
    inline const GrSurfaceProxyPriv priv() const;

    // Returns true if we are working with protected content.
    bool isProtected() const { return fIsProtected == GrProtected::kYes; }

protected:
    // Deferred version
    GrSurfaceProxy(const GrBackendFormat& format, const GrSurfaceDesc& desc,
                   GrRenderable renderable, GrSurfaceOrigin origin, const GrSwizzle& textureSwizzle,
                   SkBackingFit fit, SkBudgeted budgeted, GrProtected isProtected,
                   GrInternalSurfaceFlags surfaceFlags)
            : GrSurfaceProxy(nullptr, LazyInstantiationType::kSingleUse, format, desc, renderable,
                             origin, textureSwizzle, fit, budgeted, isProtected, surfaceFlags) {
        // Note: this ctor pulls a new uniqueID from the same pool at the GrGpuResources
    }

    // Lazy-callback version
    GrSurfaceProxy(LazyInstantiateCallback&&, LazyInstantiationType, const GrBackendFormat& format,
                   const GrSurfaceDesc&, GrRenderable, GrSurfaceOrigin,
                   const GrSwizzle& textureSwizzle, SkBackingFit, SkBudgeted, GrProtected,
                   GrInternalSurfaceFlags);

    // Wrapped version.
    GrSurfaceProxy(sk_sp<GrSurface>, GrSurfaceOrigin, const GrSwizzle& textureSwizzle,
                   SkBackingFit);

    friend class GrSurfaceProxyPriv;

    // Methods made available via GrSurfaceProxyPriv
    bool ignoredByResourceAllocator() const { return fIgnoredByResourceAllocator; }
    void setIgnoredByResourceAllocator() { fIgnoredByResourceAllocator = true; }

    void computeScratchKey(GrScratchKey*) const;

    virtual sk_sp<GrSurface> createSurface(GrResourceProvider*) const = 0;
    void assign(sk_sp<GrSurface> surface);

    sk_sp<GrSurface> createSurfaceImpl(GrResourceProvider*, int sampleCnt,
                                       int minStencilSampleCount, GrRenderable, GrMipMapped) const;

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

    bool instantiateImpl(GrResourceProvider* resourceProvider, int sampleCnt,
                         int minStencilSampleCount, GrRenderable, GrMipMapped, const GrUniqueKey*);

    // For deferred proxies this will be null until the proxy is instantiated.
    // For wrapped proxies it will point to the wrapped resource.
    sk_sp<GrSurface>       fTarget;

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
    GrSwizzle              fTextureSwizzle;

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

    bool                   fIgnoredByResourceAllocator = false;
    GrProtected            fIsProtected;

    // This entry is lazily evaluated so, when the proxy wraps a resource, the resource
    // will be called but, when the proxy is deferred, it will compute the answer itself.
    // If the proxy computes its own answer that answer is checked (in debug mode) in
    // the instantiation method.
    mutable size_t         fGpuMemorySize;

    // The last GrRenderTask that wrote to or is currently going to write to this surface
    // The GrRenderTask can be closed (e.g., no surface context is currently bound
    // to this proxy).
    // This back-pointer is required so that we can add a dependancy between
    // the GrRenderTask used to create the current contents of this surface
    // and the GrRenderTask of a destination surface to which this one is being drawn or copied.
    // This pointer is unreffed. GrRenderTasks own a ref on their surface proxies.
    GrRenderTask*          fLastRenderTask;
};

#endif
