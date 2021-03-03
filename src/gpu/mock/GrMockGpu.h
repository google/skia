/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMockGpu_DEFINED
#define GrMockGpu_DEFINED

#include "include/private/SkTHash.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrRenderTarget.h"
#include "src/gpu/GrSemaphore.h"
#include "src/gpu/GrTexture.h"

class GrMockOpsRenderPass;
struct GrMockOptions;
class GrPipeline;

class GrMockGpu : public GrGpu {
public:
    static sk_sp<GrGpu> Make(const GrMockOptions*, const GrContextOptions&, GrDirectContext*);

    ~GrMockGpu() override;

    GrThreadSafePipelineBuilder* pipelineBuilder() override;
    sk_sp<GrThreadSafePipelineBuilder> refPipelineBuilder() override;

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

    void submit(GrOpsRenderPass* renderPass) override;

    void checkFinishProcs() override {}
    void finishOutstandingGpuWork() override {}

private:
    GrMockGpu(GrDirectContext*, const GrMockOptions&, const GrContextOptions&);

    void querySampleLocations(GrRenderTarget*, SkTArray<SkPoint>* sampleLocations) override;

    void xferBarrier(GrRenderTarget*, GrXferBarrierType) override {}

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
                            sk_sp<GrGpuBuffer> transferBuffer, size_t offset,
                            size_t rowBytes) override {
        return true;
    }
    bool onTransferPixelsFrom(GrSurface* surface, int left, int top, int width, int height,
                              GrColorType surfaceColorType, GrColorType bufferColorType,
                              sk_sp<GrGpuBuffer> transferBuffer, size_t offset) override {
        return true;
    }
    bool onCopySurface(GrSurface* dst, GrSurface* src, const SkIRect& srcRect,
                       const SkIPoint& dstPoint) override {
        return true;
    }

    bool onRegenerateMipMapLevels(GrTexture*) override { return true; }

    void onResolveRenderTarget(GrRenderTarget* target, const SkIRect&) override {}

    void addFinishedProc(GrGpuFinishedProc finishedProc,
                         GrGpuFinishedContext finishedContext) override {
        SkASSERT(finishedProc);
        finishedProc(finishedContext);
    }

    GrOpsRenderPass* onGetOpsRenderPass(GrRenderTarget*,
                                        GrAttachment*,
                                        GrSurfaceOrigin,
                                        const SkIRect&,
                                        const GrOpsRenderPass::LoadAndStoreInfo&,
                                        const GrOpsRenderPass::StencilLoadAndStoreInfo&,
                                        const SkTArray<GrSurfaceProxy*, true>& sampledProxies,
                                        GrXferBarrierFlags renderPassXferBarriers) override;

    bool onSubmitToGpu(bool syncCpu) override {
        return true;
    }

    sk_sp<GrAttachment> makeStencilAttachmentForRenderTarget(const GrRenderTarget*,
                                                             SkISize dimensions,
                                                             int numStencilSamples) override;

    GrBackendFormat getPreferredStencilFormat(const GrBackendFormat&) override {
        return GrBackendFormat::MakeMock(GrColorType::kUnknown, SkImage::CompressionType::kNone,
                                         true);
    }

    sk_sp<GrAttachment> makeMSAAAttachment(SkISize dimensions,
                                           const GrBackendFormat& format,
                                           int numSamples,
                                           GrProtected isProtected) override {
        return nullptr;
    }

    GrBackendTexture onCreateBackendTexture(SkISize dimensions,
                                            const GrBackendFormat&,
                                            GrRenderable,
                                            GrMipmapped,
                                            GrProtected) override;

    bool onUpdateBackendTexture(const GrBackendTexture&,
                                sk_sp<GrRefCntedCallback> finishedCallback,
                                const BackendTextureData*) override {
        return true;
    }

    GrBackendTexture onCreateCompressedBackendTexture(SkISize dimensions,
                                                      const GrBackendFormat&,
                                                      GrMipmapped,
                                                      GrProtected) override;

    bool onUpdateCompressedBackendTexture(const GrBackendTexture&,
                                          sk_sp<GrRefCntedCallback> finishedCallback,
                                          const BackendTextureData*) override {
        return true;
    }

    void deleteBackendTexture(const GrBackendTexture&) override;

    bool compile(const GrProgramDesc&, const GrProgramInfo&) override { return false; }

#if GR_TEST_UTILS
    bool isTestingOnlyBackendTexture(const GrBackendTexture&) const override;

    GrBackendRenderTarget createTestingOnlyBackendRenderTarget(SkISize dimensions,
                                                               GrColorType,
                                                               int sampleCnt,
                                                               GrProtected) override;
    void deleteTestingOnlyBackendRenderTarget(const GrBackendRenderTarget&) override;
#endif

    const GrMockOptions fMockOptions;

    static int NextInternalTextureID();
    static int NextExternalTextureID();
    static int NextInternalRenderTargetID();
    static int NextExternalRenderTargetID();

    SkTHashSet<int> fOutstandingTestingOnlyTextureIDs;

    using INHERITED = GrGpu;
};

#endif
