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

#import <Metal/Metal.h>

class GrSemaphore;
struct GrMtlBackendContext;

class GrMtlGpu : public GrGpu {
public:
    static GrGpu* Create(GrContext* context, const GrContextOptions& options,
                         id<MTLDevice> device, id<MTLCommandQueue> queue);

    ~GrMtlGpu() override {}

    bool onGetReadPixelsInfo(GrSurface* srcSurface, int readWidth, int readHeight, size_t rowBytes,
                             GrPixelConfig readConfig, DrawPreference*,
                             ReadPixelTempDrawInfo*) override { return false; }

    bool onGetWritePixelsInfo(GrSurface* dstSurface, int width, int height,
                              GrPixelConfig srcConfig, DrawPreference*,
                              WritePixelTempDrawInfo*) override { return false; }

    bool onCopySurface(GrSurface* dst,
                       GrSurface* src,
                       const SkIRect& srcRect,
                       const SkIPoint& dstPoint) override { return false; }

    void onQueryMultisampleSpecs(GrRenderTarget* rt, const GrStencilSettings&,
                                 int* effectiveSampleCnt, SamplePattern*) override {}

    GrGpuCommandBuffer* createCommandBuffer(const GrGpuCommandBuffer::LoadAndStoreInfo&,
                                            const GrGpuCommandBuffer::LoadAndStoreInfo&) override {
        return nullptr;
    }

    GrFence SK_WARN_UNUSED_RESULT insertFence() override { return 0; }
    bool waitFence(GrFence, uint64_t) override { return true; }
    void deleteFence(GrFence) const override {}

    sk_sp<GrSemaphore> SK_WARN_UNUSED_RESULT makeSemaphore(bool isOwned) override {
        return nullptr;
    }
    sk_sp<GrSemaphore> wrapBackendSemaphore(const GrBackendSemaphore& semaphore,
                                            GrWrapOwnership ownership) override { return nullptr; }
    void insertSemaphore(sk_sp<GrSemaphore> semaphore, bool flush) override {}
    void waitSemaphore(sk_sp<GrSemaphore> semaphore) override {}
    sk_sp<GrSemaphore> prepareTextureForCrossContextUsage(GrTexture*) override { return nullptr; }

private:
    GrMtlGpu(GrContext* context, const GrContextOptions& options,
             id<MTLDevice> device, id<MTLCommandQueue> queue);

    void onResetContext(uint32_t resetBits) override {}

    void xferBarrier(GrRenderTarget*, GrXferBarrierType) override {}

    sk_sp<GrTexture> onCreateTexture(const GrSurfaceDesc& desc, SkBudgeted budgeted,
                                     const GrMipLevel texels[], int mipLevelCount) override {
        return nullptr;
    }

    sk_sp<GrTexture> onWrapBackendTexture(const GrBackendTexture&,
                                          GrSurfaceOrigin,
                                          GrWrapOwnership) override {
        return nullptr;
    }

    sk_sp<GrTexture> onWrapRenderableBackendTexture(const GrBackendTexture&,
                                                    GrSurfaceOrigin,
                                                    int sampleCnt,
                                                    GrWrapOwnership) override {
        return nullptr;
    }

    sk_sp<GrRenderTarget> onWrapBackendRenderTarget(const GrBackendRenderTarget&,
                                                    GrSurfaceOrigin) override {
        return nullptr;
    }

    sk_sp<GrRenderTarget> onWrapBackendTextureAsRenderTarget(const GrBackendTexture&,
                                                             GrSurfaceOrigin,
                                                             int sampleCnt) override {
        return nullptr;
    }

    GrBuffer* onCreateBuffer(size_t, GrBufferType, GrAccessPattern, const void*) override {
        return nullptr;
    }

    gr_instanced::InstancedRendering* onCreateInstancedRendering() override { return nullptr; }

    bool onReadPixels(GrSurface* surface,
                      int left, int top, int width, int height,
                      GrPixelConfig,
                      void* buffer,
                      size_t rowBytes) override {
        return false;
    }

    bool onWritePixels(GrSurface* surface,
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

    void onResolveRenderTarget(GrRenderTarget* target) override { return; }

    GrStencilAttachment* createStencilAttachmentForRenderTarget(const GrRenderTarget*,
                                                                int width,
                                                                int height) override {
        return nullptr;
    }

    void clearStencil(GrRenderTarget* target) override  {}

    GrBackendObject createTestingOnlyBackendTexture(void* pixels, int w, int h,
                                                    GrPixelConfig config, bool isRT) override {
        return 0;
    }
    bool isTestingOnlyBackendTexture(GrBackendObject ) const override { return false; }
    void deleteTestingOnlyBackendTexture(GrBackendObject, bool abandonTexture) override {}

    id<MTLDevice> fDevice;
    id<MTLCommandQueue> fQueue;


    typedef GrGpu INHERITED;
};

#endif

