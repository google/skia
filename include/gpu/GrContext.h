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
#include "GrTexture.h"
#include "SkMatrix.h"
#include "SkPathEffect.h"
#include "SkTypes.h"

class GrAARectRenderer;
class GrDrawTarget;
class GrFontCache;
class GrFragmentProcessor;
class GrGpu;
class GrGpuTraceMarker;
class GrIndexBuffer;
class GrIndexBufferAllocPool;
class GrInOrderDrawBuffer;
class GrLayerCache;
class GrOvalRenderer;
class GrPath;
class GrPathRenderer;
class GrPipelineBuilder;
class GrResourceEntry;
class GrResourceCache;
class GrTestTarget;
class GrTextContext;
class GrTextureParams;
class GrVertexBuffer;
class GrVertexBufferAllocPool;
class GrStrokeInfo;
class GrSoftwarePathRenderer;
class SkStrokeRec;

class SK_API GrContext : public SkRefCnt {
public:
    SK_DECLARE_INST_COUNT(GrContext)

    struct Options {
        Options() : fDrawPathToCompressedTexture(false) { }

        // EXPERIMENTAL
        // May be removed in the future, or may become standard depending
        // on the outcomes of a variety of internal tests.
        bool fDrawPathToCompressedTexture;
    };

    /**
     * Creates a GrContext for a backend context.
     */
    static GrContext* Create(GrBackend, GrBackendContext, const Options* opts = NULL);

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
    void contextDestroyed() { this->abandonContext(); }  //  legacy alias

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
     * This method should be called whenever a GrResource is unreffed or
     * switched from exclusive to non-exclusive. This
     * gives the resource cache a chance to discard unneeded resources.
     * Note: this entry point will be removed once totally ref-driven
     * cache maintenance is implemented.
     */
    void purgeCache();

    /**
     * Purge all the unlocked resources from the cache.
     * This entry point is mainly meant for timing texture uploads
     * and is not defined in normal builds of Skia.
     */
    void purgeAllUnlockedResources();

    /**
     * Sets a unique key on the resource. Upon key collision this resource takes the place of the
     * previous resource that had the key.
     */
    void addResourceToCache(const GrUniqueKey&, GrGpuResource*);

    /**
     * Finds a resource in the cache, based on the specified key. This is intended for use in
     * conjunction with addResourceToCache(). The return value will be NULL if not found. The
     * caller must balance with a call to unref().
     */
    GrGpuResource* findAndRefCachedResource(const GrUniqueKey&);

    /** Helper for casting resource to a texture. Caller must be sure that the resource cached
        with the key is either NULL or a texture and not another resource type. */
    GrTexture* findAndRefCachedTexture(const GrUniqueKey& key) {
        GrGpuResource* resource = this->findAndRefCachedResource(key);
        if (resource) {
            GrTexture* texture = static_cast<GrSurface*>(resource)->asTexture();
            SkASSERT(texture);
            return texture;
        }
        return NULL;
    }

    /**
     * Determines whether a resource is in the cache. If the resource is found it
     * will not be locked or returned. This call does not affect the priority of
     * the resource for deletion.
     */
    bool isResourceInCache(const GrUniqueKey& key) const;

    /**
     * Creates a new text rendering context that is optimal for the
     * render target and the context. Caller assumes the ownership
     * of the returned object. The returned object must be deleted
     * before the context is destroyed.
     */
    GrTextContext* createTextContext(GrRenderTarget*,
                                     const SkDeviceProperties&,
                                     bool enableDistanceFieldFonts);

    ///////////////////////////////////////////////////////////////////////////
    // Textures

    /**
     * Creates a new texture in the resource cache and returns it. The caller owns a
     * ref on the returned texture which must be balanced by a call to unref.
     *
     * @param desc      Description of the texture properties.
     * @param budgeted  Does the texture count against the resource cache budget?
     * @param srcData   Pointer to the pixel values (optional).
     * @param rowBytes  The number of bytes between rows of the texture. Zero
     *                  implies tightly packed rows. For compressed pixel configs, this
     *                  field is ignored.
     */
    GrTexture* createTexture(const GrSurfaceDesc& desc, bool budgeted, const void* srcData,
                             size_t rowBytes);

    GrTexture* createTexture(const GrSurfaceDesc& desc, bool budgeted) {
        return this->createTexture(desc, budgeted, NULL, 0);
    }

    /**
     * DEPRECATED: use createTexture().
     */
    GrTexture* createUncachedTexture(const GrSurfaceDesc& desc, void* srcData, size_t rowBytes) {
        return this->createTexture(desc, false, srcData, rowBytes);
    }

    /**
     * Enum that determines how closely a returned scratch texture must match
     * a provided GrSurfaceDesc. TODO: Remove this. createTexture() should be used
     * for exact match and refScratchTexture() should be replaced with createApproxTexture().
     */
    enum ScratchTexMatch {
        /**
         * Finds a texture that exactly matches the descriptor.
         */
        kExact_ScratchTexMatch,
        /**
         * Finds a texture that approximately matches the descriptor. Will be
         * at least as large in width and height as desc specifies. If desc
         * specifies that texture is a render target then result will be a
         * render target. If desc specifies a render target and doesn't set the
         * no stencil flag then result will have a stencil. Format and aa level
         * will always match.
         */
        kApprox_ScratchTexMatch
    };

    /**
     * Returns a texture matching the desc. It's contents are unknown. The caller
     * owns a ref on the returned texture and must balance with a call to unref.
     * It is guaranteed that the same texture will not be returned in subsequent
     * calls until all refs to the texture are dropped.
     *
     * Textures created by createTexture() hide the complications of
     * tiling non-power-of-two textures on APIs that don't support this (e.g.
     * unextended GLES2). NPOT scratch textures are not tilable on such APIs.
     *
     * internalFlag is a temporary workaround until changes in the internal
     * architecture are complete. Use the default value.
     *
     * TODO: Once internal flag can be removed, this should be replaced with
     * createApproxTexture() and exact textures should be created with
     * createTexture().
     */
    GrTexture* refScratchTexture(const GrSurfaceDesc&, ScratchTexMatch match,
                                 bool internalFlag = false);

    /**
     * Can the provided configuration act as a texture?
     */
    bool isConfigTexturable(GrPixelConfig) const;

    /**
     * Can non-power-of-two textures be used with tile modes other than clamp?
     */
    bool npotTextureTileSupport() const;

    /**
     *  Return the max width or height of a texture supported by the current GPU.
     */
    int getMaxTextureSize() const;

    /**
     *  Temporarily override the true max texture size. Note: an override
     *  larger then the true max texture size will have no effect.
     *  This entry point is mainly meant for testing texture size dependent
     *  features and is only available if defined outside of Skia (see
     *  bleed GM.
     */
    void setMaxTextureSizeOverride(int maxTextureSizeOverride);

    /**
     * Can the provided configuration act as a color render target?
     */
    bool isConfigRenderable(GrPixelConfig config, bool withMSAA) const;

    /**
     * Return the max width or height of a render target supported by the
     * current GPU.
     */
    int getMaxRenderTargetSize() const;

    /**
     * Returns the max sample count for a render target. It will be 0 if MSAA
     * is not supported.
     */
    int getMaxSampleCount() const;

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

    ///////////////////////////////////////////////////////////////////////////
    // Backend Surfaces

    /**
     * Wraps an existing texture with a GrTexture object.
     *
     * OpenGL: if the object is a texture Gr may change its GL texture params
     *         when it is drawn.
     *
     * @param  desc     description of the object to create.
     *
     * @return GrTexture object or NULL on failure.
     */
    GrTexture* wrapBackendTexture(const GrBackendTextureDesc& desc);

    /**
     * Wraps an existing render target with a GrRenderTarget object. It is
     * similar to wrapBackendTexture but can be used to draw into surfaces
     * that are not also textures (e.g. FBO 0 in OpenGL, or an MSAA buffer that
     * the client will resolve to a texture).
     *
     * @param  desc     description of the object to create.
     *
     * @return GrTexture object or NULL on failure.
     */
     GrRenderTarget* wrapBackendRenderTarget(const GrBackendRenderTargetDesc& desc);

    ///////////////////////////////////////////////////////////////////////////
    // Draws

    /**
     * Clear the entire or rect of the render target, ignoring any clips.
     * @param rect  the rect to clear or the whole thing if rect is NULL.
     * @param color the color to clear to.
     * @param canIgnoreRect allows partial clears to be converted to whole
     *                      clears on platforms for which that is cheap
     * @param target The render target to clear.
     */
    void clear(const SkIRect* rect, GrColor color, bool canIgnoreRect, GrRenderTarget* target);

    /**
     *  Draw everywhere (respecting the clip) with the paint.
     */
    void drawPaint(GrRenderTarget*, const GrClip&, const GrPaint&, const SkMatrix& viewMatrix);

    /**
     *  Draw the rect using a paint.
     *  @param paint        describes how to color pixels.
     *  @param viewMatrix   transformation matrix
     *  @param strokeInfo   the stroke information (width, join, cap), and.
     *                      the dash information (intervals, count, phase).
     *                      If strokeInfo == NULL, then the rect is filled.
     *                      Otherwise, if stroke width == 0, then the stroke
     *                      is always a single pixel thick, else the rect is
     *                      mitered/beveled stroked based on stroke width.
     *  The rects coords are used to access the paint (through texture matrix)
     */
    void drawRect(GrRenderTarget*,
                  const GrClip&,
                  const GrPaint& paint,
                  const SkMatrix& viewMatrix,
                  const SkRect&,
                  const GrStrokeInfo* strokeInfo = NULL);

    /**
     * Maps a rectangle of shader coordinates to a rectangle and draws that rectangle
     *
     * @param paint         describes how to color pixels.
     * @param viewMatrix    transformation matrix which applies to rectToDraw
     * @param rectToDraw    the rectangle to draw
     * @param localRect     the rectangle of shader coordinates applied to rectToDraw
     * @param localMatrix   an optional matrix to transform the shader coordinates before applying
     *                      to rectToDraw
     */
    void drawNonAARectToRect(GrRenderTarget*,
                             const GrClip&,
                             const GrPaint& paint,
                             const SkMatrix& viewMatrix,
                             const SkRect& rectToDraw,
                             const SkRect& localRect,
                             const SkMatrix* localMatrix = NULL);

    /**
     * Draws a non-AA rect with paint and a localMatrix
     */
    void drawNonAARectWithLocalMatrix(GrRenderTarget* rt,
                                      const GrClip& clip,
                                      const GrPaint& paint,
                                      const SkMatrix& viewMatrix,
                                      const SkRect& rect,
                                      const SkMatrix& localMatrix) {
        this->drawNonAARectToRect(rt, clip, paint, viewMatrix, rect, rect, &localMatrix);
    }

    /**
     *  Draw a roundrect using a paint.
     *
     *  @param paint        describes how to color pixels.
     *  @param viewMatrix   transformation matrix
     *  @param rrect        the roundrect to draw
     *  @param strokeInfo   the stroke information (width, join, cap) and
     *                      the dash information (intervals, count, phase).
     */
    void drawRRect(GrRenderTarget*,
                   const GrClip&,
                   const GrPaint&,
                   const SkMatrix& viewMatrix,
                   const SkRRect& rrect,
                   const GrStrokeInfo&);

    /**
     *  Shortcut for drawing an SkPath consisting of nested rrects using a paint.
     *  Does not support stroking. The result is undefined if outer does not contain
     *  inner.
     *
     *  @param paint        describes how to color pixels.
     *  @param viewMatrix   transformation matrix
     *  @param outer        the outer roundrect
     *  @param inner        the inner roundrect
     */
    void drawDRRect(GrRenderTarget*,
                    const GrClip&,
                    const GrPaint&,
                    const SkMatrix& viewMatrix,
                    const SkRRect& outer,
                    const SkRRect& inner);


    /**
     * Draws a path.
     *
     * @param paint         describes how to color pixels.
     * @param viewMatrix    transformation matrix
     * @param path          the path to draw
     * @param strokeInfo    the stroke information (width, join, cap) and
     *                      the dash information (intervals, count, phase).
     */
    void drawPath(GrRenderTarget*,
                  const GrClip&,
                  const GrPaint&,
                  const SkMatrix& viewMatrix,
                  const SkPath&,
                  const GrStrokeInfo&);

    /**
     * Draws vertices with a paint.
     *
     * @param   paint           describes how to color pixels.
     * @param   viewMatrix      transformation matrix
     * @param   primitiveType   primitives type to draw.
     * @param   vertexCount     number of vertices.
     * @param   positions       array of vertex positions, required.
     * @param   texCoords       optional array of texture coordinates used
     *                          to access the paint.
     * @param   colors          optional array of per-vertex colors, supercedes
     *                          the paint's color field.
     * @param   indices         optional array of indices. If NULL vertices
     *                          are drawn non-indexed.
     * @param   indexCount      if indices is non-null then this is the
     *                          number of indices.
     */
    void drawVertices(GrRenderTarget*,
                      const GrClip&,
                      const GrPaint& paint,
                      const SkMatrix& viewMatrix,
                      GrPrimitiveType primitiveType,
                      int vertexCount,
                      const SkPoint positions[],
                      const SkPoint texs[],
                      const GrColor colors[],
                      const uint16_t indices[],
                      int indexCount);

    /**
     * Draws an oval.
     *
     * @param paint         describes how to color pixels.
     * @param viewMatrix    transformation matrix
     * @param oval          the bounding rect of the oval.
     * @param strokeInfo    the stroke information (width, join, cap) and
     *                      the dash information (intervals, count, phase).
     */
    void drawOval(GrRenderTarget*,
                  const GrClip&,
                  const GrPaint& paint,
                  const SkMatrix& viewMatrix,
                  const SkRect& oval,
                  const GrStrokeInfo& strokeInfo);

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
     * Reads a rectangle of pixels from a render target.
     * @param target        the render target to read from.
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
     *         pixel config or because no render target is currently set and NULL was passed for
     *         target.
     */
    bool readRenderTargetPixels(GrRenderTarget* target,
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
     * Equivalent to flushSurfaceWrites but also performs MSAA resolve if necessary. This call is
     * used to make the surface contents available to be read in the backend 3D API, usually for a
     * compositing step external to Skia.
     *
     * It is not necessary to call this before reading the render target via Skia/GrContext.
     * GrContext will detect when it must perform a resolve before reading pixels back from the
     * surface or using it as a texture.
     */
    void prepareSurfaceForExternalRead(GrSurface*);

    /**
     * Provides a perfomance hint that the render target's contents are allowed
     * to become undefined.
     */
    void discardRenderTarget(GrRenderTarget*);

#ifdef SK_DEVELOPER
    void dumpFontCache() const;
#endif

    ///////////////////////////////////////////////////////////////////////////
    // Functions intended for internal use only.
    GrGpu* getGpu() { return fGpu; }
    const GrGpu* getGpu() const { return fGpu; }
    GrFontCache* getFontCache() { return fFontCache; }
    GrLayerCache* getLayerCache() { return fLayerCache.get(); }
    GrDrawTarget* getTextTarget();
    const GrIndexBuffer* getQuadIndexBuffer() const;
    GrAARectRenderer* getAARectRenderer() { return fAARectRenderer; }
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
                    const SkStrokeRec& stroke,
                    bool allowSW,
                    GrPathRendererChain::DrawType drawType = GrPathRendererChain::kColor_DrawType,
                    GrPathRendererChain::StencilSupport* stencilSupport = NULL);

    /**
     *  This returns a copy of the the GrContext::Options that was passed to the
     *  constructor of this class.
     */
    const Options& getOptions() const { return fOptions; }

    /** Prints cache stats to the string if GR_CACHE_STATS == 1. */
    void dumpCacheStats(SkString*) const;
    void printCacheStats() const;

    /** Prints GPU stats to the string if GR_GPU_STATS == 1. */
    void dumpGpuStats(SkString*) const;
    void printGpuStats() const;

private:
    GrGpu*                          fGpu;

    GrResourceCache*                fResourceCache;
    GrFontCache*                    fFontCache;
    SkAutoTDelete<GrLayerCache>     fLayerCache;

    GrPathRendererChain*            fPathRendererChain;
    GrSoftwarePathRenderer*         fSoftwarePathRenderer;

    GrVertexBufferAllocPool*        fDrawBufferVBAllocPool;
    GrIndexBufferAllocPool*         fDrawBufferIBAllocPool;
    GrInOrderDrawBuffer*            fDrawBuffer;

    // Set by OverbudgetCB() to request that GrContext flush before exiting a draw.
    bool                            fFlushToReduceCacheSize;
    GrAARectRenderer*               fAARectRenderer;
    GrOvalRenderer*                 fOvalRenderer;

    bool                            fDidTestPMConversions;
    int                             fPMToUPMConversion;
    int                             fUPMToPMConversion;

    struct CleanUpData {
        PFCleanUpFunc fFunc;
        void*         fInfo;
    };

    SkTDArray<CleanUpData>          fCleanUpData;

    int                             fMaxTextureSizeOverride;

    const Options                   fOptions;

    GrContext(const Options&); // init must be called after the constructor.
    bool init(GrBackend, GrBackendContext);
    void initMockContext();
    void initCommon();

    void setupDrawBuffer();

    class AutoCheckFlush;
    // Sets the paint and returns the target to draw into.
    GrDrawTarget* prepareToDraw(GrPipelineBuilder*,
                                GrRenderTarget* rt,
                                const GrClip&,
                                const GrPaint* paint,
                                const AutoCheckFlush*);

    // A simpler version of the above which just returns the draw target.  Clip is *NOT* set
    GrDrawTarget* prepareToDraw();

    void internalDrawPath(GrDrawTarget*,
                          GrPipelineBuilder*,
                          const SkMatrix& viewMatrix,
                          GrColor,
                          bool useAA,
                          const SkPath&,
                          const GrStrokeInfo&);

    GrTexture* internalRefScratchTexture(const GrSurfaceDesc&, uint32_t flags);

    /**
     * These functions create premul <-> unpremul effects if it is possible to generate a pair
     * of effects that make a readToUPM->writeToPM->readToUPM cycle invariant. Otherwise, they
     * return NULL.
     */
    const GrFragmentProcessor* createPMToUPMEffect(GrTexture*, bool swapRAndB, const SkMatrix&);
    const GrFragmentProcessor* createUPMToPMEffect(GrTexture*, bool swapRAndB, const SkMatrix&);

    /**
     *  This callback allows the resource cache to callback into the GrContext
     *  when the cache is still over budget after a purge.
     */
    static void OverBudgetCB(void* data);

    typedef SkRefCnt INHERITED;
};

#endif
