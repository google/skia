/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrContext_DEFINED
#define GrContext_DEFINED

#include "GrClipData.h"
#include "GrColor.h"
#include "GrPaint.h"
#include "GrPathRendererChain.h"
#include "GrPoint.h"
#include "GrRenderTarget.h"
#include "GrTexture.h"
#include "SkMatrix.h"
#include "SkTypes.h"

class GrAARectRenderer;
class GrAutoScratchTexture;
class GrDrawState;
class GrDrawTarget;
class GrEffect;
class GrFontCache;
class GrGpu;
class GrIndexBuffer;
class GrIndexBufferAllocPool;
class GrInOrderDrawBuffer;
class GrOvalRenderer;
class GrPathRenderer;
class GrResourceEntry;
class GrResourceCache;
class GrStencilBuffer;
class GrTestTarget;
class GrTextureParams;
class GrVertexBuffer;
class GrVertexBufferAllocPool;
class GrSoftwarePathRenderer;
class SkStrokeRec;

class SK_API GrContext : public SkRefCnt {
public:
    SK_DECLARE_INST_COUNT(GrContext)

    /**
     * Creates a GrContext for a backend context.
     */
    static GrContext* Create(GrBackend, GrBackendContext);

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
     * Abandons all GPU resources, assumes 3D API state is unknown. Call this
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
     * Frees GPU created by the context. Can be called to reduce GPU memory
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
     * Creates a new entry, based on the specified key and texture and returns it. The caller owns a
     * ref on the returned texture which must be balanced by a call to unref.
     *
     * @param params    The texture params used to draw a texture may help determine
     *                  the cache entry used. (e.g. different versions may exist
     *                  for different wrap modes on GPUs with limited NPOT
     *                  texture support). NULL implies clamp wrap modes.
     * @param desc      Description of the texture properties.
     * @param cacheID Cache-specific properties (e.g., texture gen ID)
     * @param srcData   Pointer to the pixel values.
     * @param rowBytes  The number of bytes between rows of the texture. Zero
     *                  implies tightly packed rows.
     */
    GrTexture* createTexture(const GrTextureParams* params,
                             const GrTextureDesc& desc,
                             const GrCacheID& cacheID,
                             void* srcData, size_t rowBytes);

    /**
     * Search for an entry based on key and dimensions. If found, ref it and return it. The return
     * value will be NULL if not found. The caller must balance with a call to unref.
     *
     *  @param desc     Description of the texture properties.
     *  @param cacheID Cache-specific properties (e.g., texture gen ID)
     *  @param params   The texture params used to draw a texture may help determine
     *                  the cache entry used. (e.g. different versions may exist
     *                  for different wrap modes on GPUs with limited NPOT
     *                  texture support). NULL implies clamp wrap modes.
     */
    GrTexture* findAndRefTexture(const GrTextureDesc& desc,
                                 const GrCacheID& cacheID,
                                 const GrTextureParams* params);
    /**
     * Determines whether a texture is in the cache. If the texture is found it
     * will not be locked or returned. This call does not affect the priority of
     * the texture for deletion.
     */
    bool isTextureInCache(const GrTextureDesc& desc,
                          const GrCacheID& cacheID,
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
     * unlocked. Call must be balanced with an unlockTexture() call. The caller
     * owns a ref on the returned texture and must balance with a call to unref.
     *
     * Textures created by createAndLockTexture() hide the complications of
     * tiling non-power-of-two textures on APIs that don't support this (e.g.
     * unextended GLES2). Tiling a NPOT texture created by lockScratchTexture on
     * such an API will create gaps in the tiling pattern. This includes clamp
     * mode. (This may be addressed in a future update.)
     */
    GrTexture* lockAndRefScratchTexture(const GrTextureDesc&, ScratchTexMatch match);

    /**
     *  When done with an entry, call unlockScratchTexture(entry) on it, which returns
     *  it to the cache, where it may be purged. This does not unref the texture.
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

    ///////////////////////////////////////////////////////////////////////////
    // Render targets

    /**
     * Sets the render target.
     * @param target    the render target to set.
     */
    void setRenderTarget(GrRenderTarget* target) {
        fRenderTarget.reset(SkSafeRef(target));
    }

    /**
     * Gets the current render target.
     * @return the currently bound render target.
     */
    const GrRenderTarget* getRenderTarget() const { return fRenderTarget.get(); }
    GrRenderTarget* getRenderTarget() { return fRenderTarget.get(); }

    GrAARectRenderer* getAARectRenderer() { return fAARectRenderer; }

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
    // Matrix state

    /**
     * Gets the current transformation matrix.
     * @return the current matrix.
     */
    const SkMatrix& getMatrix() const { return fViewMatrix; }

    /**
     * Sets the transformation matrix.
     * @param m the matrix to set.
     */
    void setMatrix(const SkMatrix& m) { fViewMatrix = m; }

    /**
     * Sets the current transformation matrix to identity.
     */
    void setIdentityMatrix() { fViewMatrix.reset(); }

    /**
     * Concats the current matrix. The passed matrix is applied before the
     * current matrix.
     * @param m the matrix to concat.
     */
    void concatMatrix(const SkMatrix& m) { fViewMatrix.preConcat(m); }


    ///////////////////////////////////////////////////////////////////////////
    // Clip state
    /**
     * Gets the current clip.
     * @return the current clip.
     */
    const GrClipData* getClip() const { return fClip; }

    /**
     * Sets the clip.
     * @param clipData  the clip to set.
     */
    void setClip(const GrClipData* clipData) { fClip = clipData; }

    ///////////////////////////////////////////////////////////////////////////
    // Draws

    /**
     * Clear the entire or rect of the render target, ignoring any clips.
     * @param rect  the rect to clear or the whole thing if rect is NULL.
     * @param color the color to clear to.
     * @param target if non-NULL, the render target to clear otherwise clear
     *               the current render target
     */
    void clear(const SkIRect* rect, GrColor color,
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
                  const SkRect&,
                  SkScalar strokeWidth = -1,
                  const SkMatrix* matrix = NULL);

    /**
     * Maps a rect of local coordinates onto the a rect of destination
     * coordinates. Each rect can optionally be transformed. The localRect
     * is stretched over the dstRect. The dstRect is transformed by the
     * context's matrix. Additional optional matrices for both rects can be
     * provided by parameters.
     *
     * @param paint         describes how to color pixels.
     * @param dstRect       the destination rect to draw.
     * @param localRect     rect of local coordinates to be mapped onto dstRect
     * @param dstMatrix     Optional matrix to transform dstRect. Applied before context's matrix.
     * @param localMatrix   Optional matrix to transform localRect.
     */
    void drawRectToRect(const GrPaint& paint,
                        const SkRect& dstRect,
                        const SkRect& localRect,
                        const SkMatrix* dstMatrix = NULL,
                        const SkMatrix* localMatrix = NULL);

    /**
     *  Draw a roundrect using a paint.
     *
     *  @param paint        describes how to color pixels.
     *  @param rrect        the roundrect to draw
     *  @param stroke       the stroke information (width, join, cap)
     */
    void drawRRect(const GrPaint& paint,
                   const SkRRect& rrect,
                   const SkStrokeRec& stroke);

    /**
     * Draws a path.
     *
     * @param paint         describes how to color pixels.
     * @param path          the path to draw
     * @param stroke        the stroke information (width, join, cap)
     */
    void drawPath(const GrPaint& paint, const SkPath& path, const SkStrokeRec& stroke);

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
     * @param oval          the bounding rect of the oval.
     * @param stroke        the stroke information (width, style)
     */
    void drawOval(const GrPaint& paint,
                  const SkRect& oval,
                  const SkStrokeRec& stroke);

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
     * Copy the src pixels [buffer, row bytes, pixel config] into a render target at the specified
     * rectangle.
     * @param target        the render target to write into. NULL means the current render target.
     * @param left          left edge of the rectangle to write (inclusive)
     * @param top           top edge of the rectangle to write (inclusive)
     * @param width         width of rectangle to write in pixels.
     * @param height        height of rectangle to write in pixels.
     * @param config        the pixel config of the source buffer
     * @param buffer        memory to read the rectangle from.
     * @param rowBytes      number of bytes between consecutive rows. Zero means rows are tightly
     *                      packed.
     * @param pixelOpsFlags see PixelOpsFlags enum above.
     *
     * @return true if the write succeeded, false if not. The write can fail because of an
     *         unsupported combination of target and pixel configs.
     */
    bool writeRenderTargetPixels(GrRenderTarget* target,
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
     * @param rowBytes      number of bytes between consecutive rows. Zero means rows are tightly
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
     * @param rowBytes      number of bytes between consecutive rows. Zero
     *                      means rows are tightly packed.
     * @param pixelOpsFlags see PixelOpsFlags enum above.
     * @return true if the write succeeded, false if not. The write can fail because of an
     *         unsupported combination of texture and pixel configs.
     */
    bool writeTexturePixels(GrTexture* texture,
                            int left, int top, int width, int height,
                            GrPixelConfig config, const void* buffer,
                            size_t rowBytes,
                            uint32_t pixelOpsFlags = 0);


    /**
     * Copies a rectangle of texels from src to dst. The size of dst is the size of the rectangle
     * copied and topLeft is the position of the rect in src. The rectangle is clipped to src's
     * bounds.
     * @param src           the texture to copy from.
     * @param dst           the render target to copy to.
     * @param topLeft       the point in src that will be copied to the top-left of dst. If NULL,
     *                      (0, 0) will be used.
     */
    void copyTexture(GrTexture* src, GrRenderTarget* dst, const SkIPoint* topLeft = NULL);

    /**
     * Resolves a render target that has MSAA. The intermediate MSAA buffer is
     * down-sampled to the associated GrTexture (accessible via
     * GrRenderTarget::asTexture()). Any pending draws to the render target will
     * be executed before the resolve.
     *
     * This is only necessary when a client wants to access the object directly
     * using the backend API directly. GrContext will detect when it must
     * perform a resolve to a GrTexture used as the source of a draw or before
     * reading pixels back from a GrTexture or GrRenderTarget.
     */
    void resolveRenderTarget(GrRenderTarget* target);

#ifdef SK_DEVELOPER
    void dumpFontCache() const;
#endif

    ///////////////////////////////////////////////////////////////////////////
    // Helpers

    class AutoRenderTarget : public ::SkNoncopyable {
    public:
        AutoRenderTarget(GrContext* context, GrRenderTarget* target) {
            fPrevTarget = context->getRenderTarget();
            SkSafeRef(fPrevTarget);
            context->setRenderTarget(target);
            fContext = context;
        }
        AutoRenderTarget(GrContext* context) {
            fPrevTarget = context->getRenderTarget();
            SkSafeRef(fPrevTarget);
            fContext = context;
        }
        ~AutoRenderTarget() {
            if (NULL != fContext) {
                fContext->setRenderTarget(fPrevTarget);
            }
            SkSafeUnref(fPrevTarget);
        }
    private:
        GrContext*      fContext;
        GrRenderTarget* fPrevTarget;
    };

    /**
     * Save/restore the view-matrix in the context. It can optionally adjust a paint to account
     * for a coordinate system change. Here is an example of how the paint param can be used:
     *
     * A GrPaint is setup with GrEffects. The stages will have access to the pre-matrix source
     * geometry positions when the draw is executed. Later on a decision is made to transform the
     * geometry to device space on the CPU. The effects now need to know that the space in which
     * the geometry will be specified has changed.
     *
     * Note that when restore is called (or in the destructor) the context's matrix will be
     * restored. However, the paint will not be restored. The caller must make a copy of the
     * paint if necessary. Hint: use SkTCopyOnFirstWrite if the AutoMatrix is conditionally
     * initialized.
     */
    class AutoMatrix : public ::SkNoncopyable {
    public:
        AutoMatrix() : fContext(NULL) {}

        ~AutoMatrix() { this->restore(); }

        /**
         * Initializes by pre-concat'ing the context's current matrix with the preConcat param.
         */
        void setPreConcat(GrContext* context, const SkMatrix& preConcat, GrPaint* paint = NULL) {
            SkASSERT(NULL != context);

            this->restore();

            fContext = context;
            fMatrix = context->getMatrix();
            this->preConcat(preConcat, paint);
        }

        /**
         * Sets the context's matrix to identity. Returns false if the inverse matrix is required to
         * update a paint but the matrix cannot be inverted.
         */
        bool setIdentity(GrContext* context, GrPaint* paint = NULL) {
            SkASSERT(NULL != context);

            this->restore();

            if (NULL != paint) {
                if (!paint->localCoordChangeInverse(context->getMatrix())) {
                    return false;
                }
            }
            fMatrix = context->getMatrix();
            fContext = context;
            context->setIdentityMatrix();
            return true;
        }

        /**
         * Replaces the context's matrix with a new matrix. Returns false if the inverse matrix is
         * required to update a paint but the matrix cannot be inverted.
         */
        bool set(GrContext* context, const SkMatrix& newMatrix, GrPaint* paint = NULL) {
            if (NULL != paint) {
                if (!this->setIdentity(context, paint)) {
                    return false;
                }
                this->preConcat(newMatrix, paint);
            } else {
                this->restore();
                fContext = context;
                fMatrix = context->getMatrix();
                context->setMatrix(newMatrix);
            }
            return true;
        }

        /**
         * If this has been initialized then the context's matrix will be further updated by
         * pre-concat'ing the preConcat param. The matrix that will be restored remains unchanged.
         * The paint is assumed to be relative to the context's matrix at the time this call is
         * made, not the matrix at the time AutoMatrix was first initialized. In other words, this
         * performs an incremental update of the paint.
         */
        void preConcat(const SkMatrix& preConcat, GrPaint* paint = NULL) {
            if (NULL != paint) {
                paint->localCoordChange(preConcat);
            }
            fContext->concatMatrix(preConcat);
        }

        /**
         * Returns false if never initialized or the inverse matrix was required to update a paint
         * but the matrix could not be inverted.
         */
        bool succeeded() const { return NULL != fContext; }

        /**
         * If this has been initialized then the context's original matrix is restored.
         */
        void restore() {
            if (NULL != fContext) {
                fContext->setMatrix(fMatrix);
                fContext = NULL;
            }
        }

    private:
        GrContext*  fContext;
        SkMatrix    fMatrix;
    };

    class AutoClip : public ::SkNoncopyable {
    public:
        // This enum exists to require a caller of the constructor to acknowledge that the clip will
        // initially be wide open. It also could be extended if there are other desirable initial
        // clip states.
        enum InitialClip {
            kWideOpen_InitialClip,
        };

        AutoClip(GrContext* context, InitialClip initialState)
        : fContext(context) {
            SkASSERT(kWideOpen_InitialClip == initialState);
            fNewClipData.fClipStack = &fNewClipStack;

            fOldClip = context->getClip();
            context->setClip(&fNewClipData);
        }

        AutoClip(GrContext* context, const SkRect& newClipRect)
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
            , fAutoRT(ctx, rt) {
            fAutoMatrix.setIdentity(ctx);
            // should never fail with no paint param.
            SkASSERT(fAutoMatrix.succeeded());
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
    GrDrawTarget* getTextTarget();
    const GrIndexBuffer* getQuadIndexBuffer() const;

    // Called by tests that draw directly to the context via GrDrawTarget
    void getTestTarget(GrTestTarget*);

    /**
     * Stencil buffers add themselves to the cache using addStencilBuffer. findStencilBuffer is
     * called to check the cache for a SB that matches an RT's criteria.
     */
    void addStencilBuffer(GrStencilBuffer* sb);
    GrStencilBuffer* findStencilBuffer(int width, int height, int sampleCnt);

    GrPathRenderer* getPathRenderer(
                    const SkPath& path,
                    const SkStrokeRec& stroke,
                    const GrDrawTarget* target,
                    bool allowSW,
                    GrPathRendererChain::DrawType drawType = GrPathRendererChain::kColor_DrawType,
                    GrPathRendererChain::StencilSupport* stencilSupport = NULL);


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

    GrGpu*                          fGpu;
    SkMatrix                        fViewMatrix;
    SkAutoTUnref<GrRenderTarget>    fRenderTarget;
    const GrClipData*               fClip;  // TODO: make this ref counted
    GrDrawState*                    fDrawState;

    GrResourceCache*                fTextureCache;
    GrFontCache*                    fFontCache;

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

    GrContext(); // init must be called after the constructor.
    bool init(GrBackend, GrBackendContext);

    void setupDrawBuffer();

    class AutoRestoreEffects;
    class AutoCheckFlush;
    /// Sets the paint and returns the target to draw into. The paint can be NULL in which case the
    /// draw state is left unmodified.
    GrDrawTarget* prepareToDraw(const GrPaint*, BufferedDraw, AutoRestoreEffects*, AutoCheckFlush*);

    void internalDrawPath(GrDrawTarget* target, bool useAA, const SkPath& path,
                          const SkStrokeRec& stroke);

    GrTexture* createResizedTexture(const GrTextureDesc& desc,
                                    const GrCacheID& cacheID,
                                    void* srcData,
                                    size_t rowBytes,
                                    bool filter);

    // Needed so GrTexture's returnToCache helper function can call
    // addExistingTextureToCache
    friend class GrTexture;

    // Add an existing texture to the texture cache. This is intended solely
    // for use with textures released from an GrAutoScratchTexture.
    void addExistingTextureToCache(GrTexture* texture);

    /**
     * These functions create premul <-> unpremul effects if it is possible to generate a pair
     * of effects that make a readToUPM->writeToPM->readToUPM cycle invariant. Otherwise, they
     * return NULL.
     */
    const GrEffectRef* createPMToUPMEffect(GrTexture* texture,
                                           bool swapRAndB,
                                           const SkMatrix& matrix);
    const GrEffectRef* createUPMToPMEffect(GrTexture* texture,
                                           bool swapRAndB,
                                           const SkMatrix& matrix);

    /**
     *  This callback allows the resource cache to callback into the GrContext
     *  when the cache is still overbudget after a purge.
     */
    static bool OverbudgetCB(void* data);

    typedef SkRefCnt INHERITED;
};

/**
 * Gets and locks a scratch texture from a descriptor using either exact or approximate criteria.
 * Unlocks texture in the destructor.
 */
class GrAutoScratchTexture : public ::SkNoncopyable {
public:
    GrAutoScratchTexture()
        : fContext(NULL)
        , fTexture(NULL) {
    }

    GrAutoScratchTexture(GrContext* context,
                         const GrTextureDesc& desc,
                         GrContext::ScratchTexMatch match = GrContext::kApprox_ScratchTexMatch)
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
            fTexture->unref();
            fTexture = NULL;
        }
    }

    /*
     * When detaching a texture we do not unlock it in the texture cache but
     * we do set the returnToCache flag. In this way the texture remains
     * "locked" in the texture cache until it is freed and recycled in
     * GrTexture::internal_dispose. In reality, the texture has been removed
     * from the cache (because this is in AutoScratchTexture) and by not
     * calling unlockScratchTexture we simply don't re-add it. It will be
     * reattached in GrTexture::internal_dispose.
     *
     * Note that the caller is assumed to accept and manage the ref to the
     * returned texture.
     */
    GrTexture* detach() {
        if (NULL == fTexture) {
            return NULL;
        }
        GrTexture* texture = fTexture;
        fTexture = NULL;

        // This GrAutoScratchTexture has a ref from lockAndRefScratchTexture, which we give up now.
        // The cache also has a ref which we are lending to the caller of detach(). When the caller
        // lets go of the ref and the ref count goes to 0 internal_dispose will see this flag is
        // set and re-ref the texture, thereby restoring the cache's ref.
        SkASSERT(texture->getRefCnt() > 1);
        texture->setFlag((GrTextureFlags) GrTexture::kReturnToCache_FlagBit);
        texture->unref();
        SkASSERT(NULL != texture->getCacheEntry());

        return texture;
    }

    GrTexture* set(GrContext* context,
                   const GrTextureDesc& desc,
                   GrContext::ScratchTexMatch match = GrContext::kApprox_ScratchTexMatch) {
        this->reset();

        fContext = context;
        if (NULL != fContext) {
            fTexture = fContext->lockAndRefScratchTexture(desc, match);
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
