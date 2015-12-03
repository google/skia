/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGpu_DEFINED
#define GrGpu_DEFINED

#include "GrPipelineBuilder.h"
#include "GrProgramDesc.h"
#include "GrStencil.h"
#include "GrTextureParamsAdjuster.h"
#include "GrXferProcessor.h"
#include "SkPath.h"

class GrBatchTracker;
class GrContext;
class GrGLContext;
class GrIndexBuffer;
class GrNonInstancedVertices;
class GrPath;
class GrPathRange;
class GrPathRenderer;
class GrPathRendererChain;
class GrPathRendering;
class GrPipeline;
class GrPrimitiveProcessor;
class GrRenderTarget;
class GrStencilAttachment;
class GrSurface;
class GrTexture;
class GrVertexBuffer;
class GrVertices;

class GrGpu : public SkRefCnt {
public:
    /**
     * Create an instance of GrGpu that matches the specified backend. If the requested backend is
     * not supported (at compile-time or run-time) this returns nullptr. The context will not be
     * fully constructed and should not be used by GrGpu until after this function returns.
     */
    static GrGpu* Create(GrBackend, GrBackendContext, const GrContextOptions&, GrContext* context);

    ////////////////////////////////////////////////////////////////////////////

    GrGpu(GrContext* context);
    ~GrGpu() override;

    GrContext* getContext() { return fContext; }
    const GrContext* getContext() const { return fContext; }

    /**
     * Gets the capabilities of the draw target.
     */
    const GrCaps* caps() const { return fCaps.get(); }

    GrPathRendering* pathRendering() { return fPathRendering.get();  }

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
    void markContextDirty(uint32_t state = kAll_GrBackendState) { fResetBits |= state; }

    /**
     * Creates a texture object. If kRenderTarget_GrSurfaceFlag the texture can
     * be used as a render target by calling GrTexture::asRenderTarget(). Not all
     * pixel configs can be used as render targets. Support for configs as textures
     * or render targets can be checked using GrCaps.
     *
     * @param desc        describes the texture to be created.
     * @param budgeted    does this texture count against the resource cache budget?
     * @param srcData     texel data to load texture. Begins with full-size
     *                    palette data for paletted textures. For compressed
     *                    formats it contains the compressed pixel data. Otherwise,
     *                    it contains width*height texels. If nullptr texture data
     *                    is uninitialized.
     * @param rowBytes    the number of bytes between consecutive rows. Zero
     *                    means rows are tightly packed. This field is ignored
     *                    for compressed formats.
     *
     * @return    The texture object if successful, otherwise nullptr.
     */
    GrTexture* createTexture(const GrSurfaceDesc& desc, bool budgeted,
                             const void* srcData, size_t rowBytes);

    /**
     * Implements GrContext::wrapBackendTexture
     */
    GrTexture* wrapBackendTexture(const GrBackendTextureDesc&, GrWrapOwnership);

    /**
     * Implements GrContext::wrapBackendTexture
     */
    GrRenderTarget* wrapBackendRenderTarget(const GrBackendRenderTargetDesc&, GrWrapOwnership);

    /**
     * Creates a vertex buffer.
     *
     * @param size    size in bytes of the vertex buffer
     * @param dynamic hints whether the data will be frequently changed
     *                by either GrVertexBuffer::map() or
     *                GrVertexBuffer::updateData().
     *
     * @return    The vertex buffer if successful, otherwise nullptr.
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
     * @return The index buffer if successful, otherwise nullptr.
     */
    GrIndexBuffer* createIndexBuffer(size_t size, bool dynamic);

    /**
     * Resolves MSAA.
     */
    void resolveRenderTarget(GrRenderTarget* target);

    /** Info struct returned by getReadPixelsInfo about performing intermediate draws before
        reading pixels for performance or correctness. */
    struct ReadPixelTempDrawInfo {
        /** If the GrGpu is requesting that the caller do a draw to an intermediate surface then
            this is descriptor for the temp surface. The draw should always be a rect with
            dst 0,0,w,h. */
        GrSurfaceDesc   fTempSurfaceDesc;
        /** Indicates whether there is a performance advantage to using an exact match texture
            (in terms of width and height) for the intermediate texture instead of approximate. */
        bool            fUseExactScratch;
        /** The caller should swap the R and B channel in the temp draw and then instead of reading
            the desired config back it should read GrPixelConfigSwapRAndB(readConfig). The swap
            during the draw and the swap at readback time cancel and the client gets the correct
            data. The swapped read back is either faster for or required by the underlying backend
            3D API. */
        bool            fSwapRAndB;
    };
    /** Describes why an intermediate draw must/should be performed before readPixels. */
    enum DrawPreference {
        /** On input means that the caller would proceed without draw if the GrGpu doesn't request
            one.
            On output means that the GrGpu is not requesting a draw. */
        kNoDraw_DrawPreference,
        /** Means that the client would prefer a draw for performance of the readback but
            can satisfy a straight readPixels call on the inputs without an intermediate draw.
            getReadPixelsInfo will never set the draw preference to this value but may leave
            it set. */
        kCallerPrefersDraw_DrawPreference,
        /** On output means that GrGpu would prefer a draw for performance of the readback but
            can satisfy a straight readPixels call on the inputs without an intermediate draw. The
            caller of getReadPixelsInfo should never specify this on intput. */
        kGpuPrefersDraw_DrawPreference,
        /** On input means that the caller requires a draw to do a transformation and there is no
            CPU fallback.
            On output means that GrGpu can only satisfy the readPixels request if the intermediate
            draw is performed.
          */
        kRequireDraw_DrawPreference
    };

    /**
     * Used to negotiate whether and how an intermediate draw should or must be performed before
     * a readPixels call. If this returns false then GrGpu could not deduce an intermediate draw
     * that would allow a successful readPixels call. The passed width, height, and rowBytes,
     * must be non-zero and already reflect clipping to the src bounds.
     */
    bool getReadPixelsInfo(GrSurface* srcSurface, int readWidth, int readHeight, size_t rowBytes,
                           GrPixelConfig readConfig, DrawPreference*, ReadPixelTempDrawInfo*);

    /** Info struct returned by getWritePixelsInfo about performing an intermediate draw in order 
        to write pixels to a GrSurface for either performance or correctness reasons. */
    struct WritePixelTempDrawInfo {
        /** If the GrGpu is requesting that the caller upload to an intermediate surface and draw
            that to the dst then this is the descriptor for the intermediate surface. The caller
            should upload the pixels such that the upper left pixel of the upload rect is at 0,0 in
            the intermediate surface.*/
        GrSurfaceDesc   fTempSurfaceDesc;
        /** If set, fTempSurfaceDesc's config will be a R/B swap of the src pixel config. The caller
            should upload the pixels as is such that R and B will be swapped in the intermediate
            surface. When the intermediate is drawn to the dst the shader should swap R/B again
            such that the correct swizzle results in the dst. This is done to work around either
            performance or API restrictions in the backend 3D API implementation. */
        bool            fSwapRAndB;
    };

    /**
     * Used to negotiate whether and how an intermediate surface should be used to write pixels to
     * a GrSurface. If this returns false then GrGpu could not deduce an intermediate draw
     * that would allow a successful transfer of the src pixels to the dst. The passed width,
     * height, and rowBytes, must be non-zero and already reflect clipping to the dst bounds.
     */
    bool getWritePixelsInfo(GrSurface* dstSurface, int width, int height, size_t rowBytes,
                            GrPixelConfig srcConfig, DrawPreference*, WritePixelTempDrawInfo*);

    /**
     * Reads a rectangle of pixels from a render target.
     *
     * @param surface       The surface to read from
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
    bool readPixels(GrSurface* surface,
                    int left, int top, int width, int height,
                    GrPixelConfig config, void* buffer, size_t rowBytes);

    /**
     * Updates the pixels in a rectangle of a surface.
     *
     * @param surface       The surface to write to.
     * @param left          left edge of the rectangle to write (inclusive)
     * @param top           top edge of the rectangle to write (inclusive)
     * @param width         width of rectangle to write in pixels.
     * @param height        height of rectangle to write in pixels.
     * @param config        the pixel config of the source buffer
     * @param buffer        memory to read pixels from
     * @param rowBytes      number of bytes between consecutive rows. Zero
     *                      means rows are tightly packed.
     */
    bool writePixels(GrSurface* surface,
                     int left, int top, int width, int height,
                     GrPixelConfig config, const void* buffer,
                     size_t rowBytes);

    /**
     * Clear the passed in render target. Ignores the draw state and clip.
     */
    void clear(const SkIRect& rect, GrColor color, GrRenderTarget* renderTarget);


    void clearStencilClip(const SkIRect& rect, bool insideClip, GrRenderTarget* renderTarget);

    /**
     * Discards the contents render target. nullptr indicates that the current render target should
     * be discarded.
     **/
    virtual void discard(GrRenderTarget* = nullptr) = 0;

    /**
     * This is can be called before allocating a texture to be a dst for copySurface. It will
     * populate the origin, config, and flags fields of the desc such that copySurface can
     * efficiently succeed. It should only succeed if it can allow copySurface to perform a copy
     * that would be more effecient than drawing the src to a dst render target.
     */
    virtual bool initCopySurfaceDstDesc(const GrSurface* src, GrSurfaceDesc* desc) const = 0;

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
    ResetTimestamp getResetTimestamp() const { return fResetTimestamp; }

    virtual void buildProgramDesc(GrProgramDesc*,
                                  const GrPrimitiveProcessor&,
                                  const GrPipeline&) const = 0;

    // Called to perform a surface to surface copy. Fallbacks to issuing a draw from the src to dst
    // take place at the GrDrawTarget level and this function implement faster copy paths. The rect
    // and point are pre-clipped. The src rect and implied dst rect are guaranteed to be within the
    // src/dst bounds and non-empty.
    bool copySurface(GrSurface* dst,
                     GrSurface* src,
                     const SkIRect& srcRect,
                     const SkIPoint& dstPoint);

    struct DrawArgs {
        DrawArgs(const GrPrimitiveProcessor* primProc,
                 const GrPipeline* pipeline,
                 const GrProgramDesc* desc)
            : fPrimitiveProcessor(primProc)
            , fPipeline(pipeline)
            , fDesc(desc) {
            SkASSERT(primProc && pipeline && desc);
        }
        const GrPrimitiveProcessor* fPrimitiveProcessor;
        const GrPipeline* fPipeline;
        const GrProgramDesc* fDesc;
    };

    void draw(const DrawArgs&, const GrVertices&);

    ///////////////////////////////////////////////////////////////////////////
    // Debugging and Stats

    class Stats {
    public:
#if GR_GPU_STATS
        Stats() { this->reset(); }

        void reset() {
            fRenderTargetBinds = 0;
            fShaderCompilations = 0;
            fTextureCreates = 0;
            fTextureUploads = 0;
            fStencilAttachmentCreates = 0;
            fNumDraws = 0;
        }

        int renderTargetBinds() const { return fRenderTargetBinds; }
        void incRenderTargetBinds() { fRenderTargetBinds++; }
        int shaderCompilations() const { return fShaderCompilations; }
        void incShaderCompilations() { fShaderCompilations++; }
        int textureCreates() const { return fTextureCreates; }
        void incTextureCreates() { fTextureCreates++; }
        int textureUploads() const { return fTextureUploads; }
        void incTextureUploads() { fTextureUploads++; }
        void incStencilAttachmentCreates() { fStencilAttachmentCreates++; }
        void incNumDraws() { fNumDraws++; }
        void dump(SkString*);
        void dumpKeyValuePairs(SkTArray<SkString>* keys, SkTArray<double>* values);

    private:
        int fRenderTargetBinds;
        int fShaderCompilations;
        int fTextureCreates;
        int fTextureUploads;
        int fStencilAttachmentCreates;
        int fNumDraws;
#else
        void dump(SkString*) {}
        void dumpKeyValuePairs(SkTArray<SkString>*, SkTArray<double>*) {}
        void incRenderTargetBinds() {}
        void incShaderCompilations() {}
        void incTextureCreates() {}
        void incTextureUploads() {}
        void incStencilAttachmentCreates() {}
        void incNumDraws() {}
#endif
    };

    Stats* stats() { return &fStats; }

    /** Creates a texture directly in the backend API without wrapping it in a GrTexture. This is
        only to be used for testing (particularly for testing the methods that import an externally
        created texture into Skia. Must be matched with a call to deleteTestingOnlyTexture(). */
    virtual GrBackendObject createTestingOnlyBackendTexture(void* pixels, int w, int h,
                                                            GrPixelConfig config) const = 0;
    /** Check a handle represents an actual texture in the backend API that has not been freed. */
    virtual bool isTestingOnlyBackendTexture(GrBackendObject) const = 0;
    /** If ownership of the backend texture has been transferred pass true for abandonTexture. This
        will do any necessary cleanup of the handle without freeing the texture in the backend
        API. */
    virtual void deleteTestingOnlyBackendTexture(GrBackendObject,
                                                 bool abandonTexture = false) const = 0;

    // width and height may be larger than rt (if underlying API allows it).
    // Returns nullptr if compatible sb could not be created, otherwise the caller owns the ref on
    // the GrStencilAttachment.
    virtual GrStencilAttachment* createStencilAttachmentForRenderTarget(const GrRenderTarget*,
                                                                        int width,
                                                                        int height) = 0;
    // clears target's entire stencil buffer to 0
    virtual void clearStencil(GrRenderTarget* target) = 0;


    // Determines whether a copy of a texture must be made in order to be compatible with
    // a given GrTextureParams. If so, the width, height and filter used for the copy are
    // output via the CopyParams.
    bool makeCopyForTextureParams(int width, int height, const GrTextureParams&,
                                  GrTextureProducer::CopyParams*) const;

    // This is only to be used in GL-specific tests.
    virtual const GrGLContext* glContextForTesting() const { return nullptr; }

    // This is only to be used by testing code
    virtual void resetShaderCacheForTesting() const {}

protected:
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

    static void ElevateDrawPreference(GrGpu::DrawPreference* preference,
                                      GrGpu::DrawPreference elevation) {
        GR_STATIC_ASSERT(GrGpu::kCallerPrefersDraw_DrawPreference > GrGpu::kNoDraw_DrawPreference);
        GR_STATIC_ASSERT(GrGpu::kGpuPrefersDraw_DrawPreference >
                         GrGpu::kCallerPrefersDraw_DrawPreference);
        GR_STATIC_ASSERT(GrGpu::kRequireDraw_DrawPreference >
                         GrGpu::kGpuPrefersDraw_DrawPreference);
        *preference = SkTMax(*preference, elevation);
    }

    void handleDirtyContext() {
        if (fResetBits) {
            this->resetContext();
        }
    }

    Stats                                   fStats;
    SkAutoTDelete<GrPathRendering>          fPathRendering;
    // Subclass must initialize this in its constructor.
    SkAutoTUnref<const GrCaps>    fCaps;

private:
    // called when the 3D context state is unknown. Subclass should emit any
    // assumed 3D context state and dirty any state cache.
    virtual void onResetContext(uint32_t resetBits) = 0;

    // Called before certain draws in order to guarantee coherent results from dst reads.
    virtual void xferBarrier(GrRenderTarget*, GrXferBarrierType) = 0;

    // overridden by backend-specific derived class to create objects.
    // Texture size and sample size will have already been validated in base class before
    // onCreateTexture/CompressedTexture are called.
    virtual GrTexture* onCreateTexture(const GrSurfaceDesc& desc,
                                       GrGpuResource::LifeCycle lifeCycle,
                                       const void* srcData, size_t rowBytes) = 0;
    virtual GrTexture* onCreateCompressedTexture(const GrSurfaceDesc& desc,
                                                 GrGpuResource::LifeCycle lifeCycle,
                                                 const void* srcData) = 0;
    virtual GrTexture* onWrapBackendTexture(const GrBackendTextureDesc&, GrWrapOwnership) = 0;
    virtual GrRenderTarget* onWrapBackendRenderTarget(const GrBackendRenderTargetDesc&,
                                                      GrWrapOwnership) = 0;
    virtual GrVertexBuffer* onCreateVertexBuffer(size_t size, bool dynamic) = 0;
    virtual GrIndexBuffer* onCreateIndexBuffer(size_t size, bool dynamic) = 0;

    // overridden by backend-specific derived class to perform the clear.
    virtual void onClear(GrRenderTarget*, const SkIRect& rect, GrColor color) = 0;


    // Overridden by backend specific classes to perform a clear of the stencil clip bits.  This is
    // ONLY used by the the clip target
    virtual void onClearStencilClip(GrRenderTarget*, const SkIRect& rect, bool insideClip) = 0;

    // overridden by backend-specific derived class to perform the draw call.
    virtual void onDraw(const DrawArgs&, const GrNonInstancedVertices&) = 0;

    virtual bool onGetReadPixelsInfo(GrSurface* srcSurface, int readWidth, int readHeight,
                                     size_t rowBytes, GrPixelConfig readConfig, DrawPreference*,
                                     ReadPixelTempDrawInfo*) = 0;
    virtual bool onGetWritePixelsInfo(GrSurface* dstSurface, int width, int height, size_t rowBytes,
                                      GrPixelConfig srcConfig, DrawPreference*,
                                      WritePixelTempDrawInfo*) = 0;

    // overridden by backend-specific derived class to perform the surface read
    virtual bool onReadPixels(GrSurface*,
                              int left, int top,
                              int width, int height,
                              GrPixelConfig,
                              void* buffer,
                              size_t rowBytes) = 0;

    // overridden by backend-specific derived class to perform the surface write
    virtual bool onWritePixels(GrSurface*,
                               int left, int top, int width, int height,
                               GrPixelConfig config, const void* buffer,
                               size_t rowBytes) = 0;

    // overridden by backend-specific derived class to perform the resolve
    virtual void onResolveRenderTarget(GrRenderTarget* target) = 0;

    // overridden by backend specific derived class to perform the copy surface
    virtual bool onCopySurface(GrSurface* dst,
                               GrSurface* src,
                               const SkIRect& srcRect,
                               const SkIPoint& dstPoint) = 0;

    void resetContext() {
        this->onResetContext(fResetBits);
        fResetBits = 0;
        ++fResetTimestamp;
    }

    ResetTimestamp                                                      fResetTimestamp;
    uint32_t                                                            fResetBits;
    // The context owns us, not vice-versa, so this ptr is not ref'ed by Gpu.
    GrContext*                                                          fContext;

    friend class GrPathRendering;
    typedef SkRefCnt INHERITED;
};

#endif
