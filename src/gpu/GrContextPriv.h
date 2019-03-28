/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrContextPriv_DEFINED
#define GrContextPriv_DEFINED

#include "GrContext.h"
#include "GrSurfaceContext.h"
#include "text/GrAtlasManager.h"

class GrBackendFormat;
class GrBackendRenderTarget;
class GrOpMemoryPool;
class GrOnFlushCallbackObject;
class GrSemaphore;
class GrSkSLFPFactory;
class GrSkSLFPFactoryCache;
class GrSurfaceProxy;
class GrTextureContext;

class SkDeferredDisplayList;
class SkTaskGroup;

/** Class that adds methods to GrContext that are only intended for use internal to Skia.
    This class is purely a privileged window into GrContext. It should never have additional
    data members or virtual methods. */
class GrContextPriv {
public:

    // from GrContext_Base
    uint32_t contextID() const { return fContext->contextID(); }

    bool matches(GrContext_Base* candidate) const { return fContext->matches(candidate); }

    const GrContextOptions& options() const { return fContext->options(); }

    bool explicitlyAllocateGPUResources() const {
        return fContext->explicitlyAllocateGPUResources();
    }

    const GrCaps* caps() const { return fContext->caps(); }
    sk_sp<const GrCaps> refCaps() const;

    sk_sp<GrSkSLFPFactoryCache> fpFactoryCache();

    GrImageContext* asImageContext() { return fContext->asImageContext(); }
    GrRecordingContext* asRecordingContext() { return fContext->asRecordingContext(); }
    GrContext* asDirectContext() { return fContext->asDirectContext(); }

    // from GrImageContext
    GrProxyProvider* proxyProvider() { return fContext->proxyProvider(); }
    const GrProxyProvider* proxyProvider() const { return fContext->proxyProvider(); }

    bool abandoned() const { return fContext->abandoned(); }

    /** This is only useful for debug purposes */
    SkDEBUGCODE(GrSingleOwner* singleOwner() const { return fContext->singleOwner(); } )

    // from GrRecordingContext
    GrDrawingManager* drawingManager() { return fContext->drawingManager(); }

    sk_sp<GrOpMemoryPool> refOpMemoryPool();
    GrOpMemoryPool* opMemoryPool() { return fContext->opMemoryPool(); }

    GrStrikeCache* getGrStrikeCache() { return fContext->getGrStrikeCache(); }
    GrTextBlobCache* getTextBlobCache() { return fContext->getTextBlobCache(); }

    /**
     * Registers an object for flush-related callbacks. (See GrOnFlushCallbackObject.)
     *
     * NOTE: the drawing manager tracks this object as a raw pointer; it is up to the caller to
     * ensure its lifetime is tied to that of the context.
     */
    void addOnFlushCallbackObject(GrOnFlushCallbackObject*);

    sk_sp<GrSurfaceContext> makeWrappedSurfaceContext(sk_sp<GrSurfaceProxy>,
                                                      sk_sp<SkColorSpace> = nullptr,
                                                      const SkSurfaceProps* = nullptr);

    sk_sp<GrSurfaceContext> makeDeferredSurfaceContext(const GrBackendFormat&,
                                                       const GrSurfaceDesc&,
                                                       GrSurfaceOrigin,
                                                       GrMipMapped,
                                                       SkBackingFit,
                                                       SkBudgeted,
                                                       sk_sp<SkColorSpace> colorSpace = nullptr,
                                                       const SkSurfaceProps* = nullptr);

    /*
     * Create a new render target context backed by a deferred-style
     * GrRenderTargetProxy. We guarantee that "asTextureProxy" will succeed for
     * renderTargetContexts created via this entry point.
     */
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

    /*
     * This method will attempt to create a renderTargetContext that has, at least, the number of
     * channels and precision per channel as requested in 'config' (e.g., A8 and 888 can be
     * converted to 8888). It may also swizzle the channels (e.g., BGRA -> RGBA).
     * SRGB-ness will be preserved.
     */
    sk_sp<GrRenderTargetContext> makeDeferredRenderTargetContextWithFallback(
                                            const GrBackendFormat& format,
                                            SkBackingFit fit,
                                            int width, int height,
                                            GrPixelConfig config,
                                            sk_sp<SkColorSpace> colorSpace,
                                            int sampleCnt = 1,
                                            GrMipMapped = GrMipMapped::kNo,
                                            GrSurfaceOrigin origin = kBottomLeft_GrSurfaceOrigin,
                                            const SkSurfaceProps* surfaceProps = nullptr,
                                            SkBudgeted budgeted = SkBudgeted::kYes);

    GrAuditTrail* auditTrail() { return fContext->auditTrail(); }

    /**
     * Create a GrContext without a resource cache
     */
    static sk_sp<GrContext> MakeDDL(const sk_sp<GrContextThreadSafeProxy>&);

    sk_sp<GrTextureContext> makeBackendTextureContext(const GrBackendTexture& tex,
                                                      GrSurfaceOrigin origin,
                                                      sk_sp<SkColorSpace> colorSpace);

    // These match the definitions in SkSurface & GrSurface.h, for whence they came
    typedef void* ReleaseContext;
    typedef void (*ReleaseProc)(ReleaseContext);

    sk_sp<GrRenderTargetContext> makeBackendTextureRenderTargetContext(
                                                         const GrBackendTexture& tex,
                                                         GrSurfaceOrigin origin,
                                                         int sampleCnt,
                                                         sk_sp<SkColorSpace> colorSpace,
                                                         const SkSurfaceProps* = nullptr,
                                                         ReleaseProc = nullptr,
                                                         ReleaseContext = nullptr);

    sk_sp<GrRenderTargetContext> makeBackendRenderTargetRenderTargetContext(
                                                              const GrBackendRenderTarget&,
                                                              GrSurfaceOrigin origin,
                                                              sk_sp<SkColorSpace> colorSpace,
                                                              const SkSurfaceProps* = nullptr,
                                                              ReleaseProc = nullptr,
                                                              ReleaseContext = nullptr);

    sk_sp<GrRenderTargetContext> makeBackendTextureAsRenderTargetRenderTargetContext(
                                                                 const GrBackendTexture& tex,
                                                                 GrSurfaceOrigin origin,
                                                                 int sampleCnt,
                                                                 sk_sp<SkColorSpace> colorSpace,
                                                                 const SkSurfaceProps* = nullptr);

    sk_sp<GrRenderTargetContext> makeVulkanSecondaryCBRenderTargetContext(
            const SkImageInfo&, const GrVkDrawableInfo&, const SkSurfaceProps* = nullptr);

    /**
     * Call to ensure all drawing to the context has been issued to the
     * underlying 3D API.
     * The 'proxy' parameter is a hint. If it is supplied the context will guarantee that
     * the draws required for that proxy are flushed but it could do more. If no 'proxy' is
     * provided then all current work will be flushed.
     */
    void flush(GrSurfaceProxy*);

    /**
     * After this returns any pending writes to the surface will have been issued to the
     * backend 3D API.
     */
    void flushSurfaceWrites(GrSurfaceProxy*);

    /**
     * After this returns any pending reads or writes to the surface will have been issued to the
     * backend 3D API.
     */
    void flushSurfaceIO(GrSurfaceProxy*);

    /**
     * Finalizes all pending reads and writes to the surface and also performs an MSAA resolve
     * if necessary.
     *
     * It is not necessary to call this before reading the render target via Skia/GrContext.
     * GrContext will detect when it must perform a resolve before reading pixels back from the
     * surface or using it as a texture.
     */
    void prepareSurfaceForExternalIO(GrSurfaceProxy*);

   /**
    * These flags can be used with the read/write pixels functions below.
    */
    enum PixelOpsFlags {
        /** The src for write or dst read is unpremultiplied. This is only respected if both the
            config src and dst configs are an RGBA/BGRA 8888 format. */
        kUnpremul_PixelOpsFlag  = 0x4,
    };

    /**
     * Reads a rectangle of pixels from a surface.
     *
     * @param src           the surface context to read from.
     * @param left          left edge of the rectangle to read (inclusive)
     * @param top           top edge of the rectangle to read (inclusive)
     * @param width         width of rectangle to read in pixels.
     * @param height        height of rectangle to read in pixels.
     * @param dstColorType  the color type of the destination buffer
     * @param dstColorSpace color space of the destination buffer
     * @param buffer        memory to read the rectangle into.
     * @param rowBytes      number of bytes bewtween consecutive rows. Zero means rows are tightly
     *                      packed.
     * @param pixelOpsFlags see PixelOpsFlags enum above.
     *
     * @return true if the read succeeded, false if not. The read can fail because of an unsupported
     *         pixel configs
     */
    bool readSurfacePixels(GrSurfaceContext* src, int left, int top, int width, int height,
                           GrColorType dstColorType, SkColorSpace* dstColorSpace, void* buffer,
                           size_t rowBytes = 0, uint32_t pixelOpsFlags = 0);

    /**
     * Writes a rectangle of pixels to a surface.
     *
     * @param dst           the surface context to write to.
     * @param left          left edge of the rectangle to write (inclusive)
     * @param top           top edge of the rectangle to write (inclusive)
     * @param width         width of rectangle to write in pixels.
     * @param height        height of rectangle to write in pixels.
     * @param srcColorType  the color type of the source buffer
     * @param srcColorSpace color space of the source buffer
     * @param buffer        memory to read pixels from
     * @param rowBytes      number of bytes between consecutive rows. Zero
     *                      means rows are tightly packed.
     * @param pixelOpsFlags see PixelOpsFlags enum above.
     * @return true if the write succeeded, false if not. The write can fail because of an
     *         unsupported combination of surface and src configs.
     */
    bool writeSurfacePixels(GrSurfaceContext* dst, int left, int top, int width, int height,
                            GrColorType srcColorType, SkColorSpace* srcColorSpace,
                            const void* buffer, size_t rowBytes, uint32_t pixelOpsFlags = 0);

    SkTaskGroup* getTaskGroup() { return fContext->fTaskGroup.get(); }

    GrResourceProvider* resourceProvider() { return fContext->fResourceProvider; }
    const GrResourceProvider* resourceProvider() const { return fContext->fResourceProvider; }

    GrResourceCache* getResourceCache() { return fContext->fResourceCache; }

    GrGpu* getGpu() { return fContext->fGpu.get(); }
    const GrGpu* getGpu() const { return fContext->fGpu.get(); }

    // This accessor should only ever be called by the GrOpFlushState.
    GrAtlasManager* getAtlasManager() {
        return fContext->onGetAtlasManager();
    }

    void moveOpListsToDDL(SkDeferredDisplayList*);
    void copyOpListsFromDDL(const SkDeferredDisplayList*, GrRenderTargetProxy* newDest);

    GrContextOptions::PersistentCache* getPersistentCache() { return fContext->fPersistentCache; }

#ifdef SK_ENABLE_DUMP_GPU
    /** Returns a string with detailed information about the context & GPU, in JSON format. */
    SkString dump() const;
#endif

#if GR_TEST_UTILS
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
    void testingOnly_setTextBlobCacheLimit(size_t bytes);

    /** Get pointer to atlas texture for given mask format. Note that this wraps an
        actively mutating texture in an SkImage. This could yield unexpected results
        if it gets cached or used more generally. */
    sk_sp<SkImage> testingOnly_getFontAtlasImage(GrMaskFormat format, unsigned int index = 0);

    /**
     * Purge all the unlocked resources from the cache.
     * This entry point is mainly meant for timing texture uploads
     * and is not defined in normal builds of Skia.
     */
    void testingOnly_purgeAllUnlockedResources();

    void testingOnly_flushAndRemoveOnFlushCallbackObject(GrOnFlushCallbackObject*);
#endif

private:
    explicit GrContextPriv(GrContext* context) : fContext(context) {}
    GrContextPriv(const GrContextPriv&); // unimpl
    GrContextPriv& operator=(const GrContextPriv&); // unimpl

    // No taking addresses of this type.
    const GrContextPriv* operator&() const;
    GrContextPriv* operator&();

    GrContext* fContext;

    friend class GrContext; // to construct/copy this type.
};

inline GrContextPriv GrContext::priv() { return GrContextPriv(this); }

inline const GrContextPriv GrContext::priv() const {
    return GrContextPriv(const_cast<GrContext*>(this));
}

#endif
