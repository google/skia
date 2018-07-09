/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrNXTGpu_DEFINED
#define GrNXTGpu_DEFINED

#include "GrGpu.h"
#include "nxt/nxtcpp.h"
#include "nxt/GrNXTBackendContext.h"
#include "nxt/GrNXTRingBuffer.h"
#include "nxt/GrNXTStagingManager.h"
#include "SkLRUCache.h"

#include <unordered_map>

class GrPipeline;
struct GrNXTProgram;

namespace SkSL {
    class Compiler;
}

class GrNXTGpu : public GrGpu {
public:
    static sk_sp<GrGpu> Make(sk_sp<const GrNXTBackendContext>, const GrContextOptions&, GrContext*);
    GrNXTGpu(GrContext* context, const GrContextOptions& options, sk_sp<const GrNXTBackendContext>);

    ~GrNXTGpu() override;

    void disconnect(DisconnectType) override;

    nxt::Device device() const { return fDevice.Clone(); }
    nxt::Queue  queue() const { return fQueue.Clone(); }

    bool onCopySurface(GrSurface* dst, GrSurfaceOrigin dstOrigin,
                       GrSurface* src, GrSurfaceOrigin srcOrigin,
                       const SkIRect& srcRect, const SkIPoint& dstPoint,
                       bool canDiscardOutsideDstRect) override;

    void xferBarrier(GrRenderTarget*, GrXferBarrierType) override {}

#if GR_TEST_UTILS
    GrBackendTexture createTestingOnlyBackendTexture(const void* pixels, int w, int h,
                                                     GrPixelConfig config,
                                                     bool isRenderTarget,
                                                     GrMipMapped) override;
    bool isTestingOnlyBackendTexture(const GrBackendTexture&) const override;
    void deleteTestingOnlyBackendTexture(const GrBackendTexture&) override;

    GrBackendRenderTarget createTestingOnlyBackendRenderTarget(int w, int h, GrColorType,
                                                               GrSRGBEncoded) override;
    void deleteTestingOnlyBackendRenderTarget(const GrBackendRenderTarget&) override;

    void testingOnly_flushGpuAndSync() override;
#endif

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

    bool onRegenerateMipMapLevels(GrTexture*) override;

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

    sk_sp<GrNXTProgram> getOrCreateRenderPipeline(GrRenderTarget*,
                                                  const GrPipeline&,
                                                  const GrPrimitiveProcessor&,
                                                  bool hasPoints,
                                                  GrPrimitiveType primitiveType);

    nxt::Sampler getOrCreateSampler(const GrSamplerState& samplerState);

    GrNXTRingBuffer::Slice allocateUniformRingBufferSlice(int size);
    GrNXTStagingBuffer* getStagingBuffer(size_t size);
    void setReadPixelsPtr(const void* ptr) { fReadPixelsPtr = ptr; }
    GrNXTStagingManager* getStagingManager() { return &fStagingManager; }
    nxt::CommandBufferBuilder getCopyBuilder() { return fCopyBuilder.Clone(); }
    void appendCommandBuffer(nxt::CommandBuffer commandBuffer);

private:
    void onResetContext(uint32_t resetBits) override {}

    sk_sp<GrTexture> onCreateTexture(const GrSurfaceDesc& desc, SkBudgeted budgeted,
                                     const GrMipLevel texels[], int mipLevelCount) override;

    sk_sp<GrTexture> onWrapBackendTexture(const GrBackendTexture&, GrWrapOwnership) override;
    sk_sp<GrTexture> onWrapRenderableBackendTexture(const GrBackendTexture&,
                                                    int sampleCnt,
                                                    GrWrapOwnership) override;
    sk_sp<GrRenderTarget> onWrapBackendRenderTarget(const GrBackendRenderTarget&) override;

    sk_sp<GrRenderTarget> onWrapBackendTextureAsRenderTarget(const GrBackendTexture&,
                                                             int sampleCnt) override;

    GrBuffer* onCreateBuffer(size_t size, GrBufferType type, GrAccessPattern,
                             const void* data) override;

    bool onReadPixels(GrSurface* surface,
                      int left, int top, int width, int height,
                      GrColorType, void* buffer, size_t rowBytes) override;

    bool onWritePixels(GrSurface* surface,
                       int left, int top, int width, int height,
                       GrColorType, const GrMipLevel texels[], int mipLevelCount) override;

    bool onTransferPixels(GrTexture*, int left, int top, int width, int height,
                          GrColorType colorType, GrBuffer* transferBuffer,
                          size_t offset, size_t rowBytes) override;

    void onFinishFlush(bool insertedSemaphores) override;

    nxt::Device                                  fDevice;
    nxt::Queue                                   fQueue;

    SkSL::Compiler* fCompiler;

    GrNXTRingBuffer                              fUniformRingBuffer;
    nxt::CommandBufferBuilder                    fCopyBuilder;
    const void*                                  fReadPixelsPtr = nullptr;

    struct ProgramDescHash {
        uint32_t operator()(const GrProgramDesc& desc) const {
            return SkOpts::hash_fn(desc.asKey(), desc.keyLength(), 0);
        }
    };

    struct SamplerHash {
        size_t operator()(const GrSamplerState& samplerState) const {
            return SkOpts::hash_fn(&samplerState, sizeof(samplerState), 0);
        }
    };

    SkLRUCache<GrProgramDesc, sk_sp<GrNXTProgram>, ProgramDescHash>    fRenderPipelineCache;
    std::unordered_map<GrSamplerState, nxt::Sampler, SamplerHash> fSamplers;
    GrNXTStagingManager fStagingManager;
    std::vector<nxt::CommandBuffer> fCommandBuffers;

    typedef GrGpu INHERITED;
};

#endif
