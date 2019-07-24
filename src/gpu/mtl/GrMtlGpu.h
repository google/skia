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

#include "src/gpu/mtl/GrMtlCaps.h"
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

class GrMtlGpu : public GrGpu {
public:
    static sk_sp<GrGpu> Make(GrContext* context, const GrContextOptions& options,
                             id<MTLDevice> device, id<MTLCommandQueue> queue);
    ~GrMtlGpu() override;

    void disconnect(DisconnectType) override;

    const GrMtlCaps& mtlCaps() const { return *fMtlCaps.get(); }

    id<MTLDevice> device() const { return fDevice; }

    GrMtlResourceProvider& resourceProvider() { return fResourceProvider; }

    GrMtlCommandBuffer* commandBuffer();

    enum SyncQueue {
        kForce_SyncQueue,
        kSkip_SyncQueue
    };

    // Commits the current command buffer to the queue and then creates a new command buffer. If
    // sync is set to kForce_SyncQueue, the function will wait for all work in the committed
    // command buffer to finish before creating a new buffer and returning.
    void submitCommandBuffer(SyncQueue sync);

    GrBackendTexture createBackendTexture(int w, int h, const GrBackendFormat&,
                                          GrMipMapped, GrRenderable,
                                          const void* pixels, size_t rowBytes,
                                          const SkColor4f* color,
                                          GrProtected isProtected) override;

    void deleteBackendTexture(const GrBackendTexture&) override;

#if GR_TEST_UTILS
    bool isTestingOnlyBackendTexture(const GrBackendTexture&) const override;

    GrBackendRenderTarget createTestingOnlyBackendRenderTarget(int w, int h, GrColorType) override;
    void deleteTestingOnlyBackendRenderTarget(const GrBackendRenderTarget&) override;

    void testingOnly_flushGpuAndSync() override;
#endif

    void copySurfaceAsResolve(GrSurface* dst, GrSurface* src);

    void copySurfaceAsBlit(GrSurface* dst, GrSurface* src, const SkIRect& srcRect,
                           const SkIPoint& dstPoint);

    bool onCopySurface(GrSurface* dst, GrSurface* src, const SkIRect& srcRect,
                       const SkIPoint& dstPoint, bool canDiscardOutsideDstRect) override;

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
    // We currently call finish procs immediately in onFinishFlush().
    void checkFinishProcs() override {}
    sk_sp<GrSemaphore> prepareTextureForCrossContextUsage(GrTexture*) override { return nullptr; }

    // When the Metal backend actually uses indirect command buffers, this function will actually do
    // what it says. For now, every command is encoded directly into the primary command buffer, so
    // this function is pretty useless, except for indicating that a render target has been drawn
    // to.
    void submitIndirectCommandBuffer(GrSurface* surface, GrSurfaceOrigin origin,
                                     const SkIRect* bounds) {
        this->didWriteToSurface(surface, origin, bounds);
    }

    void resolveRenderTargetNoFlush(GrRenderTarget* target) {
        this->internalResolveRenderTarget(target, false);
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

    sk_sp<GrTexture> onCreateTexture(const GrSurfaceDesc& desc, GrRenderable,
                                     int renderTargetSampleCnt, SkBudgeted budgeted,
                                     GrProtected, const GrMipLevel texels[],
                                     int mipLevelCount) override;
    sk_sp<GrTexture> onCreateCompressedTexture(int width, int height, SkImage::CompressionType,
                                               SkBudgeted, const void* data) override {
        return nullptr;
    }

    sk_sp<GrTexture> onWrapBackendTexture(const GrBackendTexture&, GrColorType,
                                          GrWrapOwnership, GrWrapCacheable, GrIOType) override;

    sk_sp<GrTexture> onWrapRenderableBackendTexture(const GrBackendTexture&, int sampleCnt,
                                                    GrColorType, GrWrapOwnership,
                                                    GrWrapCacheable) override;

    sk_sp<GrRenderTarget> onWrapBackendRenderTarget(const GrBackendRenderTarget&,
                                                    GrColorType) override;

    sk_sp<GrRenderTarget> onWrapBackendTextureAsRenderTarget(const GrBackendTexture&,
                                                             int sampleCnt,
                                                             GrColorType) override;

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

    void onResolveRenderTarget(GrRenderTarget* target) override {
        // This resolve is called when we are preparing an msaa surface for external I/O. It is
        // called after flushing, so we need to make sure we submit the command buffer after doing
        // the resolve so that the resolve actually happens.
        this->internalResolveRenderTarget(target, true);
    }

    void internalResolveRenderTarget(GrRenderTarget* target, bool requiresSubmit);
    void resolveTexture(id<MTLTexture> colorTexture, id<MTLTexture> resolveTexture);

    void onFinishFlush(GrSurfaceProxy*[], int n, SkSurface::BackendSurfaceAccess access,
                       const GrFlushInfo& info, const GrPrepareForExternalIORequests&) override {
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
    // Function that fills texture levels with transparent black based on levelMask.
    bool clearTexture(GrMtlTexture*, GrColorType, uint32_t levelMask);

    GrStencilAttachment* createStencilAttachmentForRenderTarget(
            const GrRenderTarget*, int width, int height, int numStencilSamples) override;

    bool createTestingOnlyMtlTextureInfo(MTLPixelFormat,
                                         int w, int h, bool texturable,
                                         bool renderable, GrMipMapped mipMapped,
                                         const void* srcData, size_t rowBytes,
                                         GrMtlTextureInfo* info);

    sk_sp<GrMtlCaps> fMtlCaps;

    id<MTLDevice> fDevice;
    id<MTLCommandQueue> fQueue;

    GrMtlCommandBuffer* fCmdBuffer;

    std::unique_ptr<SkSL::Compiler> fCompiler;

    GrMtlResourceProvider fResourceProvider;

    bool fDisconnected;

    typedef GrGpu INHERITED;
};

#endif

