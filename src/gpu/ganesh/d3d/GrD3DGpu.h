/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrD3DGpu_DEFINED
#define GrD3DGpu_DEFINED

#include "include/private/base/SkDeque.h"
#include "src/gpu/ganesh/GrGpu.h"
#include "src/gpu/ganesh/GrRenderTarget.h"
#include "src/gpu/ganesh/GrSemaphore.h"
#include "src/gpu/ganesh/GrStagingBufferManager.h"
#include "src/gpu/ganesh/d3d/GrD3DCaps.h"
#include "src/gpu/ganesh/d3d/GrD3DCommandList.h"
#include "src/gpu/ganesh/d3d/GrD3DResourceProvider.h"

struct GrD3DBackendContext;
class GrD3DOpsRenderPass;
struct GrD3DOptions;
class GrPipeline;
#if defined(GR_TEST_UTILS)
struct IDXGraphicsAnalysis;
#endif

class GrD3DGpu : public GrGpu {
public:
    static std::unique_ptr<GrGpu> Make(const GrD3DBackendContext& backendContext,
                                       const GrContextOptions&,
                                       GrDirectContext*);

    ~GrD3DGpu() override;

    const GrD3DCaps& d3dCaps() const { return static_cast<const GrD3DCaps&>(*this->caps()); }

    GrD3DResourceProvider& resourceProvider() { return fResourceProvider; }

    GrThreadSafePipelineBuilder* pipelineBuilder() override;
    sk_sp<GrThreadSafePipelineBuilder> refPipelineBuilder() override;

    ID3D12Device* device() const { return fDevice.get(); }
    ID3D12CommandQueue* queue() const { return fQueue.get(); }

    GrD3DMemoryAllocator* memoryAllocator() const { return fMemoryAllocator.get(); }

    GrD3DDirectCommandList* currentCommandList() const { return fCurrentDirectCommandList.get(); }

    GrStagingBufferManager* stagingBufferManager() override { return &fStagingBufferManager; }
    void takeOwnershipOfBuffer(sk_sp<GrGpuBuffer>) override;

    GrRingBuffer* uniformsRingBuffer() override { return &fConstantsRingBuffer; }

    bool protectedContext() const { return false; }

    void xferBarrier(GrRenderTarget*, GrXferBarrierType) override {}

    void deleteBackendTexture(const GrBackendTexture&) override;

    bool compile(const GrProgramDesc&, const GrProgramInfo&) override;

#if defined(GR_TEST_UTILS)
    bool isTestingOnlyBackendTexture(const GrBackendTexture&) const override;

    GrBackendRenderTarget createTestingOnlyBackendRenderTarget(SkISize dimensions,
                                                               GrColorType,
                                                               int sampleCnt,
                                                               GrProtected) override;
    void deleteTestingOnlyBackendRenderTarget(const GrBackendRenderTarget&) override;

    void testingOnly_startCapture() override;
    void testingOnly_stopCapture() override;

    void resetShaderCacheForTesting() const override {
        fResourceProvider.resetShaderCacheForTesting();
    }
#endif

    sk_sp<GrAttachment> makeStencilAttachment(const GrBackendFormat& /*colorFormat*/,
                                              SkISize dimensions, int numStencilSamples) override;

    GrBackendFormat getPreferredStencilFormat(const GrBackendFormat&) override {
        return GrBackendFormat::MakeDxgi(this->d3dCaps().preferredStencilFormat());
    }

    sk_sp<GrAttachment> makeMSAAAttachment(SkISize dimensions,
                                           const GrBackendFormat& format,
                                           int numSamples,
                                           GrProtected isProtected,
                                           GrMemoryless isMemoryless) override {
        return nullptr;
    }

    void addResourceBarriers(sk_sp<GrManagedResource> resource,
                             int numBarriers,
                             D3D12_RESOURCE_TRANSITION_BARRIER* barriers) const;

    void addBufferResourceBarriers(GrD3DBuffer* buffer,
                                   int numBarriers,
                                   D3D12_RESOURCE_TRANSITION_BARRIER* barriers) const;

    [[nodiscard]] std::unique_ptr<GrSemaphore> makeSemaphore(bool isOwned) override;
    std::unique_ptr<GrSemaphore> wrapBackendSemaphore(const GrBackendSemaphore&,
                                                      GrSemaphoreWrapType,
                                                      GrWrapOwnership) override;
    void insertSemaphore(GrSemaphore* semaphore) override;
    void waitSemaphore(GrSemaphore* semaphore) override;
    std::unique_ptr<GrSemaphore> prepareTextureForCrossContextUsage(GrTexture*) override {
        return nullptr;
    }

    void submit(GrOpsRenderPass* renderPass) override;
    void endRenderPass(GrRenderTarget* target, GrSurfaceOrigin origin,
                       const SkIRect& bounds);

    void checkFinishProcs() override { this->checkForFinishedCommandLists(); }
    void finishOutstandingGpuWork() override;

private:
    enum class SyncQueue {
        kForce,
        kSkip
    };

    GrD3DGpu(GrDirectContext*, const GrContextOptions&, const GrD3DBackendContext&,
             sk_sp<GrD3DMemoryAllocator>);

    void destroyResources();

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

    sk_sp<GrGpuBuffer> onCreateBuffer(size_t sizeInBytes,
                                      GrGpuBufferType,
                                      GrAccessPattern) override;

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
                       bool prepForTexSampling) override;

    bool onTransferFromBufferToBuffer(sk_sp<GrGpuBuffer> src,
                                      size_t srcOffset,
                                      sk_sp<GrGpuBuffer> dst,
                                      size_t dstOffset,
                                      size_t size) override;

    bool onTransferPixelsTo(GrTexture*,
                            SkIRect,
                            GrColorType surfaceColorType,
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

    bool onCopySurface(GrSurface* dst, const SkIRect& dstRect,
                       GrSurface* src, const SkIRect& srcRect,
                       GrSamplerState::Filter) override;

    bool onRegenerateMipMapLevels(GrTexture*) override;

    void onResolveRenderTarget(GrRenderTarget* target, const SkIRect&) override;

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

    void prepareSurfacesForBackendAccessAndStateUpdates(
            SkSpan<GrSurfaceProxy*> proxies,
            SkSurfaces::BackendSurfaceAccess access,
            const skgpu::MutableTextureState* newState) override;

    bool onSubmitToGpu(GrSyncCpu sync) override;

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

    bool submitDirectCommandList(SyncQueue sync);

    void checkForFinishedCommandLists();
    void waitForQueueCompletion();

    void copySurfaceAsCopyTexture(GrSurface* dst, GrSurface* src, GrD3DTextureResource* dstResource,
                                  GrD3DTextureResource* srcResource, const SkIRect& srcRect,
                                  const SkIPoint& dstPoint);

    void copySurfaceAsResolve(GrSurface* dst, GrSurface* src, const SkIRect& srcRect,
                              const SkIPoint& dstPoint);
    void resolveTexture(GrSurface* dst, int32_t dstX, int32_t dstY,
                        GrD3DRenderTarget* src, const SkIRect& srcRect);

    sk_sp<GrD3DTexture> createD3DTexture(SkISize,
                                         DXGI_FORMAT,
                                         GrRenderable,
                                         int renderTargetSampleCnt,
                                         skgpu::Budgeted,
                                         GrProtected,
                                         int mipLevelCount,
                                         GrMipmapStatus,
                                         std::string_view label);

    bool uploadToTexture(GrD3DTexture* tex,
                         SkIRect rect,
                         GrColorType colorType,
                         const GrMipLevel texels[],
                         int mipLevelCount);

    void readOrTransferPixels(GrD3DTextureResource* texResource,
                              SkIRect rect,
                              sk_sp<GrGpuBuffer> transferBuffer,
                              const D3D12_PLACED_SUBRESOURCE_FOOTPRINT& placedFootprint);

    bool createTextureResourceForBackendSurface(DXGI_FORMAT dxgiFormat,
                                                SkISize dimensions,
                                                GrTexturable texturable,
                                                GrRenderable renderable,
                                                skgpu::Mipmapped mipmapped,
                                                int sampleCnt,
                                                GrD3DTextureResourceInfo* info,
                                                GrProtected isProtected);

    gr_cp<ID3D12Device> fDevice;
    gr_cp<ID3D12CommandQueue> fQueue;

    sk_sp<GrD3DMemoryAllocator> fMemoryAllocator;

    GrD3DResourceProvider fResourceProvider;
    GrStagingBufferManager fStagingBufferManager;
    GrRingBuffer fConstantsRingBuffer;

    gr_cp<ID3D12Fence> fFence;
    uint64_t fCurrentFenceValue = 0;

    std::unique_ptr<GrD3DDirectCommandList> fCurrentDirectCommandList;
    // One-off special-case descriptors created directly for the mipmap compute shader
    // and hence aren't tracked by the normal path.
    skia_private::STArray<32, GrD3DDescriptorHeap::CPUHandle> fMipmapCPUDescriptors;

    struct OutstandingCommandList {
        OutstandingCommandList(std::unique_ptr<GrD3DDirectCommandList> commandList,
                               uint64_t fenceValue)
            : fCommandList(std::move(commandList)), fFenceValue(fenceValue) {
        }
        std::unique_ptr<GrD3DDirectCommandList> fCommandList;
        uint64_t fFenceValue;
    };

    SkDeque fOutstandingCommandLists;

    std::unique_ptr<GrD3DOpsRenderPass> fCachedOpsRenderPass;

#if defined(GR_TEST_UTILS)
    IDXGraphicsAnalysis* fGraphicsAnalysis;
#endif

    using INHERITED = GrGpu;
};

#endif
