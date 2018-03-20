/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrNXTGpu_DEFINED
#define GrNXTGpu_DEFINED

#include "GrGpu.h"
#include "GrGpuFactory.h"
#include "nxt/nxtcpp.h"
#include "nxt/GrNXTBackendContext.h"
#include "nxt/GrNXTRingBuffer.h"
#include "SkLRUCache.h"

class GrPipeline;
struct GrNXTProgram;

namespace SkSL {
    class Compiler;
}

class GrNXTGpu : public GrGpu {
public:
    static sk_sp<GrGpu> Make(GrBackendContext backendContext, const GrContextOptions&, GrContext*);
    static sk_sp<GrGpu> Make(sk_sp<const GrNXTBackendContext>, const GrContextOptions&, GrContext*);
    GrNXTGpu(GrContext* context, const GrContextOptions& options, sk_sp<const GrNXTBackendContext>);

    ~GrNXTGpu() override;

    void disconnect(DisconnectType) override;

    nxt::Device device() const { return fDevice.Clone(); }
    nxt::Queue  queue() const { return fQueue.Clone(); }

    bool onGetReadPixelsInfo(GrSurface*, GrSurfaceOrigin, int width, int height,
                             size_t rowBytes, GrColorType, DrawPreference*,
                             ReadPixelTempDrawInfo*) override;

    bool onGetWritePixelsInfo(GrSurface*, GrSurfaceOrigin, int width, int height,
                              GrColorType, DrawPreference*, WritePixelTempDrawInfo*) override;

    bool onCopySurface(GrSurface* dst, GrSurfaceOrigin dstOrigin,
                       GrSurface* src, GrSurfaceOrigin srcOrigin,
                       const SkIRect& srcRect, const SkIPoint& dstPoint) override;

    void xferBarrier(GrRenderTarget*, GrXferBarrierType) override {}

    GrBackendTexture createTestingOnlyBackendTexture(void* pixels, int w, int h,
                                                     GrPixelConfig config,
                                                     bool isRenderTarget,
                                                     GrMipMapped) override;
    bool isTestingOnlyBackendTexture(const GrBackendTexture&) const override;
    void deleteTestingOnlyBackendTexture(GrBackendTexture*, bool abandonTexture = false) override;

    GrStencilAttachment* createStencilAttachmentForRenderTarget(const GrRenderTarget*,
                                                                int width,
                                                                int height) override;

    void clearStencil(GrRenderTarget* target, int clearValue) override;

    GrGpuRTCommandBuffer* createCommandBuffer(
            GrRenderTarget*, GrSurfaceOrigin,
            const GrGpuRTCommandBuffer::LoadAndStoreInfo&,
            const GrGpuRTCommandBuffer::StencilLoadAndStoreInfo&) override;

    GrGpuTextureCommandBuffer* createCommandBuffer(GrTexture*, GrSurfaceOrigin) override;

    SkSL::Compiler* shaderCompiler() const {
        return fCompiler;
    }

    void onResolveRenderTarget(GrRenderTarget* target) override {
    }

    GrFence SK_WARN_UNUSED_RESULT insertFence() override;
    bool waitFence(GrFence, uint64_t timeout) override;
    void deleteFence(GrFence) const override;

    sk_sp<GrSemaphore> SK_WARN_UNUSED_RESULT makeSemaphore(bool isOwned = true) override;
    sk_sp<GrSemaphore> wrapBackendSemaphore(const GrBackendSemaphore& semaphore,
                                            GrResourceProvider::SemaphoreWrapType wrapType,
                                            GrWrapOwnership ownership) override;
    void insertSemaphore(sk_sp<GrSemaphore> semaphore, bool flush = false) override;
    void waitSemaphore(sk_sp<GrSemaphore> semaphore) override;

    sk_sp<GrSemaphore> prepareTextureForCrossContextUsage(GrTexture*) override;

    nxt::RenderPass getOrCreateRenderPass(GrRenderTarget*,
                                          GrLoadOp colorLoadOp,
                                          GrLoadOp stencilLoadOp,
                                          uint32_t* hashOut);

    sk_sp<GrNXTProgram> getOrCreateRenderPipeline(nxt::RenderPass renderPass,
                                                  uint32_t renderPassHash,
                                                  const GrPipeline&,
                                                  const GrPrimitiveProcessor&,
                                                  bool hasPoints,
                                                  GrPrimitiveType primitiveType);

    GrNXTRingBuffer::Slice allocateUniformRingBufferSlice(int size);

private:
    void onResetContext(uint32_t resetBits) override {}

    sk_sp<GrTexture> onCreateTexture(const GrSurfaceDesc& desc, SkBudgeted budgeted,
                                     GrSurfaceOrigin texelsOrigin, const GrMipLevel texels[],
                                     int mipLevelCount) override;

    sk_sp<GrTexture> onWrapBackendTexture(const GrBackendTexture&, GrWrapOwnership) override;
    sk_sp<GrTexture> onWrapRenderableBackendTexture(const GrBackendTexture&,
                                                    int sampleCnt,
                                                    GrWrapOwnership) override;
    sk_sp<GrRenderTarget> onWrapBackendRenderTarget(const GrBackendRenderTarget&) override;

    sk_sp<GrRenderTarget> onWrapBackendTextureAsRenderTarget(const GrBackendTexture&,
                                                             int sampleCnt) override;

    GrBuffer* onCreateBuffer(size_t size, GrBufferType type, GrAccessPattern,
                             const void* data) override;

    bool onReadPixels(GrSurface* surface, GrSurfaceOrigin,
                      int left, int top, int width, int height,
                      GrColorType, void* buffer, size_t rowBytes) override;

    bool onWritePixels(GrSurface* surface, GrSurfaceOrigin,
                       int left, int top, int width, int height,
                       GrColorType, const GrMipLevel texels[], int mipLevelCount) override;

    bool onTransferPixels(GrTexture*, int left, int top, int width, int height,
                          GrColorType colorType, GrBuffer* transferBuffer,
                          size_t offset, size_t rowBytes) override;

    void onFinishFlush(bool insertedSemaphores) override;

    nxt::Device                                  fDevice;
    nxt::Queue                                   fQueue;

    SkSL::Compiler* fCompiler;

    SkLRUCache<uint32_t, nxt::RenderPass>        fRenderPassCache;
    SkLRUCache<uint32_t, sk_sp<GrNXTProgram>>    fRenderPipelineCache;
    GrNXTRingBuffer                              fUniformRingBuffer;
    nxt::CommandBufferBuilder                    fCopyBuilder;

    typedef GrGpu INHERITED;
};

#endif
