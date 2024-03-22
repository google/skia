/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMockGpu_DEFINED
#define GrMockGpu_DEFINED

#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/core/SkTextureCompressionType.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrTypes.h"
#include "include/gpu/mock/GrMockTypes.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkTArray.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/core/SkTHash.h"
#include "src/gpu/ganesh/GrAttachment.h"
#include "src/gpu/ganesh/GrGpu.h"
#include "src/gpu/ganesh/GrOpsRenderPass.h"
#include "src/gpu/ganesh/GrSamplerState.h"
#include "src/gpu/ganesh/GrSemaphore.h"  // IWYU pragma: keep
#include "src/gpu/ganesh/GrXferProcessor.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string_view>

class GrBackendSemaphore;
class GrDirectContext;
class GrGpuBuffer;
class GrProgramDesc;
class GrProgramInfo;
class GrRenderTarget;
class GrSurface;
class GrSurfaceProxy;
class GrTexture;
class GrThreadSafePipelineBuilder;
struct GrContextOptions;

namespace skgpu {
class RefCntedCallback;
enum class Budgeted : bool;
enum class Mipmapped : bool;
}

class GrMockGpu : public GrGpu {
public:
    static std::unique_ptr<GrGpu> Make(const GrMockOptions*,
                                       const GrContextOptions&,
                                       GrDirectContext*);

    ~GrMockGpu() override;

    GrThreadSafePipelineBuilder* pipelineBuilder() override;
    sk_sp<GrThreadSafePipelineBuilder> refPipelineBuilder() override;

    [[nodiscard]] std::unique_ptr<GrSemaphore> makeSemaphore(bool isOwned) override {
        return nullptr;
    }
    std::unique_ptr<GrSemaphore> wrapBackendSemaphore(const GrBackendSemaphore& /* semaphore */,
                                                      GrSemaphoreWrapType /* wraptype */,
                                                      GrWrapOwnership /* ownership */) override {
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

    void xferBarrier(GrRenderTarget*, GrXferBarrierType) override {}

    sk_sp<GrTexture> onCreateTexture(SkISize,
                                     const GrBackendFormat&,
                                     GrRenderable,
                                     int renderTargetSampleCnt,
                                     skgpu::Budgeted,
                                     GrProtected,
                                     int mipLevelCount,
                                     uint32_t levelClearMask,
                                     std::string_view label) override;

    sk_sp<GrTexture> onCreateCompressedTexture(SkISize dimensions,
                                               const GrBackendFormat&,
                                               skgpu::Budgeted,
                                               skgpu::Mipmapped,
                                               GrProtected,
                                               const void* data,
                                               size_t dataSize) override;

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

    sk_sp<GrGpuBuffer> onCreateBuffer(size_t sizeInBytes,
                                      GrGpuBufferType,
                                      GrAccessPattern) override;

    bool onReadPixels(GrSurface*,
                      SkIRect,
                      GrColorType surfaceColorType,
                      GrColorType dstColorType,
                      void*,
                      size_t rowBytes) override {
        return true;
    }

    bool onWritePixels(GrSurface*,
                       SkIRect,
                       GrColorType surfaceColorType,
                       GrColorType srcColorType,
                       const GrMipLevel[],
                       int mipLevelCount,
                       bool prepForTexSampling) override {
        return true;
    }

    bool onTransferFromBufferToBuffer(sk_sp<GrGpuBuffer> src,
                                      size_t srcOffset,
                                      sk_sp<GrGpuBuffer> dst,
                                      size_t dstOffset,
                                      size_t size) override {
        return true;
    }

    bool onTransferPixelsTo(GrTexture*,
                            SkIRect,
                            GrColorType surfaceColorType,
                            GrColorType bufferColorType,
                            sk_sp<GrGpuBuffer>,
                            size_t offset,
                            size_t rowBytes) override {
        return true;
    }

    bool onTransferPixelsFrom(GrSurface*,
                              SkIRect,
                              GrColorType surfaceColorType,
                              GrColorType bufferColorType,
                              sk_sp<GrGpuBuffer>,
                              size_t offset) override {
        return true;
    }

    bool onCopySurface(GrSurface* dst, const SkIRect& dstRect,
                       GrSurface* src, const SkIRect& srcRect,
                       GrSamplerState::Filter)override {
        return true;
    }

    bool onRegenerateMipMapLevels(GrTexture*) override { return true; }

    void onResolveRenderTarget(GrRenderTarget* target, const SkIRect&) override {}

    void addFinishedProc(GrGpuFinishedProc finishedProc,
                         GrGpuFinishedContext finishedContext) override {
        SkASSERT(finishedProc);
        finishedProc(finishedContext);
    }

    GrOpsRenderPass* onGetOpsRenderPass(
            GrRenderTarget*,
            bool useMSAASurface,
            GrAttachment*,
            GrSurfaceOrigin,
            const SkIRect&,
            const GrOpsRenderPass::LoadAndStoreInfo&,
            const GrOpsRenderPass::StencilLoadAndStoreInfo&,
            const skia_private::TArray<GrSurfaceProxy*, true>& sampledProxies,
            GrXferBarrierFlags renderPassXferBarriers) override;

    bool onSubmitToGpu(GrSyncCpu) override { return true; }

    sk_sp<GrAttachment> makeStencilAttachment(const GrBackendFormat& /*colorFormat*/,
                                              SkISize dimensions, int numStencilSamples) override;

    GrBackendFormat getPreferredStencilFormat(const GrBackendFormat&) override {
        return GrBackendFormat::MakeMock(GrColorType::kUnknown, SkTextureCompressionType::kNone,
                                         true);
    }

    sk_sp<GrAttachment> makeMSAAAttachment(SkISize dimensions,
                                           const GrBackendFormat& format,
                                           int numSamples,
                                           GrProtected isProtected,
                                           GrMemoryless isMemoryless) override {
        return nullptr;
    }

    GrBackendTexture onCreateBackendTexture(SkISize dimensions,
                                            const GrBackendFormat&,
                                            GrRenderable,
                                            skgpu::Mipmapped,
                                            GrProtected,
                                            std::string_view label) override;

    bool onClearBackendTexture(const GrBackendTexture&,
                               sk_sp<skgpu::RefCntedCallback> finishedCallback,
                               std::array<float, 4> color) override {
        return true;
    }

    GrBackendTexture onCreateCompressedBackendTexture(SkISize dimensions,
                                                      const GrBackendFormat&,
                                                      skgpu::Mipmapped,
                                                      GrProtected) override;

    bool onUpdateCompressedBackendTexture(const GrBackendTexture&,
                                          sk_sp<skgpu::RefCntedCallback> finishedCallback,
                                          const void*,
                                          size_t) override {
        return true;
    }

    void deleteBackendTexture(const GrBackendTexture&) override;

    bool compile(const GrProgramDesc&, const GrProgramInfo&) override { return false; }

#if defined(GR_TEST_UTILS)
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

    skia_private::THashSet<int> fOutstandingTestingOnlyTextureIDs;

    using INHERITED = GrGpu;
};

#endif
