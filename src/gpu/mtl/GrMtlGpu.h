/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMtlGpu_DEFINED
#define GrMtlGpu_DEFINED

#include "include/gpu/GrRenderTarget.h"
#include "include/gpu/GrTexture.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrSemaphore.h"

#include "src/gpu/mtl/GrMtlBuffer.h"
#include "src/gpu/mtl/GrMtlCaps.h"
#include "src/gpu/mtl/GrMtlCopyManager.h"
#include "src/gpu/mtl/GrMtlResourceProvider.h"
#include "src/gpu/mtl/GrMtlStencilAttachment.h"

#import <Metal/Metal.h>

class GrMtlGpuRTCommandBuffer;
class GrMtlTexture;
class GrSemaphore;
struct GrMtlBackendContext;
class GrMtlCommandBuffer;

namespace SkSL {
    class Compiler;
}

// Helper macros for autorelease pools
#define SK_BEGIN_AUTORELEASE_BLOCK @autoreleasepool {
#define SK_END_AUTORELEASE_BLOCK }

class GrMtlGpu : public GrGpu {
public:
    static sk_sp<GrGpu> Make(GrContext* context, const GrContextOptions& options,
                             id<MTLDevice> device, id<MTLCommandQueue> queue);
    ~GrMtlGpu() override;

    void disconnect(DisconnectType) override;

    const GrMtlCaps& mtlCaps() const { return *fMtlCaps.get(); }

    id<MTLDevice> device() const { return fDevice; }

    GrMtlResourceProvider& resourceProvider() { return fResourceProvider; }

    GrMtlBufferManager& bufferManager() { return fBufferManager; }

    GrMtlCommandBuffer* commandBuffer();

    enum SyncQueue {
        kForce_SyncQueue,
        kSkip_SyncQueue
    };

    // Commits the current command buffer to the queue and then creates a new command buffer. If
    // sync is set to kForce_SyncQueue, the function will wait for all work in the committed
    // command buffer to finish before creating a new buffer and returning.
    void submitCommandBuffer(SyncQueue sync);

#if GR_TEST_UTILS
    GrBackendTexture createTestingOnlyBackendTexture(const void* pixels, int w, int h,
                                                     GrColorType colorType, bool isRT,
                                                     GrMipMapped, size_t rowBytes = 0) override;

    bool isTestingOnlyBackendTexture(const GrBackendTexture&) const override;

    void deleteTestingOnlyBackendTexture(const GrBackendTexture&) override;

    GrBackendRenderTarget createTestingOnlyBackendRenderTarget(int w, int h, GrColorType) override;

    void deleteTestingOnlyBackendRenderTarget(const GrBackendRenderTarget&) override;

    void testingOnly_flushGpuAndSync() override;
#endif

    bool copySurfaceAsBlit(GrSurface* dst, GrSurfaceOrigin dstOrigin,
                           GrSurface* src, GrSurfaceOrigin srcOrigin,
                           const SkIRect& srcRect, const SkIPoint& dstPoint);

    // This function is needed when we want to copy between two surfaces with different origins and
    // the destination surface is not a render target. We will first draw to a temporary render
    // target to adjust for the different origins and then blit from there to the destination.
    bool copySurfaceAsDrawThenBlit(GrSurface* dst, GrSurfaceOrigin dstOrigin,
                                   GrSurface* src, GrSurfaceOrigin srcOrigin,
                                   const SkIRect& srcRect, const SkIPoint& dstPoint);

    bool onCopySurface(GrSurface* dst, GrSurfaceOrigin dstOrigin,
                       GrSurface* src, GrSurfaceOrigin srcOrigin,
                       const SkIRect& srcRect,
                       const SkIPoint& dstPoint,
                       bool canDiscardOutsideDstRect) override;

    GrGpuRTCommandBuffer* getCommandBuffer(
                                    GrRenderTarget*, GrSurfaceOrigin, const SkRect& bounds,
                                    const GrGpuRTCommandBuffer::LoadAndStoreInfo&,
                                    const GrGpuRTCommandBuffer::StencilLoadAndStoreInfo&) override;

    GrGpuTextureCommandBuffer* getCommandBuffer(GrTexture*, GrSurfaceOrigin) override;

    SkSL::Compiler* shaderCompiler() const { return fCompiler.get(); }

    void submit(GrGpuCommandBuffer* buffer) override;

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

    // When the Metal backend actually uses indirect command buffers, this function will actually do
    // what it says. For now, every command is encoded directly into the primary command buffer, so
    // this function is pretty useless, except for indicating that a render target has been drawn
    // to.
    void submitIndirectCommandBuffer(GrSurface* surface, GrSurfaceOrigin origin,
                                     const SkIRect* bounds) {
        this->didWriteToSurface(surface, origin, bounds);
    }

private:
    GrMtlGpu(GrContext* context, const GrContextOptions& options,
             id<MTLDevice> device, id<MTLCommandQueue> queue, MTLFeatureSet featureSet);

    void destroyResources();

    void onResetContext(uint32_t resetBits) override {}

    void querySampleLocations(GrRenderTarget*, SkTArray<SkPoint>*) override {
        SkASSERT(!this->caps()->sampleLocationsSupport());
        SK_ABORT("Sample locations not yet implemented for Metal.");
    }

    void xferBarrier(GrRenderTarget*, GrXferBarrierType) override {}

    sk_sp<GrTexture> onCreateTexture(const GrSurfaceDesc& desc, SkBudgeted budgeted,
                                     const GrMipLevel texels[], int mipLevelCount) override;

    sk_sp<GrTexture> onWrapBackendTexture(const GrBackendTexture&, GrWrapOwnership, GrWrapCacheable,
                                          GrIOType) override;

    sk_sp<GrTexture> onWrapRenderableBackendTexture(const GrBackendTexture&, int sampleCnt,
                                                    GrWrapOwnership, GrWrapCacheable) override;

    sk_sp<GrRenderTarget> onWrapBackendRenderTarget(const GrBackendRenderTarget&) override;

    sk_sp<GrRenderTarget> onWrapBackendTextureAsRenderTarget(const GrBackendTexture&,
                                                             int sampleCnt) override;

    sk_sp<GrGpuBuffer> onCreateBuffer(size_t, GrGpuBufferType, GrAccessPattern,
                                      const void*) override;

    bool onReadPixels(GrSurface* surface, int left, int top, int width, int height, GrColorType,
                      void* buffer, size_t rowBytes) override;

    bool onWritePixels(GrSurface*, int left, int top, int width, int height, GrColorType,
                       const GrMipLevel[], int mipLevelCount) override;

    bool onTransferPixelsTo(GrTexture*,
                            int left, int top, int width, int height,
                            GrColorType, GrGpuBuffer*,
                            size_t offset, size_t rowBytes) override {
        // TODO: not sure this is worth the work since nobody uses it
        return false;
    }
    bool onTransferPixelsFrom(GrSurface*, int left, int top, int width, int height, GrColorType,
                              GrGpuBuffer*, size_t offset) override {
        // TODO: Will need to implement this to support async read backs.
        return false;
    }

    bool onRegenerateMipMapLevels(GrTexture*) override;

    void onResolveRenderTarget(GrRenderTarget* target) override { return; }

    void onFinishFlush(GrSurfaceProxy*, SkSurface::BackendSurfaceAccess access,
                       const GrFlushInfo& info) override {
        if (info.fFlags & kSyncCpu_GrFlushFlag) {
            this->submitCommandBuffer(kForce_SyncQueue);
            if (info.fFinishedProc) {
                info.fFinishedProc(info.fFinishedContext);
            }
        } else {
            this->submitCommandBuffer(kSkip_SyncQueue);
            // TODO: support finishedProc to actually be called when the GPU is done with the work
            // and not immediately.
            if (info.fFinishedProc) {
                info.fFinishedProc(info.fFinishedContext);
            }
        }
    }

    // Function that uploads data onto textures with private storage mode (GPU access only).
    bool uploadToTexture(GrMtlTexture* tex, int left, int top, int width, int height,
                         GrColorType dataColorType, const GrMipLevel texels[], int mipLevels);
    // Function that fills texture with transparent black
    bool clearTexture(GrMtlTexture*, GrColorType);

    GrStencilAttachment* createStencilAttachmentForRenderTarget(const GrRenderTarget*,
                                                                int width,
                                                                int height) override;

#if GR_TEST_UTILS
    bool createTestingOnlyMtlTextureInfo(GrColorType colorType, int w, int h, bool texturable,
                                         bool renderable, GrMipMapped mipMapped,
                                         const void* srcData, size_t rowBytes,
                                         GrMtlTextureInfo* info);
#endif

    sk_sp<GrMtlCaps> fMtlCaps;

    id<MTLDevice> fDevice;
    id<MTLCommandQueue> fQueue;

    GrMtlCommandBuffer* fCmdBuffer;

    std::unique_ptr<SkSL::Compiler> fCompiler;
    GrMtlCopyManager fCopyManager;
    GrMtlResourceProvider fResourceProvider;
    GrMtlBufferManager    fBufferManager;

    bool fDisconnected;

    typedef GrGpu INHERITED;
};

#endif

