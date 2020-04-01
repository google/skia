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

class GrD3DGpu : public GrGpu {
public:
    static sk_sp<GrGpu> Make(const GrD3DBackendContext& backendContext, const GrContextOptions&,
                             GrContext*);

    ~GrD3DGpu() override;

    const GrD3DCaps& d3dCaps() const { return static_cast<const GrD3DCaps&>(*fCaps); }

    ID3D12Device* device() const { return fDevice.get(); }
    ID3D12CommandQueue* queue() const { return fQueue.get(); }

    void querySampleLocations(GrRenderTarget*, SkTArray<SkPoint>* sampleLocations) override;

    void xferBarrier(GrRenderTarget*, GrXferBarrierType) override {}

    void deleteBackendTexture(const GrBackendTexture&) override;

    bool compile(const GrProgramDesc&, const GrProgramInfo&) override;

#if GR_TEST_UTILS
    bool isTestingOnlyBackendTexture(const GrBackendTexture&) const override;

    GrBackendRenderTarget createTestingOnlyBackendRenderTarget(int w, int h, GrColorType) override;
    void deleteTestingOnlyBackendRenderTarget(const GrBackendRenderTarget&) override;

    void testingOnly_flushGpuAndSync() override {}
#endif

    GrStencilAttachment* createStencilAttachmentForRenderTarget(
            const GrRenderTarget*, int width, int height, int numStencilSamples) override;

    GrOpsRenderPass* getOpsRenderPass(
            GrRenderTarget*, GrSurfaceOrigin, const SkIRect&,
            const GrOpsRenderPass::LoadAndStoreInfo&,
            const GrOpsRenderPass::StencilLoadAndStoreInfo&,
            const SkTArray<GrSurfaceProxy*, true>& sampledProxies) override;

    GrFence SK_WARN_UNUSED_RESULT insertFence() override { return 0; }
    bool waitFence(GrFence, uint64_t) override { return true; }
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

    void submit(GrOpsRenderPass* renderPass) override;

    void checkFinishProcs() override {}

private:
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
                      size_t rowBytes) override {
        return true;
    }

    bool onWritePixels(GrSurface* surface, int left, int top, int width, int height,
                       GrColorType surfaceColorType, GrColorType srcColorType,
                       const GrMipLevel texels[], int mipLevelCount,
                       bool prepForTexSampling) override {
        return true;
    }

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
                       const SkIPoint& dstPoint) override {
        return true;
    }

    bool onRegenerateMipMapLevels(GrTexture*) override { return true; }

    void onResolveRenderTarget(GrRenderTarget* target, const SkIRect&, ForExternalIO) override {}

    bool onFinishFlush(GrSurfaceProxy*[], int n, SkSurface::BackendSurfaceAccess access,
                       const GrFlushInfo& info, const GrPrepareForExternalIORequests&) override {
        if (info.fFinishedProc) {
            info.fFinishedProc(info.fFinishedContext);
        }
        return true;
    }

    GrBackendTexture onCreateBackendTexture(SkISize dimensions,
                                            const GrBackendFormat&,
                                            GrRenderable,
                                            GrMipMapped,
                                            GrProtected,
                                            const BackendTextureData*) override;
    GrBackendTexture onCreateCompressedBackendTexture(SkISize dimensions,
                                                      const GrBackendFormat&,
                                                      GrMipMapped,
                                                      GrProtected,
                                                      const BackendTextureData*) override;

    void submitDirectCommandList();

    void checkForFinishedCommandLists();

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

    typedef GrGpu INHERITED;
};

#endif
