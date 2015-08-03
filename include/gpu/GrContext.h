/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrContext_DEFINED
#define GrContext_DEFINED

#include "GrClip.h"
#include "GrColor.h"
#include "GrPaint.h"
#include "GrPathRendererChain.h"
#include "GrRenderTarget.h"
#include "GrTextureProvider.h"
#include "SkMatrix.h"
#include "SkPathEffect.h"
#include "SkTypes.h"

class GrAARectRenderer;
class GrBatchFontCache;
class GrCaps;
struct GrContextOptions;
class GrDrawContext;
class GrDrawTarget;
class GrFragmentProcessor;
class GrGpu;
class GrGpuTraceMarker;
class GrIndexBuffer;
class GrLayerCache;
class GrOvalRenderer;
class GrPath;
class GrPathRenderer;
class GrPipelineBuilder;
class GrResourceEntry;
class GrResourceCache;
class GrResourceProvider;
class GrTestTarget;
class GrTextBlobCache;
class GrTextContext;
class GrTextureParams;
class GrVertexBuffer;
class GrStrokeInfo;
class GrSoftwarePathRenderer;

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
     * Abandons all GPU resources and assumes the underlying backend 3D API
     * context is not longer usable. Call this if you have lost the associated
     * GPU context, and thus internal texture, buffer, etc. references/IDs are
     * now invalid. Should be called even when GrContext is no longer going to
     * be used for two reasons:
     *  1) ~GrContext will not try to free the objects in the 3D API.
     *  2) Any GrGpuResources created by this GrContext that outlive
     *     will be marked as invalid (GrGpuResource::wasDestroyed()) and
     *     when they're destroyed no 3D API calls will be made.
     * Content drawn since the last GrContext::flush() may be lost. After this
     * function is called the only valid action on the GrContext or
     * GrGpuResources it created is to destroy them.
     */
    void abandonContext();

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

    GrTextureProvider* textureProvider() { return fTextureProvider; }
    const GrTextureProvider* textureProvider() const { return fTextureProvider; }

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
     * Returns a helper object to orchestrate draws.
     * Callers should take a ref if they rely on the GrDrawContext sticking around.
     * NULL will be returned if the context has been abandoned.
     *
     * @param  surfaceProps the surface properties (mainly defines text drawing)
     *
     * @return a draw context
     */
    GrDrawContext* drawContext(const SkSurfaceProps* surfaceProps = NULL) {
        return fDrawingMgr.drawContext(surfaceProps);
    }

    ///////////////////////////////////////////////////////////////////////////
    // Misc.

    /**
     * Flags that affect flush() behavior.
     */
    enum FlushBits {
        /**
         * A client may reach a point where it has partially rendered a frame
         * through a GrContext that it knows the user will never see. This flag
         * causes the flush to skip submission of deferred content to the 3D API
         * during the flush.
         */
        kDiscard_FlushBit                    = 0x2,
    };

    /**
     * Call to ensure all drawing to the context has been issued to the
     * underlying 3D API.
     * @param flagsBitfield     flags that control the flushing behavior. See
     *                          FlushBits.
     */
    void flush(int flagsBitfield = 0);

    void flushIfNecessary() {
        if (fFlushToReduceCacheSize) {
            this->flush();
        }
    }

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
     * @param surface       the surface to read from.
     * @param left          left edge of the rectangle to read (inclusive)
     * @param top           top edge of the rectangle to read (inclusive)
     * @param width         width of rectangle to read in pixels.
     * @param height        height of rectangle to read in pixels.
     * @param config        the pixel config of the destination buffer
     * @param buffer        memory to read the rectangle into.
     * @param rowBytes      number of bytes bewtween consecutive rows. Zero means rows are tightly
     *                      packed.
     * @param pixelOpsFlags see PixelOpsFlags enum above.
     *
     * @return true if the read succeeded, false if not. The read can fail because of an unsupported
     *         pixel configs
     */
    bool readSurfacePixels(GrSurface* surface,
                           int left, int top, int width, int height,
                           GrPixelConfig config, void* buffer,
                           size_t rowBytes = 0,
                           uint32_t pixelOpsFlags = 0);

    /**
     * Writes a rectangle of pixels to a surface.
     * @param surface       the surface to write to.
     * @param left          left edge of the rectangle to write (inclusive)
     * @param top           top edge of the rectangle to write (inclusive)
     * @param width         width of rectangle to write in pixels.
     * @param height        height of rectangle to write in pixels.
     * @param config        the pixel config of the source buffer
     * @param buffer        memory to read pixels from
     * @param rowBytes      number of bytes between consecutive rows. Zero
     *                      means rows are tightly packed.
     * @param pixelOpsFlags see PixelOpsFlags enum above.
     * @return true if the write succeeded, false if not. The write can fail because of an
     *         unsupported combination of surface and src configs.
     */
    bool writeSurfacePixels(GrSurface* surface,
                            int left, int top, int width, int height,
                            GrPixelConfig config, const void* buffer,
                            size_t rowBytes,
                            uint32_t pixelOpsFlags = 0);

    /**
     * Copies a rectangle of texels from src to dst.
     * bounds.
     * @param dst           the surface to copy to.
     * @param src           the surface to copy from.
     * @param srcRect       the rectangle of the src that should be copied.
     * @param dstPoint      the translation applied when writing the srcRect's pixels to the dst.
     * @param pixelOpsFlags see PixelOpsFlags enum above. (kUnpremul_PixelOpsFlag is not allowed).
     */
    void copySurface(GrSurface* dst,
                     GrSurface* src,
                     const SkIRect& srcRect,
                     const SkIPoint& dstPoint,
                     uint32_t pixelOpsFlags = 0);

    /** Helper that copies the whole surface but fails when the two surfaces are not identically
        sized. */
    bool copySurface(GrSurface* dst, GrSurface* src) {
        if (NULL == dst || NULL == src || dst->width() != src->width() ||
            dst->height() != src->height()) {
            return false;
        }
        this->copySurface(dst, src, SkIRect::MakeWH(dst->width(), dst->height()),
                          SkIPoint::Make(0,0));
        return true;
    }

    /**
     * After this returns any pending writes to the surface will have been issued to the backend 3D API.
     */
    void flushSurfaceWrites(GrSurface* surface);

    /**
     * Finalizes all pending reads and writes to the surface and also performs an MSAA resolve
     * if necessary.
     *
     * It is not necessary to call this before reading the render target via Skia/GrContext.
     * GrContext will detect when it must perform a resolve before reading pixels back from the
     * surface or using it as a texture.
     */
    void prepareSurfaceForExternalIO(GrSurface*);

    /**
     * An ID associated with this context, guaranteed to be unique.
     */
    uint32_t uniqueID() { return fUniqueID; }

    ///////////////////////////////////////////////////////////////////////////
    // Functions intended for internal use only.
    GrGpu* getGpu() { return fGpu; }
    const GrGpu* getGpu() const { return fGpu; }
    GrBatchFontCache* getBatchFontCache() { return fBatchFontCache; }
    GrLayerCache* getLayerCache() { return fLayerCache.get(); }
    GrTextBlobCache* getTextBlobCache() { return fTextBlobCache; }
    bool abandoned() const { return fDrawingMgr.abandoned(); }
    GrResourceProvider* resourceProvider() { return fResourceProvider; }
    const GrResourceProvider* resourceProvider() const { return fResourceProvider; }
    GrResourceCache* getResourceCache() { return fResourceCache; }

    // Called by tests that draw directly to the context via GrDrawTarget
    void getTestTarget(GrTestTarget*);

    void addGpuTraceMarker(const GrGpuTraceMarker* marker);
    void removeGpuTraceMarker(const GrGpuTraceMarker* marker);

    GrPathRenderer* getPathRenderer(
                    const GrDrawTarget* target,
                    const GrPipelineBuilder*,
                    const SkMatrix& viewMatrix,
                    const SkPath& path,
                    const GrStrokeInfo& stroke,
                    bool allowSW,
                    GrPathRendererChain::DrawType drawType = GrPathRendererChain::kColor_DrawType,
                    GrPathRendererChain::StencilSupport* stencilSupport = NULL);

    /** Prints cache stats to the string if GR_CACHE_STATS == 1. */
    void dumpCacheStats(SkString*) const;
    void printCacheStats() const;

    /** Prints GPU stats to the string if GR_GPU_STATS == 1. */
    void dumpGpuStats(SkString*) const;
    void printGpuStats() const;

    /** Specify the TextBlob cache limit. If the current cache exceeds this limit it will purge.
        this is for testing only */
    void setTextBlobCacheLimit_ForTesting(size_t bytes);

private:
    GrGpu*                          fGpu;
    const GrCaps*                   fCaps;
    GrResourceCache*                fResourceCache;
    // this union exists because the inheritance of GrTextureProvider->GrResourceProvider
    // is in a private header.
    union {
        GrResourceProvider*         fResourceProvider;
        GrTextureProvider*          fTextureProvider;
    };

    GrBatchFontCache*               fBatchFontCache;
    SkAutoTDelete<GrLayerCache>     fLayerCache;
    SkAutoTDelete<GrTextBlobCache>  fTextBlobCache;

    GrPathRendererChain*            fPathRendererChain;
    GrSoftwarePathRenderer*         fSoftwarePathRenderer;

    // Set by OverbudgetCB() to request that GrContext flush before exiting a draw.
    bool                            fFlushToReduceCacheSize;
    bool                            fDidTestPMConversions;
    int                             fPMToUPMConversion;
    int                             fUPMToPMConversion;

    struct CleanUpData {
        PFCleanUpFunc fFunc;
        void*         fInfo;
    };

    SkTDArray<CleanUpData>          fCleanUpData;

    const uint32_t                  fUniqueID;

    GrContext(); // init must be called after the constructor.
    bool init(GrBackend, GrBackendContext, const GrContextOptions& options);

    // Currently the DrawingMgr stores a separate GrDrawContext for each
    // combination of text drawing options (pixel geometry x DFT use)
    // and hands the appropriate one back given the user's request.
    // All of the GrDrawContexts still land in the same GrDrawTarget!
    //
    // In the future this class will allocate a new GrDrawContext for
    // each GrRenderTarget/GrDrawTarget and manage the DAG.
    class DrawingMgr {
    public:
        DrawingMgr() : fDrawTarget(NULL) {
            sk_bzero(fDrawContext, sizeof(fDrawContext));
        }

        ~DrawingMgr();

        void init(GrContext* context);

        void abandon();
        bool abandoned() const { return NULL == fDrawTarget; }

        void purgeResources();
        void reset();
        void flush();

        // Callers should take a ref if they rely on the GrDrawContext sticking around.
        // NULL will be returned if the context has been abandoned.
        GrDrawContext* drawContext(const SkSurfaceProps* surfaceProps);

    private:
        void cleanup();

        friend class GrContext;  // for access to fDrawTarget for testing

        static const int kNumPixelGeometries = 5; // The different pixel geometries
        static const int kNumDFTOptions = 2;      // DFT or no DFT

        GrContext*        fContext;
        GrDrawTarget*     fDrawTarget;

        GrDrawContext*    fDrawContext[kNumPixelGeometries][kNumDFTOptions];
    };

    DrawingMgr                      fDrawingMgr;

    void initMockContext();
    void initCommon();

    /**
     * These functions create premul <-> unpremul effects if it is possible to generate a pair
     * of effects that make a readToUPM->writeToPM->readToUPM cycle invariant. Otherwise, they
     * return NULL.
     */
    const GrFragmentProcessor* createPMToUPMEffect(GrProcessorDataManager*, GrTexture*,
                                                   bool swapRAndB, const SkMatrix&);
    const GrFragmentProcessor* createUPMToPMEffect(GrProcessorDataManager*, GrTexture*,
                                                   bool swapRAndB, const SkMatrix&);
    /** Returns true if we've already determined that createPMtoUPMEffect and createUPMToPMEffect
        will fail. In such cases fall back to SW conversion. */
    bool didFailPMUPMConversionTest() const;

    /**
     *  This callback allows the resource cache to callback into the GrContext
     *  when the cache is still over budget after a purge.
     */
    static void OverBudgetCB(void* data);

    /**
     * A callback similar to the above for use by the TextBlobCache
     * TODO move textblob draw calls below context so we can use the call above.
     */
    static void TextBlobCacheOverBudgetCB(void* data);

    typedef SkRefCnt INHERITED;
};

#endif
