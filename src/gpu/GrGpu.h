/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGpu_DEFINED
#define GrGpu_DEFINED

#include "include/core/SkPath.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GrTypes.h"
#include "include/private/SkTArray.h"
#include "src/gpu/GrAllocator.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrOpsRenderPass.h"
#include "src/gpu/GrSamplePatternDictionary.h"
#include "src/gpu/GrSwizzle.h"
#include "src/gpu/GrTextureProducer.h"
#include "src/gpu/GrXferProcessor.h"
#include <map>

class GrBackendRenderTarget;
class GrBackendSemaphore;
class GrGpuBuffer;
class GrContext;
struct GrContextOptions;
class GrGLContext;
class GrMesh;
class GrPath;
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
class SkJSONWriter;

class GrGpu : public SkRefCnt {
public:
    GrGpu(GrContext* context);
    ~GrGpu() override;

    GrContext* getContext() { return fContext; }
    const GrContext* getContext() const { return fContext; }

    /**
     * Gets the capabilities of the draw target.
     */
    const GrCaps* caps() const { return fCaps.get(); }
    sk_sp<const GrCaps> refCaps() const { return fCaps; }

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
     * Creates a texture object. If renderable is kYes then the returned texture can
     * be used as a render target by calling GrTexture::asRenderTarget(). Not all
     * pixel configs can be used as render targets. Support for configs as textures
     * or render targets can be checked using GrCaps.
     *
     * @param desc           describes the texture to be created.
     * @param format         the format for the texture (not currently used).
     * @param renderable     should the resulting texture be renderable
     * @param renderTargetSampleCnt The number of samples to use for rendering if renderable is
     *                       kYes. If renderable is kNo then this must be 1.
     * @param budgeted       does this texture count against the resource cache budget?
     * @param isProtected    should the texture be created as protected.
     * @param texels         array of mipmap levels containing texel data to load.
     *                       If level i has pixels then it is assumed that its dimensions are
     *                       max(1, floor(desc.fWidth / 2)) by max(1, floor(desc.fHeight / 2)).
     *                       If texels[i].fPixels == nullptr for all i <= mipLevelCount or
     *                       mipLevelCount is 0 then the texture's contents are uninitialized.
     *                       If a level has non-null pixels, its row bytes must be a multiple of the
     *                       config's bytes-per-pixel. The row bytes must be tight to the
     *                       level width if !caps->writePixelsRowBytesSupport().
     *                       If mipLevelCount > 1 and texels[i].fPixels != nullptr for any i > 0
     *                       then all levels must have non-null pixels. All levels must have
     *                       non-null pixels if GrCaps::createTextureMustSpecifyAllLevels() is true.
     * @param textureColorType The color type interpretation of the texture for the purpose of
     *                       of uploading texel data.
     * @param srcColorType   The color type of data in texels[].
     * @param texelLevelCount the number of levels in 'texels'. May be 0, 1, or
     *                       floor(max((log2(desc.fWidth), log2(desc.fHeight)))). It must be the
     *                       latter if GrCaps::createTextureMustSpecifyAllLevels() is true.
     * @return  The texture object if successful, otherwise nullptr.
     */
    sk_sp<GrTexture> createTexture(const GrSurfaceDesc& desc,
                                   const GrBackendFormat& format,
                                   GrRenderable renderable,
                                   int renderTargetSampleCnt,
                                   SkBudgeted,
                                   GrProtected isProtected,
                                   GrColorType textureColorType,
                                   GrColorType srcColorType,
                                   const GrMipLevel texels[],
                                   int texelLevelCount);

    /**
     * Simplified createTexture() interface for when there is no initial texel data to upload.
     */
    sk_sp<GrTexture> createTexture(const GrSurfaceDesc& desc,
                                   const GrBackendFormat& format,
                                   GrRenderable renderable,
                                   int renderTargetSampleCnt,
                                   GrMipMapped,
                                   SkBudgeted budgeted,
                                   GrProtected isProtected);

    sk_sp<GrTexture> createCompressedTexture(int width, int height, const GrBackendFormat&,
                                             SkImage::CompressionType, SkBudgeted, const void* data,
                                             size_t dataSize);

    /**
     * Implements GrResourceProvider::wrapBackendTexture
     */
    sk_sp<GrTexture> wrapBackendTexture(const GrBackendTexture&, GrColorType,
                                        GrWrapOwnership, GrWrapCacheable, GrIOType);

    /**
     * Implements GrResourceProvider::wrapRenderableBackendTexture
     */
    sk_sp<GrTexture> wrapRenderableBackendTexture(const GrBackendTexture&, int sampleCnt,
                                                  GrColorType, GrWrapOwnership, GrWrapCacheable);

    /**
     * Implements GrResourceProvider::wrapBackendRenderTarget
     */
    sk_sp<GrRenderTarget> wrapBackendRenderTarget(const GrBackendRenderTarget&,
                                                  GrColorType colorType);

    /**
     * Implements GrResourceProvider::wrapBackendTextureAsRenderTarget
     */
    sk_sp<GrRenderTarget> wrapBackendTextureAsRenderTarget(const GrBackendTexture&,
                                                           int sampleCnt,
                                                           GrColorType colorType);

    /**
     * Implements GrResourceProvider::wrapVulkanSecondaryCBAsRenderTarget
     */
    sk_sp<GrRenderTarget> wrapVulkanSecondaryCBAsRenderTarget(const SkImageInfo&,
                                                              const GrVkDrawableInfo&);

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
    sk_sp<GrGpuBuffer> createBuffer(size_t size, GrGpuBufferType intendedType,
                                    GrAccessPattern accessPattern, const void* data = nullptr);

    enum class ForExternalIO : bool {
        kYes = true,
        kNo = false
    };

    /**
     * Resolves MSAA.
     */
    void resolveRenderTarget(GrRenderTarget*, const SkIRect& resolveRect, GrSurfaceOrigin,
                             ForExternalIO);

    /**
     * Uses the base of the texture to recompute the contents of the other levels.
     */
    bool regenerateMipMapLevels(GrTexture*);

    /**
     * If the backend API has stateful texture bindings, this resets them back to defaults.
     */
    void resetTextureBindings();

    /**
     * Reads a rectangle of pixels from a render target. No sRGB/linear conversions are performed.
     *
     * @param surface           The surface to read from
     * @param left              left edge of the rectangle to read (inclusive)
     * @param top               top edge of the rectangle to read (inclusive)
     * @param width             width of rectangle to read in pixels.
     * @param height            height of rectangle to read in pixels.
     * @param surfaceColorType  the color type for this use of the surface.
     * @param dstColorType      the color type of the destination buffer.
     * @param buffer            memory to read the rectangle into.
     * @param rowBytes          the number of bytes between consecutive rows. Must be a multiple of
     *                          dstColorType's bytes-per-pixel. Must be tight to width if
     *                          !caps->readPixelsRowBytesSupport().
     *
     * @return true if the read succeeded, false if not. The read can fail
     *              because of the surface doesn't support reading, the color type
     *              is not allowed for the format of the surface or if the rectangle
     *              read is not contained in the surface.
     */
    bool readPixels(GrSurface* surface, int left, int top, int width, int height,
                    GrColorType surfaceColorType, GrColorType dstColorType, void* buffer,
                    size_t rowBytes);

    /**
     * Updates the pixels in a rectangle of a surface.  No sRGB/linear conversions are performed.
     *
     * @param surface            The surface to write to.
     * @param left               left edge of the rectangle to write (inclusive)
     * @param top                top edge of the rectangle to write (inclusive)
     * @param width              width of rectangle to write in pixels.
     * @param height             height of rectangle to write in pixels.
     * @param surfaceColorType   the color type for this use of the surface.
     * @param srcColorType       the color type of the source buffer.
     * @param texels             array of mipmap levels containing texture data. Row bytes must be a
     *                           multiple of srcColorType's bytes-per-pixel. Must be tight to level
     *                           width if !caps->writePixelsRowBytesSupport().
     * @param mipLevelCount      number of levels in 'texels'
     * @param prepForTexSampling After doing write pixels should the surface be prepared for texture
     *                           sampling. This is currently only used by Vulkan for inline uploads
     *                           to set that layout back to sampled after doing the upload. Inline
     *                           uploads currently can happen between draws in a single op so it is
     *                           not trivial to break up the GrOpsTask into two tasks when we see
     *                           an inline upload. However, once we are able to support doing that
     *                           we can remove this parameter.
     *
     * @return true if the write succeeded, false if not. The read can fail
     *              because of the surface doesn't support writing (e.g. read only),
     *              the color type is not allowed for the format of the surface or
     *              if the rectangle written is not contained in the surface.
     */
    bool writePixels(GrSurface* surface, int left, int top, int width, int height,
                     GrColorType surfaceColorType, GrColorType srcColorType,
                     const GrMipLevel texels[], int mipLevelCount, bool prepForTexSampling = false);

    /**
     * Helper for the case of a single level.
     */
    bool writePixels(GrSurface* surface, int left, int top, int width, int height,
                     GrColorType surfaceColorType, GrColorType srcColorType, const void* buffer,
                     size_t rowBytes, bool prepForTexSampling = false) {
        GrMipLevel mipLevel = {buffer, rowBytes};
        return this->writePixels(surface, left, top, width, height, surfaceColorType, srcColorType,
                                 &mipLevel, 1, prepForTexSampling);
    }

    /**
     * Updates the pixels in a rectangle of a texture using a buffer. If the texture is MIP mapped,
     * the base level is written to.
     *
     * @param texture          The texture to write to.
     * @param left             left edge of the rectangle to write (inclusive)
     * @param top              top edge of the rectangle to write (inclusive)
     * @param width            width of rectangle to write in pixels.
     * @param height           height of rectangle to write in pixels.
     * @param textureColorType the color type for this use of the surface.
     * @param bufferColorType  the color type of the transfer buffer's pixel data
     * @param transferBuffer   GrBuffer to read pixels from (type must be "kXferCpuToGpu")
     * @param offset           offset from the start of the buffer
     * @param rowBytes         number of bytes between consecutive rows in the buffer. Must be a
     *                         multiple of bufferColorType's bytes-per-pixel. Must be tight to width
     *                         if !caps->writePixelsRowBytesSupport().
     */
    bool transferPixelsTo(GrTexture* texture, int left, int top, int width, int height,
                          GrColorType textureColorType, GrColorType bufferColorType,
                          GrGpuBuffer* transferBuffer, size_t offset, size_t rowBytes);

    /**
     * Reads the pixels from a rectangle of a surface into a buffer. Use
     * GrCaps::SupportedRead::fOffsetAlignmentForTransferBuffer to determine the requirements for
     * the buffer offset alignment. If the surface is a MIP mapped texture, the base level is read.
     *
     * If successful the row bytes in the buffer is always:
     *   GrColorTypeBytesPerPixel(bufferColorType) * width
     *
     * Asserts that the caller has passed a properly aligned offset and that the buffer is
     * large enough to hold the result
     *
     * @param surface          The surface to read from.
     * @param left             left edge of the rectangle to read (inclusive)
     * @param top              top edge of the rectangle to read (inclusive)
     * @param width            width of rectangle to read in pixels.
     * @param height           height of rectangle to read in pixels.
     * @param surfaceColorType the color type for this use of the surface.
     * @param bufferColorType  the color type of the transfer buffer's pixel data
     * @param transferBuffer   GrBuffer to write pixels to (type must be "kXferGpuToCpu")
     * @param offset           offset from the start of the buffer
     */
    bool transferPixelsFrom(GrSurface* surface, int left, int top, int width, int height,
                            GrColorType surfaceColorType, GrColorType bufferColorType,
                            GrGpuBuffer* transferBuffer, size_t offset);

    // Called to perform a surface to surface copy. Fallbacks to issuing a draw from the src to dst
    // take place at higher levels and this function implement faster copy paths. The rect
    // and point are pre-clipped. The src rect and implied dst rect are guaranteed to be within the
    // src/dst bounds and non-empty. They must also be in their exact device space coords, including
    // already being transformed for origin if need be. If canDiscardOutsideDstRect is set to true
    // then we don't need to preserve any data on the dst surface outside of the copy.
    bool copySurface(GrSurface* dst, GrSurface* src, const SkIRect& srcRect,
                     const SkIPoint& dstPoint);

    // Queries the per-pixel HW sample locations for the given render target, and then finds or
    // assigns a key that uniquely identifies the sample pattern. The actual sample locations can be
    // retrieved with retrieveSampleLocations().
    int findOrAssignSamplePatternKey(GrRenderTarget*);

    // Retrieves the per-pixel HW sample locations for the given sample pattern key, and, as a
    // by-product, the actual number of samples in use. (This may differ from the number of samples
    // requested by the render target.) Sample locations are returned as 0..1 offsets relative to
    // the top-left corner of the pixel.
    const SkTArray<SkPoint>& retrieveSampleLocations(int samplePatternKey) const {
        return fSamplePatternDictionary.retrieveSampleLocations(samplePatternKey);
    }

    // Returns a GrOpsRenderPass which GrOpsTasks send draw commands to instead of directly
    // to the Gpu object. The 'bounds' rect is the content rect of the renderTarget.
    virtual GrOpsRenderPass* getOpsRenderPass(
            GrRenderTarget* renderTarget, GrSurfaceOrigin, const SkIRect& bounds,
            const GrOpsRenderPass::LoadAndStoreInfo&,
            const GrOpsRenderPass::StencilLoadAndStoreInfo&,
            const SkTArray<GrSurfaceProxy*, true>& sampledProxies) = 0;

    // Called by GrDrawingManager when flushing.
    // Provides a hook for post-flush actions (e.g. Vulkan command buffer submits). This will also
    // insert any numSemaphore semaphores on the gpu and set the backendSemaphores to match the
    // inserted semaphores.
    GrSemaphoresSubmitted finishFlush(GrSurfaceProxy*[], int n,
                                      SkSurface::BackendSurfaceAccess access, const GrFlushInfo&,
                                      const GrPrepareForExternalIORequests&);

    virtual void submit(GrOpsRenderPass*) = 0;

    virtual GrFence SK_WARN_UNUSED_RESULT insertFence() = 0;
    virtual bool waitFence(GrFence, uint64_t timeout = 1000) = 0;
    virtual void deleteFence(GrFence) const = 0;

    virtual std::unique_ptr<GrSemaphore> SK_WARN_UNUSED_RESULT makeSemaphore(
            bool isOwned = true) = 0;
    virtual std::unique_ptr<GrSemaphore> wrapBackendSemaphore(const GrBackendSemaphore& semaphore,
            GrResourceProvider::SemaphoreWrapType wrapType, GrWrapOwnership ownership) = 0;
    virtual void insertSemaphore(GrSemaphore* semaphore) = 0;
    virtual void waitSemaphore(GrSemaphore* semaphore) = 0;

    virtual void checkFinishProcs() = 0;

    /**
     *  Put this texture in a safe and known state for use across multiple GrContexts. Depending on
     *  the backend, this may return a GrSemaphore. If so, other contexts should wait on that
     *  semaphore before using this texture.
     */
    virtual std::unique_ptr<GrSemaphore> prepareTextureForCrossContextUsage(GrTexture*) = 0;

    ///////////////////////////////////////////////////////////////////////////
    // Debugging and Stats

    class Stats {
    public:
#if GR_GPU_STATS
        Stats() = default;

        void reset() { *this = {}; }

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

        int transfersFromSurface() const { return fTransfersFromSurface; }
        void incTransfersFromSurface() { fTransfersFromSurface++; }

        int stencilAttachmentCreates() const { return fStencilAttachmentCreates; }
        void incStencilAttachmentCreates() { fStencilAttachmentCreates++; }

        int numDraws() const { return fNumDraws; }
        void incNumDraws() { fNumDraws++; }

        int numFailedDraws() const { return fNumFailedDraws; }
        void incNumFailedDraws() { ++fNumFailedDraws; }

        int numFinishFlushes() const { return fNumFinishFlushes; }
        void incNumFinishFlushes() { ++fNumFinishFlushes; }

        int numScratchTexturesReused() const { return fNumScratchTexturesReused; }
        void incNumScratchTexturesReused() { ++fNumScratchTexturesReused; }

#if GR_TEST_UTILS
        void dump(SkString*);
        void dumpKeyValuePairs(SkTArray<SkString>* keys, SkTArray<double>* values);
#endif
    private:
        int fRenderTargetBinds = 0;
        int fShaderCompilations = 0;
        int fTextureCreates = 0;
        int fTextureUploads = 0;
        int fTransfersToTexture = 0;
        int fTransfersFromSurface = 0;
        int fStencilAttachmentCreates = 0;
        int fNumDraws = 0;
        int fNumFailedDraws = 0;
        int fNumFinishFlushes = 0;
        int fNumScratchTexturesReused = 0;
#else

#if GR_TEST_UTILS
        void dump(SkString*) {}
        void dumpKeyValuePairs(SkTArray<SkString>*, SkTArray<double>*) {}
#endif
        void incRenderTargetBinds() {}
        void incShaderCompilations() {}
        void incTextureCreates() {}
        void incTextureUploads() {}
        void incTransfersToTexture() {}
        void incStencilAttachmentCreates() {}
        void incNumDraws() {}
        void incNumFailedDraws() {}
        void incNumFinishFlushes() {}
#endif
    };

    Stats* stats() { return &fStats; }
    void dumpJSON(SkJSONWriter*) const;

    /** Used to initialize a backend texture with either a constant color or from pixmaps. */
    class BackendTextureData {
    public:
        enum class Type { kColor, kPixmaps };
        BackendTextureData() = default;
        BackendTextureData(const SkColor4f& color) : fType(Type::kColor), fColor(color) {}
        BackendTextureData(const SkPixmap pixmaps[]) : fType(Type::kPixmaps), fPixmaps(pixmaps) {
            SkASSERT(pixmaps);
        }

        Type type() const { return fType; }
        SkColor4f color() const {
            SkASSERT(this->type() == Type::kColor);
            return fColor;
        }

        const SkPixmap& pixmap(int i) const { return fPixmaps[i]; }
        const SkPixmap* pixmaps() const { return fPixmaps; }

    private:
        Type fType = Type::kColor;
        union {
            SkColor4f fColor = {0, 0, 0, 0};
            const SkPixmap* fPixmaps;
        };
    };

    /**
     * Creates a texture directly in the backend API without wrapping it in a GrTexture.
     * Must be matched with a call to deleteBackendTexture().
     *
     * numMipLevels must be 1 or be the number of levels for a complete MIP hierarchy with
     * dimensions as the base size. Otherwise this will fail.
     *
     * If data is null the texture is uninitialized.
     *
     * If data represents a color then all texture levels are cleared to that color.
     *
     * If data represents pixmaps then it must have numMipLevels pixmaps and they must be sized
     * correctly according to the MIP sizes implied by dimensions. They must all have the same color
     * type and that color type must be compatible with the texture format.
     */
    GrBackendTexture createBackendTexture(SkISize dimensions,
                                          const GrBackendFormat&,
                                          GrRenderable,
                                          const BackendTextureData* data,
                                          int numMipLevels,
                                          GrProtected isProtected);

    /**
     * Frees a texture created by createBackendTexture(). If ownership of the backend
     * texture has been transferred to a GrContext using adopt semantics this should not be called.
     */
    virtual void deleteBackendTexture(const GrBackendTexture&) = 0;

    virtual bool precompileShader(const SkData& key, const SkData& data) { return false; }

#if GR_TEST_UTILS
    /** Check a handle represents an actual texture in the backend API that has not been freed. */
    virtual bool isTestingOnlyBackendTexture(const GrBackendTexture&) const = 0;

    virtual GrBackendRenderTarget createTestingOnlyBackendRenderTarget(int w, int h,
                                                                       GrColorType) = 0;

    virtual void deleteTestingOnlyBackendRenderTarget(const GrBackendRenderTarget&) = 0;

    // This is only to be used in GL-specific tests.
    virtual const GrGLContext* glContextForTesting() const { return nullptr; }

    // This is only to be used by testing code
    virtual void resetShaderCacheForTesting() const {}

    /**
     * Flushes all work to the gpu and forces the GPU to wait until all the gpu work has completed.
     * This is for testing purposes only.
     */
    virtual void testingOnly_flushGpuAndSync() = 0;

    /**
     * Inserted as a pair around a block of code to do a GPU frame capture.
     * Currently only works with the Metal backend.
     */
    virtual void testingOnly_startCapture() {}
    virtual void testingOnly_endCapture() {}
#endif

    // width and height may be larger than rt (if underlying API allows it).
    // Returns nullptr if compatible sb could not be created, otherwise the caller owns the ref on
    // the GrStencilAttachment.
    virtual GrStencilAttachment* createStencilAttachmentForRenderTarget(
            const GrRenderTarget*, int width, int height, int numStencilSamples) = 0;

    // Determines whether a texture will need to be rescaled in order to be used with the
    // GrSamplerState.
    static bool IsACopyNeededForRepeatWrapMode(const GrCaps*,
                                               GrTextureProxy* texProxy,
                                               SkISize dimensions,
                                               GrSamplerState::Filter,
                                               GrTextureProducer::CopyParams*,
                                               SkScalar scaleAdjust[2]);

    // Determines whether a texture will need to be copied because the draw requires mips but the
    // texutre doesn't have any. This call should be only checked if IsACopyNeededForTextureParams
    // fails. If the previous call succeeds, then a copy should be done using those params and the
    // mip mapping requirements will be handled there.
    static bool IsACopyNeededForMips(const GrCaps* caps, const GrTextureProxy* texProxy,
                                     GrSamplerState::Filter filter,
                                     GrTextureProducer::CopyParams* copyParams);

    void handleDirtyContext() {
        if (fResetBits) {
            this->resetContext();
        }
    }

    virtual void storeVkPipelineCacheData() {}

protected:
    static bool MipMapsAreCorrect(SkISize, const BackendTextureData*, int numMipLevels);

    // Handles cases where a surface will be updated without a call to flushRenderTarget.
    void didWriteToSurface(GrSurface* surface, GrSurfaceOrigin origin, const SkIRect* bounds,
                           uint32_t mipLevels = 1) const;

    Stats                            fStats;
    std::unique_ptr<GrPathRendering> fPathRendering;
    // Subclass must initialize this in its constructor.
    sk_sp<const GrCaps>              fCaps;

private:
    virtual GrBackendTexture onCreateBackendTexture(SkISize dimensions,
                                                    const GrBackendFormat&,
                                                    GrRenderable,
                                                    const BackendTextureData*,
                                                    int numMipLevels,
                                                    GrProtected isProtected) = 0;

    // called when the 3D context state is unknown. Subclass should emit any
    // assumed 3D context state and dirty any state cache.
    virtual void onResetContext(uint32_t resetBits) = 0;

    // Implementation of resetTextureBindings.
    virtual void onResetTextureBindings() {}

    // Queries the effective number of samples in use by the hardware for the given render target,
    // and queries the individual sample locations.
    virtual void querySampleLocations(GrRenderTarget*, SkTArray<SkPoint>*) = 0;

    // Called before certain draws in order to guarantee coherent results from dst reads.
    virtual void xferBarrier(GrRenderTarget*, GrXferBarrierType) = 0;

    // overridden by backend-specific derived class to create objects.
    // Texture size, renderablility, format support, sample count will have already been validated
    // in base class before onCreateTexture is called.
    // If the ith bit is set in levelClearMask then the ith MIP level should be cleared.
    virtual sk_sp<GrTexture> onCreateTexture(const GrSurfaceDesc&,
                                             const GrBackendFormat&,
                                             GrRenderable,
                                             int renderTargetSampleCnt,
                                             SkBudgeted,
                                             GrProtected,
                                             int mipLevelCoont,
                                             uint32_t levelClearMask) = 0;
    virtual sk_sp<GrTexture> onCreateCompressedTexture(int width, int height,
                                                       const GrBackendFormat&,
                                                       SkImage::CompressionType, SkBudgeted,
                                                       const void* data) = 0;
    virtual sk_sp<GrTexture> onWrapBackendTexture(const GrBackendTexture&, GrColorType,
                                                  GrWrapOwnership, GrWrapCacheable, GrIOType) = 0;
    virtual sk_sp<GrTexture> onWrapRenderableBackendTexture(const GrBackendTexture&, int sampleCnt,
                                                            GrColorType, GrWrapOwnership,
                                                            GrWrapCacheable) = 0;
    virtual sk_sp<GrRenderTarget> onWrapBackendRenderTarget(const GrBackendRenderTarget&,
                                                            GrColorType) = 0;
    virtual sk_sp<GrRenderTarget> onWrapBackendTextureAsRenderTarget(const GrBackendTexture&,
                                                                     int sampleCnt,
                                                                     GrColorType) = 0;
    virtual sk_sp<GrRenderTarget> onWrapVulkanSecondaryCBAsRenderTarget(const SkImageInfo&,
                                                                        const GrVkDrawableInfo&);

    virtual sk_sp<GrGpuBuffer> onCreateBuffer(size_t size, GrGpuBufferType intendedType,
                                              GrAccessPattern, const void* data) = 0;

    // overridden by backend-specific derived class to perform the surface read
    virtual bool onReadPixels(GrSurface*, int left, int top, int width, int height,
                              GrColorType surfaceColorType, GrColorType dstColorType, void* buffer,
                              size_t rowBytes) = 0;

    // overridden by backend-specific derived class to perform the surface write
    virtual bool onWritePixels(GrSurface*, int left, int top, int width, int height,
                               GrColorType surfaceColorType, GrColorType srcColorType,
                               const GrMipLevel texels[], int mipLevelCount,
                               bool prepForTexSampling) = 0;

    // overridden by backend-specific derived class to perform the texture transfer
    virtual bool onTransferPixelsTo(GrTexture*, int left, int top, int width, int height,
                                    GrColorType textiueColorType, GrColorType bufferColorType,
                                    GrGpuBuffer* transferBuffer, size_t offset,
                                    size_t rowBytes) = 0;
    // overridden by backend-specific derived class to perform the surface transfer
    virtual bool onTransferPixelsFrom(GrSurface*, int left, int top, int width, int height,
                                      GrColorType surfaceColorType, GrColorType bufferColorType,
                                      GrGpuBuffer* transferBuffer, size_t offset) = 0;

    // overridden by backend-specific derived class to perform the resolve
    virtual void onResolveRenderTarget(GrRenderTarget* target, const SkIRect& resolveRect,
                                       GrSurfaceOrigin resolveOrigin, ForExternalIO) = 0;

    // overridden by backend specific derived class to perform mip map level regeneration.
    virtual bool onRegenerateMipMapLevels(GrTexture*) = 0;

    // overridden by backend specific derived class to perform the copy surface
    virtual bool onCopySurface(GrSurface* dst, GrSurface* src, const SkIRect& srcRect,
                               const SkIPoint& dstPoint) = 0;

    virtual bool onFinishFlush(GrSurfaceProxy*[], int n, SkSurface::BackendSurfaceAccess access,
                               const GrFlushInfo&, const GrPrepareForExternalIORequests&) = 0;

#ifdef SK_ENABLE_DUMP_GPU
    virtual void onDumpJSON(SkJSONWriter*) const {}
#endif

    sk_sp<GrTexture> createTextureCommon(const GrSurfaceDesc& desc,
                                         const GrBackendFormat& format,
                                         GrRenderable renderable,
                                         int renderTargetSampleCnt,
                                         SkBudgeted budgeted,
                                         GrProtected isProtected,
                                         int mipLevelCnt,
                                         uint32_t levelClearMask);

    void resetContext() {
        this->onResetContext(fResetBits);
        fResetBits = 0;
    }

    uint32_t fResetBits;
    // The context owns us, not vice-versa, so this ptr is not ref'ed by Gpu.
    GrContext* fContext;
    GrSamplePatternDictionary fSamplePatternDictionary;

    friend class GrPathRendering;
    typedef SkRefCnt INHERITED;
};

#endif
