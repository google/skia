/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMtlGpu_DEFINED
#define GrMtlGpu_DEFINED

#include "include/gpu/mtl/GrMtlBackendContext.h"
#include "include/private/base/SkDeque.h"
#include "src/gpu/ganesh/mtl/GrMtlTypesPriv.h"

#include "src/gpu/ganesh/GrGpu.h"
#include "src/gpu/ganesh/GrRenderTarget.h"
#include "src/gpu/ganesh/GrRingBuffer.h"
#include "src/gpu/ganesh/GrSemaphore.h"
#include "src/gpu/ganesh/GrStagingBufferManager.h"
#include "src/gpu/ganesh/GrTexture.h"

#include "src/gpu/ganesh/mtl/GrMtlAttachment.h"
#include "src/gpu/ganesh/mtl/GrMtlCaps.h"
#include "src/gpu/ganesh/mtl/GrMtlCommandBuffer.h"
#include "src/gpu/ganesh/mtl/GrMtlResourceProvider.h"
#include "src/gpu/ganesh/mtl/GrMtlUtil.h"

#import <Metal/Metal.h>

class GrMtlOpsRenderPass;
class GrMtlTexture;
class GrSemaphore;
class GrMtlCommandBuffer;

class GrMtlGpu : public GrGpu {
public:
    static std::unique_ptr<GrGpu> Make(const GrMtlBackendContext&,
                                       const GrContextOptions&,
                                       GrDirectContext*);
    ~GrMtlGpu() override;

    void disconnect(DisconnectType) override;

    GrThreadSafePipelineBuilder* pipelineBuilder() override;
    sk_sp<GrThreadSafePipelineBuilder> refPipelineBuilder() override;

    const GrMtlCaps& mtlCaps() const { return *fMtlCaps; }

    id<MTLDevice> device() const { return fDevice; }

    GrMtlResourceProvider& resourceProvider() { return fResourceProvider; }

    GrStagingBufferManager* stagingBufferManager() override { return &fStagingBufferManager; }

    GrMtlCommandBuffer* commandBuffer();

    enum SyncQueue {
        kForce_SyncQueue,
        kSkip_SyncQueue
    };

    void deleteBackendTexture(const GrBackendTexture&) override;

    bool compile(const GrProgramDesc&, const GrProgramInfo&) override;

    bool precompileShader(const SkData& key, const SkData& data) override;

#if defined(GR_TEST_UTILS)
    bool isTestingOnlyBackendTexture(const GrBackendTexture&) const override;

    GrBackendRenderTarget createTestingOnlyBackendRenderTarget(SkISize dimensions,
                                                               GrColorType,
                                                               int sampleCnt,
                                                               GrProtected) override;
    void deleteTestingOnlyBackendRenderTarget(const GrBackendRenderTarget&) override;

    void resetShaderCacheForTesting() const override {
        fResourceProvider.resetShaderCacheForTesting();
    }
#endif

    void copySurfaceAsResolve(GrSurface* dst, GrSurface* src);

    void copySurfaceAsBlit(GrSurface* dst, GrSurface* src,
                           GrMtlAttachment* dstAttachment, GrMtlAttachment* srcAttachment,
                           const SkIRect& srcRect, const SkIPoint& dstPoint);

    bool onCopySurface(GrSurface* dst, const SkIRect& dstRect,
                       GrSurface* src, const SkIRect& srcRect,
                       GrSamplerState::Filter) override;

#if GR_METAL_SDK_VERSION >= 230
    id<MTLBinaryArchive> binaryArchive() const SK_API_AVAILABLE(macos(11.0), ios(14.0), tvos(14.0)) {
        return fBinaryArchive;
    }
#endif

    void submit(GrOpsRenderPass* renderPass) override;

    [[nodiscard]] std::unique_ptr<GrSemaphore> makeSemaphore(bool isOwned) override;
    std::unique_ptr<GrSemaphore> wrapBackendSemaphore(const GrBackendSemaphore&,
                                                      GrSemaphoreWrapType,
                                                      GrWrapOwnership) override;
    void insertSemaphore(GrSemaphore* semaphore) override;
    void waitSemaphore(GrSemaphore* semaphore) override;
    void checkFinishProcs() override { this->checkForFinishedCommandBuffers(); }
    void finishOutstandingGpuWork() override;
    std::unique_ptr<GrSemaphore> prepareTextureForCrossContextUsage(GrTexture*) override;

    GrMtlRenderCommandEncoder* loadMSAAFromResolve(GrAttachment* dst,
                                                   GrMtlAttachment* src,
                                                   const SkIRect& srcRect,
                                                   MTLRenderPassStencilAttachmentDescriptor*);

    // When the Metal backend actually uses indirect command buffers, this function will actually do
    // what it says. For now, every command is encoded directly into the primary command buffer, so
    // this function is pretty useless, except for indicating that a render target has been drawn
    // to.
    void submitIndirectCommandBuffer(GrSurface* surface, GrSurfaceOrigin origin,
                                     const SkIRect* bounds) {
        this->didWriteToSurface(surface, origin, bounds);
    }

    GrRingBuffer* uniformsRingBuffer() override { return &fUniformsRingBuffer; }

private:
    GrMtlGpu(GrDirectContext*, const GrContextOptions&, id<MTLDevice>,
             id<MTLCommandQueue>, GrMTLHandle binaryArchive);

    void destroyResources();

    void xferBarrier(GrRenderTarget*, GrXferBarrierType) override {}

    void takeOwnershipOfBuffer(sk_sp<GrGpuBuffer>) override;

    GrBackendTexture onCreateBackendTexture(SkISize dimensions,
                                            const GrBackendFormat&,
                                            GrRenderable,
                                            skgpu::Mipmapped,
                                            GrProtected,
                                            std::string_view label) override;

    bool onClearBackendTexture(const GrBackendTexture&,
                               sk_sp<skgpu::RefCntedCallback> finishedCallback,
                               std::array<float, 4> color) override;

    GrBackendTexture onCreateCompressedBackendTexture(SkISize dimensions,
                                                      const GrBackendFormat&,
                                                      skgpu::Mipmapped,
                                                      GrProtected) override;

    bool onUpdateCompressedBackendTexture(const GrBackendTexture&,
                                          sk_sp<skgpu::RefCntedCallback> finishedCallback,
                                          const void* data,
                                          size_t size) override;

    sk_sp<GrTexture> onCreateTexture(SkISize,
                                     const GrBackendFormat&,
                                     GrRenderable,
                                     int renderTargetSampleCnt,
                                     skgpu::Budgeted,
                                     GrProtected,
                                     int mipLevelCount,
                                     uint32_t levelClearMask,
                                     std::string_view label) override;
    sk_sp<GrTexture> onCreateCompressedTexture(SkISize dimensions,
                                               const GrBackendFormat&,
                                               skgpu::Budgeted,
                                               skgpu::Mipmapped,
                                               GrProtected,
                                               const void* data,
                                               size_t dataSize) override;

    sk_sp<GrTexture> onWrapBackendTexture(const GrBackendTexture&,
                                          GrWrapOwnership,
                                          GrWrapCacheable,
                                          GrIOType) override;

    sk_sp<GrTexture> onWrapCompressedBackendTexture(const GrBackendTexture&,
                                                    GrWrapOwnership,
                                                    GrWrapCacheable) override;

    sk_sp<GrTexture> onWrapRenderableBackendTexture(const GrBackendTexture&,
                                                    int sampleCnt,
                                                    GrWrapOwnership,
                                                    GrWrapCacheable) override;

    sk_sp<GrRenderTarget> onWrapBackendRenderTarget(const GrBackendRenderTarget&) override;

    sk_sp<GrGpuBuffer> onCreateBuffer(size_t, GrGpuBufferType, GrAccessPattern) override;

    bool onReadPixels(GrSurface* surface,
                      SkIRect,
                      GrColorType surfaceColorType,
                      GrColorType bufferColorType,
                      void*,
                      size_t rowBytes) override;

    bool onTransferFromBufferToBuffer(sk_sp<GrGpuBuffer> src,
                                      size_t srcOffset,
                                      sk_sp<GrGpuBuffer> dst,
                                      size_t dstOffset,
                                      size_t size) override;

    bool onWritePixels(GrSurface*,
                       SkIRect,
                       GrColorType surfaceColorType,
                       GrColorType bufferColorType,
                       const GrMipLevel[],
                       int mipLevelCount,
                       bool prepForTexSampling) override;

    bool onTransferPixelsTo(GrTexture*,
                            SkIRect,
                            GrColorType textureColorType,
                            GrColorType bufferColorType,
                            sk_sp<GrGpuBuffer>,
                            size_t offset,
                            size_t rowBytes) override;

    bool onTransferPixelsFrom(GrSurface*,
                              SkIRect,
                              GrColorType surfaceColorType,
                              GrColorType bufferColorType,
                              sk_sp<GrGpuBuffer>,
                              size_t offset) override;

    bool onRegenerateMipMapLevels(GrTexture*) override;

    void onResolveRenderTarget(GrRenderTarget* target, const SkIRect& resolveRect) override;

    void resolve(GrMtlAttachment* resolveAttachment, GrMtlAttachment* msaaAttachment);

    void addFinishedProc(GrGpuFinishedProc finishedProc,
                         GrGpuFinishedContext finishedContext) override;
    void addFinishedCallback(sk_sp<skgpu::RefCntedCallback> finishedCallback);

    GrOpsRenderPass* onGetOpsRenderPass(
            GrRenderTarget*,
            bool useMSAASurface,
            GrAttachment*,
            GrSurfaceOrigin,
            const SkIRect&,
            const GrOpsRenderPass::LoadAndStoreInfo&,
            const GrOpsRenderPass::StencilLoadAndStoreInfo&,
            const skia_private::TArray<GrSurfaceProxy*, true>& sampledProxies,
            GrXferBarrierFlags renderPassXferBarriers) override;

    bool onSubmitToGpu(GrSyncCpu sync) override;

    // Commits the current command buffer to the queue and then creates a new command buffer. If
    // sync is set to kForce_SyncQueue, the function will wait for all work in the committed
    // command buffer to finish before returning.
    bool submitCommandBuffer(SyncQueue sync);

    void checkForFinishedCommandBuffers();

    // Function that uploads data onto textures with private storage mode (GPU access only).
    bool uploadToTexture(GrMtlTexture* tex,
                         SkIRect rect,
                         GrColorType dataColorType,
                         const GrMipLevel texels[],
                         int mipLevels);

    // Function that fills texture levels with transparent black based on levelMask.
    bool clearTexture(GrMtlTexture*, size_t bbp, uint32_t levelMask);

    bool readOrTransferPixels(GrSurface* surface,
                              SkIRect rect,
                              GrColorType dstColorType,
                              id<MTLBuffer> transferBuffer,
                              size_t offset,
                              size_t imageBytes,
                              size_t rowBytes);

    sk_sp<GrAttachment> makeStencilAttachment(const GrBackendFormat& /*colorFormat*/,
                                              SkISize dimensions, int numStencilSamples) override;

    GrBackendFormat getPreferredStencilFormat(const GrBackendFormat&) override {
        return GrBackendFormat::MakeMtl(this->mtlCaps().preferredStencilFormat());
    }

    sk_sp<GrAttachment> makeMSAAAttachment(SkISize dimensions,
                                           const GrBackendFormat& format,
                                           int numSamples,
                                           GrProtected isProtected,
                                           GrMemoryless isMemoryless) override;

    bool createMtlTextureForBackendSurface(MTLPixelFormat,
                                           SkISize dimensions,
                                           int sampleCnt,
                                           GrTexturable,
                                           GrRenderable,
                                           skgpu::Mipmapped,
                                           GrMtlTextureInfo*);

#if defined(GR_TEST_UTILS)
    void testingOnly_startCapture() override;
    void testingOnly_stopCapture() override;
#endif

#ifdef SK_ENABLE_DUMP_GPU
    void onDumpJSON(SkJSONWriter*) const override;
#endif

    sk_sp<GrMtlCaps> fMtlCaps;

    id<MTLDevice> fDevice;
    id<MTLCommandQueue> fQueue;

    sk_sp<GrMtlCommandBuffer> fCurrentCmdBuffer;

    using OutstandingCommandBuffer = sk_sp<GrMtlCommandBuffer>;
    SkDeque fOutstandingCommandBuffers;

#if GR_METAL_SDK_VERSION >= 230
    id<MTLBinaryArchive> fBinaryArchive SK_API_AVAILABLE(macos(11.0), ios(14.0), tvos(14.0));
#endif

    GrMtlResourceProvider fResourceProvider;
    GrStagingBufferManager fStagingBufferManager;
    GrRingBuffer fUniformsRingBuffer;

    bool fDisconnected;

    using INHERITED = GrGpu;
};

#endif
