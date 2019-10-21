/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMockGpu_DEFINED
#define GrMockGpu_DEFINED

#include "include/gpu/GrTexture.h"
#include "include/private/SkTHash.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrRenderTarget.h"
#include "src/gpu/GrSemaphore.h"

class GrMockOpsRenderPass;
struct GrMockOptions;
class GrPipeline;

class GrMockGpu : public GrGpu {
public:
    static sk_sp<GrGpu> Make(const GrMockOptions*, const GrContextOptions&, GrContext*);

    ~GrMockGpu() override {}

    GrOpsRenderPass* getOpsRenderPass(
            GrRenderTarget*, GrSurfaceOrigin, const SkIRect&,
            const GrOpsRenderPass::LoadAndStoreInfo&,
            const GrOpsRenderPass::StencilLoadAndStoreInfo&,
            const SkTArray<GrTextureProxy*, true>& sampledProxies) override;

    GrFence SK_WARN_UNUSED_RESULT insertFence() override { return 0; }
    bool waitFence(GrFence, uint64_t) override { return true; }
    void deleteFence(GrFence) const override {}

    sk_sp<GrSemaphore> SK_WARN_UNUSED_RESULT makeSemaphore(bool isOwned) override {
        return nullptr;
    }
    sk_sp<GrSemaphore> wrapBackendSemaphore(const GrBackendSemaphore& semaphore,
                                            GrResourceProvider::SemaphoreWrapType wrapType,
                                            GrWrapOwnership ownership) override { return nullptr; }
    void insertSemaphore(sk_sp<GrSemaphore> semaphore) override {}
    void waitSemaphore(sk_sp<GrSemaphore> semaphore) override {}
    sk_sp<GrSemaphore> prepareTextureForCrossContextUsage(GrTexture*) override { return nullptr; }

    void submit(GrOpsRenderPass* renderPass) override;

    void checkFinishProcs() override {}

private:
    GrMockGpu(GrContext* context, const GrMockOptions&, const GrContextOptions&);

    void onResetContext(uint32_t resetBits) override {}

    void querySampleLocations(GrRenderTarget*, SkTArray<SkPoint>* sampleLocations) override;

    void xferBarrier(GrRenderTarget*, GrXferBarrierType) override {}

    sk_sp<GrTexture> onCreateTexture(const GrSurfaceDesc&,
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

    sk_sp<GrTexture> onWrapRenderableBackendTexture(const GrBackendTexture&,
                                                    int sampleCnt,
                                                    GrColorType,
                                                    GrWrapOwnership,
                                                    GrWrapCacheable) override;

    sk_sp<GrRenderTarget> onWrapBackendRenderTarget(const GrBackendRenderTarget&,
                                                    GrColorType) override;

    sk_sp<GrRenderTarget> onWrapBackendTextureAsRenderTarget(const GrBackendTexture&,
                                                             int sampleCnt, GrColorType) override;

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

    void onResolveRenderTarget(GrRenderTarget* target, const SkIRect&, GrSurfaceOrigin,
                               ForExternalIO) override {}

    void onFinishFlush(GrSurfaceProxy*[], int n, SkSurface::BackendSurfaceAccess access,
                       const GrFlushInfo& info, const GrPrepareForExternalIORequests&) override {
        if (info.fFinishedProc) {
            info.fFinishedProc(info.fFinishedContext);
        }
    }

    GrStencilAttachment* createStencilAttachmentForRenderTarget(
            const GrRenderTarget*, int width, int height, int numStencilSamples) override;
    GrBackendTexture onCreateBackendTexture(int w, int h, const GrBackendFormat&,
                                            GrMipMapped, GrRenderable,
                                            const SkPixmap srcData[], int numMipLevels,
                                            const SkColor4f* color, GrProtected) override;
    void deleteBackendTexture(const GrBackendTexture&) override;

#if GR_TEST_UTILS
    bool isTestingOnlyBackendTexture(const GrBackendTexture&) const override;

    GrBackendRenderTarget createTestingOnlyBackendRenderTarget(int w, int h, GrColorType) override;
    void deleteTestingOnlyBackendRenderTarget(const GrBackendRenderTarget&) override;

    void testingOnly_flushGpuAndSync() override {}
#endif

    const GrMockOptions fMockOptions;

    static int NextInternalTextureID();
    static int NextExternalTextureID();
    static int NextInternalRenderTargetID();
    static int NextExternalRenderTargetID();

    SkTHashSet<int> fOutstandingTestingOnlyTextureIDs;

    typedef GrGpu INHERITED;
};

#endif
