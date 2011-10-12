
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrGpu_DEFINED
#define GrGpu_DEFINED

#include "GrDrawTarget.h"
#include "GrRect.h"
#include "GrRefCnt.h"
#include "GrTexture.h"

class GrContext;
class GrIndexBufferAllocPool;
class GrPathRenderer;
class GrPathRendererChain;
class GrResource;
class GrStencilBuffer;
class GrVertexBufferAllocPool;

/**
 * Gpu usage statistics.
 */
struct GrGpuStats {
    uint32_t fVertexCnt;  //<! Number of vertices drawn
    uint32_t fIndexCnt;   //<! Number of indices drawn
    uint32_t fDrawCnt;    //<! Number of draws

    uint32_t fProgChngCnt;//<! Number of program changes (N/A for fixed)

    /**
     *  Number of times the texture is set in 3D API
     */
    uint32_t fTextureChngCnt;
    /**
     *  Number of times the render target is set in 3D API
     */
    uint32_t fRenderTargetChngCnt;
    /**
     *  Number of textures created (includes textures that are rendertargets).
     */
    uint32_t fTextureCreateCnt;
    /**
     *  Number of rendertargets created.
     */
    uint32_t fRenderTargetCreateCnt;
};

class GrGpu : public GrDrawTarget {

public:

    /**
     * Additional blend coeffecients for dual source blending, not exposed
     * through GrPaint/GrContext.
     */
    enum ExtendedBlendCoeffs {
        // source 2 refers to second output color when
        // using dual source blending.
        kS2C_BlendCoeff = kPublicBlendCoeffCount,
        kIS2C_BlendCoeff,
        kS2A_BlendCoeff,
        kIS2A_BlendCoeff,

        kTotalBlendCoeffCount
    };

    /**
     *  Create an instance of GrGpu that matches the specified Engine backend.
     *  If the requested engine is not supported (at compile-time or run-time)
     *  this returns NULL.
     */
    static GrGpu* Create(GrEngine, GrPlatform3DContext context3D);

    ////////////////////////////////////////////////////////////////////////////

    GrGpu();
    virtual ~GrGpu();

    // The GrContext sets itself as the owner of this Gpu object
    void setContext(GrContext* context) {
        GrAssert(NULL == fContext); 
        fContext = context;
    }
    GrContext* getContext() { return fContext; }
    const GrContext* getContext() const { return fContext; }

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
    GrTexture* createTexture(const GrTextureDesc& desc,
                             const void* srcData, size_t rowBytes);

    GrResource* createPlatformSurface(const GrPlatformSurfaceDesc& desc);

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
     * Reads a rectangle of pixels from a render target.
     * @param renderTarget  the render target to read from. NULL means the
     *                      current render target.
     * @param left          left edge of the rectangle to read (inclusive)
     * @param top           top edge of the rectangle to read (inclusive)
     * @param width         width of rectangle to read in pixels.
     * @param height        height of rectangle to read in pixels.
     * @param config        the pixel config of the destination buffer
     * @param buffer        memory to read the rectangle into.
     *
     * @return true if the read succeeded, false if not. The read can fail
     *              because of a unsupported pixel config or because no render
     *              target is currently set.
     */
    bool readPixels(GrRenderTarget* renderTarget,
                    int left, int top, int width, int height,
                    GrPixelConfig config, void* buffer);

    const GrGpuStats& getStats() const;
    void resetStats();
    void printStats() const;

    /**
     * Called to tell Gpu object that all GrResources have been lost and should
     * be abandoned. Overrides must call INHERITED::abandonResources().
     */
    virtual void abandonResources();

    /**
     * Called to tell Gpu object to release all GrResources. Overrides must call
     * INHERITED::releaseResources().
     */
    void releaseResources();

    /**
     * Add resource to list of resources. Should only be called by GrResource.
     * @param resource  the resource to add.
     */
    void insertResource(GrResource* resource);

    /**
     * Remove resource from list of resources. Should only be called by
     * GrResource.
     * @param resource  the resource to remove.
     */
    void removeResource(GrResource* resource);

    // GrDrawTarget overrides
    virtual void clear(const GrIRect* rect, GrColor color);

protected:
    enum PrivateStateBits {
        kFirstBit = (kLastPublicStateBit << 1),

        kModifyStencilClip_StateBit = kFirstBit, // allows draws to modify
                                                 // stencil bits used for
                                                 // clipping.
    };

    // keep track of whether we are using stencil clipping (as opposed to
    // scissor).
    bool    fClipInStencil;

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


    GrGpuStats fStats;

    struct GeometryPoolState {
        const GrVertexBuffer* fPoolVertexBuffer;
        int                   fPoolStartVertex;
        
        const GrIndexBuffer*  fPoolIndexBuffer;
        int                   fPoolStartIndex;
    };
    const GeometryPoolState& getGeomPoolState() { 
        return fGeomPoolStateStack.back(); 
    }

    // GrDrawTarget overrides
    virtual bool onReserveVertexSpace(GrVertexLayout vertexLayout,
                                      int vertexCount,
                                      void** vertices);
    virtual bool onReserveIndexSpace(int indexCount, void** indices);
    virtual void releaseReservedVertexSpace();
    virtual void releaseReservedIndexSpace();    
    virtual void onSetVertexSourceToArray(const void* vertexArray,
                                          int vertexCount);
    virtual void onSetIndexSourceToArray(const void* indexArray,
                                         int indexCount);
    virtual void releaseVertexArray();
    virtual void releaseIndexArray();
    virtual void geometrySourceWillPush();
    virtual void geometrySourceWillPop(const GeometrySrcState& restoredState);

    // Helpers for setting up geometry state
    void finalizeReservedVertices();
    void finalizeReservedIndices();

    // overridden by API-specific derived class to handle re-emitting 3D API
    // preample and dirtying state cache.
    virtual void resetContext() = 0;

    // overridden by API-specific derived class to create objects.
    virtual GrTexture* onCreateTexture(const GrTextureDesc& desc,
                                       const void* srcData,
                                       size_t rowBytes) = 0;
    virtual GrResource* onCreatePlatformSurface(const GrPlatformSurfaceDesc& desc) = 0;
    virtual GrVertexBuffer* onCreateVertexBuffer(uint32_t size,
                                                 bool dynamic) = 0;
    virtual GrIndexBuffer* onCreateIndexBuffer(uint32_t size,
                                               bool dynamic) = 0;

    // overridden by API-specific derivated class to perform the clear and 
    // clearRect. NULL rect means clear whole target.
    virtual void onClear(const GrIRect* rect, GrColor color) = 0;

    // overridden by API-specific derived class to perform the draw call.
    virtual void onGpuDrawIndexed(GrPrimitiveType type,
                                  uint32_t startVertex,
                                  uint32_t startIndex,
                                  uint32_t vertexCount,
                                  uint32_t indexCount) = 0;

    virtual void onGpuDrawNonIndexed(GrPrimitiveType type,
                                     uint32_t vertexCount,
                                     uint32_t numVertices) = 0;

    // overridden by API-specific derived class to perform flush
    virtual void onForceRenderTargetFlush() = 0;

    // overridden by API-specific derived class to perform the read pixels.
    virtual bool onReadPixels(GrRenderTarget* target,
                              int left, int top, int width, int height,
                              GrPixelConfig, void* buffer) = 0;

    // called to program the vertex data, indexCount will be 0 if drawing non-
    // indexed geometry. The subclass may adjust the startVertex and/or
    // startIndex since it may have already accounted for these in the setup.
    virtual void setupGeometry(int* startVertex,
                               int* startIndex,
                               int vertexCount,
                               int indexCount) = 0;

    // width and height may be larger than rt (if underlying API allows it).
    // Should attach the SB to the RT. Returns false if compatible sb could
    // not be created.
    virtual bool createStencilBufferForRenderTarget(GrRenderTarget* rt,
                                                    int width, 
                                                    int height) = 0;

    // attaches an existing SB to an existing RT.
    virtual bool attachStencilBufferToRenderTarget(GrStencilBuffer* sb,
                                                   GrRenderTarget* rt) = 0;

    // The GrGpu typically records the clients requested state and then flushes
    // deltas from previous state at draw time. This function does the
    // API-specific flush of the state
    // returns false if current state is unsupported.
    virtual bool flushGraphicsState(GrPrimitiveType type) = 0;

    // Sets the scissor rect, or disables if rect is NULL.
    virtual void flushScissor(const GrIRect* rect) = 0;

    // GrGpu subclass sets clip bit in the stencil buffer. The subclass is
    // free to clear the remaining bits to zero if masked clears are more
    // expensive than clearing all bits.
    virtual void clearStencilClip(const GrIRect& rect, bool insideClip) = 0;

    // clears the entire stencil buffer to 0
    virtual void clearStencil() = 0;

private:
    GrContext*                  fContext; // not reffed (context refs gpu)

    GrVertexBufferAllocPool*    fVertexPool;

    GrIndexBufferAllocPool*     fIndexPool;
    
    // counts number of uses of vertex/index pool in the geometry stack
    int                         fVertexPoolUseCnt;
    int                         fIndexPoolUseCnt;
    
    enum {
        kPreallocGeomPoolStateStackCnt = 4,
    };
    SkSTArray<kPreallocGeomPoolStateStackCnt,
              GeometryPoolState, true>              fGeomPoolStateStack;
    
    mutable GrIndexBuffer*      fQuadIndexBuffer; // mutable so it can be
                                                  // created on-demand

    mutable GrVertexBuffer*     fUnitSquareVertexBuffer; // mutable so it can be
                                                         // created on-demand

    // must be instantiated after GrGpu object has been given its owning
    // GrContext ptr. (GrGpu is constructed first then handed off to GrContext).
    GrPathRendererChain*        fPathRendererChain;

    bool                        fContextIsDirty;

    GrResource*                 fResourceHead;

    // Given a rt, find or create a stencil buffer and attach it
    bool attachStencilBufferToRenderTarget(GrRenderTarget* target);

    // GrDrawTarget overrides
    virtual void onDrawIndexed(GrPrimitiveType type,
                               int startVertex,
                               int startIndex,
                               int vertexCount,
                               int indexCount);
    virtual void onDrawNonIndexed(GrPrimitiveType type,
                                  int startVertex,
                                  int vertexCount);

    // readies the pools to provide vertex/index data.
    void prepareVertexPool();
    void prepareIndexPool();

    // determines the path renderer used to draw a clip path element.
    GrPathRenderer* getClipPathRenderer(const SkPath& path, GrPathFill fill);

    void handleDirtyContext() {
        if (fContextIsDirty) {
            this->resetContext();
            fContextIsDirty = false;
        }
    }

    typedef GrDrawTarget INHERITED;
};

#endif
