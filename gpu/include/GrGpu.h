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


#ifndef GrGpu_DEFINED
#define GrGpu_DEFINED

#include "GrRect.h"
#include "GrRefCnt.h"
#include "GrDrawTarget.h"
#include "GrTexture.h"

class GrVertexBufferAllocPool;
class GrIndexBufferAllocPool;

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
     * the GrGpu that the state was modified and it should resend. Shouldn't
     * be called frequently for good performance.
     */
    virtual void resetContext();

    void unimpl(const char[]);

    /**
     * Creates a texture object. If desc width or height is not a power of
     * two but underlying API requires a power of two texture then srcData
     * will be embedded in a power of two texture. The extra width and height
     * is filled as though srcData were rendered clamped into the texture.
     *
     * @param desc        describes the texture to be created.
     * @param srcData     texel data to load texture. Begins with full-size
     *                    palette data for paletted textures. Contains width*
     *                    height texels. If NULL texture data is uninitialized.
     *
     * @return    The texture object if successful, otherwise NULL.
     */
    virtual GrTexture* createTexture(const TextureDesc& desc,
                                     const void* srcData, size_t rowBytes) = 0;
    /**
     * Wraps an externally-created rendertarget in a GrRenderTarget.
     * @param platformRenderTarget  handle to the the render target in the
     *                              underlying 3D API. Interpretation depends on
     *                              GrGpu subclass in use.
     * @param width                 width of the render target
     * @param height                height of the render target
     */
    virtual GrRenderTarget* createPlatformRenderTarget(
                                                intptr_t platformRenderTarget,
                                                int width, int height) = 0;

    /**
     * Reads the current target object (e.g. FBO or IDirect3DSurface9*) and
     * viewport state from the underlying 3D API and wraps it in a
     * GrRenderTarget. The GrRenderTarget will not attempt to delete/destroy the
     * underlying object in its destructor and it is up to caller to guarantee
     * that it remains valid while the GrRenderTarget is used.
     *
     * @return the newly created GrRenderTarget
     */
    virtual GrRenderTarget* createRenderTargetFrom3DApiState() = 0;

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
    virtual GrVertexBuffer* createVertexBuffer(uint32_t size, bool dynamic) = 0;

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
    virtual GrIndexBuffer* createIndexBuffer(uint32_t size, bool dynamic) = 0;

    /**
     * Erase the entire render target, ignoring any clips/scissors.
     *
     * This is issued to the GPU driver immediately.
     */
    virtual void eraseColor(GrColor color) = 0;

    /**
     * Are 8 bit paletted textures supported.
     *
     * @return    true if 8bit palette textures are supported, false otherwise
     */
    bool supports8BitPalette() const { return f8bitPaletteSupport; }

    /**
     * If single stencil pass winding is supported then one stencil pass
     * (kWindingStencil1_PathPass) is required to do winding rule path filling
     * (or inverse winding rule). Otherwise, two passes are required
     * (kWindingStencil1_PathPass followed by kWindingStencil2_PathPass).
     *
     * @return    true if only a single stencil pass is needed.
     */
    bool supportsSingleStencilPassWinding() const
                                        { return fSingleStencilPassForWinding; }

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
    virtual void drawIndexed(PrimitiveType type,
                             int startVertex,
                             int startIndex,
                             int vertexCount,
                             int indexCount);

    virtual void drawNonIndexed(PrimitiveType type,
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
    virtual void forceRenderTargetFlush() = 0;

    virtual bool readPixels(int left, int top, int width, int height,
                            GrTexture::PixelConfig, void* buffer) = 0;


    const Stats& getStats() const;
    void resetStats();
    void printStats() const;

protected:
    /**
     * Extensions to GrDrawTarget::StencilPass to implement stencil clipping
     */
    enum GpuStencilPass {
        kSetClip_StencilPass = kDrawTargetCount_StencilPass,
                                        /* rendering a hard clip to the stencil
                                           buffer. Subsequent draws with other
                                           StencilPass values will be clipped
                                           if kClip_StateBit is set. */
        kGpuCount_StencilPass
    };

    /**
     * Extensions to GrDrawTarget::StateBits to implement stencil clipping
     */
    struct ClipState {
        bool            fClipInStencil;
        bool            fClipIsDirty;
        GrRenderTarget* fStencilClipTarget;
    } fClipState;

    // GrDrawTarget override
    virtual void clipWillBeSet(const GrClip& newClip);

    // prepares clip flushes gpu state before a draw
    bool setupClipAndFlushState(PrimitiveType type);

    struct BoundsState {
        bool    fScissorEnabled;
        GrIRect fScissorRect;
        GrIRect fViewportRect;
    };

    // defaults to false, subclass can set true to support palleted textures
    bool f8bitPaletteSupport;

    // set by subclass
    bool fNPOTTextureSupport;
    bool fNPOTTextureTileSupport;
    bool fNPOTRenderTargetSupport;

    // True if only one stencil pass is required to implement the winding path
    // fill rule. Subclass responsible for setting this value.
    bool fSingleStencilPassForWinding;

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

    // overridden by API specific GrGpu-derived class to perform the draw call.
    virtual void drawIndexedHelper(PrimitiveType type,
                                   uint32_t startVertex,
                                   uint32_t startIndex,
                                   uint32_t vertexCount,
                                   uint32_t indexCount) = 0;

    virtual void drawNonIndexedHelper(PrimitiveType type,
                                      uint32_t vertexCount,
                                      uint32_t numVertices) = 0;

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
    virtual bool flushGraphicsState(PrimitiveType type) = 0;

    // Sets the scissor rect, or disables if rect is NULL.
    virtual void flushScissor(const GrIRect* rect) = 0;

    // GrGpu subclass removes the clip from the stencil buffer
    virtual void eraseStencilClip() = 0;

private:

    void prepareVertexPool();
    void prepareIndexPool();

    GrVertexBufferAllocPool*    fVertexPool;

    GrIndexBufferAllocPool*     fIndexPool;

    mutable GrIndexBuffer*      fQuadIndexBuffer; // mutable so it can be
                                                  // created on-demand

    mutable GrVertexBuffer*     fUnitSquareVertexBuffer; // mutable so it can be
                                                         // created on-demand
    typedef GrDrawTarget INHERITED;
};

#endif
