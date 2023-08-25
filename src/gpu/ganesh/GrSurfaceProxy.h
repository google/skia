/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSurfaceProxy_DEFINED
#define GrSurfaceProxy_DEFINED

#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrTypes.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkTo.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/ResourceKey.h"
#include "src/gpu/ganesh/GrGpuResource.h"
#include "src/gpu/ganesh/GrSurface.h"

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <string_view>
#include <utility>

class GrCaps;
class GrContext_Base;
class GrRecordingContext;
class GrRenderTarget;
class GrRenderTargetProxy;
class GrRenderTask;
class GrResourceProvider;
class GrSurfaceProxyPriv;
class GrTexture;
class GrTextureProxy;
enum class SkBackingFit;
namespace skgpu {
enum class Budgeted : bool;
}

class GrSurfaceProxy : public SkNVRefCnt<GrSurfaceProxy> {
public:
    virtual ~GrSurfaceProxy();

    /**
     * Indicates "resolutions" that need to be done on a surface before its pixels can be accessed.
     * If both types of resolve are requested, the MSAA resolve will happen first.
     */
    enum class ResolveFlags {
        kNone = 0,
        kMSAA = 1 << 0,  // Blit and resolve an internal MSAA render buffer into the texture.
        kMipMaps = 1 << 1,  // Regenerate all mipmap levels.
    };

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

    /**
     * Specifies the expected properties of the GrSurface returned by a lazy instantiation
     * callback. The dimensions will be negative in the case of a fully lazy proxy.
     */
    struct LazySurfaceDesc {
        SkISize fDimensions;
        SkBackingFit fFit;
        GrRenderable fRenderable;
        GrMipmapped fMipmapped;
        int fSampleCnt;
        const GrBackendFormat& fFormat;
        GrTextureType fTextureType;
        GrProtected fProtected;
        skgpu::Budgeted fBudgeted;
        std::string_view fLabel;
    };

    struct LazyCallbackResult {
        LazyCallbackResult() = default;
        LazyCallbackResult(const LazyCallbackResult&) = default;
        LazyCallbackResult(LazyCallbackResult&& that) = default;
        LazyCallbackResult(sk_sp<GrSurface> surf,
                           bool releaseCallback = true,
                           LazyInstantiationKeyMode mode = LazyInstantiationKeyMode::kSynced);
        LazyCallbackResult(sk_sp<GrTexture> tex);

        LazyCallbackResult& operator=(const LazyCallbackResult&) = default;
        LazyCallbackResult& operator=(LazyCallbackResult&&) = default;

        sk_sp<GrSurface> fSurface;
        LazyInstantiationKeyMode fKeyMode = LazyInstantiationKeyMode::kSynced;
        /**
         * Should the callback be disposed of after it has returned or preserved until the proxy
         * is freed. Only honored if fSurface is not-null. If it is null the callback is preserved.
         */
        bool fReleaseCallback = true;
    };

    using LazyInstantiateCallback =
            std::function<LazyCallbackResult(GrResourceProvider*, const LazySurfaceDesc&)>;

    enum class UseAllocator {
        /**
         * This proxy will be instantiated outside the allocator (e.g. for proxies that are
         * instantiated in on-flush callbacks).
         */
        kNo = false,
        /**
         * GrResourceAllocator should instantiate this proxy.
         */
        kYes = true,
    };

    bool isLazy() const { return !this->isInstantiated() && SkToBool(fLazyInstantiateCallback); }

    bool isFullyLazy() const {
        bool result = fDimensions.width() < 0;
        SkASSERT(result == (fDimensions.height() < 0));
        SkASSERT(!result || this->isLazy());
        return result;
    }

    SkISize dimensions() const {
        SkASSERT(!this->isFullyLazy());
        return fDimensions;
    }
    int width() const { return this->dimensions().width(); }
    int height() const { return this->dimensions().height(); }

    SkISize backingStoreDimensions() const;

    /**
     * Helper that gets the width and height of the proxy as a bounding rectangle.
     */
    SkRect getBoundsRect() const { return SkRect::Make(this->dimensions()); }

    /* A perhaps faster check for this->dimensions() == this->backingStoreDimensions(). */
    bool isFunctionallyExact() const;

    /**
     * Helper that gets the dimensions the backing GrSurface will have as a bounding rectangle.
     */
    SkRect backingStoreBoundsRect() const {
        return SkRect::Make(this->backingStoreDimensions());
    }

    SkIRect backingStoreBoundsIRect() const {
        return SkIRect::MakeSize(this->backingStoreDimensions());
    }

    const GrBackendFormat& backendFormat() const { return fFormat; }

    bool isFormatCompressed(const GrCaps*) const;

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

    /** @return The unique key for this proxy. May be invalid. */
    virtual const skgpu::UniqueKey& getUniqueKey() const {
        // Base class never has a valid unique key.
        static const skgpu::UniqueKey kInvalidKey;
        return kInvalidKey;
    }

    bool isInstantiated() const { return SkToBool(fTarget); }

    /** Called when this task becomes a target of a GrRenderTask. */
    void isUsedAsTaskTarget() { ++fTaskTargetCount; }

    /** How many render tasks has this proxy been the target of? */
    int getTaskTargetCount() const { return fTaskTargetCount; }

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
    skgpu::Budgeted isBudgeted() const { return fBudgeted; }

    /**
     * The pixel values of this proxy's surface cannot be modified (e.g. doesn't support write
     * pixels or MIP map level regen). Read-only proxies also bypass interval tracking and
     * assignment in GrResourceAllocator.
     */
    bool readOnly() const { return fSurfaceFlags & GrInternalSurfaceFlags::kReadOnly; }
    bool framebufferOnly() const {
        return fSurfaceFlags & GrInternalSurfaceFlags::kFramebufferOnly;
    }

    /**
     * This means surface is a multisampled render target, and internally holds a non-msaa texture
     * for resolving into. The render target resolves itself by blitting into this internal texture.
     * (asTexture() might or might not return the internal texture, but if it does, we always
     * resolve the render target before accessing this texture's data.)
     */
    bool requiresManualMSAAResolve() const {
        return fSurfaceFlags & GrInternalSurfaceFlags::kRequiresManualMSAAResolve;
    }

    /**
     * Retrieves the amount of GPU memory that will be or currently is used by this resource
     * in bytes. It is approximate since we aren't aware of additional padding or copies made
     * by the driver.
     *
     * @return the amount of GPU memory used in bytes
     */
    size_t gpuMemorySize() const {
        SkASSERT(!this->isFullyLazy());
        if (kInvalidGpuMemorySize == fGpuMemorySize) {
            fGpuMemorySize = this->onUninstantiatedGpuMemorySize();
            SkASSERT(kInvalidGpuMemorySize != fGpuMemorySize);
        }
        return fGpuMemorySize;
    }

    std::string_view getLabel() const { return fLabel; }

    enum class RectsMustMatch : bool {
        kNo = false,
        kYes = true
    };

    // Helper function that creates a temporary SurfaceContext to perform the copy
    // The copy is is not a render target and not multisampled.
    //
    // The intended use of this copy call is simply to copy exact pixel values from one proxy to a
    // new one. Thus, there isn't a need for a swizzle when doing the copy. The format of the copy
    // will be the same as the src. Therefore, the copy can be used in a view with the same swizzle
    // as the original for use with a given color type.
    //
    // Optionally gets the render task that performs the copy. If it is later determined that the
    // copy is not neccessaru then the task can be marked skippable using GrRenderTask::canSkip() and
    // the copy will be elided.
    static sk_sp<GrSurfaceProxy> Copy(GrRecordingContext*,
                                      sk_sp<GrSurfaceProxy> src,
                                      GrSurfaceOrigin,
                                      GrMipmapped,
                                      SkIRect srcRect,
                                      SkBackingFit,
                                      skgpu::Budgeted,
                                      std::string_view label,
                                      RectsMustMatch = RectsMustMatch::kNo,
                                      sk_sp<GrRenderTask>* outTask = nullptr);

    // Same as above Copy but copies the entire 'src'
    static sk_sp<GrSurfaceProxy> Copy(GrRecordingContext*,
                                      sk_sp<GrSurfaceProxy> src,
                                      GrSurfaceOrigin,
                                      GrMipmapped,
                                      SkBackingFit,
                                      skgpu::Budgeted,
                                      std::string_view label,
                                      sk_sp<GrRenderTask>* outTask = nullptr);

#if defined(GR_TEST_UTILS)
    int32_t testingOnly_getBackingRefCnt() const;
    GrInternalSurfaceFlags testingOnly_getFlags() const;
    SkString dump() const;
#endif

#ifdef SK_DEBUG
    void validate(GrContext_Base*) const;
    SkString getDebugName() {
        return fDebugName.isEmpty() ? SkStringPrintf("%d", this->uniqueID().asUInt()) : fDebugName;
    }
    void setDebugName(SkString name) { fDebugName = std::move(name); }
#endif

    // Provides access to functions that aren't part of the public API.
    inline GrSurfaceProxyPriv priv();
    inline const GrSurfaceProxyPriv priv() const;  // NOLINT(readability-const-return-type)

    bool isDDLTarget() const { return fIsDDLTarget; }

    GrProtected isProtected() const { return fIsProtected; }

    bool isPromiseProxy() { return fIsPromiseProxy; }

protected:
    // Deferred version - takes a new UniqueID from the shared resource/proxy pool.
    GrSurfaceProxy(const GrBackendFormat&,
                   SkISize,
                   SkBackingFit,
                   skgpu::Budgeted,
                   GrProtected,
                   GrInternalSurfaceFlags,
                   UseAllocator,
                   std::string_view label);
    // Lazy-callback version - takes a new UniqueID from the shared resource/proxy pool.
    GrSurfaceProxy(LazyInstantiateCallback&&,
                   const GrBackendFormat&,
                   SkISize,
                   SkBackingFit,
                   skgpu::Budgeted,
                   GrProtected,
                   GrInternalSurfaceFlags,
                   UseAllocator,
                   std::string_view label);

    // Wrapped version - shares the UniqueID of the passed surface.
    // Takes UseAllocator because even though this is already instantiated it still can participate
    // in allocation by having its backing resource recycled to other uninstantiated proxies or
    // not depending on UseAllocator.
    GrSurfaceProxy(sk_sp<GrSurface>, SkBackingFit, UseAllocator);

    friend class GrSurfaceProxyPriv;

    // Methods made available via GrSurfaceProxyPriv
    bool ignoredByResourceAllocator() const { return fIgnoredByResourceAllocator; }
    void setIgnoredByResourceAllocator() { fIgnoredByResourceAllocator = true; }

    void computeScratchKey(const GrCaps&, skgpu::ScratchKey*) const;

    virtual sk_sp<GrSurface> createSurface(GrResourceProvider*) const = 0;
    void assign(sk_sp<GrSurface> surface);

    sk_sp<GrSurface> createSurfaceImpl(GrResourceProvider*, int sampleCnt, GrRenderable,
                                       GrMipmapped) const;

    // Once the dimensions of a fully-lazy proxy are decided, and before it gets instantiated, the
    // client can use this optional method to specify the proxy's dimensions. (A proxy's dimensions
    // can be less than the GPU surface that backs it. e.g., SkBackingFit::kApprox.) Otherwise,
    // the proxy's dimensions will be set to match the underlying GPU surface upon instantiation.
    void setLazyDimensions(SkISize dimensions) {
        SkASSERT(this->isFullyLazy());
        SkASSERT(!dimensions.isEmpty());
        fDimensions = dimensions;
    }

    bool instantiateImpl(GrResourceProvider* resourceProvider, int sampleCnt, GrRenderable,
                         GrMipmapped, const skgpu::UniqueKey*);

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
    // For wrapped resources, 'fFormat' and 'fDimensions' will always be filled in from the
    // wrapped resource.
    const GrBackendFormat  fFormat;
    SkISize                fDimensions;

    SkBackingFit           fFit;      // always kApprox for lazy-callback resources
                                      // always kExact for wrapped resources
    mutable skgpu::Budgeted fBudgeted;  // always kYes for lazy-callback resources
                                        // set from the backing resource for wrapped resources
                                        // mutable bc of SkSurface/SkImage wishy-washiness
                                        // Only meaningful if fLazyInstantiateCallback is non-null.
    UseAllocator           fUseAllocator;

    const UniqueID         fUniqueID; // set from the backing resource for wrapped resources

    LazyInstantiateCallback fLazyInstantiateCallback;

    SkDEBUGCODE(void validateSurface(const GrSurface*);)
    SkDEBUGCODE(virtual void onValidateSurface(const GrSurface*) = 0;)

    static const size_t kInvalidGpuMemorySize = ~static_cast<size_t>(0);
    SkDEBUGCODE(size_t getRawGpuMemorySize_debugOnly() const { return fGpuMemorySize; })

    virtual size_t onUninstantiatedGpuMemorySize() const = 0;

    virtual LazySurfaceDesc callbackDesc() const = 0;

    bool                   fIgnoredByResourceAllocator = false;
    bool                   fIsDDLTarget = false;
    bool                   fIsPromiseProxy = false;
    GrProtected            fIsProtected;

    int                     fTaskTargetCount = 0;

    const std::string fLabel;

    // This entry is lazily evaluated so, when the proxy wraps a resource, the resource
    // will be called but, when the proxy is deferred, it will compute the answer itself.
    // If the proxy computes its own answer that answer is checked (in debug mode) in
    // the instantiation method. The image may be shared between threads, hence atomic.
    mutable std::atomic<size_t>         fGpuMemorySize{kInvalidGpuMemorySize};
    SkDEBUGCODE(SkString   fDebugName;)
};

GR_MAKE_BITFIELD_CLASS_OPS(GrSurfaceProxy::ResolveFlags)

#endif
