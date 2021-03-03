/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDawnGpu_DEFINED
#define GrDawnGpu_DEFINED

#include "src/gpu/GrGpu.h"

#include "dawn/webgpu_cpp.h"
#include "src/core/SkLRUCache.h"
#include "src/gpu/GrFinishCallbacks.h"
#include "src/gpu/GrProgramDesc.h"
#include "src/gpu/GrStagingBufferManager.h"
#include "src/gpu/dawn/GrDawnRingBuffer.h"
#include "src/sksl/ir/SkSLProgram.h"

#include <unordered_map>

class GrDawnOpsRenderPass;
class GrDawnStagingBuffer;
class GrDirectContext;
class GrPipeline;
struct GrDawnProgram;

class GrDawnGpu : public GrGpu {
public:
    static sk_sp<GrGpu> Make(const wgpu::Device&, const GrContextOptions&, GrDirectContext*);

    ~GrDawnGpu() override;

    void disconnect(DisconnectType) override;

    GrThreadSafePipelineBuilder* pipelineBuilder() override;
    sk_sp<GrThreadSafePipelineBuilder> refPipelineBuilder() override;

    GrStagingBufferManager* stagingBufferManager() override { return &fStagingBufferManager; }
    void takeOwnershipOfBuffer(sk_sp<GrGpuBuffer>) override;

    const wgpu::Device& device() const { return fDevice; }
    const wgpu::Queue&  queue() const { return fQueue; }

    void xferBarrier(GrRenderTarget*, GrXferBarrierType) override {}

    void deleteBackendTexture(const GrBackendTexture&) override;

    bool compile(const GrProgramDesc&, const GrProgramInfo&) override;

#if GR_TEST_UTILS
    bool isTestingOnlyBackendTexture(const GrBackendTexture&) const override;

    GrBackendRenderTarget createTestingOnlyBackendRenderTarget(SkISize dimensions,
                                                               GrColorType,
                                                               int sampleCnt,
                                                               GrProtected) override;
    void deleteTestingOnlyBackendRenderTarget(const GrBackendRenderTarget&) override;
#endif

    sk_sp<GrAttachment> makeStencilAttachmentForRenderTarget(const GrRenderTarget*,
                                                             SkISize dimensions,
                                                             int numStencilSamples) override;

    GrBackendFormat getPreferredStencilFormat(const GrBackendFormat&) override {
        return GrBackendFormat::MakeDawn(wgpu::TextureFormat::Depth24PlusStencil8);
    }

    sk_sp<GrAttachment> makeMSAAAttachment(SkISize dimensions,
                                           const GrBackendFormat& format,
                                           int numSamples,
                                           GrProtected isProtected) override {
        return nullptr;
    }

    void submit(GrOpsRenderPass*) override;

    GrFence SK_WARN_UNUSED_RESULT insertFence() override;
    bool waitFence(GrFence) override;
    void deleteFence(GrFence) const override;

    std::unique_ptr<GrSemaphore> SK_WARN_UNUSED_RESULT makeSemaphore(bool isOwned = true) override;
    std::unique_ptr<GrSemaphore> wrapBackendSemaphore(
            const GrBackendSemaphore& semaphore,
            GrResourceProvider::SemaphoreWrapType wrapType,
            GrWrapOwnership ownership) override;
    void insertSemaphore(GrSemaphore* semaphore) override;
    void waitSemaphore(GrSemaphore* semaphore) override;
    void checkFinishProcs() override;
    void finishOutstandingGpuWork() override;

    std::unique_ptr<GrSemaphore> prepareTextureForCrossContextUsage(GrTexture*) override;

    sk_sp<GrDawnProgram> getOrCreateRenderPipeline(GrRenderTarget*, const GrProgramInfo&);

    wgpu::Sampler getOrCreateSampler(GrSamplerState samplerState);

    GrDawnRingBuffer::Slice allocateUniformRingBufferSlice(int size);
    wgpu::CommandEncoder getCopyEncoder();
    void flushCopyEncoder();
    void appendCommandBuffer(wgpu::CommandBuffer commandBuffer);

    void waitOnAllBusyStagingBuffers();
    SkSL::String SkSLToSPIRV(const char* shaderString, SkSL::ProgramKind, bool flipY,
                             uint32_t rtHeightOffset, SkSL::Program::Inputs*);
    wgpu::ShaderModule createShaderModule(const SkSL::String& spirvSource);

private:
    GrDawnGpu(GrDirectContext*, const GrContextOptions&, const wgpu::Device&);

    virtual void querySampleLocations(GrRenderTarget*, SkTArray<SkPoint>*) override {}

    sk_sp<GrTexture> onCreateTexture(SkISize,
                                     const GrBackendFormat&,
                                     GrRenderable,
                                     int renderTargetSampleCnt,
                                     SkBudgeted,
                                     GrProtected,
                                     int mipLevelCount,
                                     uint32_t levelClearMask) override;

    sk_sp<GrTexture> onCreateCompressedTexture(SkISize dimensions,
                                               const GrBackendFormat&,
                                               SkBudgeted,
                                               GrMipmapped,
                                               GrProtected,
                                               const void* data, size_t dataSize) override;

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

    GrBackendTexture onCreateBackendTexture(SkISize dimensions,
                                            const GrBackendFormat&,
                                            GrRenderable,
                                            GrMipmapped,
                                            GrProtected) override;

    bool onUpdateBackendTexture(const GrBackendTexture&,
                                sk_sp<GrRefCntedCallback> finishedCallback,
                                const BackendTextureData*) override;

    GrBackendTexture onCreateCompressedBackendTexture(SkISize dimensions,
                                                      const GrBackendFormat&,
                                                      GrMipmapped,
                                                      GrProtected) override;

    bool onUpdateCompressedBackendTexture(const GrBackendTexture&,
                                          sk_sp<GrRefCntedCallback> finishedCallback,
                                          const BackendTextureData*) override;

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
                            sk_sp<GrGpuBuffer> transferBuffer, size_t offset,
                            size_t rowBytes) override;

    bool onTransferPixelsFrom(GrSurface* surface, int left, int top, int width, int height,
                              GrColorType surfaceColorType, GrColorType bufferColorType,
                              sk_sp<GrGpuBuffer> transferBuffer, size_t offset) override;

    void onResolveRenderTarget(GrRenderTarget*, const SkIRect&) override {}

    bool onRegenerateMipMapLevels(GrTexture*) override;

    bool onCopySurface(GrSurface* dst, GrSurface* src,
                       const SkIRect& srcRect, const SkIPoint& dstPoint) override;

    void addFinishedProc(GrGpuFinishedProc finishedProc,
                         GrGpuFinishedContext finishedContext) override;

    GrOpsRenderPass* onGetOpsRenderPass(GrRenderTarget*,
                                        GrAttachment*,
                                        GrSurfaceOrigin,
                                        const SkIRect&,
                                        const GrOpsRenderPass::LoadAndStoreInfo&,
                                        const GrOpsRenderPass::StencilLoadAndStoreInfo&,
                                        const SkTArray<GrSurfaceProxy*, true>& sampledProxies,
                                        GrXferBarrierFlags renderPassXferBarriers) override;

    bool onSubmitToGpu(bool syncCpu) override;

    void uploadTextureData(GrColorType srcColorType, const GrMipLevel texels[], int mipLevelCount,
                           const SkIRect& rect, wgpu::Texture texture);

    void moveStagingBuffersToBusyAndMapAsync();
    void checkForCompletedStagingBuffers();

    wgpu::Device                                    fDevice;
    wgpu::Queue                                     fQueue;
    std::unique_ptr<GrDawnOpsRenderPass>            fOpsRenderPass;
    GrDawnRingBuffer                                fUniformRingBuffer;
    wgpu::CommandEncoder                            fCopyEncoder;
    std::vector<wgpu::CommandBuffer>                fCommandBuffers;
    GrStagingBufferManager                          fStagingBufferManager;
    std::list<sk_sp<GrGpuBuffer>>                   fBusyStagingBuffers;
    // Temporary array of staging buffers to hold refs on the staging buffers between detaching
    // from the GrStagingManager and moving them to the busy list which must happen after
    // submission.
    std::vector<sk_sp<GrGpuBuffer>>                 fSubmittedStagingBuffers;

    struct ProgramDescHash {
        uint32_t operator()(const GrProgramDesc& desc) const {
            return SkOpts::hash_fn(desc.asKey(), desc.keyLength(), 0);
        }
    };

    struct SamplerHash {
        size_t operator()(GrSamplerState samplerState) const {
            return SkOpts::hash_fn(&samplerState, sizeof(samplerState), 0);
        }
    };

    SkLRUCache<GrProgramDesc, sk_sp<GrDawnProgram>, ProgramDescHash>    fRenderPipelineCache;
    std::unordered_map<GrSamplerState, wgpu::Sampler, SamplerHash> fSamplers;

    GrFinishCallbacks         fFinishCallbacks;

    using INHERITED = GrGpu;
};

#endif
