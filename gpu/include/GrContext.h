/*
    Copyright 2010 Google Inc.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

         http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
 */

#ifndef GrContext_DEFINED
#define GrContext_DEFINED

#include "GrClip.h"
#include "GrGpu.h"
#include "GrTextureCache.h"
#include "GrPaint.h"

class GrFontCache;
class GrPathIter;
class GrVertexBufferAllocPool;
class GrIndexBufferAllocPool;
class GrInOrderDrawBuffer;
class GrPathRenderer;

class GrContext : public GrRefCnt {
public:
    /**
     * Creates a GrContext from within a 3D context.
     */
    static GrContext* Create(GrGpu::Engine engine,
                             GrGpu::Platform3DContext context3D);

    /**
     *  Helper to create a opengl-shader based context
     */
    static GrContext* CreateGLShaderContext();

    virtual ~GrContext();

    /**
     * The GrContext normally assumes that no outsider is setting state
     * within the underlying 3D API's context/device/whatever. This call informs
     * the context that the state was modified and it should resend. Shouldn't
     * be called frequently for good performance.
     */
    void resetContext();

    ///////////////////////////////////////////////////////////////////////////
    // Textures

    /**
     *  Abandons all textures. Call this if you have lost the associated GPU
     *  context, and thus internal texture references/IDs are now invalid.
     */
    void abandonAllTextures();

    /**
     *  Search for an entry with the same Key. If found, "lock" it and return it.
     *  If not found, return null.
     */
    GrTextureEntry* findAndLockTexture(GrTextureKey*,
                                       const GrSamplerState&);


    /**
     *  Create a new entry, based on the specified key and texture, and return
     *  its "locked" entry.
     *
     *  Ownership of the texture is transferred to the Entry, which will unref()
     *  it when we are purged or deleted.
     */
    GrTextureEntry* createAndLockTexture(GrTextureKey* key,
                                         const GrSamplerState&,
                                         const GrGpu::TextureDesc&,
                                         void* srcData, size_t rowBytes);

    /**
     *  When done with an entry, call unlockTexture(entry) on it, which returns
     *  it to the cache, where it may be purged.
     */
    void unlockTexture(GrTextureEntry* entry);

    /**
     *  Removes an texture from the cache. This prevents the texture from
     *  being found by a subsequent findAndLockTexture() until it is
     *  reattached. The entry still counts against the cache's budget and should
     *  be reattached when exclusive access is no longer needed.
     */
    void detachCachedTexture(GrTextureEntry*);

    /**
     * Reattaches a texture to the cache and unlocks it. Allows it to be found
     * by a subsequent findAndLock or be purged (provided its lock count is
     * now 0.)
     */
    void reattachAndUnlockCachedTexture(GrTextureEntry*);

    /**
     * Creates a texture that is outside the cache. Does not count against
     * cache's budget.
     */
    GrTexture* createUncachedTexture(const GrGpu::TextureDesc&,
                                     void* srcData,
                                     size_t rowBytes);

    /**
     *  Returns true if the specified use of an indexed texture is supported.
     */
    bool supportsIndex8PixelConfig(const GrSamplerState&, int width, int height);

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
    int getMaxTextureDimension();

    ///////////////////////////////////////////////////////////////////////////
    // Render targets

    /**
     * Wraps an externally-created rendertarget in a GrRenderTarget.
     * @param platformRenderTarget  3D API-specific render target identifier
     *                              e.g. in GL platforamRenderTarget is an FBO
     *                              id.
     * @param stencilBits           the number of stencil bits that the render
     *                              target has.
     * @param width                 width of the render target.
     * @param height                height of the render target.
     */
    GrRenderTarget* createPlatformRenderTarget(intptr_t platformRenderTarget,
                                               int stencilBits,
                                               int width, int height);

    /**
     * Reads the current target object (e.g. FBO or IDirect3DSurface9*) and
     * viewport state from the underlying 3D API and wraps it in a
     * GrRenderTarget. The GrRenderTarget will not attempt to delete/destroy the
     * underlying object in its destructor and it is up to caller to guarantee
     * that it remains valid while the GrRenderTarget is used.
     *
     * @return the newly created GrRenderTarget
     */
    GrRenderTarget* createRenderTargetFrom3DApiState() {
        return fGpu->createRenderTargetFrom3DApiState();
    }

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
    const GrClip& getClip() const { return fGpu->getClip(); }

    /**
     * Sets the clip.
     * @param clip  the clip to set.
     */
    void setClip(const GrClip& clip);

    /**
     * Convenience method for setting the clip to a rect.
     * @param rect  the rect to set as the new clip.
     */
    void setClip(const GrIRect& rect);

    ///////////////////////////////////////////////////////////////////////////
    // Draws

    /**
     *  Erase the entire render target, ignoring any clips
     */
    void eraseColor(GrColor color);

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
     * @param pathIter      the path to draw
     * @param fill          the path filling rule to use.
     * @param translate     optional additional translation applied to the
     *                      path.
     */
    void drawPath(const GrPaint& paint,
                  GrPathIter* pathIter,
                  GrPathFill fill,
                  const GrPoint* translate = NULL);
    /**
     * Helper version of drawPath that takes a GrPath
     */
    void drawPath(const GrPaint& paint,
                  const GrPath& path,
                  GrPathFill fill,
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
     * Similar to drawVertices but caller provides objects that convert to Gr
     * types. The count of vertices is given by posSrc.
     *
     * @param   paint           describes how to color pixels.
     * @param   primitiveType   primitives type to draw.
     * @param   posSrc          Source of vertex positions. Must implement
     *                              int count() const;
     *                              void writeValue(int i, GrPoint* point) const;
     *                          count returns the total number of vertices and
     *                          writeValue writes a vertex position to point.
     * @param   texSrc          optional, pass NULL to not use explicit tex
     *                          coords. If present provides tex coords with
     *                          method:
     *                              void writeValue(int i, GrPoint* point) const;
     * @param   texSrc          optional, pass NULL to not use per-vertex colors
     *                          If present provides colors with method:
     *                              void writeValue(int i, GrColor* point) const;
     * @param   indices         optional, pass NULL for non-indexed drawing. If
     *                          present supplies indices for indexed drawing
     *                          with following methods:
     *                              int count() const;
     *                              void writeValue(int i, uint16_t* point) const;
     *                          count returns the number of indices and
     *                          writeValue supplies each index.
     */
    template <typename POS_SRC,
              typename TEX_SRC,
              typename COL_SRC,
              typename IDX_SRC>
    void drawCustomVertices(const GrPaint& paint,
                            GrPrimitiveType primitiveType,
                            const POS_SRC& posSrc,
                            const TEX_SRC* texCoordSrc,
                            const COL_SRC* colorSrc,
                            const IDX_SRC* idxSrc);
    /**
     * To avoid the problem of having to create a typename for NULL parameters,
     * these reduced versions of drawCustomVertices are provided.
     */
    template <typename POS_SRC>
    void drawCustomVertices(const GrPaint& paint,
                            GrPrimitiveType primitiveType,
                            const POS_SRC& posSrc);
    template <typename POS_SRC, typename TEX_SRC>
    void drawCustomVertices(const GrPaint& paint,
                            GrPrimitiveType primitiveType,
                            const POS_SRC& posSrc,
                            const TEX_SRC* texCoordSrc);
    template <typename POS_SRC, typename TEX_SRC, typename COL_SRC>
    void drawCustomVertices(const GrPaint& paint,
                            GrPrimitiveType primitiveType,
                            const POS_SRC& posSrc,
                            const TEX_SRC* texCoordSrc,
                            const COL_SRC* colorSrc);


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
    void flush(int flagsBitfield);
    /**
     *  Return true on success, i.e. if we could copy the specified range of
     *  pixels from the current render-target into the buffer, converting into
     *  the specified pixel-config.
     */
    bool readPixels(int left, int top, int width, int height,
                    GrTexture::PixelConfig, void* buffer);

    /**
     *  Copy the src pixels [buffer, stride, pixelconfig] into the current
     *  render-target at the specified rectangle.
     */
    void writePixels(int left, int top, int width, int height,
                     GrTexture::PixelConfig, const void* buffer, size_t stride);


    ///////////////////////////////////////////////////////////////////////////
    // Statistics

    void resetStats();

    const GrGpu::Stats& getStats() const;

    void printStats() const;

    ///////////////////////////////////////////////////////////////////////////
    // Helpers

    class AutoRenderTarget : ::GrNoncopyable {
    public:
        AutoRenderTarget(GrContext* context, GrRenderTarget* target) {
            fContext = NULL;
            fPrevTarget = context->getRenderTarget();
            if (fPrevTarget != target) {
                context->setRenderTarget(target);
                fContext = context;
            }
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


    ///////////////////////////////////////////////////////////////////////////
    // Functions intended for internal use only.
    GrGpu* getGpu() { return fGpu; }
    GrFontCache* getFontCache() { return fFontCache; }
    GrDrawTarget* getTextTarget(const GrPaint& paint);
    void flushText();
    const GrIndexBuffer* getQuadIndexBuffer() const;

private:
    // used to keep track of when we need to flush the draw buffer
    enum DrawCategory {
        kBuffered_DrawCategory,      // last draw was inserted in draw buffer
        kUnbuffered_DrawCategory,    // last draw was not inserted in the draw buffer
        kText_DrawCategory           // text context was last to draw
    };
    DrawCategory fLastDrawCategory;

    GrGpu*          fGpu;
    GrTextureCache* fTextureCache;
    GrFontCache*    fFontCache;
    GrPathRenderer* fPathRenderer;

    GrVertexBufferAllocPool*    fDrawBufferVBAllocPool;
    GrIndexBufferAllocPool*     fDrawBufferIBAllocPool;
    GrInOrderDrawBuffer*        fDrawBuffer;

    GrContext(GrGpu* gpu);
    void flushDrawBuffer();

    static void SetPaint(const GrPaint& paint, GrDrawTarget* target);

    bool finalizeTextureKey(GrTextureKey*, const GrSamplerState&) const;

    GrDrawTarget* prepareToDraw(const GrPaint& paint, DrawCategory drawType);

    void drawClipIntoStencil();
};

/**
 *  Save/restore the view-matrix in the context.
 */
class GrAutoMatrix : GrNoncopyable {
public:
    GrAutoMatrix(GrContext* ctx) : fContext(ctx) {
        fMatrix = ctx->getMatrix();
    }
    GrAutoMatrix(GrContext* ctx, const GrMatrix& matrix) : fContext(ctx) {
        fMatrix = ctx->getMatrix();
        ctx->setMatrix(matrix);
    }
    ~GrAutoMatrix() {
        fContext->setMatrix(fMatrix);
    }

private:
    GrContext*  fContext;
    GrMatrix    fMatrix;
};

#endif

#include "GrContext_impl.h"
