/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/mock/GrMockBuffer.h"
#include "src/gpu/mock/GrMockCaps.h"
#include "src/gpu/mock/GrMockGpu.h"
#include "src/gpu/mock/GrMockGpuCommandBuffer.h"
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

GrGpuRTCommandBuffer* GrMockGpu::getCommandBuffer(
                                GrRenderTarget* rt, GrSurfaceOrigin origin, const SkRect& bounds,
                                const GrGpuRTCommandBuffer::LoadAndStoreInfo&,
                                const GrGpuRTCommandBuffer::StencilLoadAndStoreInfo&) {
    return new GrMockGpuRTCommandBuffer(this, rt, origin);
}

GrGpuTextureCommandBuffer* GrMockGpu::getCommandBuffer(GrTexture* texture, GrSurfaceOrigin origin) {
    return new GrMockGpuTextureCommandBuffer(texture, origin);
}

void GrMockGpu::submit(GrGpuCommandBuffer* buffer) {
    if (buffer->asRTCommandBuffer()) {
        this->submitCommandBuffer(
                        static_cast<GrMockGpuRTCommandBuffer*>(buffer->asRTCommandBuffer()));
    }

    delete buffer;
}

void GrMockGpu::submitCommandBuffer(const GrMockGpuRTCommandBuffer* cmdBuffer) {
    for (int i = 0; i < cmdBuffer->numDraws(); ++i) {
        fStats.incNumDraws();
    }
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
                                            const GrMipLevel texels[],
                                            int mipLevelCount) {
    if (fMockOptions.fFailTextureAllocations) {
        return nullptr;
    }

    GrColorType ct = format.asMockColorType();
    SkASSERT(ct != GrColorType::kUnknown);

    GrMipMapsStatus mipMapsStatus = mipLevelCount > 1 ? GrMipMapsStatus::kValid
                                                      : GrMipMapsStatus::kNotAllocated;
    GrMockTextureInfo texInfo(ct, NextInternalTextureID());
    if (renderable == GrRenderable::kYes) {
        GrMockRenderTargetInfo rtInfo(ct, NextInternalRenderTargetID());
        return sk_sp<GrTexture>(new GrMockTextureRenderTarget(this, budgeted, desc,
                                                              renderTargetSampleCnt, isProtected,
                                                              mipMapsStatus, texInfo, rtInfo));
    }
    return sk_sp<GrTexture>(
            new GrMockTexture(this, budgeted, desc, isProtected, mipMapsStatus, texInfo));
}

sk_sp<GrTexture> GrMockGpu::onCreateCompressedTexture(int width, int height, const GrBackendFormat&,
                                                      SkImage::CompressionType compressionType,
                                                      SkBudgeted budgeted, const void* data) {
    return nullptr;
}

sk_sp<GrTexture> GrMockGpu::onWrapBackendTexture(const GrBackendTexture& tex, GrColorType colorType,
                                                 GrWrapOwnership ownership,
                                                 GrWrapCacheable wrapType, GrIOType ioType) {
    GrMockTextureInfo texInfo;
    SkAssertResult(tex.getMockTextureInfo(&texInfo));

    SkASSERT(colorType == texInfo.fColorType);
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

    SkASSERT(colorType == texInfo.fColorType);
    GrSurfaceDesc desc;
    desc.fWidth = tex.width();
    desc.fHeight = tex.height();
    desc.fConfig = texInfo.pixelConfig();

    GrMipMapsStatus mipMapsStatus =
            tex.hasMipMaps() ? GrMipMapsStatus::kValid : GrMipMapsStatus::kNotAllocated;

    // The client gave us the texture ID but we supply the render target ID.
    GrMockRenderTargetInfo rtInfo(texInfo.fColorType, NextInternalRenderTargetID());

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

    SkASSERT(colorType == texInfo.fColorType);
    GrSurfaceDesc desc;
    desc.fWidth = tex.width();
    desc.fHeight = tex.height();
    desc.fConfig = texInfo.pixelConfig();

    // The client gave us the texture ID but we supply the render target ID.
    GrMockRenderTargetInfo rtInfo(texInfo.fColorType, NextInternalRenderTargetID());

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

GrBackendTexture GrMockGpu::createBackendTexture(int w, int h,
                                                 const GrBackendFormat& format,
                                                 GrMipMapped mipMapped,
                                                 GrRenderable /* renderable */,
                                                 const void* /* pixels */,
                                                 size_t /* rowBytes */,
                                                 const SkColor4f* /* color */,
                                                 GrProtected /* isProtected */) {
    auto colorType = format.asMockColorType();
    if (!this->caps()->isFormatTexturable(format)) {
        return GrBackendTexture();  // invalid
    }

    GrMockTextureInfo info(colorType, NextExternalTextureID());

    fOutstandingTestingOnlyTextureIDs.add(info.fID);
    return GrBackendTexture(w, h, mipMapped, info);
}

void GrMockGpu::deleteBackendTexture(const GrBackendTexture& tex) {
    SkASSERT(GrBackendApi::kMock == tex.backend());

    GrMockTextureInfo info;
    if (tex.getMockTextureInfo(&info)) {
        fOutstandingTestingOnlyTextureIDs.remove(info.fID);
    }
}

#if GR_TEST_UTILS
bool GrMockGpu::isTestingOnlyBackendTexture(const GrBackendTexture& tex) const {
    SkASSERT(GrBackendApi::kMock == tex.backend());

    GrMockTextureInfo info;
    if (!tex.getMockTextureInfo(&info)) {
        return false;
    }

    return fOutstandingTestingOnlyTextureIDs.contains(info.fID);
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
