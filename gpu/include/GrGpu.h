/*
    Copyright 2011 Google Inc.

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

#ifndef GrGpu_DEFINED
#define GrGpu_DEFINED

#include "GrRect.h"
#include "GrRefCnt.h"
#include "GrDrawTarget.h"
#include "GrTexture.h"

class GrVertexBufferAllocPool;
class GrIndexBufferAllocPool;
class GrPathRenderer;

class GrGpu : public GrDrawTarget {

public:
    /**
     * Possible 3D APIs that may be used by Ganesh.
     */
    enum Engine {
        kOpenGL_Shaders_Engine,
        kOpenGL_Fixed_Engine,
        kDirect3D9_Engine
    };

    /**
     * Platform specific 3D context.
     * For
     *    kOpenGL_Shaders_Engine use NULL
     *    kOpenGL_Fixed_Engine   use NULL
     *    kDirect3D9_Engine      use an IDirect3DDevice9*
     */
    typedef void* Platform3DContext;

    /**
     *  Create an instance of GrGpu that matches the specified Engine backend.
     *  If the requested engine is not supported (at compile-time or run-time)
     *  this returns NULL.
     */
    static GrGpu* Create(Engine, Platform3DContext context3D);

    /**
     * Used to control the level of antialiasing available for a rendertarget.
     * Anti-alias quality levels depend on the underlying API/GPU capabilities.
     */
    enum AALevels {
        kNone_AALevel, //<! No antialiasing available.
        kLow_AALevel,  //<! Low quality antialiased rendering. Actual
                       //   interpretation is platform-dependent.
        kMed_AALevel,  //<! Medium quality antialiased rendering. Actual
                       //   interpretation is platform-dependent.
        kHigh_AALevel, //<! High quality antialiased rendering. Actual
                       //   interpretation is platform-dependent.
    };


    /**
     * Optional bitfield flags that can be passed to createTexture.
     */
    enum TextureFlags {
        kRenderTarget_TextureFlag  = 0x1,   //<! Creates a texture that can be
                                            //   rendered to by calling
                                            //   GrGpu::setRenderTarget() with
                                            //   GrTexture::asRenderTarget().
        kNoPathRendering_TextureFlag = 0x2, //<! If the texture is used as a
                                            //   rendertarget but paths will not
                                            //   be rendered to it.
        kDynamicUpdate_TextureFlag = 0x4    //!< Hint that the CPU may modify
                                            // this texture after creation
    };

    enum {
        /**
         *  For Index8 pixel config, the colortable must be 256 entries
         */
        kColorTableSize = 256 * sizeof(GrColor)
    };
    /**
     * Describes a texture to be created.
     */
    struct TextureDesc {
        uint32_t               fFlags;  //!< bitfield of TextureFlags
        GrGpu::AALevels        fAALevel;//!< The level of antialiasing available
                                        //   for a rendertarget texture. Only
                                        //   flags contains
                                        //   kRenderTarget_TextureFlag.
        uint32_t               fWidth;  //!< Width of the texture
        uint32_t               fHeight; //!< Height of the texture
        GrTexture::PixelConfig fFormat; //!< Format of source data of the
                                        //   texture. Not guaraunteed to be the
                                        //   same as internal format used by
                                        //   3D API.
    };

    /**
     * Gpu usage statistics.
     */
    struct Stats {
        uint32_t fVertexCnt;  //<! Number of vertices drawn
        uint32_t fIndexCnt;   //<! Number of indices drawn
        uint32_t fDrawCnt;    //<! Number of draws

        uint32_t fProgChngCnt;//<! Number of program changes (N/A for fixed)

        /*
         *  Number of times the texture is set in 3D API
         */
        uint32_t fTextureChngCnt;
        /*
         *  Number of times the render target is set in 3D API
         */
        uint32_t fRenderTargetChngCnt;
        /*
         *  Number of textures created (includes textures that are rendertargets).
         */
        uint32_t fTextureCreateCnt;
        /*
         *  Number of rendertargets created.
         */
        uint32_t fRenderTargetCreateCnt;
    };

    ////////////////////////////////////////////////////////////////////////////

    GrGpu();
    virtual ~GrGpu();

    /**
     * The GrGpu object normally assumes that no outsider is setting state
     * within the underlying 3D API's context/device/whatever. This call informs
     * the GrGpu that the state was modified and it shouldn't make assumptions
     * about the state.
     */
    void markContextDirty() { fContextIsDirty = true; }

    void unimpl(const char[]);

    /**
     * Creates a texture object. If desc width or height is not a power of
     * two but underlying API requires a power of two texture then srcData
     * will be embedded in a power of two texture. The extra width and height
     * is filled as though srcData were rendered clamped into the texture.
     *
     * If kRenderTarget_TextureFlag is specified the GrRenderTarget is 
     * accessible via GrTexture::asRenderTarget(). The texture will hold a ref
     * on the render target until its releaseRenderTarget() is called or it is
     * destroyed.
     *
     * @param desc        describes the texture to be created.
     * @param srcData     texel data to load texture. Begins with full-size
     *                    palette data for paletted textures. Contains width*
     *                    height texels. If NULL texture data is uninitialized.
     *
     * @return    The texture object if successful, otherwise NULL.
     */
    GrTexture* createTexture(const TextureDesc& desc,
                             const void* srcData, size_t rowBytes);
    /**
     * Wraps an externally-created rendertarget in a GrRenderTarget.
     * @param platformRenderTarget  handle to the the render target in the
     *                              underlying 3D API. Interpretation depends on
     *                              GrGpu subclass in use.
     * @param stencilBits           number of stencil bits the target has
     * @param width                 width of the render target
     * @param height                height of the render target
     */
    virtual GrRenderTarget* createPlatformRenderTarget(
                                                intptr_t platformRenderTarget,
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
    GrRenderTarget* createRenderTargetFrom3DApiState();

    /**
     * Creates a vertex buffer.
     *
     * @param size    size in bytes of the vertex buffer
     * @param dynamic hints whether the data will be frequently changed
     *                by either GrVertexBuffer::lock or
     *                GrVertexBuffer::updateData.
     *
     * @return    The vertex buffer if successful, otherwise NULL.
     */
    GrVertexBuffer* createVertexBuffer(uint32_t size, bool dynamic);

    /**
     * Creates an index buffer.
     *
     * @param size    size in bytes of the index buffer
     * @param dynamic hints whether the data will be frequently changed
     *                by either GrIndexBuffer::lock or
     *                GrIndexBuffer::updateData.
     *
     * @return The index buffer if successful, otherwise NULL.
     */
    GrIndexBuffer* createIndexBuffer(uint32_t size, bool dynamic);

    /**
     * Erase the entire render target, ignoring any clips/scissors.
     *
     * This is issued to the GPU driver immediately.
     */
    void eraseColor(GrColor color);

    /**
     * Are 8 bit paletted textures supported.
     *
     * @return    true if 8bit palette textures are supported, false otherwise
     */
    bool supports8BitPalette() const { return f8bitPaletteSupport; }

    /**
     * returns true if two sided stenciling is supported. If false then only
     * the front face values of the GrStencilSettings
     * @return    true if only a single stencil pass is needed.
     */
    bool supportsTwoSidedStencil() const
                                        { return fTwoSidedStencilSupport; }

    /**
     * returns true if stencil wrap is supported. If false then
     * kIncWrap_StencilOp and kDecWrap_StencilOp are treated as
     * kIncClamp_StencilOp and kDecClamp_StencilOp, respectively.
     * @return    true if stencil wrap ops are supported.
     */
    bool supportsStencilWrapOps() const
                                        { return fStencilWrapOpsSupport; }

    /**
     * Checks whether locking vertex and index buffers is supported.
     *
     * @return true if locking is supported.
     */
    bool supportsBufferLocking() const { return fBufferLockSupport; }

    /**
     * Gets the minimum width of a render target. If a texture/rt is created
     * with a width less than this size the GrGpu object will clamp it to this
     * value.
     */
    int minRenderTargetWidth() const { return fMinRenderTargetWidth; }

    /**
     * Gets the minimum width of a render target. If a texture/rt is created
     * with a height less than this size the GrGpu object will clamp it to this
     * value.
     */
    int minRenderTargetHeight() const  { return fMinRenderTargetHeight; }

    /**
     * Returns true if NPOT textures can be created
     *
     * @return    true if NPOT textures can be created
     */
    bool npotTextureSupport() const { return fNPOTTextureSupport; }

    /**
     * Returns true if NPOT textures can be repeat/mirror tiled.
     *
     * @return    true if NPOT textures can be tiled
     */
    bool npotTextureTileSupport() const { return fNPOTTextureTileSupport; }

    /**
     * Returns true if a NPOT texture can be a rendertarget
     *
     * @return    the true if NPOT texture/rendertarget can be created.
     */
    bool npotRenderTargetSupport() const { return fNPOTRenderTargetSupport; }

    int maxTextureDimension() const { return fMaxTextureDimension; }

    // GrDrawTarget overrides
    virtual void drawIndexed(GrPrimitiveType type,
                             int startVertex,
                             int startIndex,
                             int vertexCount,
                             int indexCount);

    virtual void drawNonIndexed(GrPrimitiveType type,
                                int startVertex,
                                int vertexCount);

    /**
     * Returns an index buffer that can be used to render quads.
     * Six indices per quad: 0, 1, 2, 0, 2, 3, etc.
     * The max number of quads can be queried using GrIndexBuffer::maxQuads().
     * Draw with kTriangles_PrimitiveType
     * @ return the quad index buffer
     */
    const GrIndexBuffer* getQuadIndexBuffer() const;

    /**
     * Returns a vertex buffer with four position-only vertices [(0,0), (1,0),
     * (1,1), (0,1)].
     * @ return unit square vertex buffer
     */
    const GrVertexBuffer* getUnitSquareVertexBuffer() const;

    /**
     * Ensures that the current render target is actually set in the
     * underlying 3D API. Used when client wants to use 3D API to directly
     * render to the RT.
     */
    void forceRenderTargetFlush();

    /**
     * Reads a rectangle of pixels from the current render target.
     * @param left      left edge of the rectangle to read (inclusive)
     * @param top       top edge of the rectangle to read (inclusive)
     * @param width     width of rectangle to read in pixels.
     * @param height    height of rectangle to read in pixels.
     * @param buffer    memory to read the rectangle into.
     *
     * @return true if the read succeeded, false if not. The read can fail
     *              because of a unsupported pixel config or because no render
     *              target is currently set.
     */
    bool readPixels(int left, int top, int width, int height,
                    GrTexture::PixelConfig, void* buffer);


    const Stats& getStats() const;
    void resetStats();
    void printStats() const;

protected:
    enum PrivateStateBits {
        kFirstBit = (kLastPublicStateBit << 1),

        kModifyStencilClip_StateBit = kFirstBit, // allows draws to modify
                                                 // stencil bits used for
                                                 // clipping.
    };

    /**
     * Extensions to GrDrawTarget::StateBits to implement stencil clipping
     */
    struct ClipState {
        bool            fClipInStencil;
        bool            fClipIsDirty;
    } fClipState;

    // GrDrawTarget override
    virtual void clipWillBeSet(const GrClip& newClip);

    // prepares clip flushes gpu state before a draw
    bool setupClipAndFlushState(GrPrimitiveType type);

    // Functions used to map clip-respecting stencil tests into normal
    // stencil funcs supported by GPUs.
    static GrStencilFunc ConvertStencilFunc(bool stencilInClip,
                                            GrStencilFunc func);
    static void ConvertStencilFuncAndMask(GrStencilFunc func,
                                          bool clipInStencil,
                                          unsigned int clipBit,
                                          unsigned int userBits,
                                          unsigned int* ref,
                                          unsigned int* mask);

    // stencil settings to clip drawing when stencil clipping is in effect
    // and the client isn't using the stencil test.
    static const GrStencilSettings gClipStencilSettings;

    // defaults to false, subclass can set true to support palleted textures
    bool f8bitPaletteSupport;

    // set by subclass
    bool fNPOTTextureSupport;
    bool fNPOTTextureTileSupport;
    bool fNPOTRenderTargetSupport;
    bool fTwoSidedStencilSupport;
    bool fStencilWrapOpsSupport;

    // set by subclass to true if index and vertex buffers can be locked, false
    // otherwise.
    bool fBufferLockSupport;

    // set by subclass
    int fMinRenderTargetWidth;
    int fMinRenderTargetHeight;
    int fMaxTextureDimension;

    Stats           fStats;

    const GrVertexBuffer*           fCurrPoolVertexBuffer;
    int                             fCurrPoolStartVertex;

    const GrIndexBuffer*            fCurrPoolIndexBuffer;
    int                             fCurrPoolStartIndex;

    // GrDrawTarget overrides
    virtual bool acquireGeometryHelper(GrVertexLayout vertexLayout,
                                       void**         vertices,
                                       void**         indices);
    virtual void releaseGeometryHelper();

    virtual void setVertexSourceToArrayHelper(const void* vertexArray,
                                              int vertexCount);

    virtual void setIndexSourceToArrayHelper(const void* indexArray,
                                             int indexCount);
    // Helpers for setting up geometry state
    void finalizeReservedVertices();
    void finalizeReservedIndices();

    // overridden by API-specific derived class to handle re-emitting 3D API
    // preample and dirtying state cache.
    virtual void resetContext() = 0;

    // overridden by API-specific derived class to create objects.
    virtual GrTexture* createTextureHelper(const TextureDesc& desc,
                                           const void* srcData,
                                           size_t rowBytes) = 0;
    virtual GrRenderTarget* createPlatformRenderTargetHelper(
                                                intptr_t platformRenderTarget,
                                                int stencilBits,
                                                int width, int height) = 0;
    virtual GrRenderTarget* createRenderTargetFrom3DApiStateHelper() = 0;
    virtual GrVertexBuffer* createVertexBufferHelper(uint32_t size,
                                                     bool dynamic) = 0;
    virtual GrIndexBuffer* createIndexBufferHelper(uint32_t size,
                                                   bool dynamic) = 0;

    // overridden by API-specific derivated class to perform the erase.
    virtual void eraseColorHelper(GrColor color) = 0;

    // overridden by API-specific derived class to perform the draw call.
    virtual void drawIndexedHelper(GrPrimitiveType type,
                                   uint32_t startVertex,
                                   uint32_t startIndex,
                                   uint32_t vertexCount,
                                   uint32_t indexCount) = 0;

    virtual void drawNonIndexedHelper(GrPrimitiveType type,
                                      uint32_t vertexCount,
                                      uint32_t numVertices) = 0;

    // overridden by API-specific derived class to perform flush
    virtual void forceRenderTargetFlushHelper() = 0;

    // overridden by API-specific derived class to perform the read pixels.
    virtual bool readPixelsHelper(int left, int top, int width, int height,
                                  GrTexture::PixelConfig, void* buffer) = 0;

    // called to program the vertex data, indexCount will be 0 if drawing non-
    // indexed geometry. The subclass may adjust the startVertex and/or
    // startIndex since it may have already accounted for these in the setup.
    virtual void setupGeometry(int* startVertex,
                               int* startIndex,
                               int vertexCount,
                               int indexCount) = 0;


    // The GrGpu typically records the clients requested state and then flushes
    // deltas from previous state at draw time. This function does the
    // API-specific flush of the state
    // returns false if current state is unsupported.
    virtual bool flushGraphicsState(GrPrimitiveType type) = 0;

    // Sets the scissor rect, or disables if rect is NULL.
    virtual void flushScissor(const GrIRect* rect) = 0;

    // GrGpu subclass removes the clip from the stencil buffer
    virtual void eraseStencilClip(const GrIRect& rect) = 0;

private:
    // readies the pools to provide vertex/index data.
    void prepareVertexPool();
    void prepareIndexPool();

    GrPathRenderer* getPathRenderer();

    void handleDirtyContext() {
        if (fContextIsDirty) {
            this->resetContext();
            fContextIsDirty = false;
        }
    }

    GrVertexBufferAllocPool*    fVertexPool;

    GrIndexBufferAllocPool*     fIndexPool;

    mutable GrIndexBuffer*      fQuadIndexBuffer; // mutable so it can be
                                                  // created on-demand

    mutable GrVertexBuffer*     fUnitSquareVertexBuffer; // mutable so it can be
                                                         // created on-demand

    GrPathRenderer*             fPathRenderer;

    bool                        fContextIsDirty;

    // when in an internal draw these indicate whether the pools are in use
    // by one of the outer draws. If false then it is safe to reset the
    // pool.
    bool                        fVertexPoolInUse;
    bool                        fIndexPoolInUse;

    // used to save and restore state when the GrGpu needs
    // to make its geometry pools available internally
    class AutoInternalDrawGeomRestore {
    public:
        AutoInternalDrawGeomRestore(GrGpu* gpu) : fAgsr(gpu) {
            fGpu = gpu;

            fVertexPoolWasInUse = gpu->fVertexPoolInUse;
            fIndexPoolWasInUse  = gpu->fIndexPoolInUse;

            gpu->fVertexPoolInUse = fVertexPoolWasInUse ||
                                   (kBuffer_GeometrySrcType !=
                                    gpu->fGeometrySrc.fVertexSrc);
            gpu->fIndexPoolInUse  = fIndexPoolWasInUse ||
                                   (kBuffer_GeometrySrcType !=
                                    gpu->fGeometrySrc.fIndexSrc);;

            fSavedPoolVertexBuffer = gpu->fCurrPoolVertexBuffer;
            fSavedPoolStartVertex  = gpu->fCurrPoolStartVertex;
            fSavedPoolIndexBuffer  = gpu->fCurrPoolIndexBuffer;
            fSavedPoolStartIndex   = gpu->fCurrPoolStartIndex;

            fSavedReservedGeometry = gpu->fReservedGeometry;
            gpu->fReservedGeometry.fLocked = false;
        }
        ~AutoInternalDrawGeomRestore() {
            fGpu->fCurrPoolVertexBuffer = fSavedPoolVertexBuffer;
            fGpu->fCurrPoolStartVertex  = fSavedPoolStartVertex;
            fGpu->fCurrPoolIndexBuffer  = fSavedPoolIndexBuffer;
            fGpu->fCurrPoolStartIndex   = fSavedPoolStartIndex;
            fGpu->fVertexPoolInUse = fVertexPoolWasInUse;
            fGpu->fIndexPoolInUse  = fIndexPoolWasInUse;
            fGpu->fReservedGeometry = fSavedReservedGeometry;
        }
    private:
        AutoGeometrySrcRestore  fAgsr;
        GrGpu*                  fGpu;
        const GrVertexBuffer*   fSavedPoolVertexBuffer;
        int                     fSavedPoolStartVertex;
        const GrIndexBuffer*    fSavedPoolIndexBuffer;
        int                     fSavedPoolStartIndex;
        bool                    fVertexPoolWasInUse;
        bool                    fIndexPoolWasInUse;
        ReservedGeometry        fSavedReservedGeometry;
    };

    typedef GrDrawTarget INHERITED;
};

#endif
