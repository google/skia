
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrContext_DEFINED
#define GrContext_DEFINED

#include "GrConfig.h"
#include "GrPaint.h"
#include "GrAARectRenderer.h"
#include "GrClipData.h"
// not strictly needed but requires WK change in LayerTextureUpdaterCanvas to
// remove.
#include "GrRenderTarget.h"
#include "SkClipStack.h"

class GrAutoScratchTexture;
class GrCacheKey;
class GrDrawState;
class GrDrawTarget;
class GrFontCache;
class GrGpu;
class GrIndexBuffer;
class GrIndexBufferAllocPool;
class GrInOrderDrawBuffer;
class GrPathRenderer;
class GrPathRendererChain;
class GrResourceEntry;
class GrResourceCache;
class GrStencilBuffer;
class GrVertexBuffer;
class GrVertexBufferAllocPool;
class GrSoftwarePathRenderer;

class GR_API GrContext : public GrRefCnt {
public:
    SK_DECLARE_INST_COUNT(GrContext)

    /**
     * Creates a GrContext from within a 3D context.
     */
    static GrContext* Create(GrEngine engine,
                             GrPlatform3DContext context3D);

    /**
     * Returns the number of GrContext instances for the current thread.
     */
    static int GetThreadInstanceCount();

    virtual ~GrContext();

    /**
     * The GrContext normally assumes that no outsider is setting state
     * within the underlying 3D API's context/device/whatever. This call informs
     * the context that the state was modified and it should resend. Shouldn't
     * be called frequently for good performance.
     */
    void resetContext();

    /**
     * Abandons all gpu resources, assumes 3D API state is unknown. Call this
     * if you have lost the associated GPU context, and thus internal texture,
     * buffer, etc. references/IDs are now invalid. Should be called even when
     * GrContext is no longer going to be used for two reasons:
     *  1) ~GrContext will not try to free the objects in the 3D API.
     *  2) If you've created GrResources that outlive the GrContext they will
     *     be marked as invalid (GrResource::isValid()) and won't attempt to
     *     free their underlying resource in the 3D API.
     * Content drawn since the last GrContext::flush() may be lost.
     */
    void contextLost();

    /**
     * Similar to contextLost, but makes no attempt to reset state.
     * Use this method when GrContext destruction is pending, but
     * the graphics context is destroyed first.
     */
    void contextDestroyed();

    /**
     * Frees gpu created by the context. Can be called to reduce GPU memory
     * pressure.
     */
    void freeGpuResources();

    /**
     * Returns the number of bytes of GPU memory hosted by the texture cache.
     */
    size_t getGpuTextureCacheBytes() const;

    ///////////////////////////////////////////////////////////////////////////
    // Textures

    /**
     *  Create a new entry, based on the specified key and texture, and return
     *  a "locked" texture. Must call be balanced with an unlockTexture() call.
     *
     * @param params    The tex params used to draw a texture may help determine
     *                  the cache entry used. (e.g. different versions may exist
     *                  for different wrap modes on GPUs with limited NPOT
     *                  texture support). NULL implies clamp wrap modes.
     * @param desc      Description of the texture properties.
     * @param cacheData Cache-specific properties (e.g., texture gen ID)
     * @param srcData   Pointer to the pixel values.
     * @param rowBytes  The number of bytes between rows of the texture. Zero
     *                  implies tightly packed rows.
     */
    GrTexture* createTexture(const GrTextureParams* params,
                             const GrTextureDesc& desc,
                             const GrCacheData& cacheData,
                             void* srcData, size_t rowBytes);

    /**
     * Look for a texture that matches 'key' in the cache. If not found,
     * return NULL.
     */
    GrTexture* findTexture(const GrCacheKey& key);

    /**
     *  Search for an entry based on key and dimensions. If found,
     *  return it. The return value will be NULL if not found.
     *
     *  @param desc     Description of the texture properties.
     *  @param cacheData Cache-specific properties (e.g., texture gen ID)
     *  @param params   The tex params used to draw a texture may help determine
     *                  the cache entry used. (e.g. different versions may exist
     *                  for different wrap modes on GPUs with limited NPOT
     *                  texture support). NULL implies clamp wrap modes.
     */
    GrTexture* findTexture(const GrTextureDesc& desc,
                           const GrCacheData& cacheData,
                           const GrTextureParams* params);
    /**
     * Determines whether a texture is in the cache. If the texture is found it
     * will not be locked or returned. This call does not affect the priority of
     * the texture for deletion.
     */
    bool isTextureInCache(const GrTextureDesc& desc,
                          const GrCacheData& cacheData,
                          const GrTextureParams* params) const;

    /**
     * Enum that determines how closely a returned scratch texture must match
     * a provided GrTextureDesc.
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
     * Returns a texture matching the desc. It's contents are unknown. Subsequent
     * requests with the same descriptor are not guaranteed to return the same
     * texture. The same texture is guaranteed not be returned again until it is
     * unlocked. Call must be balanced with an unlockTexture() call.
     *
     * Textures created by createAndLockTexture() hide the complications of
     * tiling non-power-of-two textures on APIs that don't support this (e.g.
     * unextended GLES2). Tiling a npot texture created by lockScratchTexture on
     * such an API will create gaps in the tiling pattern. This includes clamp
     * mode. (This may be addressed in a future update.)
     */
    GrTexture* lockScratchTexture(const GrTextureDesc& desc,
                                  ScratchTexMatch match);

    /**
     *  When done with an entry, call unlockTexture(entry) on it, which returns
     *  it to the cache, where it may be purged.
     */
    void unlockScratchTexture(GrTexture* texture);

    /**
     * This method should be called whenever a GrTexture is unreffed or
     * switched from exclusive to non-exclusive. This
     * gives the resource cache a chance to discard unneeded textures.
     * Note: this entry point will be removed once totally ref-driven
     * cache maintenance is implemented
     */
    void purgeCache();

    /**
     * Creates a texture that is outside the cache. Does not count against
     * cache's budget.
     */
    GrTexture* createUncachedTexture(const GrTextureDesc& desc,
                                     void* srcData,
                                     size_t rowBytes);

    /**
     * Returns true if the specified use of an indexed texture is supported.
     * Support may depend upon whether the texture params indicate that the
     * texture will be tiled. Passing NULL for the texture params indicates
     * clamp mode.
     */
    bool supportsIndex8PixelConfig(const GrTextureParams*,
                                   int width,
                                   int height) const;

    /**
     *  Return the current texture cache limits.
     *
     *  @param maxTextures If non-null, returns maximum number of textures that
     *                     can be held in the cache.
     *  @param maxTextureBytes If non-null, returns maximum number of bytes of
     *                         texture memory that can be held in the cache.
     */
    void getTextureCacheLimits(int* maxTextures, size_t* maxTextureBytes) const;

    /**
     *  Specify the texture cache limits. If the current cache exceeds either
     *  of these, it will be purged (LRU) to keep the cache within these limits.
     *
     *  @param maxTextures The maximum number of textures that can be held in
     *                     the cache.
     *  @param maxTextureBytes The maximum number of bytes of texture memory
     *                         that can be held in the cache.
     */
    void setTextureCacheLimits(int maxTextures, size_t maxTextureBytes);

    /**
     *  Return the max width or height of a texture supported by the current gpu
     */
    int getMaxTextureSize() const;

    /**
     * Return the max width or height of a render target supported by the
     * current gpu
     */
    int getMaxRenderTargetSize() const;

    ///////////////////////////////////////////////////////////////////////////
    // Render targets

    /**
     * Sets the render target.
     * @param target    the render target to set. (should not be NULL.)
     */
    void setRenderTarget(GrRenderTarget* target);

    /**
     * Gets the current render target.
     * @return the currently bound render target. Should never be NULL.
     */
    const GrRenderTarget* getRenderTarget() const;
    GrRenderTarget* getRenderTarget();

    GrAARectRenderer* getAARectRenderer() { return fAARectRenderer; }

    /**
     * Can the provided configuration act as a color render target?
     */
    bool isConfigRenderable(GrPixelConfig config) const;

    ///////////////////////////////////////////////////////////////////////////
    // Platform Surfaces

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
    GrTexture* createPlatformTexture(const GrPlatformTextureDesc& desc);

    /**
     * Wraps an existing render target with a GrRenderTarget object. It is
     * similar to createPlatformTexture but can be used to draw into surfaces
     * that are not also textures (e.g. FBO 0 in OpenGL, or an MSAA buffer that
     * the client will resolve to a texture).
     *
     * @param  desc     description of the object to create.
     *
     * @return GrTexture object or NULL on failure.
     */
     GrRenderTarget* createPlatformRenderTarget(
                                    const GrPlatformRenderTargetDesc& desc);

    ///////////////////////////////////////////////////////////////////////////
    // Matrix state

    /**
     * Gets the current transformation matrix.
     * @return the current matrix.
     */
    const GrMatrix& getMatrix() const;

    /**
     * Sets the transformation matrix.
     * @param m the matrix to set.
     */
    void setMatrix(const GrMatrix& m);

    /**
     * Concats the current matrix. The passed matrix is applied before the
     * current matrix.
     * @param m the matrix to concat.
     */
    void concatMatrix(const GrMatrix& m) const;


    ///////////////////////////////////////////////////////////////////////////
    // Clip state
    /**
     * Gets the current clip.
     * @return the current clip.
     */
    const GrClipData* getClip() const;

    /**
     * Sets the clip.
     * @param clipData  the clip to set.
     */
    void setClip(const GrClipData* clipData);

    ///////////////////////////////////////////////////////////////////////////
    // Draws

    /**
     * Clear the entire or rect of the render target, ignoring any clips.
     * @param rect  the rect to clear or the whole thing if rect is NULL.
     * @param color the color to clear to.
     * @param target if non-NULL, the render target to clear otherwise clear
     *               the current render target
     */
    void clear(const GrIRect* rect, GrColor color,
               GrRenderTarget* target = NULL);

    /**
     *  Draw everywhere (respecting the clip) with the paint.
     */
    void drawPaint(const GrPaint& paint);

    /**
     *  Draw the rect using a paint.
     *  @param paint        describes how to color pixels.
     *  @param strokeWidth  If strokeWidth < 0, then the rect is filled, else
     *                      the rect is mitered stroked based on strokeWidth. If
     *                      strokeWidth == 0, then the stroke is always a single
     *                      pixel thick.
     *  @param matrix       Optional matrix applied to the rect. Applied before
     *                      context's matrix or the paint's matrix.
     *  The rects coords are used to access the paint (through texture matrix)
     */
    void drawRect(const GrPaint& paint,
                  const GrRect&,
                  GrScalar strokeWidth = -1,
                  const GrMatrix* matrix = NULL);

    /**
     * Maps a rect of paint coordinates onto the a rect of destination
     * coordinates. Each rect can optionally be transformed. The srcRect
     * is stretched over the dstRect. The dstRect is transformed by the
     * context's matrix and the srcRect is transformed by the paint's matrix.
     * Additional optional matrices can be provided by parameters.
     *
     * @param paint     describes how to color pixels.
     * @param dstRect   the destination rect to draw.
     * @param srcRect   rect of paint coordinates to be mapped onto dstRect
     * @param dstMatrix Optional matrix to transform dstRect. Applied before
     *                  context's matrix.
     * @param srcMatrix Optional matrix to transform srcRect Applied before
     *                  paint's matrix.
     */
    void drawRectToRect(const GrPaint& paint,
                        const GrRect& dstRect,
                        const GrRect& srcRect,
                        const GrMatrix* dstMatrix = NULL,
                        const GrMatrix* srcMatrix = NULL);

    /**
     * Draws a path.
     *
     * @param paint         describes how to color pixels.
     * @param path          the path to draw
     * @param fill          the path filling rule to use.
     * @param translate     optional additional translation applied to the
     *                      path.
     */
    void drawPath(const GrPaint& paint, const SkPath& path, GrPathFill fill,
                  const GrPoint* translate = NULL);

    /**
     * Draws vertices with a paint.
     *
     * @param   paint           describes how to color pixels.
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
    void drawVertices(const GrPaint& paint,
                      GrPrimitiveType primitiveType,
                      int vertexCount,
                      const GrPoint positions[],
                      const GrPoint texs[],
                      const GrColor colors[],
                      const uint16_t indices[],
                      int indexCount);

    /**
     * Draws an oval.
     *
     * @param paint         describes how to color pixels.
     * @param rect          the bounding rect of the oval.
     * @param strokeWidth   if strokeWidth < 0, then the oval is filled, else
     *                      the rect is stroked based on strokeWidth. If
     *                      strokeWidth == 0, then the stroke is always a single
     *                      pixel thick.
     */
    void drawOval(const GrPaint& paint,
                  const GrRect& rect,
                  SkScalar strokeWidth);

    ///////////////////////////////////////////////////////////////////////////
    // Misc.

    /**
     * Flags that affect flush() behavior.
     */
    enum FlushBits {
        /**
         * A client may want Gr to bind a GrRenderTarget in the 3D API so that
         * it can be rendered to directly. However, Gr lazily sets state. Simply
         * calling setRenderTarget() followed by flush() without flags may not
         * bind the render target. This flag forces the context to bind the last
         * set render target in the 3D API.
         */
        kForceCurrentRenderTarget_FlushBit   = 0x1,
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
        /** The GrContext will not be flushed. This means that the read or write may occur before
            previous draws have executed. */
        kDontFlush_PixelOpsFlag = 0x1,
        /** The src for write or dst read is unpremultiplied. This is only respected if both the
            config src and dst configs are an RGBA/BGRA 8888 format. */
        kUnpremul_PixelOpsFlag  = 0x2,
    };

    /**
     * Reads a rectangle of pixels from a render target.
     * @param target        the render target to read from. NULL means the current render target.
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
     * Copy the src pixels [buffer, rowbytes, pixelconfig] into a render target at the specified
     * rectangle.
     * @param target        the render target to write into. NULL means the current render target.
     * @param left          left edge of the rectangle to write (inclusive)
     * @param top           top edge of the rectangle to write (inclusive)
     * @param width         width of rectangle to write in pixels.
     * @param height        height of rectangle to write in pixels.
     * @param config        the pixel config of the source buffer
     * @param buffer        memory to read the rectangle from.
     * @param rowBytes      number of bytes bewtween consecutive rows. Zero means rows are tightly
     *                      packed.
     * @param pixelOpsFlags see PixelOpsFlags enum above.
     */
    void writeRenderTargetPixels(GrRenderTarget* target,
                                 int left, int top, int width, int height,
                                 GrPixelConfig config, const void* buffer,
                                 size_t rowBytes = 0,
                                 uint32_t pixelOpsFlags = 0);

    /**
     * Reads a rectangle of pixels from a texture.
     * @param texture       the texture to read from.
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
     *         pixel config.
     */
    bool readTexturePixels(GrTexture* texture,
                           int left, int top, int width, int height,
                           GrPixelConfig config, void* buffer,
                           size_t rowBytes = 0,
                           uint32_t pixelOpsFlags = 0);

    /**
     * Writes a rectangle of pixels to a texture.
     * @param texture       the render target to read from.
     * @param left          left edge of the rectangle to write (inclusive)
     * @param top           top edge of the rectangle to write (inclusive)
     * @param width         width of rectangle to write in pixels.
     * @param height        height of rectangle to write in pixels.
     * @param config        the pixel config of the source buffer
     * @param buffer        memory to read pixels from
     * @param rowBytes      number of bytes bewtween consecutive rows. Zero
     *                      means rows are tightly packed.
     * @param pixelOpsFlags see PixelOpsFlags enum above.
     */
    void writeTexturePixels(GrTexture* texture,
                            int left, int top, int width, int height,
                            GrPixelConfig config, const void* buffer,
                            size_t rowBytes,
                            uint32_t pixelOpsFlags = 0);


    /**
     * Copies all texels from one texture to another.
     * @param src           the texture to copy from.
     * @param dst           the render target to copy to.
     */
    void copyTexture(GrTexture* src, GrRenderTarget* dst);

    /**
     * Resolves a render target that has MSAA. The intermediate MSAA buffer is
     * downsampled to the associated GrTexture (accessible via
     * GrRenderTarget::asTexture()). Any pending draws to the render target will
     * be executed before the resolve.
     *
     * This is only necessary when a client wants to access the object directly
     * using the underlying graphics API. GrContext will detect when it must
     * perform a resolve to a GrTexture used as the source of a draw or before
     * reading pixels back from a GrTexture or GrRenderTarget.
     */
    void resolveRenderTarget(GrRenderTarget* target);

    /**
     * Applies a 2D Gaussian blur to a given texture.
     * @param srcTexture      The source texture to be blurred.
     * @param canClobberSrc   If true, srcTexture may be overwritten, and
     *                        may be returned as the result.
     * @param rect            The destination rectangle.
     * @param sigmaX          The blur's standard deviation in X.
     * @param sigmaY          The blur's standard deviation in Y.
     * @return the blurred texture, which may be srcTexture ref'ed, or a
     * new texture.  It is the caller's responsibility to unref this texture.
     */
     GrTexture* gaussianBlur(GrTexture* srcTexture,
                             bool canClobberSrc,
                             const SkRect& rect,
                             float sigmaX, float sigmaY);

    /**
     * Zooms a subset of the texture to a larger size with a nice edge.
     * The inner rectangle is a simple scaling of the texture by a factor of
     * |zoom|.  The outer |inset| pixels transition from the background texture
     * to the zoomed coordinate system at a rate of
     * (distance_to_edge / inset) ^2, producing a rounded lens effect.
     * @param srcTexture      The source texture to be zoomed.
     * @param dstRect         The destination rectangle.
     * @param srcRect         The source rectangle.  Must be smaller than
     *                        dstRect
     * @param inset           Number of pixels to blend along the edges.
     * @return the zoomed texture, which is dstTexture.
     */
     GrTexture* zoom(GrTexture* srcTexture,
                     const SkRect& dstRect, const SkRect& srcRect, float inset);

    ///////////////////////////////////////////////////////////////////////////
    // Helpers

    class AutoRenderTarget : ::GrNoncopyable {
    public:
        AutoRenderTarget(GrContext* context, GrRenderTarget* target) {
            fPrevTarget = context->getRenderTarget();
            context->setRenderTarget(target);
            fContext = context;
        }
        AutoRenderTarget(GrContext* context) {
            fPrevTarget = context->getRenderTarget();
            fContext = context;
        }
        ~AutoRenderTarget() {
            if (fContext) {
                fContext->setRenderTarget(fPrevTarget);
            }
        }
    private:
        GrContext*      fContext;
        GrRenderTarget* fPrevTarget;
    };

    /**
     *  Save/restore the view-matrix in the context.
     */
    class AutoMatrix : GrNoncopyable {
    public:
        enum InitialMatrix {
            kPreserve_InitialMatrix,
            kIdentity_InitialMatrix,
        };

        AutoMatrix() : fContext(NULL) {}

        AutoMatrix(GrContext* ctx, InitialMatrix initialState) : fContext(ctx) {
            fMatrix = ctx->getMatrix();
            switch (initialState) {
                case kPreserve_InitialMatrix:
                    break;
                case kIdentity_InitialMatrix:
                    ctx->setMatrix(GrMatrix::I());
                    break;
                default:
                    GrCrash("Unexpected initial matrix state");
            }
        }

        AutoMatrix(GrContext* ctx, const GrMatrix& matrix) : fContext(ctx) {
            fMatrix = ctx->getMatrix();
            ctx->setMatrix(matrix);
        }

        void set(GrContext* ctx) {
            if (NULL != fContext) {
                fContext->setMatrix(fMatrix);
            }
            fMatrix = ctx->getMatrix();
            fContext = ctx;
        }

        void set(GrContext* ctx, const GrMatrix& matrix) {
            if (NULL != fContext) {
                fContext->setMatrix(fMatrix);
            }
            fMatrix = ctx->getMatrix();
            ctx->setMatrix(matrix);
            fContext = ctx;
        }

        ~AutoMatrix() {
            if (NULL != fContext) {
                fContext->setMatrix(fMatrix);
            }
        }

    private:
        GrContext*  fContext;
        GrMatrix    fMatrix;
    };

    class AutoClip : GrNoncopyable {
    public:
        // This enum exists to require a caller of the constructor to acknowledge that the clip will
        // initially be wide open. It also could be extended if there are other desirable initial
        // clip states.
        enum InitialClip {
            kWideOpen_InitialClip,
        };

        AutoClip(GrContext* context, InitialClip initialState) {
            GrAssert(kWideOpen_InitialClip == initialState);
            fOldClip = context->getClip();
            fNewClipData.fClipStack = &fNewClipStack;
            context->setClip(&fNewClipData);
            fContext = context;
        }

        AutoClip(GrContext* context, const GrRect& newClipRect)
        : fContext(context)
        , fNewClipStack(newClipRect) {
            fNewClipData.fClipStack = &fNewClipStack;

            fOldClip = fContext->getClip();
            fContext->setClip(&fNewClipData);
        }

        ~AutoClip() {
            if (NULL != fContext) {
                fContext->setClip(fOldClip);
            }
        }
    private:
        GrContext*        fContext;
        const GrClipData* fOldClip;

        SkClipStack       fNewClipStack;
        GrClipData        fNewClipData;
    };

    class AutoWideOpenIdentityDraw {
    public:
        AutoWideOpenIdentityDraw(GrContext* ctx, GrRenderTarget* rt)
            : fAutoClip(ctx, AutoClip::kWideOpen_InitialClip)
            , fAutoRT(ctx, rt)
            , fAutoMatrix(ctx, AutoMatrix::kIdentity_InitialMatrix) {
        }
    private:
        AutoClip fAutoClip;
        AutoRenderTarget fAutoRT;
        AutoMatrix fAutoMatrix;
    };

    ///////////////////////////////////////////////////////////////////////////
    // Functions intended for internal use only.
    GrGpu* getGpu() { return fGpu; }
    const GrGpu* getGpu() const { return fGpu; }
    GrFontCache* getFontCache() { return fFontCache; }
    GrDrawTarget* getTextTarget(const GrPaint& paint);
    const GrIndexBuffer* getQuadIndexBuffer() const;

    /**
     * Stencil buffers add themselves to the cache using
     * addStencilBuffer. findStencilBuffer is called to check the
     * cache for a SB that matches an RT's criteria.
     */
    void addStencilBuffer(GrStencilBuffer* sb);
    GrStencilBuffer* findStencilBuffer(int width, int height, int sampleCnt);

    GrPathRenderer* getPathRenderer(const SkPath& path,
                                    GrPathFill fill,
                                    const GrDrawTarget* target,
                                    bool antiAlias,
                                    bool allowSW);

#if GR_CACHE_STATS
    void printCacheStats() const;
#endif

private:
    // Used to indicate whether a draw should be performed immediately or queued in fDrawBuffer.
    enum BufferedDraw {
        kYes_BufferedDraw,
        kNo_BufferedDraw,
    };
    BufferedDraw fLastDrawWasBuffered;

    GrGpu*              fGpu;
    GrDrawState*        fDrawState;

    GrResourceCache*    fTextureCache;
    GrFontCache*        fFontCache;

    GrPathRendererChain*        fPathRendererChain;
    GrSoftwarePathRenderer*     fSoftwarePathRenderer;

    GrVertexBufferAllocPool*    fDrawBufferVBAllocPool;
    GrIndexBufferAllocPool*     fDrawBufferIBAllocPool;
    GrInOrderDrawBuffer*        fDrawBuffer;

    GrAARectRenderer*           fAARectRenderer;

    bool                        fDidTestPMConversions;
    int                         fPMToUPMConversion;
    int                         fUPMToPMConversion;

    GrContext(GrGpu* gpu);

    void setupDrawBuffer();

    void flushDrawBuffer();

    void setPaint(const GrPaint& paint);

    /// Sets the paint and returns the target to draw into. The paint can be NULL in which case the
    /// draw state is left unmodified.
    GrDrawTarget* prepareToDraw(const GrPaint*, BufferedDraw);

    void internalDrawPath(const GrPaint& paint, const SkPath& path,
                          GrPathFill fill, const GrPoint* translate);

    GrTexture* createResizedTexture(const GrTextureDesc& desc,
                                    const GrCacheData& cacheData,
                                    void* srcData,
                                    size_t rowBytes,
                                    bool needsFiltering);

    // Needed so GrTexture's returnToCache helper function can call
    // addExistingTextureToCache
    friend class GrTexture;

    // Add an existing texture to the texture cache. This is intended solely
    // for use with textures released from an GrAutoScratchTexture.
    void addExistingTextureToCache(GrTexture* texture);

    GrCustomStage* createPMToUPMEffect(GrTexture* texture, bool swapRAndB);
    GrCustomStage* createUPMToPMEffect(GrTexture* texture, bool swapRAndB);

    typedef GrRefCnt INHERITED;
};

/**
 * Gets and locks a scratch texture from a descriptor using
 * either exact or approximate criteria. Unlocks texture in
 * the destructor.
 */
class GrAutoScratchTexture : ::GrNoncopyable {
public:
    GrAutoScratchTexture()
        : fContext(NULL)
        , fTexture(NULL) {
    }

    GrAutoScratchTexture(GrContext* context,
                         const GrTextureDesc& desc,
                         GrContext::ScratchTexMatch match =
                            GrContext::kApprox_ScratchTexMatch)
      : fContext(NULL)
      , fTexture(NULL) {
      this->set(context, desc, match);
    }

    ~GrAutoScratchTexture() {
        this->reset();
    }

    void reset() {
        if (NULL != fContext && NULL != fTexture) {
            fContext->unlockScratchTexture(fTexture);
            fTexture = NULL;
        }
    }

    /*
     * When detaching a texture we do not unlock it in the texture cache but
     * we do set the returnToCache flag. In this way the texture remains
     * "locked" in the texture cache until it is freed and recycled in
     * GrTexture::internal_dispose. In reality, the texture has been removed
     * from the cache (because this is in AutoScratchTexture) and by not
     * calling unlockTexture we simply don't re-add it. It will be reattached
     * in GrTexture::internal_dispose.
     *
     * Note that the caller is assumed to accept and manage the ref to the
     * returned texture.
     */
    GrTexture* detach() {
        GrTexture* temp = fTexture;

        // Conceptually the texture's cache entry loses its ref to the
        // texture while the caller of this method gets a ref.
        GrAssert(NULL != temp->getCacheEntry());

        fTexture = NULL;

        temp->setFlag((GrTextureFlags) GrTexture::kReturnToCache_FlagBit);
        return temp;
    }

    GrTexture* set(GrContext* context,
                   const GrTextureDesc& desc,
                   GrContext::ScratchTexMatch match =
                        GrContext::kApprox_ScratchTexMatch) {
        this->reset();

        fContext = context;
        if (NULL != fContext) {
            fTexture = fContext->lockScratchTexture(desc, match);
            if (NULL == fTexture) {
                fContext = NULL;
            }
            return fTexture;
        } else {
            return NULL;
        }
    }

    GrTexture* texture() { return fTexture; }

private:
    GrContext*                    fContext;
    GrTexture*                    fTexture;
};

#endif
