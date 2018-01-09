/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMtlGpu_DEFINED
#define GrMtlGpu_DEFINED

#include "GrGpu.h"
#include "GrRenderTarget.h"
#include "GrSemaphore.h"
#include "GrTexture.h"

#include "GrMtlCaps.h"

#import <Metal/Metal.h>

class GrSemaphore;
struct GrMtlBackendContext;

class GrMtlGpu : public GrGpu {
public:
    static sk_sp<GrGpu> Make(GrContext* context, const GrContextOptions& options,
                             id<MTLDevice> device, id<MTLCommandQueue> queue);

    ~GrMtlGpu() override {}

    const GrMtlCaps& mtlCaps() const { return *fMtlCaps.get(); }

    id<MTLDevice> device() const { return fDevice; }

    bool onGetReadPixelsInfo(GrSurface* srcSurface, GrSurfaceOrigin origin,
                             int readWidth, int readHeight, size_t rowBytes,
                             GrPixelConfig readConfig, DrawPreference*,
                             ReadPixelTempDrawInfo*) override { return false; }

    bool onGetWritePixelsInfo(GrSurface* dstSurface, GrSurfaceOrigin dstOrigin,
                              int width, int height,
                              GrPixelConfig srcConfig, DrawPreference*,
                              WritePixelTempDrawInfo*) override { return false; }

    bool onCopySurface(GrSurface* dst, GrSurfaceOrigin dstOrigin,
                       GrSurface* src, GrSurfaceOrigin srcOrigin,
                       const SkIRect& srcRect,
                       const SkIPoint& dstPoint) override { return false; }

    void onQueryMultisampleSpecs(GrRenderTarget*, GrSurfaceOrigin, const GrStencilSettings&,
                                 int* effectiveSampleCnt, SamplePattern*) override {}

    GrGpuRTCommandBuffer* createCommandBuffer(
                                    GrRenderTarget*, GrSurfaceOrigin,
                                    const GrGpuRTCommandBuffer::LoadAndStoreInfo&,
                                    const GrGpuRTCommandBuffer::StencilLoadAndStoreInfo&) override {
        return nullptr;
    }

    GrGpuTextureCommandBuffer* createCommandBuffer(GrTexture*, GrSurfaceOrigin) override {
        return nullptr;
    }

    GrFence SK_WARN_UNUSED_RESULT insertFence() override { return 0; }
    bool waitFence(GrFence, uint64_t) override { return true; }
    void deleteFence(GrFence) const override {}

    sk_sp<GrSemaphore> SK_WARN_UNUSED_RESULT makeSemaphore(bool isOwned) override {
        return nullptr;
    }
    sk_sp<GrSemaphore> prepareTextureForCrossContextUsage(GrTexture*) override { return nullptr; }

private:
    GrMtlGpu(GrContext* context, const GrContextOptions& options,
             id<MTLDevice> device, id<MTLCommandQueue> queue, MTLFeatureSet featureSet);

    void onResetContext(uint32_t resetBits) override {}

    void xferBarrier(GrRenderTarget*, GrXferBarrierType) override {}

    sk_sp<GrTexture> onCreateTexture(const GrSurfaceDesc& desc, SkBudgeted budgeted,
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

    GrBuffer* onCreateBuffer(size_t, GrBufferType, GrAccessPattern, const void*) override {
        return nullptr;
    }

    bool onReadPixels(GrSurface* surface, GrSurfaceOrigin,
                      int left, int top, int width, int height,
                      GrPixelConfig,
                      void* buffer,
                      size_t rowBytes) override {
        return false;
    }

    bool onWritePixels(GrSurface* surface, GrSurfaceOrigin,
                       int left, int top, int width, int height,
                       GrPixelConfig config,
                       const GrMipLevel texels[], int mipLevelCount) override {
        return false;
    }

    bool onTransferPixels(GrTexture* texture,
                          int left, int top, int width, int height,
                          GrPixelConfig config, GrBuffer* transferBuffer,
                          size_t offset, size_t rowBytes) override {
        return false;
    }

    void onResolveRenderTarget(GrRenderTarget* target, GrSurfaceOrigin) override { return; }

    void onFinishFlush(bool insertedSemaphores) override {}

    sk_sp<GrSemaphore> onWrapBackendSemaphore(const GrBackendSemaphore& semaphore,
                                            GrWrapOwnership ownership) override { return nullptr; }
    void onInsertSemaphore(sk_sp<GrSemaphore> semaphore, bool flush) override {}
    void onWaitSemaphore(sk_sp<GrSemaphore> semaphore) override {}

    GrStencilAttachment* createStencilAttachmentForRenderTarget(const GrRenderTarget*,
                                                                int width,
                                                                int height) override {
        return nullptr;
    }

    void clearStencil(GrRenderTarget* target, int clearValue) override  {}

    GrBackendTexture createTestingOnlyBackendTexture(void* pixels, int w, int h,
                                                     GrPixelConfig config, bool isRT,
                                                     GrMipMapped) override {
        return GrBackendTexture();
    }
    bool isTestingOnlyBackendTexture(const GrBackendTexture&) const override { return false; }
    void deleteTestingOnlyBackendTexture(GrBackendTexture*, bool abandon = false) override {}

    sk_sp<GrMtlCaps> fMtlCaps;

    id<MTLDevice> fDevice;
    id<MTLCommandQueue> fQueue;


    typedef GrGpu INHERITED;
};

#endif

