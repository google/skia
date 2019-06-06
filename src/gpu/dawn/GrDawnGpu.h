/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDawnGpu_DEFINED
#define GrDawnGpu_DEFINED

#include "src/gpu/GrGpu.h"
#include "dawn/dawncpp.h"
#include "src/gpu/dawn/GrDawnRingBuffer.h"
#include "src/gpu/dawn/GrDawnStagingManager.h"
#include "src/core/SkLRUCache.h"

#include <unordered_map>

class GrPipeline;
struct GrDawnProgram;
class GrDawnGpuRTCommandBuffer;

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

    bool onCopySurface(GrSurface* dst, GrSurfaceOrigin dstOrigin,
                       GrSurface* src, GrSurfaceOrigin srcOrigin,
                       const SkIRect& srcRect, const SkIPoint& dstPoint,
                       bool canDiscardOutsideDstRect) override;

    void xferBarrier(GrRenderTarget*, GrXferBarrierType) override {}

    GrBackendTexture createBackendTexture(int w, int h,
                                          const GrBackendFormat &,
                                          GrMipMapped,
                                          GrRenderable,
                                          const void* pixels,
                                          size_t rowBytes,
                                          const SkColor4f& color) override;
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
                                                                int height) override;

    GrGpuRTCommandBuffer* getCommandBuffer(
            GrRenderTarget*, GrSurfaceOrigin, const SkRect& bounds,
            const GrGpuRTCommandBuffer::LoadAndStoreInfo&,
            const GrGpuRTCommandBuffer::StencilLoadAndStoreInfo&) override;

    GrGpuTextureCommandBuffer* getCommandBuffer(GrTexture*, GrSurfaceOrigin) override;

    SkSL::Compiler* shaderCompiler() const {
        return fCompiler;
    }

    void onResolveRenderTarget(GrRenderTarget* target) override {
    }

    bool onRegenerateMipMapLevels(GrTexture*) override;

    void submit(GrGpuCommandBuffer* cb) override;
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
    void setReadPixelsPtr(const void* ptr) { fReadPixelsPtr = ptr; }
    GrDawnStagingManager* getStagingManager() { return &fStagingManager; }
    dawn::CommandEncoder getCopyEncoder() { return fCopyEncoder; }
    void appendCommandBuffer(dawn::CommandBuffer commandBuffer);

private:
    void onResetContext(uint32_t resetBits) override {}

    virtual void querySampleLocations(GrRenderTarget*, SkTArray<SkPoint>*) override {}

    sk_sp<GrTexture> onCreateTexture(const GrSurfaceDesc& desc, SkBudgeted budgeted,
                                     const GrMipLevel texels[], int mipLevelCount) override;

    sk_sp<GrTexture> onWrapBackendTexture(const GrBackendTexture&, GrWrapOwnership,
                                          GrWrapCacheable, GrIOType) override;
    sk_sp<GrTexture> onWrapRenderableBackendTexture(const GrBackendTexture&,
                                                    int sampleCnt,
                                                    GrWrapOwnership, GrWrapCacheable) override;
    sk_sp<GrRenderTarget> onWrapBackendRenderTarget(const GrBackendRenderTarget&) override;

    sk_sp<GrRenderTarget> onWrapBackendTextureAsRenderTarget(const GrBackendTexture&,
                                                             int sampleCnt) override;

    sk_sp<GrGpuBuffer> onCreateBuffer(size_t size, GrGpuBufferType type, GrAccessPattern,
                                      const void* data) override;

    bool onReadPixels(GrSurface* surface,
                      int left, int top, int width, int height,
                      GrColorType, void* buffer, size_t rowBytes) override;

    bool onWritePixels(GrSurface* surface,
                       int left, int top, int width, int height,
                       GrColorType, const GrMipLevel texels[], int mipLevelCount) override;

    bool onTransferPixelsTo(GrTexture*, int left, int top, int width, int height,
                            GrColorType colorType, GrGpuBuffer* transferBuffer,
                            size_t offset, size_t rowBytes) override;

    bool onTransferPixelsFrom(GrSurface* surface, int left, int top, int width, int height,
                              GrColorType, GrGpuBuffer* transferBuffer, size_t offset) override;

    void onFinishFlush(GrSurfaceProxy*[], int n, SkSurface::BackendSurfaceAccess access,
                       const GrFlushInfo& info, const GrPrepareForExternalIORequests&) override;

    dawn::Device                                 fDevice;
    dawn::Queue                                  fQueue;

    SkSL::Compiler* fCompiler;

    std::unique_ptr<GrDawnGpuRTCommandBuffer>    fCachedRTCommandBuffer;

    GrDawnRingBuffer                             fUniformRingBuffer;
    dawn::CommandEncoder                         fCopyEncoder;
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

    SkLRUCache<GrProgramDesc, sk_sp<GrDawnProgram>, ProgramDescHash>    fRenderPipelineCache;
    std::unordered_map<GrSamplerState, dawn::Sampler, SamplerHash> fSamplers;
    GrDawnStagingManager fStagingManager;
    std::vector<dawn::CommandBuffer> fCommandBuffers;

    typedef GrGpu INHERITED;
};

#endif
