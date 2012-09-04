
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
#include "GrClipMaskManager.h"

class GrContext;
class GrIndexBufferAllocPool;
class GrPath;
class GrPathRenderer;
class GrPathRendererChain;
class GrResource;
class GrStencilBuffer;
class GrVertexBufferAllocPool;

class GrGpu : public GrDrawTarget {

public:

    /**
     * Additional blend coeffecients for dual source blending, not exposed
     * through GrPaint/GrContext.
     */
    enum ExtendedBlendCoeffs {
        // source 2 refers to second output color when
        // using dual source blending.
        kS2C_GrBlendCoeff = kPublicGrBlendCoeffCount,
        kIS2C_GrBlendCoeff,
        kS2A_GrBlendCoeff,
        kIS2A_GrBlendCoeff,

        kTotalGrBlendCoeffCount
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
        fClipMaskManager.setContext(context);
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

    /**
     * Implements GrContext::createPlatformTexture
     */
    GrTexture* createPlatformTexture(const GrPlatformTextureDesc& desc);

    /**
     * Implements GrContext::createPlatformTexture
     */
    GrRenderTarget* createPlatformRenderTarget(const GrPlatformRenderTargetDesc& desc);

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
     * Creates a path object that can be stenciled using stencilPath(). It is
     * only legal to call this if the caps report support for path stenciling.
     */
    GrPath* createPath(const SkPath& path);

    /**
     * Returns an index buffer that can be used to render quads.
     * Six indices per quad: 0, 1, 2, 0, 2, 3, etc.
     * The max number of quads can be queried using GrIndexBuffer::maxQuads().
     * Draw with kTriangles_GrPrimitiveType
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
     * Resolves MSAA.
     */
    void resolveRenderTarget(GrRenderTarget* target);

    /**
     * Ensures that the current render target is actually set in the
     * underlying 3D API. Used when client wants to use 3D API to directly
     * render to the RT.
     */
    void forceRenderTargetFlush();

    /**
     * readPixels with some configs may be slow. Given a desired config this
     * function returns a fast-path config. The returned config must have the
     * same components and component sizes. The caller is free to ignore the
     * result and call readPixels with the original config.
     */
    virtual GrPixelConfig preferredReadPixelsConfig(GrPixelConfig config)
                                                                        const {
        return config;
    }

    /**
     * Same as above but applies to writeTexturePixels
     */
    virtual GrPixelConfig preferredWritePixelsConfig(GrPixelConfig config)
                                                                        const {
        return config;
    }

    /**
     * OpenGL's readPixels returns the result bottom-to-top while the skia
     * API is top-to-bottom. Thus we have to do a y-axis flip. The obvious
     * solution is to have the subclass do the flip using either the CPU or GPU.
     * However, the caller (GrContext) may have transformations to apply and can
     * simply fold in the y-flip for free. On the other hand, the subclass may
     * be able to do it for free itself. For example, the subclass may have to
     * do memcpys to handle rowBytes that aren't tight. It could do the y-flip
     * concurrently.
     *
     * This function returns true if a y-flip is required to put the pixels in
     * top-to-bottom order and the subclass cannot do it for free.
     *
     * See read pixels for the params
     * @return true if calling readPixels with the same set of params will
     *              produce bottom-to-top data
     */
     virtual bool readPixelsWillPayForYFlip(GrRenderTarget* renderTarget,
                                            int left, int top,
                                            int width, int height,
                                            GrPixelConfig config,
                                            size_t rowBytes) const = 0;
     /**
      * This should return true if reading a NxM rectangle of pixels from a
      * render target is faster if the target has dimensons N and M and the read
      * rectangle has its top-left at 0,0.
      */
     virtual bool fullReadPixelsIsFasterThanPartial() const { return false; };

    /**
     * Reads a rectangle of pixels from a render target.
     *
     * @param renderTarget  the render target to read from. NULL means the
     *                      current render target.
     * @param left          left edge of the rectangle to read (inclusive)
     * @param top           top edge of the rectangle to read (inclusive)
     * @param width         width of rectangle to read in pixels.
     * @param height        height of rectangle to read in pixels.
     * @param config        the pixel config of the destination buffer
     * @param buffer        memory to read the rectangle into.
     * @param rowBytes      the number of bytes between consecutive rows. Zero
     *                      means rows are tightly packed.
     * @param invertY       buffer should be populated bottom-to-top as opposed
     *                      to top-to-bottom (skia's usual order)
     *
     * @return true if the read succeeded, false if not. The read can fail
     *              because of a unsupported pixel config or because no render
     *              target is currently set.
     */
    bool readPixels(GrRenderTarget* renderTarget,
                    int left, int top, int width, int height,
                    GrPixelConfig config, void* buffer, size_t rowBytes,
                    bool invertY);

    /**
     * Updates the pixels in a rectangle of a texture.
     *
     * @param left          left edge of the rectangle to write (inclusive)
     * @param top           top edge of the rectangle to write (inclusive)
     * @param width         width of rectangle to write in pixels.
     * @param height        height of rectangle to write in pixels.
     * @param config        the pixel config of the source buffer
     * @param buffer        memory to read pixels from
     * @param rowBytes      number of bytes bewtween consecutive rows. Zero
     *                      means rows are tightly packed.
     */
    void writeTexturePixels(GrTexture* texture,
                            int left, int top, int width, int height,
                            GrPixelConfig config, const void* buffer,
                            size_t rowBytes);

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
    virtual void clear(const GrIRect* rect,
                       GrColor color,
                       GrRenderTarget* renderTarget = NULL) SK_OVERRIDE;

    virtual void purgeResources() SK_OVERRIDE {
        // The clip mask manager can rebuild all its clip masks so just
        // get rid of them all.
        fClipMaskManager.releaseResources();
    }

    // After the client interacts directly with the 3D context state the GrGpu
    // must resync its internal state and assumptions about 3D context state.
    // Each time this occurs the GrGpu bumps a timestamp.
    // state of the 3D context
    // At 10 resets / frame and 60fps a 64bit timestamp will overflow in about
    // a billion years.
    typedef uint64_t ResetTimestamp;

    // This timestamp is always older than the current timestamp
    static const ResetTimestamp kExpiredTimestamp = 0;
    // Returns a timestamp based on the number of times the context was reset.
    // This timestamp can be used to lazily detect when cached 3D context state
    // is dirty.
    ResetTimestamp getResetTimestamp() const {
        return fResetTimestamp;
    }

    /**
     * Can the provided configuration act as a color render target?
     */
    bool isConfigRenderable(GrPixelConfig config) const {
        GrAssert(kGrPixelConfigCount > config);
        return fConfigRenderSupport[config];
    }

    /**
     * These methods are called by the clip manager's setupClipping function
     * which (called as part of GrGpu's implementation of onDraw* and
     * onStencilPath member functions.) The GrGpu subclass should flush the
     * stencil state to the 3D API in its implementation of flushGraphicsState.
     */
    void enableScissor(const GrIRect& rect) {
        fScissorState.fEnabled = true;
        fScissorState.fRect = rect;
    }
    void disableScissor() { fScissorState.fEnabled = false; }

    /**
     * Like the scissor methods above this is called by setupClipping and
     * should be flushed by the GrGpu subclass in flushGraphicsState. These
     * stencil settings should be used in place of those on the GrDrawState.
     * They have been adjusted to account for any interactions between the
     * GrDrawState's stencil settings and stencil clipping.
     */
    void setStencilSettings(const GrStencilSettings& settings) {
        fStencilSettings = settings;
    }
    void disableStencil() { fStencilSettings.setDisabled(); }

    // GrGpu subclass sets clip bit in the stencil buffer. The subclass is
    // free to clear the remaining bits to zero if masked clears are more
    // expensive than clearing all bits.
    virtual void clearStencilClip(const GrIRect& rect, bool insideClip) = 0;

    enum PrivateDrawStateStateBits {
        kFirstBit = (GrDrawState::kLastPublicStateBit << 1),

        kModifyStencilClip_StateBit = kFirstBit, // allows draws to modify
                                                 // stencil bits used for
                                                 // clipping.
    };

protected:
    enum DrawType {
        kDrawPoints_DrawType,
        kDrawLines_DrawType,
        kDrawTriangles_DrawType,
        kStencilPath_DrawType,
    };

    DrawType PrimTypeToDrawType(GrPrimitiveType type) {
        switch (type) {
            case kTriangles_GrPrimitiveType:
            case kTriangleStrip_GrPrimitiveType:
            case kTriangleFan_GrPrimitiveType:
                return kDrawTriangles_DrawType;
            case kPoints_GrPrimitiveType:
                return kDrawPoints_DrawType;
            case kLines_GrPrimitiveType:
            case kLineStrip_GrPrimitiveType:
                return kDrawLines_DrawType;
            default:
                GrCrash("Unexpected primitive type");
                return kDrawTriangles_DrawType;
        }
    }

    // prepares clip flushes gpu state before a draw
    bool setupClipAndFlushState(DrawType);

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

    GrClipMaskManager           fClipMaskManager;

    struct GeometryPoolState {
        const GrVertexBuffer* fPoolVertexBuffer;
        int                   fPoolStartVertex;

        const GrIndexBuffer*  fPoolIndexBuffer;
        int                   fPoolStartIndex;
    };
    const GeometryPoolState& getGeomPoolState() {
        return fGeomPoolStateStack.back();
    }

    // The state of the scissor is controlled by the clip manager
    struct ScissorState {
        bool    fEnabled;
        GrIRect fRect;
    } fScissorState;

    // The final stencil settings to use as determined by the clip manager.
    GrStencilSettings fStencilSettings;

    // Derived classes need access to this so they can fill it out in their
    // constructors
    bool    fConfigRenderSupport[kGrPixelConfigCount];

    // GrDrawTarget overrides
    virtual bool onReserveVertexSpace(GrVertexLayout vertexLayout,
                                      int vertexCount,
                                      void** vertices) SK_OVERRIDE;
    virtual bool onReserveIndexSpace(int indexCount,
                                     void** indices) SK_OVERRIDE;
    virtual void releaseReservedVertexSpace() SK_OVERRIDE;
    virtual void releaseReservedIndexSpace() SK_OVERRIDE;
    virtual void onSetVertexSourceToArray(const void* vertexArray,
                                          int vertexCount) SK_OVERRIDE;
    virtual void onSetIndexSourceToArray(const void* indexArray,
                                         int indexCount) SK_OVERRIDE;
    virtual void releaseVertexArray() SK_OVERRIDE;
    virtual void releaseIndexArray() SK_OVERRIDE;
    virtual void geometrySourceWillPush() SK_OVERRIDE;
    virtual void geometrySourceWillPop(
        const GeometrySrcState& restoredState) SK_OVERRIDE;

    // Helpers for setting up geometry state
    void finalizeReservedVertices();
    void finalizeReservedIndices();

    // called when the 3D context state is unknown. Subclass should emit any
    // assumed 3D context state and dirty any state cache.
    virtual void onResetContext() = 0;


    // overridden by API-specific derived class to create objects.
    virtual GrTexture* onCreateTexture(const GrTextureDesc& desc,
                                       const void* srcData,
                                       size_t rowBytes) = 0;
    virtual GrTexture* onCreatePlatformTexture(const GrPlatformTextureDesc& desc) = 0;
    virtual GrRenderTarget* onCreatePlatformRenderTarget(const GrPlatformRenderTargetDesc& desc) = 0;
    virtual GrVertexBuffer* onCreateVertexBuffer(uint32_t size,
                                                 bool dynamic) = 0;
    virtual GrIndexBuffer* onCreateIndexBuffer(uint32_t size,
                                               bool dynamic) = 0;
    virtual GrPath* onCreatePath(const SkPath& path) = 0;

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
    // when GrDrawTarget::stencilPath is called the draw state's current stencil
    // settings are ignored. Instead the GrGpu decides the stencil rules
    // necessary to stencil the path. These are still subject to filtering by
    // the clip mask manager.
    virtual void setStencilPathSettings(const GrPath&,
                                        GrPathFill,
                                        GrStencilSettings* settings) = 0;
    // overridden by API-specific derived class to perform the path stenciling.
    virtual void onGpuStencilPath(const GrPath*, GrPathFill) = 0;

    // overridden by API-specific derived class to perform flush
    virtual void onForceRenderTargetFlush() = 0;

    // overridden by API-specific derived class to perform the read pixels.
    virtual bool onReadPixels(GrRenderTarget* target,
                              int left, int top, int width, int height,
                              GrPixelConfig,
                              void* buffer,
                              size_t rowBytes,
                              bool invertY) = 0;

    // overridden by API-specific derived class to perform the texture update
    virtual void onWriteTexturePixels(GrTexture* texture,
                                      int left, int top, int width, int height,
                                      GrPixelConfig config, const void* buffer,
                                      size_t rowBytes) = 0;

    // overridden by API-specific derived class to perform the resolve
    virtual void onResolveRenderTarget(GrRenderTarget* target) = 0;

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
    virtual bool flushGraphicsState(DrawType) = 0;

    // clears the entire stencil buffer to 0
    virtual void clearStencil() = 0;

private:
    GrContext*                  fContext; // not reffed (context refs gpu)

    ResetTimestamp              fResetTimestamp;

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

    bool                        fContextIsDirty;

    typedef SkTDLinkedList<GrResource> ResourceList;
    ResourceList                fResourceList;

    // Given a rt, find or create a stencil buffer and attach it
    bool attachStencilBufferToRenderTarget(GrRenderTarget* target);

    // GrDrawTarget overrides
    virtual void onDrawIndexed(GrPrimitiveType type,
                               int startVertex,
                               int startIndex,
                               int vertexCount,
                               int indexCount) SK_OVERRIDE;
    virtual void onDrawNonIndexed(GrPrimitiveType type,
                                  int startVertex,
                                  int vertexCount) SK_OVERRIDE;
    virtual void onStencilPath(const GrPath* path, GrPathFill fill) SK_OVERRIDE;

    // readies the pools to provide vertex/index data.
    void prepareVertexPool();
    void prepareIndexPool();

    void resetContext() {
        // We call this because the client may have messed with the
        // stencil buffer. Perhaps we should detect whether it is a
        // internally created stencil buffer and if so skip the invalidate.
        fClipMaskManager.invalidateStencilMask();
        this->onResetContext();
        ++fResetTimestamp;
    }

    void handleDirtyContext() {
        if (fContextIsDirty) {
            this->resetContext();
            fContextIsDirty = false;
        }
    }

    typedef GrDrawTarget INHERITED;
};

#endif
