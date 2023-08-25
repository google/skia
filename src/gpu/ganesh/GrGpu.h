/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGpu_DEFINED
#define GrGpu_DEFINED

#include "include/core/SkPath.h"
#include "include/core/SkSpan.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GrTypes.h"
#include "include/private/base/SkTArray.h"
#include "src/base/SkTInternalLList.h"
#include "src/gpu/RefCntedCallback.h"
#include "src/gpu/Swizzle.h"
#include "src/gpu/ganesh/GrAttachment.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrGpuBuffer.h"
#include "src/gpu/ganesh/GrOpsRenderPass.h"
#include "src/gpu/ganesh/GrPixmap.h"
#include "src/gpu/ganesh/GrXferProcessor.h"

class GrAttachment;
class GrBackendRenderTarget;
class GrBackendSemaphore;
struct GrContextOptions;
class GrDirectContext;
class GrGLContext;
class GrPipeline;
class GrGeometryProcessor;
class GrRenderTarget;
class GrRingBuffer;
class GrSemaphore;
class GrStagingBufferManager;
class GrStencilSettings;
class GrSurface;
class GrTexture;
class GrThreadSafePipelineBuilder;
struct GrVkDrawableInfo;
class SkJSONWriter;
enum class SkTextureCompressionType;

namespace SkSL {
    class Compiler;
}

class GrGpu : public SkRefCnt {
public:
    GrGpu(GrDirectContext* direct);
    ~GrGpu() override;

    GrDirectContext* getContext() { return fContext; }
    const GrDirectContext* getContext() const { return fContext; }

    /**
     * Gets the capabilities of the draw target.
     */
    const GrCaps* caps() const { return fCaps.get(); }
    sk_sp<const GrCaps> refCaps() const { return fCaps; }

    virtual GrStagingBufferManager* stagingBufferManager() { return nullptr; }

    virtual GrRingBuffer* uniformsRingBuffer() { return nullptr; }

    SkSL::Compiler* shaderCompiler() const { return fCompiler.get(); }

    enum class DisconnectType {
        // No cleanup should be attempted, immediately cease making backend API calls
        kAbandon,
        // Free allocated resources (not known by GrResourceCache) before returning and
        // ensure no backend backend 3D API calls will be made after disconnect() returns.
        kCleanup,
    };

    // Called by context when the underlying backend context is already or will be destroyed
    // before GrDirectContext.
    virtual void disconnect(DisconnectType);

    virtual GrThreadSafePipelineBuilder* pipelineBuilder() = 0;
    virtual sk_sp<GrThreadSafePipelineBuilder> refPipelineBuilder() = 0;

    // Called by GrDirectContext::isContextLost. Returns true if the backend Gpu object has gotten
    // into an unrecoverable, lost state.
    virtual bool isDeviceLost() const { return false; }

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
     * @param dimensions     dimensions of the texture to be created.
     * @param format         the format for the texture (not currently used).
     * @param renderable     should the resulting texture be renderable
     * @param renderTargetSampleCnt The number of samples to use for rendering if renderable is
     *                       kYes. If renderable is kNo then this must be 1.
     * @param budgeted       does this texture count against the resource cache budget?
     * @param isProtected    should the texture be created as protected.
     * @param texels         array of mipmap levels containing texel data to load.
     *                       If level i has pixels then it is assumed that its dimensions are
     *                       max(1, floor(dimensions.fWidth / 2)) by
     *                       max(1, floor(dimensions.fHeight / 2)).
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
     *                       floor(max((log2(dimensions.fWidth), log2(dimensions.fHeight)))). It
     *                       must be the latter if GrCaps::createTextureMustSpecifyAllLevels() is
     *                       true.
     * @return  The texture object if successful, otherwise nullptr.
     */
    sk_sp<GrTexture> createTexture(SkISize dimensions,
                                   const GrBackendFormat& format,
                                   GrTextureType textureType,
                                   GrRenderable renderable,
                                   int renderTargetSampleCnt,
                                   skgpu::Budgeted budgeted,
                                   GrProtected isProtected,
                                   GrColorType textureColorType,
                                   GrColorType srcColorType,
                                   const GrMipLevel texels[],
                                   int texelLevelCount,
                                   std::string_view label);

    /**
     * Simplified createTexture() interface for when there is no initial texel data to upload.
     */
    sk_sp<GrTexture> createTexture(SkISize dimensions,
                                   const GrBackendFormat& format,
                                   GrTextureType textureType,
                                   GrRenderable renderable,
                                   int renderTargetSampleCnt,
                                   GrMipmapped mipmapped,
                                   skgpu::Budgeted budgeted,
                                   GrProtected isProtected,
                                   std::string_view label);

    sk_sp<GrTexture> createCompressedTexture(SkISize dimensions,
                                             const GrBackendFormat& format,
                                             skgpu::Budgeted budgeted,
                                             GrMipmapped mipmapped,
                                             GrProtected isProtected,
                                             const void* data,
                                             size_t dataSize);

    /**
     * Implements GrResourceProvider::wrapBackendTexture
     */
    sk_sp<GrTexture> wrapBackendTexture(const GrBackendTexture&,
                                        GrWrapOwnership,
                                        GrWrapCacheable,
                                        GrIOType);

    sk_sp<GrTexture> wrapCompressedBackendTexture(const GrBackendTexture&,
                                                  GrWrapOwnership,
                                                  GrWrapCacheable);

    /**
     * Implements GrResourceProvider::wrapRenderableBackendTexture
     */
    sk_sp<GrTexture> wrapRenderableBackendTexture(const GrBackendTexture&,
                                                  int sampleCnt,
                                                  GrWrapOwnership,
                                                  GrWrapCacheable);

    /**
     * Implements GrResourceProvider::wrapBackendRenderTarget
     */
    sk_sp<GrRenderTarget> wrapBackendRenderTarget(const GrBackendRenderTarget&);

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
     *
     * @return the buffer if successful, otherwise nullptr.
     */
    sk_sp<GrGpuBuffer> createBuffer(size_t size,
                                    GrGpuBufferType intendedType,
                                    GrAccessPattern accessPattern);

    /**
     * Resolves MSAA. The resolveRect must already be in the native destination space.
     */
    void resolveRenderTarget(GrRenderTarget*, const SkIRect& resolveRect);

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
     * @param surface           the surface to read from
     * @param rect              the rectangle of pixels to read
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
    bool readPixels(GrSurface* surface,
                    SkIRect rect,
                    GrColorType surfaceColorType,
                    GrColorType dstColorType,
                    void* buffer,
                    size_t rowBytes);

    /**
     * Updates the pixels in a rectangle of a surface.  No sRGB/linear conversions are performed.
     *
     * @param surface            the surface to write to.
     * @param rect               the rectangle of pixels to overwrite
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
     *                           not trivial to break up the OpsTask into two tasks when we see
     *                           an inline upload. However, once we are able to support doing that
     *                           we can remove this parameter.
     *
     * @return true if the write succeeded, false if not. The read can fail
     *              because of the surface doesn't support writing (e.g. read only),
     *              the color type is not allowed for the format of the surface or
     *              if the rectangle written is not contained in the surface.
     */
    bool writePixels(GrSurface* surface,
                     SkIRect rect,
                     GrColorType surfaceColorType,
                     GrColorType srcColorType,
                     const GrMipLevel texels[],
                     int mipLevelCount,
                     bool prepForTexSampling = false);

    /**
     * Helper for the case of a single level.
     */
    bool writePixels(GrSurface* surface,
                     SkIRect rect,
                     GrColorType surfaceColorType,
                     GrColorType srcColorType,
                     const void* buffer,
                     size_t rowBytes,
                     bool prepForTexSampling = false) {
        GrMipLevel mipLevel = {buffer, rowBytes, nullptr};
        return this->writePixels(surface,
                                 rect,
                                 surfaceColorType,
                                 srcColorType,
                                 &mipLevel,
                                 1,
                                 prepForTexSampling);
    }

    /**
     * Transfer bytes from one GPU buffer to another. The src buffer must have type kXferCpuToGpu
     * and the dst buffer must not. Neither buffer may currently be mapped. The offsets and size
     * must be aligned to GrCaps::transferFromBufferToBufferAlignment.
     *
     * @param src        the buffer to read from
     * @param srcOffset  the aligned offset at the src at which the transfer begins.
     * @param dst        the buffer to write to
     * @param dstOffset  the aligned offset in the dst at which the transfer begins
     * @param size       the aligned number of bytes to transfer;
     */
    bool transferFromBufferToBuffer(sk_sp<GrGpuBuffer> src,
                                    size_t srcOffset,
                                    sk_sp<GrGpuBuffer> dst,
                                    size_t dstOffset,
                                    size_t size);

    /**
     * Updates the pixels in a rectangle of a texture using a buffer. If the texture is MIP mapped,
     * the base level is written to.
     *
     * @param texture          the texture to write to.
     * @param rect             the rectangle of pixels in the texture to overwrite
     * @param textureColorType the color type for this use of the surface.
     * @param bufferColorType  the color type of the transfer buffer's pixel data
     * @param transferBuffer   GrBuffer to read pixels from (type must be "kXferCpuToGpu")
     * @param offset           offset from the start of the buffer
     * @param rowBytes         number of bytes between consecutive rows in the buffer. Must be a
     *                         multiple of bufferColorType's bytes-per-pixel. Must be tight to
     *                         rect.width() if !caps->writePixelsRowBytesSupport().
     */
    bool transferPixelsTo(GrTexture* texture,
                          SkIRect rect,
                          GrColorType textureColorType,
                          GrColorType bufferColorType,
                          sk_sp<GrGpuBuffer> transferBuffer,
                          size_t offset,
                          size_t rowBytes);

    /**
     * Reads the pixels from a rectangle of a surface into a buffer. Use
     * GrCaps::SupportedRead::fOffsetAlignmentForTransferBuffer to determine the requirements for
     * the buffer offset alignment. If the surface is a MIP mapped texture, the base level is read.
     *
     * If successful the row bytes in the buffer is always:
     *   GrColorTypeBytesPerPixel(bufferColorType) * rect.width()
     *
     * Asserts that the caller has passed a properly aligned offset and that the buffer is
     * large enough to hold the result
     *
     * @param surface          the surface to read from.
     * @param rect             the rectangle of pixels to read
     * @param surfaceColorType the color type for this use of the surface.
     * @param bufferColorType  the color type of the transfer buffer's pixel data
     * @param transferBuffer   GrBuffer to write pixels to (type must be "kXferGpuToCpu")
     * @param offset           offset from the start of the buffer
     */
    bool transferPixelsFrom(GrSurface* surface,
                            SkIRect rect,
                            GrColorType surfaceColorType,
                            GrColorType bufferColorType,
                            sk_sp<GrGpuBuffer> transferBuffer,
                            size_t offset);

    // Called to perform a surface to surface copy. Fallbacks to issuing a draw from the src to dst
    // take place at higher levels and this function implement faster copy paths. The src and dst
    // rects are pre-clipped. The src rect and dst rect are guaranteed to be within the
    // src/dst bounds and non-empty. They must also be in their exact device space coords, including
    // already being transformed for origin if need be. If canDiscardOutsideDstRect is set to true
    // then we don't need to preserve any data on the dst surface outside of the copy.
    //
    // Backends may or may not support src and dst rects with differing dimensions. This can assume
    // that GrCaps.canCopySurface() returned true for these surfaces and rects.
    bool copySurface(GrSurface* dst, const SkIRect& dstRect,
                     GrSurface* src, const SkIRect& srcRect,
                     GrSamplerState::Filter filter);

    // Returns a GrOpsRenderPass which OpsTasks send draw commands to instead of directly
    // to the Gpu object. The 'bounds' rect is the content rect of the renderTarget.
    // If a 'stencil' is provided it will be the one bound to 'renderTarget'. If one is not
    // provided but 'renderTarget' has a stencil buffer then that is a signal that the
    // render target's stencil buffer should be ignored.
    GrOpsRenderPass* getOpsRenderPass(
            GrRenderTarget* renderTarget,
            bool useMSAASurface,
            GrAttachment* stencil,
            GrSurfaceOrigin,
            const SkIRect& bounds,
            const GrOpsRenderPass::LoadAndStoreInfo&,
            const GrOpsRenderPass::StencilLoadAndStoreInfo&,
            const skia_private::TArray<GrSurfaceProxy*, true>& sampledProxies,
            GrXferBarrierFlags renderPassXferBarriers);

    // Called by GrDrawingManager when flushing.
    // Provides a hook for post-flush actions (e.g. Vulkan command buffer submits). This will also
    // insert any numSemaphore semaphores on the gpu and set the backendSemaphores to match the
    // inserted semaphores.
    void executeFlushInfo(SkSpan<GrSurfaceProxy*>,
                          SkSurfaces::BackendSurfaceAccess access,
                          const GrFlushInfo&,
                          const skgpu::MutableTextureState* newState);

    // Called before render tasks are executed during a flush.
    virtual void willExecute() {}

    bool submitToGpu(bool syncCpu);

    virtual void submit(GrOpsRenderPass*) = 0;

    [[nodiscard]] virtual GrFence insertFence() = 0;
    virtual bool waitFence(GrFence) = 0;
    virtual void deleteFence(GrFence) = 0;

    [[nodiscard]] virtual std::unique_ptr<GrSemaphore> makeSemaphore(bool isOwned = true) = 0;
    virtual std::unique_ptr<GrSemaphore> wrapBackendSemaphore(const GrBackendSemaphore&,
                                                              GrSemaphoreWrapType,
                                                              GrWrapOwnership) = 0;
    virtual void insertSemaphore(GrSemaphore* semaphore) = 0;
    virtual void waitSemaphore(GrSemaphore* semaphore) = 0;

    virtual void addFinishedProc(GrGpuFinishedProc finishedProc,
                                 GrGpuFinishedContext finishedContext) = 0;
    virtual void checkFinishProcs() = 0;
    virtual void finishOutstandingGpuWork() = 0;

    virtual void takeOwnershipOfBuffer(sk_sp<GrGpuBuffer>) {}

    /**
     * Checks if we detected an OOM from the underlying 3D API and if so returns true and resets
     * the internal OOM state to false. Otherwise, returns false.
     */
    bool checkAndResetOOMed();

    /**
     *  Put this texture in a safe and known state for use across multiple contexts. Depending on
     *  the backend, this may return a GrSemaphore. If so, other contexts should wait on that
     *  semaphore before using this texture.
     */
    virtual std::unique_ptr<GrSemaphore> prepareTextureForCrossContextUsage(GrTexture*) = 0;

    /**
     * Frees any backend specific objects that are not currently in use by the GPU. This is called
     * when the client is trying to free up as much GPU memory as possible. We will not release
     * resources connected to programs/pipelines since the cost to recreate those is significantly
     * higher that other resources.
     */
    virtual void releaseUnlockedBackendObjects() {}

    ///////////////////////////////////////////////////////////////////////////
    // Debugging and Stats

    class Stats {
    public:
#if GR_GPU_STATS
        Stats() = default;

        void reset() { *this = {}; }

        int textureCreates() const { return fTextureCreates; }
        void incTextureCreates() { fTextureCreates++; }

        int textureUploads() const { return fTextureUploads; }
        void incTextureUploads() { fTextureUploads++; }

        int transfersToTexture() const { return fTransfersToTexture; }
        void incTransfersToTexture() { fTransfersToTexture++; }

        int transfersFromSurface() const { return fTransfersFromSurface; }
        void incTransfersFromSurface() { fTransfersFromSurface++; }

        void incBufferTransfers() { fBufferTransfers++; }
        int bufferTransfers() const { return fBufferTransfers; }

        int stencilAttachmentCreates() const { return fStencilAttachmentCreates; }
        void incStencilAttachmentCreates() { fStencilAttachmentCreates++; }

        int msaaAttachmentCreates() const { return fMSAAAttachmentCreates; }
        void incMSAAAttachmentCreates() { fMSAAAttachmentCreates++; }

        int numDraws() const { return fNumDraws; }
        void incNumDraws() { fNumDraws++; }

        int numFailedDraws() const { return fNumFailedDraws; }
        void incNumFailedDraws() { ++fNumFailedDraws; }

        int numSubmitToGpus() const { return fNumSubmitToGpus; }
        void incNumSubmitToGpus() { ++fNumSubmitToGpus; }

        int numScratchTexturesReused() const { return fNumScratchTexturesReused; }
        void incNumScratchTexturesReused() { ++fNumScratchTexturesReused; }

        int numScratchMSAAAttachmentsReused() const { return fNumScratchMSAAAttachmentsReused; }
        void incNumScratchMSAAAttachmentsReused() { ++fNumScratchMSAAAttachmentsReused; }

        int renderPasses() const { return fRenderPasses; }
        void incRenderPasses() { fRenderPasses++; }

        int numReorderedDAGsOverBudget() const { return fNumReorderedDAGsOverBudget; }
        void incNumReorderedDAGsOverBudget() { fNumReorderedDAGsOverBudget++; }

#if defined(GR_TEST_UTILS)
        void dump(SkString*);
        void dumpKeyValuePairs(
                skia_private::TArray<SkString>* keys, skia_private::TArray<double>* values);
#endif
    private:
        int fTextureCreates = 0;
        int fTextureUploads = 0;
        int fTransfersToTexture = 0;
        int fTransfersFromSurface = 0;
        int fBufferTransfers = 0;
        int fStencilAttachmentCreates = 0;
        int fMSAAAttachmentCreates = 0;
        int fNumDraws = 0;
        int fNumFailedDraws = 0;
        int fNumSubmitToGpus = 0;
        int fNumScratchTexturesReused = 0;
        int fNumScratchMSAAAttachmentsReused = 0;
        int fRenderPasses = 0;
        int fNumReorderedDAGsOverBudget = 0;

#else  // !GR_GPU_STATS

#if defined(GR_TEST_UTILS)
        void dump(SkString*) {}
        void dumpKeyValuePairs(skia_private::TArray<SkString>*, skia_private::TArray<double>*) {}
#endif
        void incTextureCreates() {}
        void incTextureUploads() {}
        void incTransfersToTexture() {}
        void incBufferTransfers() {}
        void incTransfersFromSurface() {}
        void incStencilAttachmentCreates() {}
        void incMSAAAttachmentCreates() {}
        void incNumDraws() {}
        void incNumFailedDraws() {}
        void incNumSubmitToGpus() {}
        void incNumScratchTexturesReused() {}
        void incNumScratchMSAAAttachmentsReused() {}
        void incRenderPasses() {}
        void incNumReorderedDAGsOverBudget() {}
#endif
    };

    Stats* stats() { return &fStats; }
    void dumpJSON(SkJSONWriter*) const;


    /**
     * Creates a texture directly in the backend API without wrapping it in a GrTexture.
     * Must be matched with a call to deleteBackendTexture().
     *
     * If data is null the texture is uninitialized.
     *
     * If data represents a color then all texture levels are cleared to that color.
     *
     * If data represents pixmaps then it must have a either one pixmap or, if mipmapping
     * is specified, a complete MIP hierarchy of pixmaps. Additionally, if provided, the mip
     * levels must be sized correctly according to the MIP sizes implied by dimensions. They
     * must all have the same color type and that color type must be compatible with the
     * texture format.
     */
    GrBackendTexture createBackendTexture(SkISize dimensions,
                                          const GrBackendFormat&,
                                          GrRenderable,
                                          GrMipmapped,
                                          GrProtected,
                                          std::string_view label);

    bool clearBackendTexture(const GrBackendTexture&,
                             sk_sp<skgpu::RefCntedCallback> finishedCallback,
                             std::array<float, 4> color);

    /**
     * Same as the createBackendTexture case except compressed backend textures can
     * never be renderable.
     */
    GrBackendTexture createCompressedBackendTexture(SkISize dimensions,
                                                    const GrBackendFormat&,
                                                    GrMipmapped,
                                                    GrProtected);

    bool updateCompressedBackendTexture(const GrBackendTexture&,
                                        sk_sp<skgpu::RefCntedCallback> finishedCallback,
                                        const void* data,
                                        size_t length);

    virtual bool setBackendTextureState(const GrBackendTexture&,
                                        const skgpu::MutableTextureState&,
                                        skgpu::MutableTextureState* previousState,
                                        sk_sp<skgpu::RefCntedCallback> finishedCallback) {
        return false;
    }

    virtual bool setBackendRenderTargetState(const GrBackendRenderTarget&,
                                             const skgpu::MutableTextureState&,
                                             skgpu::MutableTextureState* previousState,
                                             sk_sp<skgpu::RefCntedCallback> finishedCallback) {
        return false;
    }

    /**
     * Frees a texture created by createBackendTexture(). If ownership of the backend
     * texture has been transferred to a context using adopt semantics this should not be called.
     */
    virtual void deleteBackendTexture(const GrBackendTexture&) = 0;

    /**
     * In this case we have a program descriptor and a program info but no render target.
     */
    virtual bool compile(const GrProgramDesc&, const GrProgramInfo&) = 0;

    virtual bool precompileShader(const SkData& key, const SkData& data) { return false; }

#if defined(GR_TEST_UTILS)
    /** Check a handle represents an actual texture in the backend API that has not been freed. */
    virtual bool isTestingOnlyBackendTexture(const GrBackendTexture&) const = 0;

    /**
     * Creates a GrBackendRenderTarget that can be wrapped using
     * SkSurfaces::WrapBackendRenderTarget. Ideally this is a non-textureable allocation to
     * differentiate from testing with SkSurfaces::WrapBackendTexture. When sampleCnt > 1 this
     * is used to test client wrapped allocations with MSAA where Skia does not allocate a separate
     * buffer for resolving. If the color is non-null the backing store should be cleared to the
     * passed in color.
     */
    virtual GrBackendRenderTarget createTestingOnlyBackendRenderTarget(
            SkISize dimensions,
            GrColorType,
            int sampleCount = 1,
            GrProtected = GrProtected::kNo) = 0;

    /**
     * Deletes a GrBackendRenderTarget allocated with the above. Synchronization to make this safe
     * is up to the caller.
     */
    virtual void deleteTestingOnlyBackendRenderTarget(const GrBackendRenderTarget&) = 0;

    // This is only to be used in GL-specific tests.
    virtual const GrGLContext* glContextForTesting() const { return nullptr; }

    // This is only to be used by testing code
    virtual void resetShaderCacheForTesting() const {}

    /**
     * Inserted as a pair around a block of code to do a GPU frame capture.
     * Currently only works with the Metal backend.
     */
    virtual void testingOnly_startCapture() {}
    virtual void testingOnly_stopCapture() {}
#endif

    // width and height may be larger than rt (if underlying API allows it).
    // Returns nullptr if compatible sb could not be created, otherwise the caller owns the ref on
    // the GrAttachment.
    virtual sk_sp<GrAttachment> makeStencilAttachment(const GrBackendFormat& colorFormat,
                                                      SkISize dimensions,
                                                      int numStencilSamples) = 0;

    virtual GrBackendFormat getPreferredStencilFormat(const GrBackendFormat&) = 0;

    // Creates an MSAA surface to be used as an MSAA attachment on a framebuffer.
    virtual sk_sp<GrAttachment> makeMSAAAttachment(SkISize dimensions,
                                                   const GrBackendFormat& format,
                                                   int numSamples,
                                                   GrProtected isProtected,
                                                   GrMemoryless isMemoryless) = 0;

    void handleDirtyContext() {
        if (fResetBits) {
            this->resetContext();
        }
    }

    virtual void storeVkPipelineCacheData() {}

    // Called before certain draws in order to guarantee coherent results from dst reads.
    virtual void xferBarrier(GrRenderTarget*, GrXferBarrierType) = 0;

protected:
    static bool CompressedDataIsCorrect(SkISize dimensions,
                                        SkTextureCompressionType,
                                        GrMipmapped,
                                        const void* data,
                                        size_t length);

    // If the surface is a texture this marks its mipmaps as dirty.
    void didWriteToSurface(GrSurface* surface,
                           GrSurfaceOrigin origin,
                           const SkIRect* bounds,
                           uint32_t mipLevels = 1) const;

    void setOOMed() { fOOMed = true; }

    Stats                            fStats;

    // Subclass must call this to initialize caps & compiler in its constructor.
    void initCapsAndCompiler(sk_sp<const GrCaps> caps);

private:
    virtual GrBackendTexture onCreateBackendTexture(SkISize dimensions,
                                                    const GrBackendFormat&,
                                                    GrRenderable,
                                                    GrMipmapped,
                                                    GrProtected,
                                                    std::string_view label) = 0;

    virtual GrBackendTexture onCreateCompressedBackendTexture(
            SkISize dimensions, const GrBackendFormat&, GrMipmapped, GrProtected) = 0;

    virtual bool onClearBackendTexture(const GrBackendTexture&,
                                       sk_sp<skgpu::RefCntedCallback> finishedCallback,
                                       std::array<float, 4> color) = 0;

    virtual bool onUpdateCompressedBackendTexture(const GrBackendTexture&,
                                                  sk_sp<skgpu::RefCntedCallback> finishedCallback,
                                                  const void* data,
                                                  size_t length) = 0;

    // called when the 3D context state is unknown. Subclass should emit any
    // assumed 3D context state and dirty any state cache.
    virtual void onResetContext(uint32_t resetBits) {}

    // Implementation of resetTextureBindings.
    virtual void onResetTextureBindings() {}

    // overridden by backend-specific derived class to create objects.
    // Texture size, renderablility, format support, sample count will have already been validated
    // in base class before onCreateTexture is called.
    // If the ith bit is set in levelClearMask then the ith MIP level should be cleared.
    virtual sk_sp<GrTexture> onCreateTexture(SkISize dimensions,
                                             const GrBackendFormat&,
                                             GrRenderable,
                                             int renderTargetSampleCnt,
                                             skgpu::Budgeted,
                                             GrProtected,
                                             int mipLevelCoont,
                                             uint32_t levelClearMask,
                                             std::string_view label) = 0;
    virtual sk_sp<GrTexture> onCreateCompressedTexture(SkISize dimensions,
                                                       const GrBackendFormat&,
                                                       skgpu::Budgeted,
                                                       GrMipmapped,
                                                       GrProtected,
                                                       const void* data,
                                                       size_t dataSize) = 0;
    virtual sk_sp<GrTexture> onWrapBackendTexture(const GrBackendTexture&,
                                                  GrWrapOwnership,
                                                  GrWrapCacheable,
                                                  GrIOType) = 0;

    virtual sk_sp<GrTexture> onWrapCompressedBackendTexture(const GrBackendTexture&,
                                                            GrWrapOwnership,
                                                            GrWrapCacheable) = 0;

    virtual sk_sp<GrTexture> onWrapRenderableBackendTexture(const GrBackendTexture&,
                                                            int sampleCnt,
                                                            GrWrapOwnership,
                                                            GrWrapCacheable) = 0;
    virtual sk_sp<GrRenderTarget> onWrapBackendRenderTarget(const GrBackendRenderTarget&) = 0;
    virtual sk_sp<GrRenderTarget> onWrapVulkanSecondaryCBAsRenderTarget(const SkImageInfo&,
                                                                        const GrVkDrawableInfo&);

    virtual sk_sp<GrGpuBuffer> onCreateBuffer(size_t size,
                                              GrGpuBufferType intendedType,
                                              GrAccessPattern) = 0;

    // overridden by backend-specific derived class to perform the surface read
    virtual bool onReadPixels(GrSurface*,
                              SkIRect,
                              GrColorType surfaceColorType,
                              GrColorType dstColorType,
                              void*,
                              size_t rowBytes) = 0;

    // overridden by backend-specific derived class to perform the surface write
    virtual bool onWritePixels(GrSurface*,
                               SkIRect,
                               GrColorType surfaceColorType,
                               GrColorType srcColorType,
                               const GrMipLevel[],
                               int mipLevelCount,
                               bool prepForTexSampling) = 0;

    // overridden by backend-specific derived class to perform the buffer transfer
    virtual bool onTransferFromBufferToBuffer(sk_sp<GrGpuBuffer> src,
                                              size_t srcOffset,
                                              sk_sp<GrGpuBuffer> dst,
                                              size_t dstOffset,
                                              size_t size) = 0;

    // overridden by backend-specific derived class to perform the texture transfer
    virtual bool onTransferPixelsTo(GrTexture*,
                                    SkIRect,
                                    GrColorType textureColorType,
                                    GrColorType bufferColorType,
                                    sk_sp<GrGpuBuffer> transferBuffer,
                                    size_t offset,
                                    size_t rowBytes) = 0;

    // overridden by backend-specific derived class to perform the surface transfer
    virtual bool onTransferPixelsFrom(GrSurface*,
                                      SkIRect,
                                      GrColorType surfaceColorType,
                                      GrColorType bufferColorType,
                                      sk_sp<GrGpuBuffer> transferBuffer,
                                      size_t offset) = 0;

    // overridden by backend-specific derived class to perform the resolve
    virtual void onResolveRenderTarget(GrRenderTarget* target, const SkIRect& resolveRect) = 0;

    // overridden by backend specific derived class to perform mip map level regeneration.
    virtual bool onRegenerateMipMapLevels(GrTexture*) = 0;

    // overridden by backend specific derived class to perform the copy surface
    virtual bool onCopySurface(GrSurface* dst, const SkIRect& dstRect,
                               GrSurface* src, const SkIRect& srcRect,
                               GrSamplerState::Filter) = 0;

    virtual GrOpsRenderPass* onGetOpsRenderPass(
            GrRenderTarget* renderTarget,
            bool useMSAASurface,
            GrAttachment* stencil,
            GrSurfaceOrigin,
            const SkIRect& bounds,
            const GrOpsRenderPass::LoadAndStoreInfo&,
            const GrOpsRenderPass::StencilLoadAndStoreInfo&,
            const skia_private::TArray<GrSurfaceProxy*, true>& sampledProxies,
            GrXferBarrierFlags renderPassXferBarriers) = 0;

    virtual void prepareSurfacesForBackendAccessAndStateUpdates(
            SkSpan<GrSurfaceProxy*> proxies,
            SkSurfaces::BackendSurfaceAccess access,
            const skgpu::MutableTextureState* newState) {}

    virtual bool onSubmitToGpu(bool syncCpu) = 0;

    void reportSubmitHistograms();
    virtual void onReportSubmitHistograms() {}

#ifdef SK_ENABLE_DUMP_GPU
    virtual void onDumpJSON(SkJSONWriter*) const {}
#endif

    sk_sp<GrTexture> createTextureCommon(SkISize,
                                         const GrBackendFormat&,
                                         GrTextureType textureType,
                                         GrRenderable,
                                         int renderTargetSampleCnt,
                                         skgpu::Budgeted,
                                         GrProtected,
                                         int mipLevelCnt,
                                         uint32_t levelClearMask,
                                         std::string_view label);

    void resetContext() {
        this->onResetContext(fResetBits);
        fResetBits = 0;
    }

    void callSubmittedProcs(bool success);

    sk_sp<const GrCaps>             fCaps;
    // Compiler used for compiling SkSL into backend shader code. We only want to create the
    // compiler once, as there is significant overhead to the first compile.
    std::unique_ptr<SkSL::Compiler> fCompiler;

    uint32_t fResetBits;
    // The context owns us, not vice-versa, so this ptr is not ref'ed by Gpu.
    GrDirectContext* fContext;

    struct SubmittedProc {
        SubmittedProc(GrGpuSubmittedProc proc, GrGpuSubmittedContext context)
                : fProc(proc), fContext(context) {}

        GrGpuSubmittedProc fProc;
        GrGpuSubmittedContext fContext;
    };
    skia_private::STArray<4, SubmittedProc> fSubmittedProcs;

    bool fOOMed = false;

#if SK_HISTOGRAMS_ENABLED
    int fCurrentSubmitRenderPassCount = 0;
#endif

    using INHERITED = SkRefCnt;
};

#endif
