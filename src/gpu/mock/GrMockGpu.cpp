/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/mock/GrMockGpu.h"

#include "src/gpu/GrThreadSafePipelineBuilder.h"
#include "src/gpu/mock/GrMockAttachment.h"
#include "src/gpu/mock/GrMockBuffer.h"
#include "src/gpu/mock/GrMockCaps.h"
#include "src/gpu/mock/GrMockOpsRenderPass.h"
#include "src/gpu/mock/GrMockTexture.h"

#include <atomic>

int GrMockGpu::NextInternalTextureID() {
    static std::atomic<int> nextID{1};
    int id;
    do {
        id = nextID.fetch_add(1, std::memory_order_relaxed);
    } while (0 == id);  // Reserve 0 for an invalid ID.
    return id;
}

int GrMockGpu::NextExternalTextureID() {
    // We use negative ints for the "testing only external textures" so they can easily be
    // identified when debugging.
    static std::atomic<int> nextID{-1};
    return nextID.fetch_add(-1, std::memory_order_relaxed);
}

int GrMockGpu::NextInternalRenderTargetID() {
    // We start off with large numbers to differentiate from texture IDs, even though they're
    // technically in a different space.
    static std::atomic<int> nextID{SK_MaxS32};
    return nextID.fetch_add(-1, std::memory_order_relaxed);
}

int GrMockGpu::NextExternalRenderTargetID() {
    // We use large negative ints for the "testing only external render targets" so they can easily
    // be identified when debugging.
    static std::atomic<int> nextID{SK_MinS32};
    return nextID.fetch_add(1, std::memory_order_relaxed);
}

sk_sp<GrGpu> GrMockGpu::Make(const GrMockOptions* mockOptions,
                             const GrContextOptions& contextOptions, GrDirectContext* direct) {
    static const GrMockOptions kDefaultOptions = GrMockOptions();
    if (!mockOptions) {
        mockOptions = &kDefaultOptions;
    }
    return sk_sp<GrGpu>(new GrMockGpu(direct, *mockOptions, contextOptions));
}

GrOpsRenderPass* GrMockGpu::onGetOpsRenderPass(GrRenderTarget* rt,
                                               bool /*useMSAASurface*/,
                                               GrAttachment*,
                                               GrSurfaceOrigin origin,
                                               const SkIRect& bounds,
                                               const GrOpsRenderPass::LoadAndStoreInfo& colorInfo,
                                               const GrOpsRenderPass::StencilLoadAndStoreInfo&,
                                               const SkTArray<GrSurfaceProxy*,true>& sampledProxies,
                                               GrXferBarrierFlags renderPassXferBarriers) {
    return new GrMockOpsRenderPass(this, rt, origin, colorInfo);
}

void GrMockGpu::submit(GrOpsRenderPass* renderPass) {
    for (int i = 0; i < static_cast<GrMockOpsRenderPass*>(renderPass)->numDraws(); ++i) {
        fStats.incNumDraws();
    }
    delete renderPass;
}

GrMockGpu::GrMockGpu(GrDirectContext* direct, const GrMockOptions& options,
                     const GrContextOptions& contextOptions)
        : INHERITED(direct)
        , fMockOptions(options) {
    this->initCapsAndCompiler(sk_make_sp<GrMockCaps>(contextOptions, options));
}

GrMockGpu::~GrMockGpu() {}

GrThreadSafePipelineBuilder* GrMockGpu::pipelineBuilder() {
    return nullptr;
}

sk_sp<GrThreadSafePipelineBuilder> GrMockGpu::refPipelineBuilder() {
    return nullptr;
}

sk_sp<GrTexture> GrMockGpu::onCreateTexture(SkISize dimensions,
                                            const GrBackendFormat& format,
                                            GrRenderable renderable,
                                            int renderTargetSampleCnt,
                                            SkBudgeted budgeted,
                                            GrProtected isProtected,
                                            int mipLevelCount,
                                            uint32_t levelClearMask) {
    if (fMockOptions.fFailTextureAllocations) {
        return nullptr;
    }

    // Compressed formats should go through onCreateCompressedTexture
    SkASSERT(format.asMockCompressionType() == SkImage::CompressionType::kNone);

    GrColorType ct = format.asMockColorType();
    SkASSERT(ct != GrColorType::kUnknown);

    GrMipmapStatus mipmapStatus =
            mipLevelCount > 1 ? GrMipmapStatus::kDirty : GrMipmapStatus::kNotAllocated;
    GrMockTextureInfo texInfo(ct, SkImage::CompressionType::kNone, NextInternalTextureID());
    if (renderable == GrRenderable::kYes) {
        GrMockRenderTargetInfo rtInfo(ct, NextInternalRenderTargetID());
        return sk_sp<GrTexture>(new GrMockTextureRenderTarget(this, budgeted, dimensions,
                                                              renderTargetSampleCnt, isProtected,
                                                              mipmapStatus, texInfo, rtInfo));
    }
    return sk_sp<GrTexture>(
            new GrMockTexture(this, budgeted, dimensions, isProtected, mipmapStatus, texInfo));
}

// TODO: why no 'isProtected' ?!
sk_sp<GrTexture> GrMockGpu::onCreateCompressedTexture(SkISize dimensions,
                                                      const GrBackendFormat& format,
                                                      SkBudgeted budgeted,
                                                      GrMipmapped mipMapped,
                                                      GrProtected isProtected,
                                                      const void* data, size_t dataSize) {
    if (fMockOptions.fFailTextureAllocations) {
        return nullptr;
    }

#ifdef SK_DEBUG
    // Uncompressed formats should go through onCreateTexture
    SkImage::CompressionType compression = format.asMockCompressionType();
    SkASSERT(compression != SkImage::CompressionType::kNone);
#endif

    GrMipmapStatus mipmapStatus = (mipMapped == GrMipmapped::kYes)
                                                                ? GrMipmapStatus::kValid
                                                                : GrMipmapStatus::kNotAllocated;
    GrMockTextureInfo texInfo(GrColorType::kUnknown,
                              format.asMockCompressionType(),
                              NextInternalTextureID());

    return sk_sp<GrTexture>(
            new GrMockTexture(this, budgeted, dimensions, isProtected, mipmapStatus, texInfo));
}

sk_sp<GrTexture> GrMockGpu::onWrapBackendTexture(const GrBackendTexture& tex,
                                                 GrWrapOwnership ownership,
                                                 GrWrapCacheable wrapType,
                                                 GrIOType ioType) {
    GrMockTextureInfo texInfo;
    SkAssertResult(tex.getMockTextureInfo(&texInfo));

    SkImage::CompressionType compression = texInfo.compressionType();
    if (compression != SkImage::CompressionType::kNone) {
        return nullptr;
    }

    GrMipmapStatus mipmapStatus = tex.hasMipmaps() ? GrMipmapStatus::kValid
                                                   : GrMipmapStatus::kNotAllocated;
    auto isProtected = GrProtected(tex.isProtected());
    return sk_sp<GrTexture>(new GrMockTexture(this, tex.dimensions(), isProtected, mipmapStatus,
                                              texInfo, wrapType, ioType));
}

sk_sp<GrTexture> GrMockGpu::onWrapCompressedBackendTexture(const GrBackendTexture& tex,
                                                           GrWrapOwnership ownership,
                                                           GrWrapCacheable wrapType) {
    return nullptr;
}

sk_sp<GrTexture> GrMockGpu::onWrapRenderableBackendTexture(const GrBackendTexture& tex,
                                                           int sampleCnt,
                                                           GrWrapOwnership ownership,
                                                           GrWrapCacheable cacheable) {
    GrMockTextureInfo texInfo;
    SkAssertResult(tex.getMockTextureInfo(&texInfo));
    SkASSERT(texInfo.compressionType() == SkImage::CompressionType::kNone);

    GrMipmapStatus mipmapStatus =
            tex.hasMipmaps() ? GrMipmapStatus::kValid : GrMipmapStatus::kNotAllocated;

    // The client gave us the texture ID but we supply the render target ID.
    GrMockRenderTargetInfo rtInfo(texInfo.colorType(), NextInternalRenderTargetID());

    auto isProtected = GrProtected(tex.isProtected());
    return sk_sp<GrTexture>(new GrMockTextureRenderTarget(this, tex.dimensions(), sampleCnt,
                                                          isProtected, mipmapStatus, texInfo,
                                                          rtInfo, cacheable));
}

sk_sp<GrRenderTarget> GrMockGpu::onWrapBackendRenderTarget(const GrBackendRenderTarget& rt) {
    GrMockRenderTargetInfo info;
    SkAssertResult(rt.getMockRenderTargetInfo(&info));

    auto isProtected = GrProtected(rt.isProtected());
    return sk_sp<GrRenderTarget>(new GrMockRenderTarget(this, GrMockRenderTarget::kWrapped,
                                                        rt.dimensions(), rt.sampleCnt(),
                                                        isProtected, info));
}

sk_sp<GrGpuBuffer> GrMockGpu::onCreateBuffer(size_t sizeInBytes, GrGpuBufferType type,
                                             GrAccessPattern accessPattern, const void*) {
    return sk_sp<GrGpuBuffer>(new GrMockBuffer(this, sizeInBytes, type, accessPattern));
}

sk_sp<GrAttachment> GrMockGpu::makeStencilAttachment(const GrBackendFormat& /*colorFormat*/,
                                                     SkISize dimensions, int numStencilSamples) {
    fStats.incStencilAttachmentCreates();
    return sk_sp<GrAttachment>(
            new GrMockAttachment(this, dimensions, GrAttachment::UsageFlags::kStencilAttachment,
                                 numStencilSamples));
}

GrBackendTexture GrMockGpu::onCreateBackendTexture(SkISize dimensions,
                                                   const GrBackendFormat& format,
                                                   GrRenderable,
                                                   GrMipmapped mipMapped,
                                                   GrProtected) {
    SkImage::CompressionType compression = format.asMockCompressionType();
    if (compression != SkImage::CompressionType::kNone) {
        return {}; // should go through onCreateCompressedBackendTexture
    }

    auto colorType = format.asMockColorType();
    if (!this->caps()->isFormatTexturable(format, GrTextureType::k2D)) {
        return GrBackendTexture();  // invalid
    }

    GrMockTextureInfo info(colorType, SkImage::CompressionType::kNone, NextExternalTextureID());

    fOutstandingTestingOnlyTextureIDs.add(info.id());
    return GrBackendTexture(dimensions.width(), dimensions.height(), mipMapped, info);
}

GrBackendTexture GrMockGpu::onCreateCompressedBackendTexture(
        SkISize dimensions, const GrBackendFormat& format, GrMipmapped mipMapped,
         GrProtected) {
    SkImage::CompressionType compression = format.asMockCompressionType();
    if (compression == SkImage::CompressionType::kNone) {
        return {}; // should go through onCreateBackendTexture
    }

    if (!this->caps()->isFormatTexturable(format, GrTextureType::k2D)) {
        return {};
    }

    GrMockTextureInfo info(GrColorType::kUnknown, compression, NextExternalTextureID());

    fOutstandingTestingOnlyTextureIDs.add(info.id());
    return GrBackendTexture(dimensions.width(), dimensions.height(), mipMapped, info);
}

void GrMockGpu::deleteBackendTexture(const GrBackendTexture& tex) {
    SkASSERT(GrBackendApi::kMock == tex.backend());

    GrMockTextureInfo info;
    if (tex.getMockTextureInfo(&info)) {
        fOutstandingTestingOnlyTextureIDs.remove(info.id());
    }
}

#if GR_TEST_UTILS
bool GrMockGpu::isTestingOnlyBackendTexture(const GrBackendTexture& tex) const {
    SkASSERT(GrBackendApi::kMock == tex.backend());

    GrMockTextureInfo info;
    if (!tex.getMockTextureInfo(&info)) {
        return false;
    }

    return fOutstandingTestingOnlyTextureIDs.contains(info.id());
}

GrBackendRenderTarget GrMockGpu::createTestingOnlyBackendRenderTarget(SkISize dimensions,
                                                                      GrColorType colorType,
                                                                      int sampleCnt,
                                                                      GrProtected) {
    GrMockRenderTargetInfo info(colorType, NextExternalRenderTargetID());
    static constexpr int kStencilBits = 8;
    return GrBackendRenderTarget(dimensions.width(), dimensions.height(), sampleCnt, kStencilBits,
                                 info);
}

void GrMockGpu::deleteTestingOnlyBackendRenderTarget(const GrBackendRenderTarget&) {}
#endif
