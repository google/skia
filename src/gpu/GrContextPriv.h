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

class GrBackendRenderTarget;
class GrOpMemoryPool;
class GrOnFlushCallbackObject;
class GrSemaphore;
class GrSurfaceProxy;
class GrTextureContext;

class SkDeferredDisplayList;

/** Class that adds methods to GrContext that are only intended for use internal to Skia.
    This class is purely a privileged window into GrContext. It should never have additional
    data members or virtual methods. */
class GrContextPriv {
public:
    /**
     * Create a GrContext without a resource cache
     */
    static sk_sp<GrContext> MakeDDL(const sk_sp<GrContextThreadSafeProxy>&);

    const GrCaps* caps() const { return fContext->fCaps.get(); }

    sk_sp<GrOpMemoryPool> refOpMemoryPool();
    GrOpMemoryPool* opMemoryPool();

    GrDrawingManager* drawingManager() { return fContext->fDrawingManager.get(); }

    sk_sp<GrSurfaceContext> makeWrappedSurfaceContext(sk_sp<GrSurfaceProxy>,
                                                      sk_sp<SkColorSpace> = nullptr,
                                                      const SkSurfaceProps* = nullptr);

    sk_sp<GrSurfaceContext> makeDeferredSurfaceContext(const GrSurfaceDesc&,
                                                       GrSurfaceOrigin,
                                                       GrMipMapped,
                                                       SkBackingFit,
                                                       SkBudgeted,
                                                       sk_sp<SkColorSpace> colorSpace = nullptr,
                                                       const SkSurfaceProps* = nullptr);

    sk_sp<GrTextureContext> makeBackendTextureContext(const GrBackendTexture& tex,
                                                      GrSurfaceOrigin origin,
                                                      sk_sp<SkColorSpace> colorSpace);

    sk_sp<GrRenderTargetContext> makeBackendTextureRenderTargetContext(
                                                         const GrBackendTexture& tex,
                                                         GrSurfaceOrigin origin,
                                                         int sampleCnt,
                                                         sk_sp<SkColorSpace> colorSpace,
                                                         const SkSurfaceProps* = nullptr);

    sk_sp<GrRenderTargetContext> makeBackendRenderTargetRenderTargetContext(
                                                              const GrBackendRenderTarget&,
                                                              GrSurfaceOrigin origin,
                                                              sk_sp<SkColorSpace> colorSpace,
                                                              const SkSurfaceProps* = nullptr);

    sk_sp<GrRenderTargetContext> makeBackendTextureAsRenderTargetRenderTargetContext(
                                                                 const GrBackendTexture& tex,
                                                                 GrSurfaceOrigin origin,
                                                                 int sampleCnt,
                                                                 sk_sp<SkColorSpace> colorSpace,
                                                                 const SkSurfaceProps* = nullptr);

    bool disableGpuYUVConversion() const { return fContext->fDisableGpuYUVConversion; }
    bool sharpenMipmappedTextures() const { return fContext->fSharpenMipmappedTextures; }

    /**
     * Call to ensure all drawing to the context has been issued to the
     * underlying 3D API.
     * The 'proxy' parameter is a hint. If it is supplied the context will guarantee that
     * the draws required for that proxy are flushed but it could do more. If no 'proxy' is
     * provided then all current work will be flushed.
     */
    void flush(GrSurfaceProxy*);

    /**
     * Registers an object for flush-related callbacks. (See GrOnFlushCallbackObject.)
     *
     * NOTE: the drawing manager tracks this object as a raw pointer; it is up to the caller to
     * ensure its lifetime is tied to that of the context.
     */
    void addOnFlushCallbackObject(GrOnFlushCallbackObject*);

    void testingOnly_flushAndRemoveOnFlushCallbackObject(GrOnFlushCallbackObject*);

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
        /** The GrContext will not be flushed before the surface read or write. This means that
            the read or write may occur before previous draws have executed. */
        kDontFlush_PixelOpsFlag = 0x1,
        /** Any surface writes should be flushed to the backend 3D API after the surface operation
            is complete */
        kFlushWrites_PixelOp = 0x2,
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

    GrBackend getBackend() const { return fContext->fBackend; }

    SkTaskGroup* getTaskGroup() { return fContext->fTaskGroup.get(); }

    GrProxyProvider* proxyProvider() { return fContext->fProxyProvider; }
    const GrProxyProvider* proxyProvider() const { return fContext->fProxyProvider; }

    GrResourceProvider* resourceProvider() { return fContext->fResourceProvider; }
    const GrResourceProvider* resourceProvider() const { return fContext->fResourceProvider; }

    GrResourceCache* getResourceCache() { return fContext->fResourceCache; }

    GrTextureStripAtlasManager* textureStripAtlasManager() {
        return fContext->fTextureStripAtlasManager.get();
    }

    GrGpu* getGpu() { return fContext->fGpu.get(); }
    const GrGpu* getGpu() const { return fContext->fGpu.get(); }

    GrGlyphCache* getGlyphCache() { return fContext->fGlyphCache; }
    GrTextBlobCache* getTextBlobCache() { return fContext->fTextBlobCache.get(); }

    // This accessor should only ever be called by the GrOpFlushState.
    GrAtlasManager* getAtlasManager() {
        return fContext->onGetAtlasManager();
    }

    void moveOpListsToDDL(SkDeferredDisplayList*);
    void copyOpListsFromDDL(const SkDeferredDisplayList*, GrRenderTargetProxy* newDest);

    /**
     * Purge all the unlocked resources from the cache.
     * This entry point is mainly meant for timing texture uploads
     * and is not defined in normal builds of Skia.
     */
    void purgeAllUnlockedResources_ForTesting();


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

    /** Get pointer to atlas texture for given mask format. Note that this wraps an
        actively mutating texture in an SkImage. This could yield unexpected results
        if it gets cached or used more generally. */
    sk_sp<SkImage> getFontAtlasImage_ForTesting(GrMaskFormat format, unsigned int index = 0);

    GrAuditTrail* getAuditTrail() { return &fContext->fAuditTrail; }

    GrContextOptions::PersistentCache* getPersistentCache() { return fContext->fPersistentCache; }

    /** This is only useful for debug purposes */
    SkDEBUGCODE(GrSingleOwner* debugSingleOwner() const { return &fContext->fSingleOwner; } )

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

inline GrContextPriv GrContext::contextPriv() { return GrContextPriv(this); }

inline const GrContextPriv GrContext::contextPriv () const {
    return GrContextPriv(const_cast<GrContext*>(this));
}

#endif
