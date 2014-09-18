/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGpu_DEFINED
#define GrGpu_DEFINED

#include "GrDrawTarget.h"
#include "GrClipMaskManager.h"
#include "GrPathRendering.h"
#include "SkPath.h"

class GrContext;
class GrIndexBufferAllocPool;
class GrPath;
class GrPathRange;
class GrPathRenderer;
class GrPathRendererChain;
class GrStencilBuffer;
class GrVertexBufferAllocPool;

class GrGpu : public GrDrawTarget {
public:

    /**
     * Additional blend coefficients for dual source blending, not exposed
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
     * Create an instance of GrGpu that matches the specified backend. If the requested backend is
     * not supported (at compile-time or run-time) this returns NULL. The context will not be
     * fully constructed and should not be used by GrGpu until after this function returns.
     */
    static GrGpu* Create(GrBackend, GrBackendContext, GrContext* context);

    ////////////////////////////////////////////////////////////////////////////

    GrGpu(GrContext* context);
    virtual ~GrGpu();

    GrContext* getContext() { return this->INHERITED::getContext(); }
    const GrContext* getContext() const { return this->INHERITED::getContext(); }

    GrPathRendering* pathRendering() {
        return fPathRendering.get();
    }

    // Called by GrContext when the underlying backend context has been destroyed.
    // GrGpu should use this to ensure that no backend API calls will be made from
    // here onward, including in its destructor. Subclasses should call
    // INHERITED::contextAbandoned() if they override this.
    virtual void contextAbandoned();

    /**
     * The GrGpu object normally assumes that no outsider is setting state
     * within the underlying 3D API's context/device/whatever. This call informs
     * the GrGpu that the state was modified and it shouldn't make assumptions
     * about the state.
     */
    void markContextDirty(uint32_t state = kAll_GrBackendState) {
        fResetBits |= state;
    }

    void unimpl(const char[]);

    /**
     * Creates a texture object. If desc width or height is not a power of
     * two but underlying API requires a power of two texture then srcData
     * will be embedded in a power of two texture. The extra width and height
     * is filled as though srcData were rendered clamped into the texture.
     * The exception is when using compressed data formats. In this case, the
     * desc width and height must be a multiple of the compressed format block
     * size otherwise this function returns NULL. Similarly, if the underlying
     * API requires a power of two texture and the source width and height are not
     * a power of two, then this function returns NULL.
     *
     * If kRenderTarget_TextureFlag is specified the GrRenderTarget is
     * accessible via GrTexture::asRenderTarget(). The texture will hold a ref
     * on the render target until the texture is destroyed. Compressed textures
     * cannot have the kRenderTarget_TextureFlag set.
     *
     * @param desc        describes the texture to be created.
     * @param srcData     texel data to load texture. Begins with full-size
     *                    palette data for paletted textures. For compressed
     *                    formats it contains the compressed pixel data. Otherwise,
     *                    it contains width*height texels. If NULL texture data
     *                    is uninitialized.
     * @param rowBytes    the number of bytes between consecutive rows. Zero
     *                    means rows are tightly packed. This field is ignored
     *                    for compressed formats.
     *
     * @return    The texture object if successful, otherwise NULL.
     */
    GrTexture* createTexture(const GrTextureDesc& desc,
                             const void* srcData, size_t rowBytes);

    /**
     * Implements GrContext::wrapBackendTexture
     */
    GrTexture* wrapBackendTexture(const GrBackendTextureDesc&);

    /**
     * Implements GrContext::wrapBackendTexture
     */
    GrRenderTarget* wrapBackendRenderTarget(const GrBackendRenderTargetDesc&);

    /**
     * Creates a vertex buffer.
     *
     * @param size    size in bytes of the vertex buffer
     * @param dynamic hints whether the data will be frequently changed
     *                by either GrVertexBuffer::map() or
     *                GrVertexBuffer::updateData().
     *
     * @return    The vertex buffer if successful, otherwise NULL.
     */
    GrVertexBuffer* createVertexBuffer(size_t size, bool dynamic);

    /**
     * Creates an index buffer.
     *
     * @param size    size in bytes of the index buffer
     * @param dynamic hints whether the data will be frequently changed
     *                by either GrIndexBuffer::map() or
     *                GrIndexBuffer::updateData().
     *
     * @return The index buffer if successful, otherwise NULL.
     */
    GrIndexBuffer* createIndexBuffer(size_t size, bool dynamic);

    /**
     * Returns an index buffer that can be used to render quads.
     * Six indices per quad: 0, 1, 2, 0, 2, 3, etc.
     * The max number of quads can be queried using GrIndexBuffer::maxQuads().
     * Draw with kTriangles_GrPrimitiveType
     * @ return the quad index buffer
     */
    const GrIndexBuffer* getQuadIndexBuffer() const;

    /**
     * Resolves MSAA.
     */
    void resolveRenderTarget(GrRenderTarget* target);

    /**
     * Gets a preferred 8888 config to use for writing/reading pixel data to/from a surface with
     * config surfaceConfig. The returned config must have at least as many bits per channel as the
     * readConfig or writeConfig param.
     */
    virtual GrPixelConfig preferredReadPixelsConfig(GrPixelConfig readConfig,
                                                    GrPixelConfig surfaceConfig) const {
        return readConfig;
    }
    virtual GrPixelConfig preferredWritePixelsConfig(GrPixelConfig writeConfig,
                                                     GrPixelConfig surfaceConfig) const {
        return writeConfig;
    }

    /**
     * Called before uploading writing pixels to a GrTexture when the src pixel config doesn't
     * match the texture's config.
     */
    virtual bool canWriteTexturePixels(const GrTexture*, GrPixelConfig srcConfig) const = 0;

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
                    GrPixelConfig config, void* buffer, size_t rowBytes);

    /**
     * Updates the pixels in a rectangle of a texture.
     *
     * @param left          left edge of the rectangle to write (inclusive)
     * @param top           top edge of the rectangle to write (inclusive)
     * @param width         width of rectangle to write in pixels.
     * @param height        height of rectangle to write in pixels.
     * @param config        the pixel config of the source buffer
     * @param buffer        memory to read pixels from
     * @param rowBytes      number of bytes between consecutive rows. Zero
     *                      means rows are tightly packed.
     */
    bool writeTexturePixels(GrTexture* texture,
                            int left, int top, int width, int height,
                            GrPixelConfig config, const void* buffer,
                            size_t rowBytes);

    // GrDrawTarget overrides
    virtual void clear(const SkIRect* rect,
                       GrColor color,
                       bool canIgnoreRect,
                       GrRenderTarget* renderTarget = NULL) SK_OVERRIDE;

    virtual void purgeResources() SK_OVERRIDE {
        // The clip mask manager can rebuild all its clip masks so just
        // get rid of them all.
        fClipMaskManager.purgeResources();
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
     * These methods are called by the clip manager's setupClipping function
     * which (called as part of GrGpu's implementation of onDraw and
     * onStencilPath member functions.) The GrGpu subclass should flush the
     * stencil state to the 3D API in its implementation of flushGraphicsState.
     */
    void enableScissor(const SkIRect& rect) {
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
    virtual void clearStencilClip(GrRenderTarget*, const SkIRect& rect, bool insideClip) = 0;

    enum PrivateDrawStateStateBits {
        kFirstBit = (GrDrawState::kLastPublicStateBit << 1),

        kModifyStencilClip_StateBit = kFirstBit, // allows draws to modify
                                                 // stencil bits used for
                                                 // clipping.
    };

    void getPathStencilSettingsForFillType(SkPath::FillType fill, GrStencilSettings* outStencilSettings);

    enum DrawType {
        kDrawPoints_DrawType,
        kDrawLines_DrawType,
        kDrawTriangles_DrawType,
        kStencilPath_DrawType,
        kDrawPath_DrawType,
        kDrawPaths_DrawType,
    };

    static bool IsPathRenderingDrawType(DrawType type) {
        return kDrawPath_DrawType == type || kDrawPaths_DrawType == type;
    }

    GrContext::GPUStats* gpuStats() { return &fGPUStats; }

protected:
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
                SkFAIL("Unexpected primitive type");
                return kDrawTriangles_DrawType;
        }
    }

    // prepares clip flushes gpu state before a draw
    bool setupClipAndFlushState(DrawType,
                                const GrDeviceCoordTexture* dstCopy,
                                GrDrawState::AutoRestoreEffects* are,
                                const SkRect* devBounds);

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

    GrContext::GPUStats         fGPUStats;

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
        SkIRect fRect;
    } fScissorState;

    // The final stencil settings to use as determined by the clip manager.
    GrStencilSettings fStencilSettings;

    // Helpers for setting up geometry state
    void finalizeReservedVertices();
    void finalizeReservedIndices();

    SkAutoTDelete<GrPathRendering> fPathRendering;

private:
    // GrDrawTarget overrides
    virtual bool onReserveVertexSpace(size_t vertexSize, int vertexCount, void** vertices) SK_OVERRIDE;
    virtual bool onReserveIndexSpace(int indexCount, void** indices) SK_OVERRIDE;
    virtual void releaseReservedVertexSpace() SK_OVERRIDE;
    virtual void releaseReservedIndexSpace() SK_OVERRIDE;
    virtual void onSetVertexSourceToArray(const void* vertexArray, int vertexCount) SK_OVERRIDE;
    virtual void onSetIndexSourceToArray(const void* indexArray, int indexCount) SK_OVERRIDE;
    virtual void releaseVertexArray() SK_OVERRIDE;
    virtual void releaseIndexArray() SK_OVERRIDE;
    virtual void geometrySourceWillPush() SK_OVERRIDE;
    virtual void geometrySourceWillPop(const GeometrySrcState& restoredState) SK_OVERRIDE;


    // called when the 3D context state is unknown. Subclass should emit any
    // assumed 3D context state and dirty any state cache.
    virtual void onResetContext(uint32_t resetBits) = 0;

    // overridden by backend-specific derived class to create objects.
    virtual GrTexture* onCreateTexture(const GrTextureDesc& desc,
                                       const void* srcData,
                                       size_t rowBytes) = 0;
    virtual GrTexture* onCreateCompressedTexture(const GrTextureDesc& desc,
                                                 const void* srcData) = 0;
    virtual GrTexture* onWrapBackendTexture(const GrBackendTextureDesc&) = 0;
    virtual GrRenderTarget* onWrapBackendRenderTarget(const GrBackendRenderTargetDesc&) = 0;
    virtual GrVertexBuffer* onCreateVertexBuffer(size_t size, bool dynamic) = 0;
    virtual GrIndexBuffer* onCreateIndexBuffer(size_t size, bool dynamic) = 0;

    // overridden by backend-specific derived class to perform the clear and
    // clearRect. NULL rect means clear whole target. If canIgnoreRect is
    // true, it is okay to perform a full clear instead of a partial clear
    virtual void onClear(GrRenderTarget*, const SkIRect* rect, GrColor color,
                         bool canIgnoreRect) = 0;

    // overridden by backend-specific derived class to perform the draw call.
    virtual void onGpuDraw(const DrawInfo&) = 0;

    // overridden by backend-specific derived class to perform the read pixels.
    virtual bool onReadPixels(GrRenderTarget* target,
                              int left, int top, int width, int height,
                              GrPixelConfig,
                              void* buffer,
                              size_t rowBytes) = 0;

    // overridden by backend-specific derived class to perform the texture update
    virtual bool onWriteTexturePixels(GrTexture* texture,
                                      int left, int top, int width, int height,
                                      GrPixelConfig config, const void* buffer,
                                      size_t rowBytes) = 0;

    // overridden by backend-specific derived class to perform the resolve
    virtual void onResolveRenderTarget(GrRenderTarget* target) = 0;

    // width and height may be larger than rt (if underlying API allows it).
    // Should attach the SB to the RT. Returns false if compatible sb could
    // not be created.
    virtual bool createStencilBufferForRenderTarget(GrRenderTarget*, int width, int height) = 0;

    // attaches an existing SB to an existing RT.
    virtual bool attachStencilBufferToRenderTarget(GrStencilBuffer*, GrRenderTarget*) = 0;

    // The GrGpu typically records the clients requested state and then flushes
    // deltas from previous state at draw time. This function does the
    // backend-specific flush of the state.
    // returns false if current state is unsupported.
    virtual bool flushGraphicsState(DrawType, const GrDeviceCoordTexture* dstCopy) = 0;

    // clears target's entire stencil buffer to 0
    virtual void clearStencil(GrRenderTarget* target) = 0;

    // Given a rt, find or create a stencil buffer and attach it
    bool attachStencilBufferToRenderTarget(GrRenderTarget* target);

    // GrDrawTarget overrides
    virtual void onDraw(const DrawInfo&) SK_OVERRIDE;
    virtual void onStencilPath(const GrPath*, SkPath::FillType) SK_OVERRIDE;
    virtual void onDrawPath(const GrPath*, SkPath::FillType,
                            const GrDeviceCoordTexture* dstCopy) SK_OVERRIDE;
    virtual void onDrawPaths(const GrPathRange*,
                             const uint32_t indices[], int count,
                             const float transforms[], PathTransformType,
                             SkPath::FillType, const GrDeviceCoordTexture*) SK_OVERRIDE;

    // readies the pools to provide vertex/index data.
    void prepareVertexPool();
    void prepareIndexPool();

    void resetContext() {
        // We call this because the client may have messed with the
        // stencil buffer. Perhaps we should detect whether it is a
        // internally created stencil buffer and if so skip the invalidate.
        fClipMaskManager.invalidateStencilMask();
        this->onResetContext(fResetBits);
        fResetBits = 0;
        ++fResetTimestamp;
    }

    void handleDirtyContext() {
        if (fResetBits) {
            this->resetContext();
        }
    }

    enum {
        kPreallocGeomPoolStateStackCnt = 4,
    };
    SkSTArray<kPreallocGeomPoolStateStackCnt, GeometryPoolState, true>  fGeomPoolStateStack;
    ResetTimestamp                                                      fResetTimestamp;
    uint32_t                                                            fResetBits;
    GrVertexBufferAllocPool*                                            fVertexPool;
    GrIndexBufferAllocPool*                                             fIndexPool;
    // counts number of uses of vertex/index pool in the geometry stack
    int                                                                 fVertexPoolUseCnt;
    int                                                                 fIndexPoolUseCnt;
    // these are mutable so they can be created on-demand
    mutable GrIndexBuffer*                                              fQuadIndexBuffer;

    typedef GrDrawTarget INHERITED;
};

#endif
