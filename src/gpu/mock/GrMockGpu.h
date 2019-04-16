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
    static sk_sp<GrGpu> Make(const GrMockOptions*, const GrContextOptions&, GrContext*);

    ~GrMockGpu() override {}

    GrGpuRTCommandBuffer* getCommandBuffer(
                                    GrRenderTarget*, GrSurfaceOrigin, const SkRect&,
                                    const GrGpuRTCommandBuffer::LoadAndStoreInfo&,
                                    const GrGpuRTCommandBuffer::StencilLoadAndStoreInfo&) override;

    GrGpuTextureCommandBuffer* getCommandBuffer(GrTexture*, GrSurfaceOrigin) override;

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

    void submit(GrGpuCommandBuffer* buffer) override;

private:
    GrMockGpu(GrContext* context, const GrMockOptions&, const GrContextOptions&);

    void submitCommandBuffer(const GrMockGpuRTCommandBuffer*);

    void onResetContext(uint32_t resetBits) override {}

    void querySampleLocations(
            GrRenderTarget*, const GrStencilSettings&, SkTArray<SkPoint>*) override {
        SkASSERT(!this->caps()->sampleLocationsSupport());
        SK_ABORT("Sample locations not implemented for mock GPU.");
    }

    void xferBarrier(GrRenderTarget*, GrXferBarrierType) override {}

    sk_sp<GrTexture> onCreateTexture(const GrSurfaceDesc&, SkBudgeted, const GrMipLevel[],
                                     int mipLevelCount) override;

    sk_sp<GrTexture> onWrapBackendTexture(const GrBackendTexture&, GrWrapOwnership, GrWrapCacheable,
                                          GrIOType) override;

    sk_sp<GrTexture> onWrapRenderableBackendTexture(const GrBackendTexture&,
                                                    int sampleCnt,
                                                    GrWrapOwnership,
                                                    GrWrapCacheable) override;

    sk_sp<GrRenderTarget> onWrapBackendRenderTarget(const GrBackendRenderTarget&) override;

    sk_sp<GrRenderTarget> onWrapBackendTextureAsRenderTarget(const GrBackendTexture&,
                                                             int sampleCnt) override;

    sk_sp<GrGpuBuffer> onCreateBuffer(size_t sizeInBytes, GrGpuBufferType, GrAccessPattern,
                                      const void*) override;

    bool onReadPixels(GrSurface* surface, int left, int top, int width, int height, GrColorType,
                      void* buffer, size_t rowBytes) override {
        return true;
    }

    bool onWritePixels(GrSurface* surface, int left, int top, int width, int height, GrColorType,
                       const GrMipLevel texels[], int mipLevelCount) override {
        return true;
    }

    bool onTransferPixels(GrTexture* texture, int left, int top, int width, int height, GrColorType,
                          GrGpuBuffer* transferBuffer, size_t offset, size_t rowBytes) override {
        return true;
    }

    bool onCopySurface(GrSurface* dst, GrSurfaceOrigin dstOrigin, GrSurface* src,
                       GrSurfaceOrigin srcOrigin, const SkIRect& srcRect,
                       const SkIPoint& dstPoint, bool canDiscardOutsideDstRect) override {
        return true;
    }

    bool onRegenerateMipMapLevels(GrTexture*) override { return true; }

    void onResolveRenderTarget(GrRenderTarget* target) override { return; }

    void onFinishFlush(GrSurfaceProxy*, SkSurface::BackendSurfaceAccess access,
                       GrFlushFlags flags, bool insertedSemaphores,
                       GrGpuFinishedProc finishedProc,
                       GrGpuFinishedContext finishedContext) override {
        if (finishedProc) {
            finishedProc(finishedContext);
        }
    }

    GrStencilAttachment* createStencilAttachmentForRenderTarget(const GrRenderTarget*,
                                                                int width,
                                                                int height) override;
#if GR_TEST_UTILS
    GrBackendTexture createTestingOnlyBackendTexture(const void* pixels, int w, int h,
                                                     GrColorType, bool isRT,
                                                     GrMipMapped, size_t rowBytes = 0) override;
    bool isTestingOnlyBackendTexture(const GrBackendTexture&) const override;
    void deleteTestingOnlyBackendTexture(const GrBackendTexture&) override;

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
