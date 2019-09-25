/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDawnGpu_DEFINED
#define GrDawnGpu_DEFINED

#include "src/gpu/GrGpu.h"
#include "dawn/dawncpp.h"
#include "src/core/SkLRUCache.h"
#include "src/gpu/dawn/GrDawnRingBuffer.h"
#include "src/gpu/dawn/GrDawnStagingManager.h"

#include <unordered_map>

class GrDawnOpsRenderPass;
class GrPipeline;
struct GrDawnProgram;

namespace SkSL {
    class Compiler;
}

class GrDawnGpu : public GrGpu {
public:
    static sk_sp<GrGpu> Make(const dawn::Device& device, const GrContextOptions&, GrContext*);
    GrDawnGpu(GrContext* context, const GrContextOptions& options, const dawn::Device& device);

    ~GrDawnGpu() override;

    void disconnect(DisconnectType) override;

    const dawn::Device& device() const { return fDevice; }
    const dawn::Queue&  queue() const { return fQueue; }

    void xferBarrier(GrRenderTarget*, GrXferBarrierType) override {}

    GrBackendTexture createBackendTexture(int w, int h,
                                          const GrBackendFormat &,
                                          GrMipMapped,
                                          GrRenderable,
                                          const void* pixels,
                                          size_t rowBytes,
                                          const SkColor4f* color,
                                          GrProtected isProtected) override;
    void deleteBackendTexture(const GrBackendTexture&) override;
#if GR_TEST_UTILS
    bool isTestingOnlyBackendTexture(const GrBackendTexture&) const override;

    GrBackendRenderTarget createTestingOnlyBackendRenderTarget(int w, int h, GrColorType) override;
    void deleteTestingOnlyBackendRenderTarget(const GrBackendRenderTarget&) override;

    void testingOnly_flushGpuAndSync() override;
#endif
    void flush();

    GrStencilAttachment* createStencilAttachmentForRenderTarget(const GrRenderTarget*,
                                                                int width,
                                                                int height,
                                                                int numStencilSamples) override;

    GrOpsRenderPass* getOpsRenderPass(
            GrRenderTarget*, GrSurfaceOrigin, const SkRect& bounds,
            const GrOpsRenderPass::LoadAndStoreInfo&,
            const GrOpsRenderPass::StencilLoadAndStoreInfo&,
            const SkTArray<GrTextureProxy*, true>& sampledProxies) override;

    SkSL::Compiler* shaderCompiler() const {
        return fCompiler.get();
    }

    void submit(GrOpsRenderPass*) override;

    GrFence SK_WARN_UNUSED_RESULT insertFence() override;
    bool waitFence(GrFence, uint64_t timeout) override;
    void deleteFence(GrFence) const override;

    sk_sp<GrSemaphore> SK_WARN_UNUSED_RESULT makeSemaphore(bool isOwned = true) override;
    sk_sp<GrSemaphore> wrapBackendSemaphore(const GrBackendSemaphore& semaphore,
                                            GrResourceProvider::SemaphoreWrapType wrapType,
                                            GrWrapOwnership ownership) override;
    void insertSemaphore(sk_sp<GrSemaphore> semaphore) override;
    void waitSemaphore(sk_sp<GrSemaphore> semaphore) override;
    void checkFinishProcs() override;

    sk_sp<GrSemaphore> prepareTextureForCrossContextUsage(GrTexture*) override;

    sk_sp<GrDawnProgram> getOrCreateRenderPipeline(GrRenderTarget*,
                                                   GrSurfaceOrigin origin,
                                                   const GrPipeline&,
                                                   const GrPrimitiveProcessor&,
                                                   const GrTextureProxy* const* primProcProxies,
                                                   bool hasPoints,
                                                   GrPrimitiveType primitiveType);

    dawn::Sampler getOrCreateSampler(const GrSamplerState& samplerState);

    GrDawnRingBuffer::Slice allocateUniformRingBufferSlice(int size);
    GrDawnStagingBuffer* getStagingBuffer(size_t size);
    GrDawnStagingManager* getStagingManager() { return &fStagingManager; }
    dawn::CommandEncoder getCopyEncoder();
    void flushCopyEncoder();
    void appendCommandBuffer(dawn::CommandBuffer commandBuffer);

private:
    void onResetContext(uint32_t resetBits) override {}

    virtual void querySampleLocations(GrRenderTarget*, SkTArray<SkPoint>*) override {}

    sk_sp<GrTexture> onCreateTexture(const GrSurfaceDesc& desc,
                                     const GrBackendFormat&,
                                     GrRenderable,
                                     int renderTargetSampleCnt,
                                     SkBudgeted,
                                     GrProtected,
                                     int mipLevelCount,
                                     uint32_t levelClearMask) override;

    sk_sp<GrTexture> onCreateCompressedTexture(int width, int height, const GrBackendFormat&,
                                               SkImage::CompressionType, SkBudgeted,
                                               const void* data) override;

    sk_sp<GrTexture> onWrapBackendTexture(const GrBackendTexture&, GrColorType, GrWrapOwnership,
                                          GrWrapCacheable, GrIOType) override;
    sk_sp<GrTexture> onWrapRenderableBackendTexture(const GrBackendTexture&, int sampleCnt,
                                                    GrColorType, GrWrapOwnership,
                                                    GrWrapCacheable) override;
    sk_sp<GrRenderTarget> onWrapBackendRenderTarget(const GrBackendRenderTarget&,
                                                    GrColorType) override;

    sk_sp<GrRenderTarget> onWrapBackendTextureAsRenderTarget(const GrBackendTexture&, int sampleCnt,
                                                             GrColorType) override;

    sk_sp<GrGpuBuffer> onCreateBuffer(size_t size, GrGpuBufferType type, GrAccessPattern,
                                      const void* data) override;

    bool onReadPixels(GrSurface* surface, int left, int top, int width, int height,
                      GrColorType surfaceColorType, GrColorType dstColorType, void* buffer,
                      size_t rowBytes) override;

    bool onWritePixels(GrSurface* surface, int left, int top, int width, int height,
                       GrColorType surfaceColorType, GrColorType srcColorType,
                       const GrMipLevel texels[], int mipLevelCount,
                       bool prepForTexSampling) override;

    bool onTransferPixelsTo(GrTexture*, int left, int top, int width, int height,
                            GrColorType textureColorType, GrColorType bufferColorType,
                            GrGpuBuffer* transferBuffer, size_t offset, size_t rowBytes) override;

    bool onTransferPixelsFrom(GrSurface* surface, int left, int top, int width, int height,
                              GrColorType surfaceColorType, GrColorType bufferColorType,
                              GrGpuBuffer* transferBuffer, size_t offset) override;

    void onResolveRenderTarget(GrRenderTarget* target, ForExternalIO) override {
    }

    bool onRegenerateMipMapLevels(GrTexture*) override;

    bool onCopySurface(GrSurface* dst, GrSurface* src,
                       const SkIRect& srcRect, const SkIPoint& dstPoint) override;

    void onFinishFlush(GrSurfaceProxy*[], int n, SkSurface::BackendSurfaceAccess access,
                       const GrFlushInfo& info, const GrPrepareForExternalIORequests&) override;

    dawn::Device                                    fDevice;
    dawn::Queue                                     fQueue;
    std::unique_ptr<SkSL::Compiler>                 fCompiler;
    std::unique_ptr<GrDawnOpsRenderPass>            fOpsRenderPass;
    GrDawnRingBuffer                                fUniformRingBuffer;
    dawn::CommandEncoder                            fCopyEncoder;
    std::vector<dawn::CommandBuffer>                fCommandBuffers;

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

    SkLRUCache<GrProgramDesc, sk_sp<GrDawnProgram>, ProgramDescHash>    fRenderPipelineCache;
    std::unordered_map<GrSamplerState, dawn::Sampler, SamplerHash> fSamplers;
    GrDawnStagingManager fStagingManager;

    typedef GrGpu INHERITED;
};

#endif
