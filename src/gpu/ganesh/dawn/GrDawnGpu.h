/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDawnGpu_DEFINED
#define GrDawnGpu_DEFINED

#include "src/gpu/ganesh/GrGpu.h"

#include "src/core/SkLRUCache.h"
#include "src/core/SkTHash.h"
#include "src/gpu/ganesh/GrFinishCallbacks.h"
#include "src/gpu/ganesh/GrProgramDesc.h"
#include "src/gpu/ganesh/GrStagingBufferManager.h"
#include "src/gpu/ganesh/dawn/GrDawnAsyncWait.h"
#include "src/gpu/ganesh/dawn/GrDawnRingBuffer.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "webgpu/webgpu_cpp.h"

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

    sk_sp<GrAttachment> makeStencilAttachment(const GrBackendFormat& /*colorFormat*/,
                                              SkISize dimensions, int numStencilSamples) override;

    GrBackendFormat getPreferredStencilFormat(const GrBackendFormat&) override {
        return GrBackendFormat::MakeDawn(wgpu::TextureFormat::Depth24PlusStencil8);
    }

    sk_sp<GrAttachment> makeMSAAAttachment(SkISize dimensions,
                                           const GrBackendFormat& format,
                                           int numSamples,
                                           GrProtected isProtected,
                                           GrMemoryless isMemoryless) override {
        return nullptr;
    }

    void submit(GrOpsRenderPass*) override;

    GrFence SK_WARN_UNUSED_RESULT insertFence() override;
    bool waitFence(GrFence) override;
    void deleteFence(GrFence) override;

    std::unique_ptr<GrSemaphore> SK_WARN_UNUSED_RESULT makeSemaphore(bool isOwned = true) override;
    std::unique_ptr<GrSemaphore> wrapBackendSemaphore(const GrBackendSemaphore&,
                                                      GrSemaphoreWrapType,
                                                      GrWrapOwnership) override;
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

    std::string SkSLToSPIRV(const char* shaderString,
                            SkSL::ProgramKind,
                            uint32_t rtFlipOffset,
                            SkSL::Program::Inputs*);
    wgpu::ShaderModule createShaderModule(const std::string& spirvSource);

private:
    GrDawnGpu(GrDirectContext*, const GrContextOptions&, const wgpu::Device&);

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
                                               GrMipmapped,
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

    GrBackendTexture onCreateBackendTexture(SkISize dimensions,
                                            const GrBackendFormat&,
                                            GrRenderable,
                                            GrMipmapped,
                                            GrProtected,
                                            std::string_view label) override;

    bool onClearBackendTexture(const GrBackendTexture&,
                               sk_sp<skgpu::RefCntedCallback> finishedCallback,
                               std::array<float, 4> color) override;

    GrBackendTexture onCreateCompressedBackendTexture(SkISize dimensions,
                                                      const GrBackendFormat&,
                                                      GrMipmapped,
                                                      GrProtected) override;

    bool onUpdateCompressedBackendTexture(const GrBackendTexture&,
                                          sk_sp<skgpu::RefCntedCallback> finishedCallback,
                                          const void* data,
                                          size_t size) override;

    sk_sp<GrGpuBuffer> onCreateBuffer(size_t size, GrGpuBufferType type, GrAccessPattern) override;

    bool onReadPixels(GrSurface*,
                      SkIRect,
                      GrColorType surfaceColorType,
                      GrColorType dstColorType,
                      void*,
                      size_t rowBytes) override;

    bool onWritePixels(GrSurface*,
                       SkIRect,
                       GrColorType surfaceColorType,
                       GrColorType srcColorType,
                       const GrMipLevel[],
                       int mipLevelCount,
                       bool) override;

    bool onTransferFromBufferToBuffer(sk_sp<GrGpuBuffer> src,
                                      size_t srcOffset,
                                      sk_sp<GrGpuBuffer> dst,
                                      size_t dstOffset,
                                      size_t size) override;

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

    void onResolveRenderTarget(GrRenderTarget*, const SkIRect&) override {}

    bool onRegenerateMipMapLevels(GrTexture*) override;

    bool onCopySurface(GrSurface* dst, const SkIRect& dstRect,
                       GrSurface* src, const SkIRect& srcRect,
                       GrSamplerState::Filter) override;

    void addFinishedProc(GrGpuFinishedProc finishedProc,
                         GrGpuFinishedContext finishedContext) override;

    GrOpsRenderPass* onGetOpsRenderPass(GrRenderTarget*,
                                        bool useMSAASurface,
                                        GrAttachment*,
                                        GrSurfaceOrigin,
                                        const SkIRect&,
                                        const GrOpsRenderPass::LoadAndStoreInfo&,
                                        const GrOpsRenderPass::StencilLoadAndStoreInfo&,
                                        const SkTArray<GrSurfaceProxy*, true>& sampledProxies,
                                        GrXferBarrierFlags renderPassXferBarriers) override;

    bool onSubmitToGpu(bool syncCpu) override;
    void onSubmittedWorkDone(WGPUQueueWorkDoneStatus status);
    void mapPendingStagingBuffers();

    GrDawnAsyncWait* createFence();
    void destroyFence(GrDawnAsyncWait* fence);

    void uploadTextureData(GrColorType srcColorType, const GrMipLevel texels[], int mipLevelCount,
                           const SkIRect& rect, wgpu::Texture texture);

    wgpu::Device                                    fDevice;
    wgpu::Queue                                     fQueue;
    std::unique_ptr<GrDawnOpsRenderPass>            fOpsRenderPass;
    GrDawnRingBuffer                                fUniformRingBuffer;
    wgpu::CommandEncoder                            fCopyEncoder;
    std::vector<wgpu::CommandBuffer>                fCommandBuffers;
    GrStagingBufferManager                          fStagingBufferManager;

    // Temporary array of staging buffers to hold refs on the staging buffers after detaching
    // from the GrStagingManager. During submission, the buffers are requested to asynchronously map
    // (which Dawn will ensure will happen after the submitted work completes) and this list gets
    // cleared. The buffers are returned to their backing resource provider by dropping their
    // reference once the map request completes asynchronously.
    //
    // NOTE: In general operation the buffers will be mapped to memory when they are made available.
    // However, it is possible for the map operation to fail (e.g. due to a lost connection to the
    // GPU), in which case the buffers will still be made available but in an unmapped state. If a
    // client requests to map such a buffer, GrDawnBuffer will try to map itself again if necessary.
    std::vector<sk_sp<GrGpuBuffer>>                 fSubmittedStagingBuffers;

    // Fence that tracks the completion of all outstanding asynchronous buffer mapping requests.
    // This is necessary to ensure a clean shut down since we need to ensure that buffers are not
    // returned to the resource provider AFTER the provider is destroyed.
    class PendingMapAsyncRequests {
    public:
        explicit PendingMapAsyncRequests(const wgpu::Device& device);
        void addOne();
        void completeOne();
        void waitUntilDone() const;

    private:
        int fCount = 0;
        GrDawnAsyncWait wait_;
    };
    PendingMapAsyncRequests fPendingMapAsyncRequests;

    // Every time command buffers are submitted to the queue (in onSubmitToGpu) we register a single
    // OnSubmittedWorkDone callback which is responsible for signaling all fences added via
    // `insertFence`.
    //
    // NOTE: We use this approach instead of registering an individual callback for each
    // fence because Dawn currently does not support unregistering a callback to prevent a potential
    // use-after-free.
    bool fSubmittedWorkDoneCallbackPending = false;
    SkTHashSet<GrDawnAsyncWait*> fQueueFences;

    struct ProgramDescHash {
        uint32_t operator()(const GrProgramDesc& desc) const {
            return SkOpts::hash_fn(desc.asKey(), desc.keyLength(), 0);
        }
    };

    struct SamplerHash {
        size_t operator()(GrSamplerState samplerState) const {
            // In WebGPU it is required that minFilter, magFilter, and mipmapFilter are all
            // "linear" when maxAnisotropy is > 1.
            return samplerState.asKey(/*anisoIsOrthogonal=*/false);
        }
    };

    SkLRUCache<GrProgramDesc, sk_sp<GrDawnProgram>, ProgramDescHash>    fRenderPipelineCache;
    std::unordered_map<GrSamplerState, wgpu::Sampler, SamplerHash> fSamplers;

    GrFinishCallbacks         fFinishCallbacks;

    using INHERITED = GrGpu;
};

#endif
