/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrD3DGpu_DEFINED
#define GrD3DGpu_DEFINED

#include "include/private/SkDeque.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrRenderTarget.h"
#include "src/gpu/GrSemaphore.h"
#include "src/gpu/d3d/GrD3DCaps.h"
#include "src/gpu/d3d/GrD3DCommandList.h"
#include "src/gpu/d3d/GrD3DResourceProvider.h"

struct GrD3DBackendContext;
class GrD3DOpsRenderPass;
struct GrD3DOptions;
class GrPipeline;
#if GR_TEST_UTILS
struct IDXGraphicsAnalysis;
#endif

namespace SkSL {
    class Compiler;
}

class GrD3DGpu : public GrGpu {
public:
    static sk_sp<GrGpu> Make(const GrD3DBackendContext& backendContext, const GrContextOptions&,
                             GrContext*);

    ~GrD3DGpu() override;

    const GrD3DCaps& d3dCaps() const { return static_cast<const GrD3DCaps&>(*fCaps); }

    GrD3DResourceProvider& resourceProvider() { return fResourceProvider; }

    ID3D12Device* device() const { return fDevice.get(); }
    ID3D12CommandQueue* queue() const { return fQueue.get(); }

    GrD3DDirectCommandList* currentCommandList() const { return fCurrentDirectCommandList.get(); }

    bool protectedContext() const { return false; }

    void querySampleLocations(GrRenderTarget*, SkTArray<SkPoint>* sampleLocations) override;

    void xferBarrier(GrRenderTarget*, GrXferBarrierType) override {}

    void deleteBackendTexture(const GrBackendTexture&) override;

    bool compile(const GrProgramDesc&, const GrProgramInfo&) override;

#if GR_TEST_UTILS
    bool isTestingOnlyBackendTexture(const GrBackendTexture&) const override;

    GrBackendRenderTarget createTestingOnlyBackendRenderTarget(int w, int h, GrColorType) override;
    void deleteTestingOnlyBackendRenderTarget(const GrBackendRenderTarget&) override;

    void testingOnly_flushGpuAndSync() override;

    void testingOnly_startCapture() override;
    void testingOnly_endCapture() override;
#endif

    GrStencilAttachment* createStencilAttachmentForRenderTarget(
            const GrRenderTarget*, int width, int height, int numStencilSamples) override;

    GrOpsRenderPass* getOpsRenderPass(
            GrRenderTarget*, GrSurfaceOrigin, const SkIRect&,
            const GrOpsRenderPass::LoadAndStoreInfo&,
            const GrOpsRenderPass::StencilLoadAndStoreInfo&,
            const SkTArray<GrSurfaceProxy*, true>& sampledProxies) override;

    void addResourceBarriers(sk_sp<GrManagedResource> resource,
                             int numBarriers,
                             D3D12_RESOURCE_TRANSITION_BARRIER* barriers) const;

    GrFence SK_WARN_UNUSED_RESULT insertFence() override { return 0; }
    bool waitFence(GrFence) override { return true; }
    void deleteFence(GrFence) const override {}

    std::unique_ptr<GrSemaphore> SK_WARN_UNUSED_RESULT makeSemaphore(bool isOwned) override {
        return nullptr;
    }
    std::unique_ptr<GrSemaphore> wrapBackendSemaphore(
            const GrBackendSemaphore& semaphore,
            GrResourceProvider::SemaphoreWrapType wrapType,
            GrWrapOwnership ownership) override {
        return nullptr;
    }
    void insertSemaphore(GrSemaphore* semaphore) override {}
    void waitSemaphore(GrSemaphore* semaphore) override {}
    std::unique_ptr<GrSemaphore> prepareTextureForCrossContextUsage(GrTexture*) override {
        return nullptr;
    }

    void clear(const GrFixedClip& clip, const SkPMColor4f& color, GrRenderTarget*);

    void submit(GrOpsRenderPass* renderPass) override;

    void checkFinishProcs() override {}

    SkSL::Compiler* shaderCompiler() const {
        return fCompiler.get();
    }

private:
    enum class SyncQueue {
        kForce,
        kSkip
    };

    GrD3DGpu(GrContext* context, const GrContextOptions&, const GrD3DBackendContext&);

    void destroyResources();

    void onResetContext(uint32_t resetBits) override {}

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
                                               GrMipMapped,
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

    sk_sp<GrRenderTarget> onWrapBackendTextureAsRenderTarget(const GrBackendTexture&,
                                                             int sampleCnt) override;

    sk_sp<GrGpuBuffer> onCreateBuffer(size_t sizeInBytes, GrGpuBufferType, GrAccessPattern,
                                      const void*) override;

    bool onReadPixels(GrSurface* surface, int left, int top, int width, int height,
                      GrColorType surfaceColorType, GrColorType dstColorType, void* buffer,
                      size_t rowBytes) override;

    bool onWritePixels(GrSurface* surface, int left, int top, int width, int height,
                       GrColorType surfaceColorType, GrColorType srcColorType,
                       const GrMipLevel texels[], int mipLevelCount,
                       bool prepForTexSampling) override;

    bool onTransferPixelsTo(GrTexture* texture, int left, int top, int width, int height,
                            GrColorType surfaceColorType, GrColorType bufferColorType,
                            GrGpuBuffer* transferBuffer, size_t offset, size_t rowBytes) override {
        return true;
    }
    bool onTransferPixelsFrom(GrSurface* surface, int left, int top, int width, int height,
                              GrColorType surfaceColorType, GrColorType bufferColorType,
                              GrGpuBuffer* transferBuffer, size_t offset) override {
        return true;
    }
    bool onCopySurface(GrSurface* dst, GrSurface* src, const SkIRect& srcRect,
                       const SkIPoint& dstPoint) override;

    bool onRegenerateMipMapLevels(GrTexture*) override { return true; }

    void onResolveRenderTarget(GrRenderTarget* target, const SkIRect&, ForExternalIO) override {}

    void addFinishedProc(GrGpuFinishedProc finishedProc,
                         GrGpuFinishedContext finishedContext) override {
        // TODO: have this actually wait before calling the proc
        SkASSERT(finishedProc);
        finishedProc(finishedContext);
    }

    bool onSubmitToGpu(bool syncCpu) override;

    GrBackendTexture onCreateBackendTexture(SkISize dimensions,
                                            const GrBackendFormat&,
                                            GrRenderable,
                                            GrMipMapped,
                                            GrProtected) override;

    bool onUpdateBackendTexture(const GrBackendTexture&,
                                sk_sp<GrRefCntedCallback> finishedCallback,
                                const BackendTextureData*) override;

    GrBackendTexture onCreateCompressedBackendTexture(SkISize dimensions,
                                                      const GrBackendFormat&,
                                                      GrMipMapped,
                                                      GrProtected,
                                                      sk_sp<GrRefCntedCallback> finishedCallback,
                                                      const BackendTextureData*) override;

    bool submitDirectCommandList(SyncQueue sync);

    void checkForFinishedCommandLists();
    void waitForQueueCompletion();

    void copySurfaceAsCopyTexture(GrSurface* dst, GrSurface* src, GrD3DTextureResource* dstResource,
                                  GrD3DTextureResource* srcResource, const SkIRect& srcRect,
                                  const SkIPoint& dstPoint);

    void copySurfaceAsResolve(GrSurface* dst, GrSurface* src, const SkIRect& srcRect,
                              const SkIPoint& dstPoint);

    bool uploadToTexture(GrD3DTexture* tex, int left, int top, int width, int height,
                         GrColorType colorType, const GrMipLevel* texels, int mipLevelCount);

    bool createTextureResourceForBackendSurface(DXGI_FORMAT dxgiFormat,
                                                SkISize dimensions,
                                                GrTexturable texturable,
                                                GrRenderable renderable,
                                                GrMipMapped mipMapped,
                                                GrD3DTextureResourceInfo* info,
                                                GrProtected isProtected);

    gr_cp<ID3D12Device> fDevice;
    gr_cp<ID3D12CommandQueue> fQueue;

    GrD3DResourceProvider fResourceProvider;

    gr_cp<ID3D12Fence> fFence;
    uint64_t fCurrentFenceValue = 0;

    std::unique_ptr<GrD3DDirectCommandList> fCurrentDirectCommandList;

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

#if GR_TEST_UTILS
    IDXGraphicsAnalysis* fGraphicsAnalysis;
#endif

    std::unique_ptr<SkSL::Compiler> fCompiler;

    typedef GrGpu INHERITED;
};

#endif
