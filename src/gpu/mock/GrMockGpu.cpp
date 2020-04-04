/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/mock/GrMockBuffer.h"
#include "src/gpu/mock/GrMockCaps.h"
#include "src/gpu/mock/GrMockGpu.h"
#include "src/gpu/mock/GrMockOpsRenderPass.h"
#include "src/gpu/mock/GrMockStencilAttachment.h"
#include "src/gpu/mock/GrMockTexture.h"
#include <atomic>

int GrMockGpu::NextInternalTextureID() {
    static std::atomic<int> nextID{1};
    int id;
    do {
        id = nextID.fetch_add(1);
    } while (0 == id);  // Reserve 0 for an invalid ID.
    return id;
}

int GrMockGpu::NextExternalTextureID() {
    // We use negative ints for the "testing only external textures" so they can easily be
    // identified when debugging.
    static std::atomic<int> nextID{-1};
    return nextID--;
}

int GrMockGpu::NextInternalRenderTargetID() {
    // We start off with large numbers to differentiate from texture IDs, even though they're
    // technically in a different space.
    static std::atomic<int> nextID{SK_MaxS32};
    return nextID--;
}

int GrMockGpu::NextExternalRenderTargetID() {
    // We use large negative ints for the "testing only external render targets" so they can easily
    // be identified when debugging.
    static std::atomic<int> nextID{SK_MinS32};
    return nextID++;
}

sk_sp<GrGpu> GrMockGpu::Make(const GrMockOptions* mockOptions,
                             const GrContextOptions& contextOptions, GrContext* context) {
    static const GrMockOptions kDefaultOptions = GrMockOptions();
    if (!mockOptions) {
        mockOptions = &kDefaultOptions;
    }
    return sk_sp<GrGpu>(new GrMockGpu(context, *mockOptions, contextOptions));
}

GrOpsRenderPass* GrMockGpu::getOpsRenderPass(
                                GrRenderTarget* rt, GrSurfaceOrigin origin, const SkIRect& bounds,
                                const GrOpsRenderPass::LoadAndStoreInfo& colorInfo,
                                const GrOpsRenderPass::StencilLoadAndStoreInfo&,
                                const SkTArray<GrSurfaceProxy*, true>& sampledProxies) {
    return new GrMockOpsRenderPass(this, rt, origin, colorInfo);
}

void GrMockGpu::submit(GrOpsRenderPass* renderPass) {
    for (int i = 0; i < static_cast<GrMockOpsRenderPass*>(renderPass)->numDraws(); ++i) {
        fStats.incNumDraws();
    }
    delete renderPass;
}

GrMockGpu::GrMockGpu(GrContext* context, const GrMockOptions& options,
                     const GrContextOptions& contextOptions)
        : INHERITED(context)
        , fMockOptions(options) {
    fCaps.reset(new GrMockCaps(contextOptions, options));
}

void GrMockGpu::querySampleLocations(GrRenderTarget* rt, SkTArray<SkPoint>* sampleLocations) {
    sampleLocations->reset();
    int numRemainingSamples = rt->numSamples();
    while (numRemainingSamples > 0) {
        // Use standard D3D sample locations.
        switch (numRemainingSamples) {
            case 0:
            case 1:
                sampleLocations->push_back().set(.5, .5);
                break;
            case 2:
                sampleLocations->push_back().set(.75, .75);
                sampleLocations->push_back().set(.25, .25);
                break;
            case 3:
            case 4:
                sampleLocations->push_back().set(.375, .125);
                sampleLocations->push_back().set(.875, .375);
                sampleLocations->push_back().set(.125, .625);
                sampleLocations->push_back().set(.625, .875);
                break;
            case 5:
            case 6:
            case 7:
            case 8:
                sampleLocations->push_back().set(.5625, .3125);
                sampleLocations->push_back().set(.4375, .6875);
                sampleLocations->push_back().set(.8125, .5625);
                sampleLocations->push_back().set(.3125, .1875);
                sampleLocations->push_back().set(.1875, .8125);
                sampleLocations->push_back().set(.0625, .4375);
                sampleLocations->push_back().set(.6875, .4375);
                sampleLocations->push_back().set(.4375, .0625);
                break;
            default:
                sampleLocations->push_back().set(.5625, .5625);
                sampleLocations->push_back().set(.4375, .3125);
                sampleLocations->push_back().set(.3125, .6250);
                sampleLocations->push_back().set(.2500, .4375);
                sampleLocations->push_back().set(.1875, .3750);
                sampleLocations->push_back().set(.6250, .8125);
                sampleLocations->push_back().set(.8125, .6875);
                sampleLocations->push_back().set(.6875, .1875);
                sampleLocations->push_back().set(.3750, .8750);
                sampleLocations->push_back().set(.5000, .0625);
                sampleLocations->push_back().set(.2500, .1250);
                sampleLocations->push_back().set(.1250, .2500);
                sampleLocations->push_back().set(.0000, .5000);
                sampleLocations->push_back().set(.4375, .2500);
                sampleLocations->push_back().set(.8750, .4375);
                sampleLocations->push_back().set(.0625, .0000);
                break;
        }
        numRemainingSamples = rt->numSamples() - sampleLocations->count();
    }
}

sk_sp<GrTexture> GrMockGpu::onCreateTexture(const GrSurfaceDesc& desc,
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

    GrMipMapsStatus mipMapsStatus =
            mipLevelCount > 1 ? GrMipMapsStatus::kDirty : GrMipMapsStatus::kNotAllocated;
    GrMockTextureInfo texInfo(ct, SkImage::CompressionType::kNone, NextInternalTextureID());
    if (renderable == GrRenderable::kYes) {
        GrMockRenderTargetInfo rtInfo(ct, NextInternalRenderTargetID());
        return sk_sp<GrTexture>(new GrMockTextureRenderTarget(this, budgeted, desc,
                                                              renderTargetSampleCnt, isProtected,
                                                              mipMapsStatus, texInfo, rtInfo));
    }
    return sk_sp<GrTexture>(
            new GrMockTexture(this, budgeted, desc, isProtected, mipMapsStatus, texInfo));
}

// TODO: why no 'isProtected' ?!
sk_sp<GrTexture> GrMockGpu::onCreateCompressedTexture(int width, int height,
                                                      const GrBackendFormat& format,
                                                      SkImage::CompressionType compressionType,
                                                      SkBudgeted budgeted,
                                                      const void* data) {
    if (fMockOptions.fFailTextureAllocations) {
        return nullptr;
    }

    // Uncompressed formats should go through onCreateTexture
    SkImage::CompressionType compression = format.asMockCompressionType();
    SkASSERT(compression != SkImage::CompressionType::kNone);
    SkASSERT(compression == compressionType);

    GrSurfaceDesc desc;
    desc.fWidth = width;
    desc.fHeight = height;
    desc.fConfig = GrCompressionTypeToPixelConfig(compression);

    GrMipMapsStatus mipMapsStatus = GrMipMapsStatus::kNotAllocated;
    GrMockTextureInfo texInfo(GrColorType::kUnknown,
                              format.asMockCompressionType(),
                              NextInternalTextureID());

    return sk_sp<GrTexture>(new GrMockTexture(this, budgeted, desc, GrProtected::kNo,
                                              mipMapsStatus, texInfo));
}

sk_sp<GrTexture> GrMockGpu::onWrapBackendTexture(const GrBackendTexture& tex, GrColorType colorType,
                                                 GrWrapOwnership ownership,
                                                 GrWrapCacheable wrapType, GrIOType ioType) {
    GrMockTextureInfo texInfo;
    SkAssertResult(tex.getMockTextureInfo(&texInfo));

    SkImage::CompressionType compression = texInfo.compressionType();
    if (compression == SkImage::CompressionType::kNone) {
        SkASSERT(colorType == texInfo.colorType());
    }

    GrSurfaceDesc desc;
    desc.fWidth = tex.width();
    desc.fHeight = tex.height();
    desc.fConfig = texInfo.pixelConfig();

    GrMipMapsStatus mipMapsStatus = tex.hasMipMaps() ? GrMipMapsStatus::kValid
                                                     : GrMipMapsStatus::kNotAllocated;
    auto isProtected = GrProtected(tex.isProtected());
    return sk_sp<GrTexture>(
            new GrMockTexture(this, desc, isProtected, mipMapsStatus, texInfo, wrapType, ioType));
}

sk_sp<GrTexture> GrMockGpu::onWrapRenderableBackendTexture(const GrBackendTexture& tex,
                                                           int sampleCnt,
                                                           GrColorType colorType,
                                                           GrWrapOwnership ownership,
                                                           GrWrapCacheable cacheable) {
    GrMockTextureInfo texInfo;
    SkAssertResult(tex.getMockTextureInfo(&texInfo));
    SkASSERT(texInfo.compressionType() == SkImage::CompressionType::kNone);

    SkASSERT(colorType == texInfo.colorType());
    GrSurfaceDesc desc;
    desc.fWidth = tex.width();
    desc.fHeight = tex.height();
    desc.fConfig = texInfo.pixelConfig();

    GrMipMapsStatus mipMapsStatus =
            tex.hasMipMaps() ? GrMipMapsStatus::kValid : GrMipMapsStatus::kNotAllocated;

    // The client gave us the texture ID but we supply the render target ID.
    GrMockRenderTargetInfo rtInfo(texInfo.colorType(), NextInternalRenderTargetID());

    auto isProtected = GrProtected(tex.isProtected());
    return sk_sp<GrTexture>(new GrMockTextureRenderTarget(
            this, desc, sampleCnt, isProtected, mipMapsStatus, texInfo, rtInfo, cacheable));
}

sk_sp<GrRenderTarget> GrMockGpu::onWrapBackendRenderTarget(const GrBackendRenderTarget& rt,
                                                           GrColorType colorType) {
    GrMockRenderTargetInfo info;
    SkAssertResult(rt.getMockRenderTargetInfo(&info));

    SkASSERT(colorType == info.colorType());
    GrSurfaceDesc desc;
    desc.fWidth = rt.width();
    desc.fHeight = rt.height();
    desc.fConfig = info.pixelConfig();

    auto isProtected = GrProtected(rt.isProtected());
    return sk_sp<GrRenderTarget>(new GrMockRenderTarget(this, GrMockRenderTarget::kWrapped, desc,
                                                        rt.sampleCnt(), isProtected, info));
}

sk_sp<GrRenderTarget> GrMockGpu::onWrapBackendTextureAsRenderTarget(const GrBackendTexture& tex,
                                                                    int sampleCnt,
                                                                    GrColorType colorType) {
    GrMockTextureInfo texInfo;
    SkAssertResult(tex.getMockTextureInfo(&texInfo));
    SkASSERT(texInfo.compressionType() == SkImage::CompressionType::kNone);

    SkASSERT(colorType == texInfo.colorType());
    GrSurfaceDesc desc;
    desc.fWidth = tex.width();
    desc.fHeight = tex.height();
    desc.fConfig = texInfo.pixelConfig();

    // The client gave us the texture ID but we supply the render target ID.
    GrMockRenderTargetInfo rtInfo(texInfo.colorType(), NextInternalRenderTargetID());

    auto isProtected = GrProtected(tex.isProtected());
    return sk_sp<GrRenderTarget>(new GrMockRenderTarget(this, GrMockRenderTarget::kWrapped, desc,
                                                        sampleCnt, isProtected, rtInfo));
}

sk_sp<GrGpuBuffer> GrMockGpu::onCreateBuffer(size_t sizeInBytes, GrGpuBufferType type,
                                             GrAccessPattern accessPattern, const void*) {
    return sk_sp<GrGpuBuffer>(new GrMockBuffer(this, sizeInBytes, type, accessPattern));
}

GrStencilAttachment* GrMockGpu::createStencilAttachmentForRenderTarget(
        const GrRenderTarget* rt, int width, int height, int numStencilSamples) {
    SkASSERT(numStencilSamples == rt->numSamples());
    static constexpr int kBits = 8;
    fStats.incStencilAttachmentCreates();
    return new GrMockStencilAttachment(this, width, height, kBits, rt->numSamples());
}

GrBackendTexture GrMockGpu::onCreateBackendTexture(SkISize dimensions,
                                                   const GrBackendFormat& format,
                                                   GrRenderable,
                                                   const BackendTextureData*,
                                                   int numMipLevels,
                                                   GrProtected) {
    SkImage::CompressionType compression = format.asMockCompressionType();
    if (compression != SkImage::CompressionType::kNone) {
        return {}; // should go through onCreateCompressedBackendTexture
    }

    auto colorType = format.asMockColorType();
    if (!this->caps()->isFormatTexturable(format)) {
        return GrBackendTexture();  // invalid
    }

    GrMockTextureInfo info(colorType, SkImage::CompressionType::kNone, NextExternalTextureID());

    fOutstandingTestingOnlyTextureIDs.add(info.id());
    auto mipMapped = numMipLevels > 1 ? GrMipMapped::kYes : GrMipMapped::kNo;
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

GrBackendRenderTarget GrMockGpu::createTestingOnlyBackendRenderTarget(int w, int h,
                                                                      GrColorType colorType) {
    GrMockRenderTargetInfo info(colorType, NextExternalRenderTargetID());
    static constexpr int kSampleCnt = 1;
    static constexpr int kStencilBits = 8;
    return GrBackendRenderTarget(w, h, kSampleCnt, kStencilBits, info);
}

void GrMockGpu::deleteTestingOnlyBackendRenderTarget(const GrBackendRenderTarget&) {}
#endif
