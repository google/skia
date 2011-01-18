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
#include "GrSamplerState.h"
#include "GrTextureCache.h"
#include "GrInOrderDrawBuffer.h"
#include "GrVertexBufferAllocPool.h"

class GrFontCache;
class GrPathIter;

//TODO: move GrGpu enums/nested types here

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
     * Wraps an externally-created rendertarget in a GrRenderTarget.
     * e.g. in GL platforamRenderTarget is an FBO id.
     */
    GrRenderTarget* createPlatformRenderTarget(intptr_t platformRenderTarget,
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
     *  Returns true if the specified use of an indexed texture is supported.
     */
    bool supportsIndex8PixelConfig(const GrSamplerState&, int width, int height);

    ///////////////////////////////////////////////////////////////////////////

    GrRenderTarget* currentRenderTarget() const;
    void getViewMatrix(GrMatrix* m) const;
    const GrClip& getClip() const { return fGpu->getClip(); }

    void setRenderTarget(GrRenderTarget* target);

    void setTexture(int stage, GrTexture* texture);
    void setSamplerState(int stage, const GrSamplerState&);
    void setTextureMatrix(int stage, const GrMatrix& m);

    void setAntiAlias(bool);
    void setDither(bool);
    void setAlpha(uint8_t alpha);
    void setColor(GrColor color);
    void setPointSize(float size);
    void setBlendFunc(GrGpu::BlendCoeff srcCoef, GrGpu::BlendCoeff dstCoef);
    void setViewMatrix(const GrMatrix& m);
    void setClip(const GrClip&);

    /**
     *  Erase the entire render target, ignoring any clips/scissors.
     */
    void eraseColor(GrColor color);

    /**
     *  Draw everywhere (respecting the clip) with the current color.
     */
    void drawFull(bool useTexture);

    /**
     *  Draw the rect, respecting the current texture if useTexture is true.
     *  If strokeWidth < 0, then the rect is filled, else the rect is stroked
     *  based on strokeWidth. If strokeWidth == 0, then the stroke is always
     *  a single pixel thick.
     */
    void drawRect(const GrRect&, bool useTexture, GrScalar strokeWidth);

    void fillRect(const GrRect& rect, bool useTexture) {
        this->drawRect(rect, useTexture, -1);
    }

    /**
     * Path filling rules
     */
    enum PathFills {
        kWinding_PathFill,
        kEvenOdd_PathFill,
        kInverseWinding_PathFill,
        kInverseEvenOdd_PathFill,
        kHairLine_PathFill,

        kPathFillCount
    };

    /**
     * Tessellates and draws a path.
     *
     * @param path          the path to draw
     * @param paint         the paint to set before drawing
     * @param useTexture    if true the path vertices will also be used as
     *                      texture coorindates referencing last texture passed
     *                      to setTexture.
     */
    void drawPath(GrPathIter* path,
                  PathFills fill,
                  bool useTexture,
                  const GrPoint* translate = NULL);

    /**
     * Call to ensure all drawing to the context has been issued to the
     * underlying 3D API.
     * if flushRenderTarget is true then after the call the last
     * rendertarget set will be current in the underlying 3D API, otherwise
     * it may not be. It is useful to set if the caller plans to use the 3D
     * context outside of Ganesh to render into the current RT.
     */
    void flush(bool flushRenderTarget);

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

    /* -------------------------------------------------------
     * Mimicking the GrGpu interface for now
     * TODO: define appropriate higher-level API for context
     */

    GrVertexBuffer* createVertexBuffer(uint32_t size, bool dynamic);

    GrIndexBuffer* createIndexBuffer(uint32_t size, bool dynamic);

    bool reserveAndLockGeometry(GrVertexLayout    vertexLayout,
                                uint32_t          vertexCount,
                                uint32_t          indexCount,
                                void**            vertices,
                                void**            indices);

    void drawIndexed(GrGpu::PrimitiveType type,
                     uint32_t startVertex,
                     uint32_t startIndex,
                     uint32_t vertexCount,
                     uint32_t indexCount);

    void drawNonIndexed(GrGpu::PrimitiveType type,
                        uint32_t startVertex,
                        uint32_t vertexCount);

    void setVertexSourceToArray(const void* array,
                                GrVertexLayout vertexLayout);
    void setIndexSourceToArray(const void* array);
    void setVertexSourceToBuffer(GrVertexBuffer* buffer,
                                GrVertexLayout vertexLayout);
    void setIndexSourceToBuffer(GrIndexBuffer* buffer);

    void releaseReservedGeometry();

    void resetStats();

    const GrGpu::Stats& getStats() const;

    void printStats() const;

    class AutoRenderTarget : ::GrNoncopyable {
    public:
        AutoRenderTarget(GrContext* context, GrRenderTarget* target) {
            fContext = NULL;
            fPrevTarget = context->currentRenderTarget();
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

    /* -------------------------------------------------------
     */

    // Intended only to be used within Ganesh:
    GrGpu* getGpu() { return fGpu; }
    GrFontCache* getFontCache() { return fFontCache; }
    GrDrawTarget* getTextTarget();
    void flushText();

    const GrIndexBuffer* quadIndexBuffer() const;
    int maxQuadsInIndexBuffer() const;

private:
    GrGpu*          fGpu;
    GrTextureCache* fTextureCache;
    GrFontCache*    fFontCache;

    GrVertexBufferAllocPool fVBAllocPool;
    GrInOrderDrawBuffer     fTextDrawBuffer;

    GrContext(GrGpu* gpu);
    bool finalizeTextureKey(GrTextureKey*, const GrSamplerState&) const;

    void drawClipIntoStencil();
};

/**
 *  Save/restore the view-matrix in the context.
 */
class GrAutoViewMatrix : GrNoncopyable {
public:
    GrAutoViewMatrix(GrContext* ctx) : fContext(ctx) {
        ctx->getViewMatrix(&fMatrix);
    }
    GrAutoViewMatrix(GrContext* ctx, const GrMatrix& matrix) : fContext(ctx) {
        ctx->getViewMatrix(&fMatrix);
        ctx->setViewMatrix(matrix);
    }
    ~GrAutoViewMatrix() {
        fContext->setViewMatrix(fMatrix);
    }

private:
    GrContext*  fContext;
    GrMatrix    fMatrix;
};

#endif

