/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrContext_DEFINED
#define GrContext_DEFINED

#include "GrCaps.h"
#include "GrColor.h"
#include "GrRenderTarget.h"
#include "SkMatrix.h"
#include "SkPathEffect.h"
#include "SkTypes.h"
#include "../private/GrAuditTrail.h"
#include "../private/GrSingleOwner.h"

class GrAtlasGlyphCache;
struct GrContextOptions;
class GrContextPriv;
class GrContextThreadSafeProxy;
class GrDrawingManager;
struct GrDrawOpAtlasConfig;
class GrRenderTargetContext;
class GrFragmentProcessor;
class GrGpu;
class GrIndexBuffer;
class GrOvalRenderer;
class GrPath;
class GrPipelineBuilder;
class GrResourceEntry;
class GrResourceCache;
class GrResourceProvider;
class GrSamplerParams;
class GrSurfaceProxy;
class GrTextBlobCache;
class GrTextContext;
class GrTextureProxy;
class GrVertexBuffer;
class GrSwizzle;
class SkTraceMemoryDump;

class SkImage;
class SkSurfaceProps;

class SK_API GrContext : public SkRefCnt {
public:
    /**
     * Creates a GrContext for a backend context.
     */
    static GrContext* Create(GrBackend, GrBackendContext, const GrContextOptions& options);
    static GrContext* Create(GrBackend, GrBackendContext);

    /**
     * Only defined in test apps.
     */
    static GrContext* CreateMockContext();

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
     * Abandons all GPU resources and assumes the underlying backend 3D API context is not longer
     * usable. Call this if you have lost the associated GPU context, and thus internal texture,
     * buffer, etc. references/IDs are now invalid. Calling this ensures that the destructors of the
     * GrContext and any of its created resource objects will not make backend 3D API calls. Content
     * rendered but not previously flushed may be lost. After this function is called all subsequent
     * calls on the GrContext will fail or be no-ops.
     *
     * The typical use case for this function is that the underlying 3D context was lost and further
     * API calls may crash.
     */
    void abandonContext();

    /**
     * This is similar to abandonContext() however the underlying 3D context is not yet lost and
     * the GrContext will cleanup all allocated resources before returning. After returning it will
     * assume that the underlying context may no longer be valid.
     *
     * The typical use case for this function is that the client is going to destroy the 3D context
     * but can't guarantee that GrContext will be destroyed first (perhaps because it may be ref'ed
     * elsewhere by either the client or Skia objects).
     */
    void releaseResourcesAndAbandonContext();

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
    void freeGpuResources();

    /**
     * Purge all the unlocked resources from the cache.
     * This entry point is mainly meant for timing texture uploads
     * and is not defined in normal builds of Skia.
     */
    void purgeAllUnlockedResources();

    /**
     * Purge GPU resources that haven't been used in the past 'ms' milliseconds, regardless of
     * whether the context is currently under budget.
     */
    void purgeResourcesNotUsedInMs(std::chrono::milliseconds ms);

    /** Access the context capabilities */
    const GrCaps* caps() const { return fCaps; }

    /**
     * Returns the recommended sample count for a render target when using this
     * context.
     *
     * @param  config the configuration of the render target.
     * @param  dpi the display density in dots per inch.
     *
     * @return sample count that should be perform well and have good enough
     *         rendering quality for the display. Alternatively returns 0 if
     *         MSAA is not supported or recommended to be used by default.
     */
    int getRecommendedSampleCount(GrPixelConfig config, SkScalar dpi) const;

    /**
     * Create both a GrRenderTarget and a matching GrRenderTargetContext to wrap it.
     * We guarantee that "asTexture" will succeed for renderTargetContexts created
     * via this entry point.
     */
    sk_sp<GrRenderTargetContext> makeRenderTargetContext(
                                                 SkBackingFit fit,
                                                 int width, int height,
                                                 GrPixelConfig config,
                                                 sk_sp<SkColorSpace> colorSpace,
                                                 int sampleCnt = 0,
                                                 GrSurfaceOrigin origin = kBottomLeft_GrSurfaceOrigin,
                                                 const SkSurfaceProps* surfaceProps = nullptr,
                                                 SkBudgeted = SkBudgeted::kYes);

    // Create a new render target context as above but have it backed by a deferred-style
    // GrRenderTargetProxy rather than one that is backed by an actual GrRenderTarget
    sk_sp<GrRenderTargetContext> makeDeferredRenderTargetContext(
                                                 SkBackingFit fit, 
                                                 int width, int height,
                                                 GrPixelConfig config,
                                                 sk_sp<SkColorSpace> colorSpace,
                                                 int sampleCnt = 0,
                                                 GrSurfaceOrigin origin = kBottomLeft_GrSurfaceOrigin,
                                                 const SkSurfaceProps* surfaceProps = nullptr,
                                                 SkBudgeted = SkBudgeted::kYes);
    /*
     * This method will attempt to create a renderTargetContext that has, at least, the number of
     * channels and precision per channel as requested in 'config' (e.g., A8 and 888 can be
     * converted to 8888). It may also swizzle the channels (e.g., BGRA -> RGBA).
     * SRGB-ness will be preserved.
     */
    sk_sp<GrRenderTargetContext> makeRenderTargetContextWithFallback(
                                                 SkBackingFit fit,
                                                 int width, int height,
                                                 GrPixelConfig config,
                                                 sk_sp<SkColorSpace> colorSpace,
                                                 int sampleCnt = 0,
                                                 GrSurfaceOrigin origin = kBottomLeft_GrSurfaceOrigin,
                                                 const SkSurfaceProps* surfaceProps = nullptr,
                                                 SkBudgeted budgeted = SkBudgeted::kYes);

    // Create a new render target context as above but have it backed by a deferred-style
    // GrRenderTargetProxy rather than one that is backed by an actual GrRenderTarget
    sk_sp<GrRenderTargetContext> makeDeferredRenderTargetContextWithFallback(
                                                 SkBackingFit fit,
                                                 int width, int height,
                                                 GrPixelConfig config,
                                                 sk_sp<SkColorSpace> colorSpace,
                                                 int sampleCnt = 0,
                                                 GrSurfaceOrigin origin = kBottomLeft_GrSurfaceOrigin,
                                                 const SkSurfaceProps* surfaceProps = nullptr,
                                                 SkBudgeted budgeted = SkBudgeted::kYes);

    ///////////////////////////////////////////////////////////////////////////
    // Misc.

    /**
     * Call to ensure all drawing to the context has been issued to the
     * underlying 3D API.
     */
    void flush();

    /**
     * An ID associated with this context, guaranteed to be unique.
     */
    uint32_t uniqueID() { return fUniqueID; }

    ///////////////////////////////////////////////////////////////////////////
    // Functions intended for internal use only.
    GrGpu* getGpu() { return fGpu; }
    const GrGpu* getGpu() const { return fGpu; }
    GrAtlasGlyphCache* getAtlasGlyphCache() { return fAtlasGlyphCache; }
    GrTextBlobCache* getTextBlobCache() { return fTextBlobCache.get(); }
    bool abandoned() const;
    GrResourceProvider* resourceProvider() { return fResourceProvider; }
    const GrResourceProvider* resourceProvider() const { return fResourceProvider; }
    GrResourceCache* getResourceCache() { return fResourceCache; }

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
    sk_sp<SkImage> getFontAtlasImage_ForTesting(GrMaskFormat format);

    GrAuditTrail* getAuditTrail() { return &fAuditTrail; }

    /** This is only useful for debug purposes */
    SkDEBUGCODE(GrSingleOwner* debugSingleOwner() const { return &fSingleOwner; } )

    // Provides access to functions that aren't part of the public API.
    GrContextPriv contextPriv();
    const GrContextPriv contextPriv() const;

private:
    GrGpu*                                  fGpu;
    const GrCaps*                           fCaps;
    GrResourceCache*                        fResourceCache;
    GrResourceProvider*                     fResourceProvider;

    sk_sp<GrContextThreadSafeProxy>         fThreadSafeProxy;

    GrAtlasGlyphCache*                      fAtlasGlyphCache;
    std::unique_ptr<GrTextBlobCache>        fTextBlobCache;

    bool                                    fDisableGpuYUVConversion;
    bool                                    fDidTestPMConversions;
    int                                     fPMToUPMConversion;
    int                                     fUPMToPMConversion;

    // In debug builds we guard against improper thread handling
    // This guard is passed to the GrDrawingManager and, from there to all the
    // GrRenderTargetContexts.  It is also passed to the GrResourceProvider and SkGpuDevice.
    mutable GrSingleOwner                   fSingleOwner;

    struct CleanUpData {
        PFCleanUpFunc fFunc;
        void*         fInfo;
    };

    SkTDArray<CleanUpData>                  fCleanUpData;

    const uint32_t                          fUniqueID;

    std::unique_ptr<GrDrawingManager>       fDrawingManager;

    GrAuditTrail                            fAuditTrail;

    // TODO: have the GrClipStackClip use renderTargetContexts and rm this friending
    friend class GrContextPriv;

    GrContext(); // init must be called after the constructor.
    bool init(GrBackend, GrBackendContext, const GrContextOptions& options);

    void initMockContext();
    void initCommon(const GrContextOptions&);

    /**
     * These functions create premul <-> unpremul effects if it is possible to generate a pair
     * of effects that make a readToUPM->writeToPM->readToUPM cycle invariant. Otherwise, they
     * return NULL. They also can perform a swizzle as part of the draw.
     */
    sk_sp<GrFragmentProcessor> createPMToUPMEffect(sk_sp<GrFragmentProcessor>, GrPixelConfig);
    sk_sp<GrFragmentProcessor> createUPMToPMEffect(sk_sp<GrFragmentProcessor>, GrPixelConfig);
    /** Called before either of the above two functions to determine the appropriate fragment
        processors for conversions. */
    void testPMConversionsIfNecessary(uint32_t flags);
    /** Returns true if we've determined that createPMtoUPMEffect and createUPMToPMEffect will
        succeed for the passed in config. Otherwise we fall back to SW conversion. */
    bool validPMUPMConversionExists(GrPixelConfig) const;

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
private:
    GrContextThreadSafeProxy(sk_sp<const GrCaps> caps, uint32_t uniqueID)
        : fCaps(std::move(caps))
        , fContextUniqueID(uniqueID) {}

    sk_sp<const GrCaps> fCaps;
    uint32_t            fContextUniqueID;

    friend class GrContext;
    friend class SkImage;

    typedef SkRefCnt INHERITED;
};

#endif
