/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGpu_DEFINED
#define GrGpu_DEFINED

#include "GrGpuCommandBuffer.h"
#include "GrProgramDesc.h"
#include "GrSwizzle.h"
#include "GrAllocator.h"
#include "GrTextureProducer.h"
#include "GrTypes.h"
#include "GrXferProcessor.h"
#include "SkPath.h"
#include "SkTArray.h"
#include <map>

class GrBuffer;
class GrContext;
struct GrContextOptions;
class GrGLContext;
class GrMesh;
class GrNonInstancedVertices;
class GrPath;
class GrPathRange;
class GrPathRenderer;
class GrPathRendererChain;
class GrPathRendering;
class GrPipeline;
class GrPrimitiveProcessor;
class GrRenderTarget;
class GrSemaphore;
class GrStencilAttachment;
class GrStencilSettings;
class GrSurface;
class GrTexture;

namespace gr_instanced { class InstancedRendering; }

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

    enum class DisconnectType {
        // No cleanup should be attempted, immediately cease making backend API calls
        kAbandon,
        // Free allocated resources (not known by GrResourceCache) before returning and
        // ensure no backend backend 3D API calls will be made after disconnect() returns.
        kCleanup,
    };

    // Called by GrContext when the underlying backend context is already or will be destroyed
    // before GrContext.
    virtual void disconnect(DisconnectType);

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
     * @param texels      array of mipmap levels containing texel data to load.
     *                    Each level begins with full-size palette data for paletted textures.
     *                    For compressed formats the level contains the compressed pixel data.
     *                    Otherwise, it contains width*height texels. If there is only one
     *                    element and it contains nullptr fPixels, texture data is
     *                    uninitialized.
     * @return    The texture object if successful, otherwise nullptr.
     */
    GrTexture* createTexture(const GrSurfaceDesc& desc, SkBudgeted budgeted,
                             const SkTArray<GrMipLevel>& texels);

    /**
     * Simplified createTexture() interface for when there is no initial texel data to upload.
     */
    GrTexture* createTexture(const GrSurfaceDesc& desc, SkBudgeted budgeted) {
        return this->createTexture(desc, budgeted, SkTArray<GrMipLevel>());
    }

    /** Simplified createTexture() interface for when there is only a base level */
    GrTexture* createTexture(const GrSurfaceDesc& desc, SkBudgeted budgeted, const void* level0Data,
                             size_t rowBytes) {
        SkASSERT(level0Data);
        GrMipLevel level = { level0Data, rowBytes };
        SkSTArray<1, GrMipLevel> array;
        array.push_back() = level;
        return this->createTexture(desc, budgeted, array);
    }

    /**
     * Implements GrResourceProvider::wrapBackendTexture
     */
    sk_sp<GrTexture> wrapBackendTexture(const GrBackendTextureDesc&, GrWrapOwnership);

    /**
     * Implements GrResourceProvider::wrapBackendRenderTarget
     */
    sk_sp<GrRenderTarget> wrapBackendRenderTarget(const GrBackendRenderTargetDesc&);

    /**
     * Implements GrResourceProvider::wrapBackendTextureAsRenderTarget
     */
    sk_sp<GrRenderTarget> wrapBackendTextureAsRenderTarget(const GrBackendTextureDesc&);

    /**
     * Creates a buffer in GPU memory. For a client-side buffer use GrBuffer::CreateCPUBacked.
     *
     * @param size            size of buffer to create.
     * @param intendedType    hint to the graphics subsystem about what the buffer will be used for.
     * @param accessPattern   hint to the graphics subsystem about how the data will be accessed.
     * @param data            optional data with which to initialize the buffer.
     *
     * @return the buffer if successful, otherwise nullptr.
     */
    GrBuffer* createBuffer(size_t size, GrBufferType intendedType, GrAccessPattern accessPattern,
                           const void* data = nullptr);

    /**
     * Creates an instanced rendering object if it is supported on this platform.
     */
    gr_instanced::InstancedRendering* createInstancedRendering();

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
        SkBackingFit    fTempSurfaceFit;
        /** Swizzle to apply during the draw. This is used to compensate for either feature or
            performance limitations in the underlying 3D API. */
        GrSwizzle       fSwizzle;
        /** The config that should be used to read from the temp surface after the draw. This may be
            different than the original read config in order to compensate for swizzling. The
            read data will effectively be in the original read config. */
        GrPixelConfig   fReadConfig;
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
        /** Swizzle to apply during the draw. This is used to compensate for either feature or
            performance limitations in the underlying 3D API. */
        GrSwizzle       fSwizzle;
        /** The config that should be specified when uploading the *original* data to the temp
            surface before the draw. This may be different than the original src data config in
            order to compensate for swizzling that will occur when drawing. */
        GrPixelConfig   fWriteConfig;
    };

    /**
     * Used to negotiate whether and how an intermediate surface should be used to write pixels to
     * a GrSurface. If this returns false then GrGpu could not deduce an intermediate draw
     * that would allow a successful transfer of the src pixels to the dst. The passed width,
     * height, and rowBytes, must be non-zero and already reflect clipping to the dst bounds.
     */
    bool getWritePixelsInfo(GrSurface* dstSurface, int width, int height,
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
     * @param texels        array of mipmap levels containing texture data
     */
    bool writePixels(GrSurface* surface,
                     int left, int top, int width, int height,
                     GrPixelConfig config,
                     const SkTArray<GrMipLevel>& texels);

    /**
     * This function is a shim which creates a SkTArray<GrMipLevel> of size 1.
     * It then calls writePixels with that SkTArray.
     *
     * @param buffer   memory to read pixels from.
     * @param rowBytes number of bytes between consecutive rows. Zero
     *                 means rows are tightly packed.
     */
    bool writePixels(GrSurface* surface,
                     int left, int top, int width, int height,
                     GrPixelConfig config, const void* buffer,
                     size_t rowBytes);

    /**
     * Updates the pixels in a rectangle of a surface using a buffer
     *
     * @param surface          The surface to write to.
     * @param left             left edge of the rectangle to write (inclusive)
     * @param top              top edge of the rectangle to write (inclusive)
     * @param width            width of rectangle to write in pixels.
     * @param height           height of rectangle to write in pixels.
     * @param config           the pixel config of the source buffer
     * @param transferBuffer   GrBuffer to read pixels from (type must be "kCpuToGpu")
     * @param offset           offset from the start of the buffer
     * @param rowBytes         number of bytes between consecutive rows. Zero
     *                         means rows are tightly packed.
     */
    bool transferPixels(GrSurface* surface,
                        int left, int top, int width, int height,
                        GrPixelConfig config, GrBuffer* transferBuffer,
                        size_t offset, size_t rowBytes, GrFence* fence);

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

    // Called to perform a surface to surface copy. Fallbacks to issuing a draw from the src to dst
    // take place at the GrOpList level and this function implement faster copy paths. The rect
    // and point are pre-clipped. The src rect and implied dst rect are guaranteed to be within the
    // src/dst bounds and non-empty.
    bool copySurface(GrSurface* dst,
                     GrSurface* src,
                     const SkIRect& srcRect,
                     const SkIPoint& dstPoint);

    struct MultisampleSpecs {
        MultisampleSpecs(uint8_t uniqueID, int effectiveSampleCnt, const SkPoint* locations)
            : fUniqueID(uniqueID),
              fEffectiveSampleCnt(effectiveSampleCnt),
              fSampleLocations(locations) {}

        // Nonzero ID that uniquely identifies these multisample specs.
        uint8_t          fUniqueID;
        // The actual number of samples the GPU will run. NOTE: this value can be greater than the
        // the render target's sample count.
        int              fEffectiveSampleCnt;
        // If sample locations are supported, points to the subpixel locations at which the GPU will
        // sample. Pixel center is at (.5, .5), and (0, 0) indicates the top left corner.
        const SkPoint*   fSampleLocations;
    };

    // Finds a render target's multisample specs. The pipeline is only needed in case we need to
    // flush the draw state prior to querying multisample info. The pipeline is not expected to
    // affect the multisample information itself.
    const MultisampleSpecs& queryMultisampleSpecs(const GrPipeline&);

    // Finds the multisample specs with a given unique id.
    const MultisampleSpecs& getMultisampleSpecs(uint8_t uniqueID) {
        SkASSERT(uniqueID > 0 && uniqueID < fMultisampleSpecs.count());
        return fMultisampleSpecs[uniqueID];
    }

    // Creates a GrGpuCommandBuffer in which the GrOpList can send draw commands to instead of
    // directly to the Gpu object. This currently does not take a GrRenderTarget. The command buffer
    // is expected to infer the render target from the first draw, clear, or discard. This is an
    // awkward workaround that goes away after MDB is complete and the render target is known from
    // the GrRenderTargetOpList.
    virtual GrGpuCommandBuffer* createCommandBuffer(
            const GrGpuCommandBuffer::LoadAndStoreInfo& colorInfo,
            const GrGpuCommandBuffer::LoadAndStoreInfo& stencilInfo) = 0;

    // Called by GrOpList when flushing.
    // Provides a hook for post-flush actions (e.g. Vulkan command buffer submits).
    virtual void finishOpList() {}

    virtual GrFence SK_WARN_UNUSED_RESULT insertFence() = 0;
    virtual bool waitFence(GrFence, uint64_t timeout = 1000) = 0;
    virtual void deleteFence(GrFence) const = 0;

    virtual sk_sp<GrSemaphore> SK_WARN_UNUSED_RESULT makeSemaphore() = 0;
    virtual void insertSemaphore(sk_sp<GrSemaphore> semaphore) = 0;
    virtual void waitSemaphore(sk_sp<GrSemaphore> semaphore) = 0;

    // Ensures that all queued up driver-level commands have been sent to the GPU. For example, on
    // OpenGL, this calls glFlush.
    virtual void flush() = 0;

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
            fTransfersToTexture = 0;
            fStencilAttachmentCreates = 0;
            fNumDraws = 0;
            fNumFailedDraws = 0;
        }

        int renderTargetBinds() const { return fRenderTargetBinds; }
        void incRenderTargetBinds() { fRenderTargetBinds++; }
        int shaderCompilations() const { return fShaderCompilations; }
        void incShaderCompilations() { fShaderCompilations++; }
        int textureCreates() const { return fTextureCreates; }
        void incTextureCreates() { fTextureCreates++; }
        int textureUploads() const { return fTextureUploads; }
        void incTextureUploads() { fTextureUploads++; }
        int transfersToTexture() const { return fTransfersToTexture; }
        void incTransfersToTexture() { fTransfersToTexture++; }
        void incStencilAttachmentCreates() { fStencilAttachmentCreates++; }
        void incNumDraws() { fNumDraws++; }
        void incNumFailedDraws() { ++fNumFailedDraws; }
        void dump(SkString*);
        void dumpKeyValuePairs(SkTArray<SkString>* keys, SkTArray<double>* values);
        int numDraws() const { return fNumDraws; }
        int numFailedDraws() const { return fNumFailedDraws; }
    private:
        int fRenderTargetBinds;
        int fShaderCompilations;
        int fTextureCreates;
        int fTextureUploads;
        int fTransfersToTexture;
        int fStencilAttachmentCreates;
        int fNumDraws;
        int fNumFailedDraws;
#else
        void dump(SkString*) {}
        void dumpKeyValuePairs(SkTArray<SkString>*, SkTArray<double>*) {}
        void incRenderTargetBinds() {}
        void incShaderCompilations() {}
        void incTextureCreates() {}
        void incTextureUploads() {}
        void incTransfersToTexture() {}
        void incStencilAttachmentCreates() {}
        void incNumDraws() {}
        void incNumFailedDraws() {}
#endif
    };

    Stats* stats() { return &fStats; }

    /** Creates a texture directly in the backend API without wrapping it in a GrTexture. This is
        only to be used for testing (particularly for testing the methods that import an externally
        created texture into Skia. Must be matched with a call to deleteTestingOnlyTexture(). */
    virtual GrBackendObject createTestingOnlyBackendTexture(void* pixels, int w, int h,
                                                            GrPixelConfig config,
                                                            bool isRenderTarget = false) = 0;
    /** Check a handle represents an actual texture in the backend API that has not been freed. */
    virtual bool isTestingOnlyBackendTexture(GrBackendObject) const = 0;
    /** If ownership of the backend texture has been transferred pass true for abandonTexture. This
        will do any necessary cleanup of the handle without freeing the texture in the backend
        API. */
    virtual void deleteTestingOnlyBackendTexture(GrBackendObject,
                                                 bool abandonTexture = false) = 0;

    // width and height may be larger than rt (if underlying API allows it).
    // Returns nullptr if compatible sb could not be created, otherwise the caller owns the ref on
    // the GrStencilAttachment.
    virtual GrStencilAttachment* createStencilAttachmentForRenderTarget(const GrRenderTarget*,
                                                                        int width,
                                                                        int height) = 0;
    // clears target's entire stencil buffer to 0
    virtual void clearStencil(GrRenderTarget* target) = 0;

    // draws an outline rectangle for debugging/visualization purposes.
    virtual void drawDebugWireRect(GrRenderTarget*, const SkIRect&, GrColor) = 0;

    // Determines whether a texture will need to be rescaled in order to be used with the
    // GrSamplerParams. This variation is called when the caller will create a new texture using the
    // resource provider from a non-texture src (cpu-backed image, ...).
    bool isACopyNeededForTextureParams(int width, int height, const GrSamplerParams&,
                                       GrTextureProducer::CopyParams*,
                                       SkScalar scaleAdjust[2]) const;

    // Like the above but this variation should be called when the caller is not creating the
    // original texture but rather was handed the original texture. It adds additional checks
    // relevant to original textures that were created external to Skia via
    // GrResourceProvider::wrap methods.
    bool isACopyNeededForTextureParams(GrTextureProxy* proxy, const GrSamplerParams& params,
                                       GrTextureProducer::CopyParams* copyParams,
                                       SkScalar scaleAdjust[2]) const {
        if (this->isACopyNeededForTextureParams(proxy->width(), proxy->height(), params,
                                                copyParams, scaleAdjust)) {
            return true;
        }
        return this->onIsACopyNeededForTextureParams(proxy, params, copyParams, scaleAdjust);
    }

    // This is only to be used in GL-specific tests.
    virtual const GrGLContext* glContextForTesting() const { return nullptr; }

    // This is only to be used by testing code
    virtual void resetShaderCacheForTesting() const {}

    void handleDirtyContext() {
        if (fResetBits) {
            this->resetContext();
        }
    }

protected:
    static void ElevateDrawPreference(GrGpu::DrawPreference* preference,
                                      GrGpu::DrawPreference elevation) {
        GR_STATIC_ASSERT(GrGpu::kCallerPrefersDraw_DrawPreference > GrGpu::kNoDraw_DrawPreference);
        GR_STATIC_ASSERT(GrGpu::kGpuPrefersDraw_DrawPreference >
                         GrGpu::kCallerPrefersDraw_DrawPreference);
        GR_STATIC_ASSERT(GrGpu::kRequireDraw_DrawPreference >
                         GrGpu::kGpuPrefersDraw_DrawPreference);
        *preference = SkTMax(*preference, elevation);
    }

    // Handles cases where a surface will be updated without a call to flushRenderTarget
    void didWriteToSurface(GrSurface* surface, const SkIRect* bounds, uint32_t mipLevels = 1) const;

    Stats                            fStats;
    std::unique_ptr<GrPathRendering> fPathRendering;
    // Subclass must initialize this in its constructor.
    sk_sp<const GrCaps>              fCaps;

    typedef SkTArray<SkPoint, true> SamplePattern;

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
                                       SkBudgeted budgeted,
                                       const SkTArray<GrMipLevel>& texels) = 0;
    virtual GrTexture* onCreateCompressedTexture(const GrSurfaceDesc& desc,
                                                 SkBudgeted budgeted,
                                                 const SkTArray<GrMipLevel>& texels) = 0;

    virtual sk_sp<GrTexture> onWrapBackendTexture(const GrBackendTextureDesc&, GrWrapOwnership) = 0;
    virtual sk_sp<GrRenderTarget> onWrapBackendRenderTarget(const GrBackendRenderTargetDesc&) = 0;
    virtual sk_sp<GrRenderTarget> onWrapBackendTextureAsRenderTarget(const GrBackendTextureDesc&)=0;
    virtual GrBuffer* onCreateBuffer(size_t size, GrBufferType intendedType, GrAccessPattern,
                                     const void* data) = 0;

    virtual gr_instanced::InstancedRendering* onCreateInstancedRendering() = 0;

    virtual bool onIsACopyNeededForTextureParams(GrTextureProxy* proxy, const GrSamplerParams&,
                                                 GrTextureProducer::CopyParams*,
                                                 SkScalar scaleAdjust[2]) const {
        return false;
    }

    virtual bool onGetReadPixelsInfo(GrSurface* srcSurface, int readWidth, int readHeight,
                                     size_t rowBytes, GrPixelConfig readConfig, DrawPreference*,
                                     ReadPixelTempDrawInfo*) = 0;
    virtual bool onGetWritePixelsInfo(GrSurface* dstSurface, int width, int height,
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
                               GrPixelConfig config,
                               const SkTArray<GrMipLevel>& texels) = 0;

    // overridden by backend-specific derived class to perform the surface write
    virtual bool onTransferPixels(GrSurface*,
                                  int left, int top, int width, int height,
                                  GrPixelConfig config, GrBuffer* transferBuffer,
                                  size_t offset, size_t rowBytes) = 0;

    // overridden by backend-specific derived class to perform the resolve
    virtual void onResolveRenderTarget(GrRenderTarget* target) = 0;

    // overridden by backend specific derived class to perform the copy surface
    virtual bool onCopySurface(GrSurface* dst,
                               GrSurface* src,
                               const SkIRect& srcRect,
                               const SkIPoint& dstPoint) = 0;

    // overridden by backend specific derived class to perform the multisample queries
    virtual void onQueryMultisampleSpecs(GrRenderTarget*, const GrStencilSettings&,
                                         int* effectiveSampleCnt, SamplePattern*) = 0;

    void resetContext() {
        this->onResetContext(fResetBits);
        fResetBits = 0;
        ++fResetTimestamp;
    }

    struct SamplePatternComparator {
        bool operator()(const SamplePattern&, const SamplePattern&) const;
    };

    typedef std::map<SamplePattern, uint8_t, SamplePatternComparator> MultisampleSpecsIdMap;

    ResetTimestamp                         fResetTimestamp;
    uint32_t                               fResetBits;
    MultisampleSpecsIdMap                  fMultisampleSpecsIdMap;
    SkSTArray<1, MultisampleSpecs, true>   fMultisampleSpecs;
    // The context owns us, not vice-versa, so this ptr is not ref'ed by Gpu.
    GrContext*                             fContext;

    friend class GrPathRendering;
    friend class gr_instanced::InstancedRendering;
    typedef SkRefCnt INHERITED;
};

#endif
