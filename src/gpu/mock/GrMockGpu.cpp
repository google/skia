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

sk_sp<GrTexture> GrMockGpu::onCreateTexture(const GrSurfaceDesc& desc, SkBudgeted budgeted,
                                            const GrMipLevel texels[], int mipLevelCount) {
    if (fMockOptions.fFailTextureAllocations) {
        return nullptr;
    }

    GrColorType ct = GrPixelConfigToColorType(desc.fConfig);
    if (GrColorType::kUnknown == ct) {
        return nullptr;
    }

    GrMipMapsStatus mipMapsStatus = mipLevelCount > 1 ? GrMipMapsStatus::kValid
                                                      : GrMipMapsStatus::kNotAllocated;
    GrMockTextureInfo texInfo(ct, NextInternalTextureID());
    if (desc.fFlags & kRenderTarget_GrSurfaceFlag) {
        GrMockRenderTargetInfo rtInfo(ct, NextInternalRenderTargetID());
        return sk_sp<GrTexture>(new GrMockTextureRenderTarget(this, budgeted, desc, mipMapsStatus,
                                                              texInfo, rtInfo));
    }
    return sk_sp<GrTexture>(new GrMockTexture(this, budgeted, desc, mipMapsStatus, texInfo));
}

sk_sp<GrTexture> GrMockGpu::onCreateCompressedTexture(int width, int height,
                                                      SkImage::CompressionType compressionType,
                                                      SkBudgeted budgeted, const void* data) {
    return nullptr;
}

sk_sp<GrTexture> GrMockGpu::onWrapBackendTexture(const GrBackendTexture& tex,
                                                 GrWrapOwnership ownership,
                                                 GrWrapCacheable wrapType, GrIOType ioType) {
    GrMockTextureInfo info;
    SkAssertResult(tex.getMockTextureInfo(&info));

    GrSurfaceDesc desc;
    desc.fWidth = tex.width();
    desc.fHeight = tex.height();
    desc.fConfig = info.pixelConfig();

    GrMipMapsStatus mipMapsStatus = tex.hasMipMaps() ? GrMipMapsStatus::kValid
                                                     : GrMipMapsStatus::kNotAllocated;

    return sk_sp<GrTexture>(new GrMockTexture(this, desc, mipMapsStatus, info, wrapType, ioType));
}

sk_sp<GrTexture> GrMockGpu::onWrapRenderableBackendTexture(const GrBackendTexture& tex,
                                                           int sampleCnt,
                                                           GrWrapOwnership ownership,
                                                           GrWrapCacheable cacheable) {
    GrMockTextureInfo texInfo;
    SkAssertResult(tex.getMockTextureInfo(&texInfo));

    GrSurfaceDesc desc;
    desc.fFlags = kRenderTarget_GrSurfaceFlag;
    desc.fWidth = tex.width();
    desc.fHeight = tex.height();
    desc.fConfig = texInfo.pixelConfig();

    GrMipMapsStatus mipMapsStatus =
            tex.hasMipMaps() ? GrMipMapsStatus::kValid : GrMipMapsStatus::kNotAllocated;

    // The client gave us the texture ID but we supply the render target ID.
    GrMockRenderTargetInfo rtInfo(texInfo.fColorType, NextInternalRenderTargetID());

    return sk_sp<GrTexture>(
            new GrMockTextureRenderTarget(this, desc, mipMapsStatus, texInfo, rtInfo, cacheable));
}

sk_sp<GrRenderTarget> GrMockGpu::onWrapBackendRenderTarget(const GrBackendRenderTarget& rt) {
    GrMockRenderTargetInfo info;
    SkAssertResult(rt.getMockRenderTargetInfo(&info));

    GrSurfaceDesc desc;
    desc.fFlags = kRenderTarget_GrSurfaceFlag;
    desc.fWidth = rt.width();
    desc.fHeight = rt.height();
    desc.fConfig = info.pixelConfig();

    return sk_sp<GrRenderTarget>(
            new GrMockRenderTarget(this, GrMockRenderTarget::kWrapped, desc, info));
}

sk_sp<GrRenderTarget> GrMockGpu::onWrapBackendTextureAsRenderTarget(const GrBackendTexture& tex,
                                                                    int sampleCnt) {
    GrMockTextureInfo texInfo;
    SkAssertResult(tex.getMockTextureInfo(&texInfo));

    GrSurfaceDesc desc;
    desc.fFlags = kRenderTarget_GrSurfaceFlag;
    desc.fWidth = tex.width();
    desc.fHeight = tex.height();
    desc.fConfig = texInfo.pixelConfig();
    desc.fSampleCnt = sampleCnt;

    // The client gave us the texture ID but we supply the render target ID.
    GrMockRenderTargetInfo rtInfo(texInfo.fColorType, NextInternalRenderTargetID());

    return sk_sp<GrRenderTarget>(
            new GrMockRenderTarget(this, GrMockRenderTarget::kWrapped, desc, rtInfo));
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

    if (!format.getMockColorType()) {
        return GrBackendTexture();  // invalid;
    }

    if (!this->caps()->isFormatTexturable(*format.getMockColorType(), format)) {
        return GrBackendTexture();  // invalid
    }

    GrMockTextureInfo info(*format.getMockColorType(), NextExternalTextureID());

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
