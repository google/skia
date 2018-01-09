/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDDLGpu_DEFINED
#define GrDDLGpu_DEFINED

#include "GrGpu.h"

#include "GrRenderTarget.h"
#include "GrSemaphore.h"
#include "GrTexture.h"

class GrDDLGpu : public GrGpu {
public:
    static sk_sp<GrGpu> Make(GrContext*, sk_sp<const GrCaps>);

    ~GrDDLGpu() override {}

    bool onGetReadPixelsInfo(GrSurface* srcSurface, GrSurfaceOrigin srcOrigin,
                             int readWidth, int readHeight, size_t rowBytes,
                             GrPixelConfig readConfig, DrawPreference*,
                             ReadPixelTempDrawInfo*) override {
        SkASSERT(0);
        return true;
    }

    bool onGetWritePixelsInfo(GrSurface* dstSurface, GrSurfaceOrigin dstOrigin,
                              int width, int height,
                              GrPixelConfig srcConfig, DrawPreference*,
                              WritePixelTempDrawInfo*) override {
        SkASSERT(0);
        return true;
    }

    bool onCopySurface(GrSurface* dst, GrSurfaceOrigin dstOrigin,
                       GrSurface* src, GrSurfaceOrigin srcOrigin,
                       const SkIRect& srcRect, const SkIPoint& dstPoint) override {
        SkASSERT(0);
        return true;
    }

    void onQueryMultisampleSpecs(GrRenderTarget* rt, GrSurfaceOrigin, const GrStencilSettings&,
                                 int* effectiveSampleCnt, SamplePattern*) override {
        SkASSERT(0);
        *effectiveSampleCnt = 0; // ??
    }

    GrGpuRTCommandBuffer* createCommandBuffer(
                                    GrRenderTarget*, GrSurfaceOrigin,
                                    const GrGpuRTCommandBuffer::LoadAndStoreInfo&,
                                    const GrGpuRTCommandBuffer::StencilLoadAndStoreInfo&) override;

    GrGpuTextureCommandBuffer* createCommandBuffer(GrTexture*, GrSurfaceOrigin) override;

    GrFence SK_WARN_UNUSED_RESULT insertFence() override {
        SkASSERT(0);
        return 0;
    }
    bool waitFence(GrFence, uint64_t) override {
        SkASSERT(0);
        return true;
    }
    void deleteFence(GrFence) const override {
        SkASSERT(0);
    }

    sk_sp<GrSemaphore> SK_WARN_UNUSED_RESULT makeSemaphore(bool isOwned) override {
        SkASSERT(0);
        return nullptr;
    }
    sk_sp<GrSemaphore> prepareTextureForCrossContextUsage(GrTexture*) override {
        SkASSERT(0);
        return nullptr;
    }

    void submitCommandBuffer(const GrGpuRTCommandBuffer*);

private:
    GrDDLGpu(GrContext* context, sk_sp<const GrCaps> caps);

    void onResetContext(uint32_t resetBits) override { SkASSERT(0); }

    void xferBarrier(GrRenderTarget*, GrXferBarrierType) override { SkASSERT(0); }

    sk_sp<GrTexture> onCreateTexture(const GrSurfaceDesc&, SkBudgeted,
                                     const GrMipLevel texels[], int mipLevelCount) override;

    sk_sp<GrTexture> onWrapBackendTexture(const GrBackendTexture&, GrWrapOwnership) override {
        SkASSERT(0);
        return nullptr;
    }

    sk_sp<GrTexture> onWrapRenderableBackendTexture(const GrBackendTexture&,
                                                    int sampleCnt,
                                                    GrWrapOwnership) override {
        SkASSERT(0);
        return nullptr;
    }

    sk_sp<GrRenderTarget> onWrapBackendRenderTarget(const GrBackendRenderTarget&) override {
        SkASSERT(0);
        return nullptr;
    }

    sk_sp<GrRenderTarget> onWrapBackendTextureAsRenderTarget(const GrBackendTexture&,
                                                             int sampleCnt) override {
        SkASSERT(0);
        return nullptr;
    }

    GrBuffer* onCreateBuffer(size_t sizeInBytes, GrBufferType, GrAccessPattern,
                             const void*) override;

    bool onReadPixels(GrSurface* surface, GrSurfaceOrigin,
                      int left, int top, int width, int height,
                      GrPixelConfig,
                      void* buffer,
                      size_t rowBytes) override {
        SkASSERT(0);
        return true;
    }

    bool onWritePixels(GrSurface* surface, GrSurfaceOrigin,
                       int left, int top, int width, int height,
                       GrPixelConfig config,
                       const GrMipLevel texels[], int mipLevelCount) override {
        SkASSERT(0);
        return true;
    }

    bool onTransferPixels(GrTexture* texture,
                          int left, int top, int width, int height,
                          GrPixelConfig config, GrBuffer* transferBuffer,
                          size_t offset, size_t rowBytes) override {
        SkASSERT(0);
        return true;
    }

    void onResolveRenderTarget(GrRenderTarget* target, GrSurfaceOrigin) override {
        SkASSERT(0);
        return;
    }

    void onFinishFlush(bool insertedSemaphores) override { SkASSERT(0); }

    sk_sp<GrSemaphore> onWrapBackendSemaphore(const GrBackendSemaphore& semaphore,
                                            GrWrapOwnership ownership) override {
        SkASSERT(0);
        return nullptr;
    }
    void onInsertSemaphore(sk_sp<GrSemaphore> semaphore, bool flush) override {
        SkASSERT(0);
    }
    void onWaitSemaphore(sk_sp<GrSemaphore> semaphore) override {
        SkASSERT(0);
    }

    GrStencilAttachment* createStencilAttachmentForRenderTarget(const GrRenderTarget*,
                                                                int width,
                                                                int height) override;
    void clearStencil(GrRenderTarget*, int clearValue) override  { SkASSERT(0); }

    GrBackendTexture createTestingOnlyBackendTexture(void* pixels, int w, int h, GrPixelConfig,
                                                     bool isRT, GrMipMapped) override;
    bool isTestingOnlyBackendTexture(const GrBackendTexture&) const override;
    void deleteTestingOnlyBackendTexture(GrBackendTexture*, bool abandon = false) override;

    typedef GrGpu INHERITED;
};

#endif
