/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrContext_DEFINED
#define GrContext_DEFINED

#include "SkMatrix.h"
#include "SkPathEffect.h"
#include "SkTypes.h"
#include "../private/GrAuditTrail.h"
#include "../private/GrSingleOwner.h"
#include "../private/GrSkSLFPFactoryCache.h"
#include "GrContextOptions.h"

// We shouldn't need this but currently Android is relying on this being include transitively.
#include "SkUnPreMultiply.h"

class GrAtlasManager;
class GrBackendFormat;
class GrBackendSemaphore;
class GrCaps;
class GrContextPriv;
class GrContextThreadSafeProxy;
class GrContextThreadSafeProxyPriv;
class GrDrawingManager;
class GrFragmentProcessor;
struct GrGLInterface;
class GrGlyphCache;
class GrGpu;
struct GrMockOptions;
class GrOpMemoryPool;
class GrPath;
class GrProxyProvider;
class GrRenderTargetContext;
class GrResourceCache;
class GrResourceProvider;
class GrSamplerState;
class GrSurfaceProxy;
class GrSwizzle;
class GrTextBlobCache;
class GrTextContext;
class GrTextureProxy;
struct GrVkBackendContext;

class SkImage;
class SkSurfaceCharacterization;
class SkSurfaceProps;
class SkTaskGroup;
class SkTraceMemoryDump;
struct SkYUVAIndex;

#include "GrCaps.h"

class GrImageContext;
class GrRecordingContext;

// This context only has the caps and the unique ID - thread-safe context
class SK_API GrContextWeakest : public SkRefCnt {
public:
    virtual ~GrContextWeakest();

    /*
     * The 3D API backing this context
     */
    GrBackendApi backend() const { return fBackend1; }

    const GrContextOptions& options() const { return fOptions1; }

    /**
     * An ID associated with this context, guaranteed to be unique.
     */
    uint32_t uniqueID() const { return fUniqueID1; }

    // TODO: hide these ?'
    const GrCaps* caps() const { return fCaps1.get(); }
    sk_sp<const GrCaps> refCaps() const { return fCaps1; }

    bool matches(GrContext* context) const;

    virtual sk_sp<GrContextThreadSafeProxy> threadSafeProxy() { return fThreadSafeProxy1; }

    sk_sp<GrSkSLFPFactoryCache> getFPFactoryCache() { return fFPFactoryCache1; }

    GrContextWeakest* weakest() { return this; }
    virtual GrImageContext* asImageContext() { return nullptr; }
    virtual GrRecordingContext* asRecordingContext() { return nullptr; }
    virtual GrContext* asDirectContext() { return nullptr; }

protected:
    // must be called after the ctor!
    bool initWeakest(sk_sp<const GrCaps> caps,
                     sk_sp<GrContextThreadSafeProxy> threadSafeProxy,
                     sk_sp<GrSkSLFPFactoryCache> cache) {
        fCaps1 = std::move(caps);
        fThreadSafeProxy1 = std::move(threadSafeProxy);
        fFPFactoryCache1 = std::move(cache);
        return true;
    };

private:
    friend class GrImageContext;           // for ctor
    friend class GrContextThreadSafeProxy; // for ctor

    GrContextWeakest(GrBackendApi backend, const GrContextOptions& options, uint32_t uniqueID);

    const GrBackendApi              fBackend1;
    const GrContextOptions          fOptions1;
    const uint32_t                  fUniqueID1;
    sk_sp<const GrCaps>             fCaps1;
    sk_sp<GrContextThreadSafeProxy> fThreadSafeProxy1;
    sk_sp<GrSkSLFPFactoryCache>     fFPFactoryCache1;

    typedef SkRefCnt INHERITED;
};

// This context can create GrSurfaceProxies (and thus has a proxy provider) - **new**
class SK_API GrImageContext : public GrContextWeakest {
public:

    GrImageContext* asImageContext() override { return this; }

    GrProxyProvider* proxyProvider();

private:
    friend class GrRecordingContext;     // for ctor
    friend class GrImageCreationContext; // for ctor

    GrImageContext(GrBackendApi backend, const GrContextOptions& options, uint32_t uniqueID)
            : INHERITED(backend, options, uniqueID) {}

    typedef GrContextWeakest INHERITED;
};

// This context can record ops (and thus has a drawing manager) - recording/DDL context
class SK_API GrRecordingContext : public GrImageContext {
public:
    GrRecordingContext* asRecordingContext() override { return this; }

    sk_sp<GrSurfaceContext> makeWrappedSurfaceContext(sk_sp<GrSurfaceProxy>,
                                                      sk_sp<SkColorSpace> = nullptr,
                                                      const SkSurfaceProps* = nullptr);

    sk_sp<GrRenderTargetContext> makeDeferredRenderTargetContext(
                                                 const GrBackendFormat& format,
                                                 SkBackingFit fit,
                                                 int width, int height,
                                                 GrPixelConfig config,
                                                 sk_sp<SkColorSpace> colorSpace,
                                                 int sampleCnt = 1,
                                                 GrMipMapped = GrMipMapped::kNo,
                                                 GrSurfaceOrigin origin = kBottomLeft_GrSurfaceOrigin,
                                                 const SkSurfaceProps* surfaceProps = nullptr,
                                                 SkBudgeted = SkBudgeted::kYes);

    sk_sp<GrRenderTargetContext> makeDeferredRenderTargetContextWithFallback(
                                                                 const GrBackendFormat& format,
                                                                 SkBackingFit fit,
                                                                 int width, int height,
                                                                 GrPixelConfig config,
                                                                 sk_sp<SkColorSpace> colorSpace,
                                                                 int sampleCnt,
                                                                 GrMipMapped mipMapped,
                                                                 GrSurfaceOrigin origin,
                                                                 const SkSurfaceProps* surfaceProps,
                                                                 SkBudgeted budgeted);
private:
    friend class GrContext;    // for ctor
    friend class GrDDLContext; // for ctor

    GrRecordingContext(GrBackendApi backend, const GrContextOptions& options, uint32_t uniqueID)
        : INHERITED(backend, options, uniqueID) {}

    typedef GrImageContext INHERITED;
};

// This context can flush (and thus has a resourceProvider and a gpu) - direct context
class SK_API GrContext : public GrRecordingContext {
public:
    GrContext* asDirectContext() override { return this; }

    /**
     * Creates a GrContext for a backend context. If no GrGLInterface is provided then the result of
     * GrGLMakeNativeInterface() is used if it succeeds.
     */
    static sk_sp<GrContext> MakeGL(sk_sp<const GrGLInterface>, const GrContextOptions&);
    static sk_sp<GrContext> MakeGL(sk_sp<const GrGLInterface>);
    static sk_sp<GrContext> MakeGL(const GrContextOptions&);
    static sk_sp<GrContext> MakeGL();

    static sk_sp<GrContext> MakeVulkan(const GrVkBackendContext&, const GrContextOptions&);
    static sk_sp<GrContext> MakeVulkan(const GrVkBackendContext&);

#ifdef SK_METAL
    /**
     * Makes a GrContext which uses Metal as the backend. The device parameter is an MTLDevice
     * and queue is an MTLCommandQueue which should be used by the backend. These objects must
     * have a ref on them which can be transferred to Ganesh which will release the ref when the
     * GrContext is destroyed.
     */
    static sk_sp<GrContext> MakeMetal(void* device, void* queue, const GrContextOptions& options);
    static sk_sp<GrContext> MakeMetal(void* device, void* queue);
#endif

    static sk_sp<GrContext> MakeMock(const GrMockOptions*, const GrContextOptions&);
    static sk_sp<GrContext> MakeMock(const GrMockOptions*);

    virtual ~GrContext();

//    sk_sp<GrContextThreadSafeProxy> threadSafeProxy();

    /**
     * The GrContext normally assumes that no outsider is setting state
     * within the underlying 3D API's context/device/whatever. This call informs
     * the context that the state was modified and it should resend. Shouldn't
     * be called frequently for good performance.
     * The flag bits, state, is dpendent on which backend is used by the
     * context, either GL or D3D (possible in future).
     */
    void resetContext(uint32_t state = kAll_GrBackendState);

    /**
     * Abandons all GPU resources and assumes the underlying backend 3D API context is no longer
     * usable. Call this if you have lost the associated GPU context, and thus internal texture,
     * buffer, etc. references/IDs are now invalid. Calling this ensures that the destructors of the
     * GrContext and any of its created resource objects will not make backend 3D API calls. Content
     * rendered but not previously flushed may be lost. After this function is called all subsequent
     * calls on the GrContext will fail or be no-ops.
     *
     * The typical use case for this function is that the underlying 3D context was lost and further
     * API calls may crash.
     */
    virtual void abandonContext();

    /**
     * Returns true if the context was abandoned.
     */
    bool abandoned() const;

    /**
     * This is similar to abandonContext() however the underlying 3D context is not yet lost and
     * the GrContext will cleanup all allocated resources before returning. After returning it will
     * assume that the underlying context may no longer be valid.
     *
     * The typical use case for this function is that the client is going to destroy the 3D context
     * but can't guarantee that GrContext will be destroyed first (perhaps because it may be ref'ed
     * elsewhere by either the client or Skia objects).
     */
    virtual void releaseResourcesAndAbandonContext();

    ///////////////////////////////////////////////////////////////////////////
    // Resource Cache

    /**
     *  Return the current GPU resource cache limits.
     *
     *  @param maxResources If non-null, returns maximum number of resources that
     *                      can be held in the cache.
     *  @param maxResourceBytes If non-null, returns maximum number of bytes of
     *                          video memory that can be held in the cache.
     */
    void getResourceCacheLimits(int* maxResources, size_t* maxResourceBytes) const;

    /**
     *  Gets the current GPU resource cache usage.
     *
     *  @param resourceCount If non-null, returns the number of resources that are held in the
     *                       cache.
     *  @param maxResourceBytes If non-null, returns the total number of bytes of video memory held
     *                          in the cache.
     */
    void getResourceCacheUsage(int* resourceCount, size_t* resourceBytes) const;

    /**
     *  Gets the number of bytes in the cache consumed by purgeable (e.g. unlocked) resources.
     */
    size_t getResourceCachePurgeableBytes() const;

    /**
     *  Specify the GPU resource cache limits. If the current cache exceeds either
     *  of these, it will be purged (LRU) to keep the cache within these limits.
     *
     *  @param maxResources The maximum number of resources that can be held in
     *                      the cache.
     *  @param maxResourceBytes The maximum number of bytes of video memory
     *                          that can be held in the cache.
     */
    void setResourceCacheLimits(int maxResources, size_t maxResourceBytes);

    /**
     * Frees GPU created by the context. Can be called to reduce GPU memory
     * pressure.
     */
    virtual void freeGpuResources();

    /**
     * Purge GPU resources that haven't been used in the past 'msNotUsed' milliseconds or are
     * otherwise marked for deletion, regardless of whether the context is under budget.
     */
    void performDeferredCleanup(std::chrono::milliseconds msNotUsed);

    // Temporary compatibility API for Android.
    void purgeResourcesNotUsedInMs(std::chrono::milliseconds msNotUsed) {
        this->performDeferredCleanup(msNotUsed);
    }

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

    /**
     * This entry point is intended for instances where an app has been backgrounded or
     * suspended.
     * If 'scratchResourcesOnly' is true all unlocked scratch resources will be purged but the
     * unlocked resources with persistent data will remain. If 'scratchResourcesOnly' is false
     * then all unlocked resources will be purged.
     * In either case, after the unlocked resources are purged a separate pass will be made to
     * ensure that resource usage is under budget (i.e., even if 'scratchResourcesOnly' is true
     * some resources with persistent data may be purged to be under budget).
     *
     * @param scratchResourcesOnly   If true only unlocked scratch resources will be purged prior
     *                               enforcing the budget requirements.
     */
    void purgeUnlockedResources(bool scratchResourcesOnly);

    /**
     * Gets the maximum supported texture size.
     */
    int maxTextureSize() const;

    /**
     * Gets the maximum supported render target size.
     */
    int maxRenderTargetSize() const;

    /**
     * Can a SkImage be created with the given color type.
     */
    bool colorTypeSupportedAsImage(SkColorType) const;

    /**
     * Can a SkSurface be created with the given color type. To check whether MSAA is supported
     * use maxSurfaceSampleCountForColorType().
     */
    bool colorTypeSupportedAsSurface(SkColorType colorType) const {
        return this->maxSurfaceSampleCountForColorType(colorType) > 0;
    }

    /**
     * Gets the maximum supported sample count for a color type. 1 is returned if only non-MSAA
     * rendering is supported for the color type. 0 is returned if rendering to this color type
     * is not supported at all.
     */
    int maxSurfaceSampleCountForColorType(SkColorType) const;

    ///////////////////////////////////////////////////////////////////////////
    // Misc.

    /**
     * Call to ensure all drawing to the context has been issued to the underlying 3D API.
     */
    void flush();

    /**
     * Call to ensure all drawing to the context has been issued to the underlying 3D API. After
     * issuing all commands, numSemaphore semaphores will be signaled by the gpu. The client passes
     * in an array of numSemaphores GrBackendSemaphores. In general these GrBackendSemaphore's can
     * be either initialized or not. If they are initialized, the backend uses the passed in
     * semaphore. If it is not initialized, a new semaphore is created and the GrBackendSemaphore
     * object is initialized with that semaphore.
     *
     * The client will own and be responsible for deleting the underlying semaphores that are stored
     * and returned in initialized GrBackendSemaphore objects. The GrBackendSemaphore objects
     * themselves can be deleted as soon as this function returns.
     *
     * If the backend API is OpenGL only uninitialized GrBackendSemaphores are supported.
     * If the backend API is Vulkan either initialized or unitialized semaphores are supported.
     * If unitialized, the semaphores which are created will be valid for use only with the VkDevice
     * with which they were created.
     *
     * If this call returns GrSemaphoresSubmited::kNo, the GPU backend will not have created or
     * added any semaphores to signal on the GPU. Thus the client should not have the GPU wait on
     * any of the semaphores. However, any pending commands to the context will still be flushed.
     */
    GrSemaphoresSubmitted flushAndSignalSemaphores(int numSemaphores,
                                                   GrBackendSemaphore signalSemaphores[]);

    // Provides access to functions that aren't part of the public API.
    GrContextPriv contextPriv();
    const GrContextPriv contextPriv() const;

    /** Enumerates all cached GPU resources and dumps their memory to traceMemoryDump. */
    // Chrome is using this!
    void dumpMemoryStatistics(SkTraceMemoryDump* traceMemoryDump) const;

    bool supportsDistanceFieldText() const;

    void storeVkPipelineCacheData();

protected:
    GrContext(GrBackendApi, const GrContextOptions& options, int32_t id = SK_InvalidGenID);

    virtual GrAtlasManager* onGetAtlasManager() = 0;

//private:
    // fTaskGroup must appear before anything that uses it (e.g. fGpu), so that it is destroyed
    // after all of its users. Clients of fTaskGroup will generally want to ensure that they call
    // wait() on it as they are being destroyed, to avoid the possibility of pending tasks being
    // invoked after objects they depend upon have already been destroyed.
    std::unique_ptr<SkTaskGroup>            fTaskGroup;
    sk_sp<GrGpu>                            fGpu;
    GrResourceCache*                        fResourceCache;
    GrResourceProvider*                     fResourceProvider;
    GrProxyProvider*                        fProxyProvider;

    // All the GrOp-derived classes use this pool.
    sk_sp<GrOpMemoryPool>                   fOpMemoryPool;

    GrGlyphCache*                           fGlyphCache;
    std::unique_ptr<GrTextBlobCache>        fTextBlobCache;

    bool                                    fDisableGpuYUVConversion;
    bool                                    fSharpenMipmappedTextures;
    bool                                    fDidTestPMConversions;
    // true if the PM/UPM conversion succeeded; false otherwise
    bool                                    fPMUPMConversionsRoundTrip;

    // In debug builds we guard against improper thread handling
    // This guard is passed to the GrDrawingManager and, from there to all the
    // GrRenderTargetContexts.  It is also passed to the GrResourceProvider and SkGpuDevice.
    mutable GrSingleOwner                   fSingleOwner;

    std::unique_ptr<GrDrawingManager>       fDrawingManager;

    GrAuditTrail                            fAuditTrail;

    GrContextOptions::PersistentCache*      fPersistentCache;

    // TODO: have the GrClipStackClip use renderTargetContexts and rm this friending
    friend class GrContextPriv;

    /**
     * These functions create premul <-> unpremul effects, using the specialized round-trip effects
     * from GrConfigConversionEffect.
     */
    std::unique_ptr<GrFragmentProcessor> createPMToUPMEffect(std::unique_ptr<GrFragmentProcessor>);
    std::unique_ptr<GrFragmentProcessor> createUPMToPMEffect(std::unique_ptr<GrFragmentProcessor>);

    /**
     * Returns true if createPMToUPMEffect and createUPMToPMEffect will succeed. In other words,
     * did we find a pair of round-trip preserving conversion effects?
     */
    bool validPMUPMConversionExists();

    /**
     * A callback similar to the above for use by the TextBlobCache
     * TODO move textblob draw calls below context so we can use the call above.
     */
    static void TextBlobCacheOverBudgetCB(void* data);

    typedef GrRecordingContext INHERITED;
};

/**
 * Can be used to perform actions related to the generating GrContext in a thread safe manner. The
 * proxy does not access the 3D API (e.g. OpenGL) that backs the generating GrContext.
 */
class SK_API GrContextThreadSafeProxy : public GrContextWeakest {
public:
    ~GrContextThreadSafeProxy() override;

    sk_sp<GrContextThreadSafeProxy> threadSafeProxy() override {
        SkASSERT(!INHERITED::threadSafeProxy());   // we don't want a ref counting loop
        return sk_ref_sp<GrContextThreadSafeProxy>(this);
    }

    /**
     *  Create a surface characterization for a DDL that will be replayed into the GrContext
     *  that created this proxy. On failure the resulting characterization will be invalid (i.e.,
     *  "!c.isValid()").
     *
     *  @param cacheMaxResourceBytes The max resource bytes limit that will be in effect when the
     *                               DDL created with this characterization is replayed.
     *                               Note: the contract here is that the DDL will be created as
     *                               if it had a full 'cacheMaxResourceBytes' to use. If replayed
     *                               into a GrContext that already has locked GPU memory, the
     *                               replay can exceed the budget. To rephrase, all resource
     *                               allocation decisions are made at record time and at playback
     *                               time the budget limits will be ignored.
     *  @param ii                    The image info specifying properties of the SkSurface that
     *                               the DDL created with this characterization will be replayed
     *                               into.
     *                               Note: Ganesh doesn't make use of the SkImageInfo's alphaType
     *  @param backendFormat         Information about the format of the GPU surface that will
     *                               back the SkSurface upon replay
     *  @param sampleCount           The sample count of the SkSurface that the DDL created with
     *                               this characterization will be replayed into
     *  @param origin                The origin of the SkSurface that the DDL created with this
     *                               characterization will be replayed into
     *  @param surfaceProps          The surface properties of the SkSurface that the DDL created
     *                               with this characterization will be replayed into
     *  @param isMipMapped           Will the surface the DDL will be replayed into have space
     *                               allocated for mipmaps?
     *  @param willUseGLFBO0         Will the surface the DDL will be replayed into be backed by GL
     *                               FBO 0. This flag is only valid if using an GL backend.
     */
    SkSurfaceCharacterization createCharacterization(
                                  size_t cacheMaxResourceBytes,
                                  const SkImageInfo& ii, const GrBackendFormat& backendFormat,
                                  int sampleCount, GrSurfaceOrigin origin,
                                  const SkSurfaceProps& surfaceProps,
                                  bool isMipMapped, bool willUseGLFBO0 = false);

    // Matches the defines in SkImage_GpuBase.h
    typedef void* TextureContext;
    typedef void (*TextureReleaseProc)(TextureContext textureContext);
    typedef void (*TextureFulfillProc)(TextureContext textureContext, GrBackendTexture* outTexture);
    typedef void (*PromiseDoneProc)(TextureContext textureContext);

    /**
        Create a new SkImage that is very similar to an SkImage created by MakeFromTexture. The main
        difference is that the client doesn't have the backend texture on the gpu yet but they know
        all the properties of the texture. So instead of passing in a GrBackendTexture the client
        supplies a GrBackendFormat, width, height, and GrMipMapped state.

        When we actually send the draw calls to the GPU, we will call the textureFulfillProc and
        the client will return a GrBackendTexture to us. The properties of the GrBackendTexture must
        match those set during the SkImage creation, and it must have a valid backend gpu texture.
        The gpu texture supplied by the client must stay valid until we call the textureReleaseProc.

        When we are done with the texture returned by the textureFulfillProc we will call the
        textureReleaseProc passing in the textureContext. This is a signal to the client that they
        are free to delete the underlying gpu texture. If future draws also use the same promise
        image we will call the textureFulfillProc again if we've already called the
        textureReleaseProc. We will always call textureFulfillProc and textureReleaseProc in pairs.
        In other words we will never call textureFulfillProc or textureReleaseProc multiple times
        for the same textureContext before calling the other.

        We call the promiseDoneProc when we will no longer call the textureFulfillProc again. We
        pass in the textureContext as a parameter to the promiseDoneProc. We also guarantee that
        there will be no outstanding textureReleaseProcs that still need to be called when we call
        the textureDoneProc. Thus when the textureDoneProc gets called the client is able to cleanup
        all GPU objects and meta data needed for the textureFulfill call.

        This call is only valid if the SkDeferredDisplayListRecorder is backed by a gpu context.

        @param backendFormat       format of promised gpu texture
        @param width               width of promised gpu texture
        @param height              height of promised gpu texture
        @param mipMapped           mip mapped state of promised gpu texture
        @param origin              one of: kBottomLeft_GrSurfaceOrigin, kTopLeft_GrSurfaceOrigin
        @param colorType           one of: kUnknown_SkColorType, kAlpha_8_SkColorType,
                                   kRGB_565_SkColorType, kARGB_4444_SkColorType,
                                   kRGBA_8888_SkColorType, kBGRA_8888_SkColorType,
                                   kGray_8_SkColorType, kRGBA_F16_SkColorType
        @param alphaType           one of: kUnknown_SkAlphaType, kOpaque_SkAlphaType,
                                   kPremul_SkAlphaType, kUnpremul_SkAlphaType
        @param colorSpace          range of colors; may be nullptr
        @param textureFulfillProc  function called to get actual gpu texture
        @param textureReleaseProc  function called when texture can be released
        @param promiseDoneProc     function called when we will no longer call textureFulfillProc
        @param textureContext      state passed to textureFulfillProc and textureReleaseProc
        @return                    created SkImage, or nullptr
     */
    sk_sp<SkImage> makePromiseTexture(const GrBackendFormat& backendFormat,
                                      int width,
                                      int height,
                                      GrMipMapped mipMapped,
                                      GrSurfaceOrigin origin,
                                      SkColorType colorType,
                                      SkAlphaType alphaType,
                                      sk_sp<SkColorSpace> colorSpace,
                                      TextureFulfillProc textureFulfillProc,
                                      TextureReleaseProc textureReleaseProc,
                                      PromiseDoneProc promiseDoneProc,
                                      TextureContext textureContext);

    /**
        This entry point operates the same as 'makePromiseTexture' except that its
        textureFulfillProc can be called up to four times to fetch the required YUVA
        planes (passing a different textureContext to each call). So, if the 'yuvaIndices'
        indicate that only the first two backend textures are used, 'textureFulfillProc' will
        be called with the first two 'textureContexts'.
     */
    sk_sp<SkImage> makeYUVAPromiseTexture(SkYUVColorSpace yuvColorSpace,
                                          const GrBackendFormat yuvaFormats[],
                                          const SkISize yuvaSizes[],
                                          const SkYUVAIndex yuvaIndices[4],
                                          int imageWidth,
                                          int imageHeight,
                                          GrSurfaceOrigin imageOrigin,
                                          sk_sp<SkColorSpace> imageColorSpace,
                                          TextureFulfillProc textureFulfillProc,
                                          TextureReleaseProc textureReleaseProc,
                                          PromiseDoneProc promiseDoneProc,
                                          TextureContext textureContexts[]);

    bool operator==(const GrContextThreadSafeProxy& that) const {
        // Each GrContext should only ever have a single thread-safe proxy.
        SkASSERT((this == &that) == (this->uniqueID() == that.uniqueID()));
        return this == &that;
    }

    bool operator!=(const GrContextThreadSafeProxy& that) const { return !(*this == that); }

    // Provides access to functions that aren't part of the public API.
    GrContextThreadSafeProxyPriv priv();
    const GrContextThreadSafeProxyPriv priv() const;

    static sk_sp<GrContextThreadSafeProxy> Make(GrBackendApi backend,
                                                uint32_t uniqueID,
                                                sk_sp<const GrCaps> caps,
                                                const GrContextOptions& options,
                                                sk_sp<GrSkSLFPFactoryCache> cache) {
        sk_sp<GrContextThreadSafeProxy> context(new GrContextThreadSafeProxy(backend, options, uniqueID));

        if (!context->initWeakest(std::move(caps), nullptr, std::move(cache))) {
            return nullptr;
        }

        return context;
    }

private:
    GrContextThreadSafeProxy(GrBackendApi backend, const GrContextOptions& options, uint32_t uniqueID);

#if 0
    // DDL TODO: need to add unit tests for backend & maybe options
    GrContextThreadSafeProxy(sk_sp<const GrCaps> caps,
                             uint32_t uniqueID,
                             GrBackendApi backend,
                             const GrContextOptions& options,
                             sk_sp<GrSkSLFPFactoryCache> cache);
#endif

    sk_sp<GrSkSLFPFactoryCache> fFPFactoryCache;

    friend class GrContextThreadSafeProxyPriv;

    typedef GrContextWeakest INHERITED;
};

#endif
