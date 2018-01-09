/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMockGpu_DEFINED
#define GrMockGpu_DEFINED

#include "GrGpu.h"
#include "GrRenderTarget.h"
#include "GrSemaphore.h"
#include "GrTexture.h"
#include "SkTHash.h"

class GrMockGpuRTCommandBuffer;
struct GrMockOptions;
class GrPipeline;

class GrMockGpu : public GrGpu {
public:
    static sk_sp<GrGpu> Make(GrBackendContext, const GrContextOptions&, GrContext*);
    static sk_sp<GrGpu> Make(const GrMockOptions*, const GrContextOptions&, GrContext*);

    ~GrMockGpu() override {}

    bool onGetReadPixelsInfo(GrSurface* srcSurface, GrSurfaceOrigin srcOrigin,
                             int readWidth, int readHeight, size_t rowBytes,
                             GrPixelConfig readConfig, DrawPreference*,
                             ReadPixelTempDrawInfo*) override { return true; }

    bool onGetWritePixelsInfo(GrSurface* dstSurface, GrSurfaceOrigin dstOrigin,
                              int width, int height,
                              GrPixelConfig srcConfig, DrawPreference*,
                              WritePixelTempDrawInfo*) override { return true; }

    bool onCopySurface(GrSurface* dst, GrSurfaceOrigin dstOrigin,
                       GrSurface* src, GrSurfaceOrigin srcOrigin,
                       const SkIRect& srcRect, const SkIPoint& dstPoint) override { return true; }

    void onQueryMultisampleSpecs(GrRenderTarget* rt, GrSurfaceOrigin, const GrStencilSettings&,
                                 int* effectiveSampleCnt, SamplePattern*) override {
        *effectiveSampleCnt = rt->numStencilSamples();
    }

    GrGpuRTCommandBuffer* createCommandBuffer(
                                    GrRenderTarget*, GrSurfaceOrigin,
                                    const GrGpuRTCommandBuffer::LoadAndStoreInfo&,
                                    const GrGpuRTCommandBuffer::StencilLoadAndStoreInfo&) override;

    GrGpuTextureCommandBuffer* createCommandBuffer(GrTexture*, GrSurfaceOrigin) override;

    GrFence SK_WARN_UNUSED_RESULT insertFence() override { return 0; }
    bool waitFence(GrFence, uint64_t) override { return true; }
    void deleteFence(GrFence) const override {}

    sk_sp<GrSemaphore> SK_WARN_UNUSED_RESULT makeSemaphore(bool isOwned) override {
        return nullptr;
    }
    sk_sp<GrSemaphore> prepareTextureForCrossContextUsage(GrTexture*) override { return nullptr; }

    void submitCommandBuffer(const GrMockGpuRTCommandBuffer*);

private:
    GrMockGpu(GrContext* context, const GrMockOptions&, const GrContextOptions&);

    void onResetContext(uint32_t resetBits) override {}

    void xferBarrier(GrRenderTarget*, GrXferBarrierType) override {}

    sk_sp<GrTexture> onCreateTexture(const GrSurfaceDesc&, SkBudgeted,
                                     const GrMipLevel texels[], int mipLevelCount) override;

    sk_sp<GrTexture> onWrapBackendTexture(const GrBackendTexture&, GrWrapOwnership) override {
        return nullptr;
    }

    sk_sp<GrTexture> onWrapRenderableBackendTexture(const GrBackendTexture&,
                                                    int sampleCnt,
                                                    GrWrapOwnership) override {
        return nullptr;
    }

    sk_sp<GrRenderTarget> onWrapBackendRenderTarget(const GrBackendRenderTarget&) override {
        return nullptr;
    }

    sk_sp<GrRenderTarget> onWrapBackendTextureAsRenderTarget(const GrBackendTexture&,
                                                             int sampleCnt) override {
        return nullptr;
    }

    GrBuffer* onCreateBuffer(size_t sizeInBytes, GrBufferType, GrAccessPattern,
                             const void*) override;

    bool onReadPixels(GrSurface* surface, GrSurfaceOrigin,
                      int left, int top, int width, int height,
                      GrPixelConfig,
                      void* buffer,
                      size_t rowBytes) override {
        return true;
    }

    bool onWritePixels(GrSurface* surface, GrSurfaceOrigin,
                       int left, int top, int width, int height,
                       GrPixelConfig config,
                       const GrMipLevel texels[], int mipLevelCount) override {
        return true;
    }

    bool onTransferPixels(GrTexture* texture,
                          int left, int top, int width, int height,
                          GrPixelConfig config, GrBuffer* transferBuffer,
                          size_t offset, size_t rowBytes) override {
        return true;
    }

    void onResolveRenderTarget(GrRenderTarget* target, GrSurfaceOrigin) override { return; }

    void onFinishFlush(bool insertedSemaphores) override {}

    sk_sp<GrSemaphore> onWrapBackendSemaphore(const GrBackendSemaphore& semaphore,
                                            GrWrapOwnership ownership) override { return nullptr; }
    void onInsertSemaphore(sk_sp<GrSemaphore> semaphore, bool flush) override {}
    void onWaitSemaphore(sk_sp<GrSemaphore> semaphore) override {}

    GrStencilAttachment* createStencilAttachmentForRenderTarget(const GrRenderTarget*,
                                                                int width,
                                                                int height) override;
    void clearStencil(GrRenderTarget*, int clearValue) override  {}

    GrBackendTexture createTestingOnlyBackendTexture(void* pixels, int w, int h, GrPixelConfig,
                                                    bool isRT, GrMipMapped) override;
    bool isTestingOnlyBackendTexture(const GrBackendTexture&) const override;
    void deleteTestingOnlyBackendTexture(GrBackendTexture*, bool abandonTexture = false) override;

    static int NextInternalTextureID();
    static int NextExternalTextureID();

    SkTHashSet<int> fOutstandingTestingOnlyTextureIDs;

    typedef GrGpu INHERITED;
};

#endif
