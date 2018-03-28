/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrContext_DEFINED
#define GrContext_DEFINED

#include "GrCaps.h"
#include "SkMatrix.h"
#include "SkPathEffect.h"
#include "SkTypes.h"
#include "../private/GrAuditTrail.h"
#include "../private/GrSingleOwner.h"
#include "GrContextOptions.h"

class GrAtlasManager;
class GrBackendFormat;
class GrBackendSemaphore;
class GrContextPriv;
class GrContextThreadSafeProxy;
class GrDrawingManager;
struct GrDrawOpAtlasConfig;
class GrFragmentProcessor;
struct GrGLInterface;
class GrGlyphCache;
class GrGpu;
class GrIndexBuffer;
struct GrMockOptions;
class GrOvalRenderer;
class GrPath;
class GrProxyProvider;
class GrRenderTargetContext;
class GrResourceEntry;
class GrResourceCache;
class GrResourceProvider;
class GrRestrictedAtlasManager;
class GrSamplerState;
class GrSurfaceProxy;
class GrSwizzle;
class GrTextBlobCache;
class GrTextContext;
class GrTextureProxy;
class GrVertexBuffer;
struct GrVkBackendContext;

class SkImage;
class SkSurfaceCharacterization;
class SkSurfaceProps;
class SkTaskGroup;
class SkTraceMemoryDump;

class SK_API GrContext : public SkRefCnt {
public:
    /**
     * Creates a GrContext for a backend context.
     */
    static GrContext* Create(GrBackend, GrBackendContext, const GrContextOptions& options);
    static GrContext* Create(GrBackend, GrBackendContext);

    static sk_sp<GrContext> MakeGL(sk_sp<const GrGLInterface>, const GrContextOptions&);
    static sk_sp<GrContext> MakeGL(sk_sp<const GrGLInterface>);
    // Deprecated
    static sk_sp<GrContext> MakeGL(const GrGLInterface*);
    static sk_sp<GrContext> MakeGL(const GrGLInterface*, const GrContextOptions&);

#ifdef SK_VULKAN
    static sk_sp<GrContext> MakeVulkan(sk_sp<const GrVkBackendContext>, const GrContextOptions&);
    static sk_sp<GrContext> MakeVulkan(sk_sp<const GrVkBackendContext>);
#endif

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

    sk_sp<GrContextThreadSafeProxy> threadSafeProxy();

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
     * Callback function to allow classes to cleanup on GrContext destruction.
     * The 'info' field is filled in with the 'info' passed to addCleanUp.
     */
    typedef void (*PFCleanUpFunc)(const GrContext* context, void* info);

    /**
     * Add a function to be called from within GrContext's destructor.
     * This gives classes a chance to free resources held on a per context basis.
     * The 'info' parameter will be stored and passed to the callback function.
     */
    void addCleanUp(PFCleanUpFunc cleanUp, void* info) {
        CleanUpData* entry = fCleanUpData.push();

        entry->fFunc = cleanUp;
        entry->fInfo = info;
    }

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
     * Purge all the unlocked resources from the cache.
     * This entry point is mainly meant for timing texture uploads
     * and is not defined in normal builds of Skia.
     */
    void purgeAllUnlockedResources();

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

    /** Access the context capabilities */
    const GrCaps* caps() const { return fCaps.get(); }

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

    /*
     * Create a new render target context backed by a deferred-style
     * GrRenderTargetProxy. We guarantee that "asTextureProxy" will succeed for
     * renderTargetContexts created via this entry point.
     */
    sk_sp<GrRenderTargetContext> makeDeferredRenderTargetContext(
                                                 SkBackingFit fit,
                                                 int width, int height,
                                                 GrPixelConfig config,
                                                 sk_sp<SkColorSpace> colorSpace,
                                                 int sampleCnt = 1,
                                                 GrMipMapped = GrMipMapped::kNo,
                                                 GrSurfaceOrigin origin = kBottomLeft_GrSurfaceOrigin,
                                                 const SkSurfaceProps* surfaceProps = nullptr,
                                                 SkBudgeted = SkBudgeted::kYes);
    /*
     * This method will attempt to create a renderTargetContext that has, at least, the number of
     * channels and precision per channel as requested in 'config' (e.g., A8 and 888 can be
     * converted to 8888). It may also swizzle the channels (e.g., BGRA -> RGBA).
     * SRGB-ness will be preserved.
     */
    sk_sp<GrRenderTargetContext> makeDeferredRenderTargetContextWithFallback(
                                                 SkBackingFit fit,
                                                 int width, int height,
                                                 GrPixelConfig config,
                                                 sk_sp<SkColorSpace> colorSpace,
                                                 int sampleCnt = 1,
                                                 GrMipMapped = GrMipMapped::kNo,
                                                 GrSurfaceOrigin origin = kBottomLeft_GrSurfaceOrigin,
                                                 const SkSurfaceProps* surfaceProps = nullptr,
                                                 SkBudgeted budgeted = SkBudgeted::kYes);

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

    /**
     * An ID associated with this context, guaranteed to be unique.
     */
    uint32_t uniqueID() { return fUniqueID; }

    ///////////////////////////////////////////////////////////////////////////
    // Functions intended for internal use only.
    bool abandoned() const;

    /** Reset GPU stats */
    void resetGpuStats() const ;

    /** Prints cache stats to the string if GR_CACHE_STATS == 1. */
    void dumpCacheStats(SkString*) const;
    void dumpCacheStatsKeyValuePairs(SkTArray<SkString>* keys, SkTArray<double>* values) const;
    void printCacheStats() const;

    /** Prints GPU stats to the string if GR_GPU_STATS == 1. */
    void dumpGpuStats(SkString*) const;
    void dumpGpuStatsKeyValuePairs(SkTArray<SkString>* keys, SkTArray<double>* values) const;
    void printGpuStats() const;

    /** Returns a string with detailed information about the context & GPU, in JSON format. */
    SkString dump() const;

    /** Specify the TextBlob cache limit. If the current cache exceeds this limit it will purge.
        this is for testing only */
    void setTextBlobCacheLimit_ForTesting(size_t bytes);

    /** Specify the sizes of the GrAtlasTextContext atlases.  The configs pointer below should be
        to an array of 3 entries */
    void setTextContextAtlasSizes_ForTesting(const GrDrawOpAtlasConfig* configs);

    /** Enumerates all cached GPU resources and dumps their memory to traceMemoryDump. */
    void dumpMemoryStatistics(SkTraceMemoryDump* traceMemoryDump) const;

    /** Get pointer to atlas texture for given mask format. Note that this wraps an
        actively mutating texture in an SkImage. This could yield unexpected results
        if it gets cached or used more generally. */
    sk_sp<SkImage> getFontAtlasImage_ForTesting(GrMaskFormat format, unsigned int index = 0);

    GrAuditTrail* getAuditTrail() { return &fAuditTrail; }

    GrContextOptions::PersistentCache* getPersistentCache() { return fPersistentCache; }

    /** This is only useful for debug purposes */
    SkDEBUGCODE(GrSingleOwner* debugSingleOwner() const { return &fSingleOwner; } )

    // Provides access to functions that aren't part of the public API.
    GrContextPriv contextPriv();
    const GrContextPriv contextPriv() const;

protected:
    GrContext(GrBackend, int32_t id = SK_InvalidGenID);

    bool initCommon(const GrContextOptions&);
    virtual bool init(const GrContextOptions&) = 0; // must be called after the ctor!

    virtual GrAtlasManager* onGetFullAtlasManager() = 0;
    virtual GrRestrictedAtlasManager* onGetRestrictedAtlasManager() = 0;

    const GrBackend                         fBackend;
    sk_sp<const GrCaps>                     fCaps;
    sk_sp<GrContextThreadSafeProxy>         fThreadSafeProxy;

private:
    sk_sp<GrGpu>                            fGpu;
    GrResourceCache*                        fResourceCache;
    GrResourceProvider*                     fResourceProvider;
    GrProxyProvider*                        fProxyProvider;


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

    std::unique_ptr<SkTaskGroup>            fTaskGroup;

    struct CleanUpData {
        PFCleanUpFunc fFunc;
        void*         fInfo;
    };

    SkTDArray<CleanUpData>                  fCleanUpData;

    const uint32_t                          fUniqueID;

    std::unique_ptr<GrDrawingManager>       fDrawingManager;

    GrAuditTrail                            fAuditTrail;

    GrContextOptions::PersistentCache*      fPersistentCache;

    // TODO: have the GrClipStackClip use renderTargetContexts and rm this friending
    friend class GrContextPriv;

    /**
     * These functions create premul <-> unpremul effects. If the second argument is 'true', they
     * use the specialized round-trip effects from GrConfigConversionEffect, otherwise they
     * create effects that do naive multiply or divide.
     */
    std::unique_ptr<GrFragmentProcessor> createPMToUPMEffect(std::unique_ptr<GrFragmentProcessor>,
                                                             bool useConfigConversionEffect);
    std::unique_ptr<GrFragmentProcessor> createUPMToPMEffect(std::unique_ptr<GrFragmentProcessor>,
                                                             bool useConfigConversionEffect);

    /**
     * Returns true if createPMtoUPMEffect and createUPMToPMEffect will succeed for non-sRGB 8888
     * configs. In other words, did we find a pair of round-trip preserving conversion effects?
     */
    bool validPMUPMConversionExists();

    /**
     * A callback similar to the above for use by the TextBlobCache
     * TODO move textblob draw calls below context so we can use the call above.
     */
    static void TextBlobCacheOverBudgetCB(void* data);

    typedef SkRefCnt INHERITED;
};

/**
 * Can be used to perform actions related to the generating GrContext in a thread safe manner. The
 * proxy does not access the 3D API (e.g. OpenGL) that backs the generating GrContext.
 */
class GrContextThreadSafeProxy : public SkRefCnt {
public:
    bool matches(GrContext* context) const { return context->uniqueID() == fContextUniqueID; }

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
     */
    SkSurfaceCharacterization createCharacterization(
                                  size_t cacheMaxResourceBytes,
                                  const SkImageInfo& ii, const GrBackendFormat& backendFormat,
                                  int sampleCount, GrSurfaceOrigin origin,
                                  const SkSurfaceProps& surfaceProps,
                                  bool isMipMapped);

private:
    // DDL TODO: need to add unit tests for backend & maybe options
    GrContextThreadSafeProxy(sk_sp<const GrCaps> caps,
                             uint32_t uniqueID,
                             GrBackend backend,
                             const GrContextOptions& options)
        : fCaps(std::move(caps))
        , fContextUniqueID(uniqueID)
        , fBackend(backend)
        , fOptions(options) {
    }

    sk_sp<const GrCaps>    fCaps;
    const uint32_t         fContextUniqueID;
    const GrBackend        fBackend;
    const GrContextOptions fOptions;

    friend class GrDirectContext; // To construct this object
    friend class GrContextPriv;   // for access to 'fOptions' in MakeDDL
    friend class GrDDLContext;    // to implement the GrDDLContext ctor (access to all members)

    typedef SkRefCnt INHERITED;
};

#endif
