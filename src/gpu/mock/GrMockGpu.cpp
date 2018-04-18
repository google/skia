/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrMockGpu.h"
#include "GrMockBuffer.h"
#include "GrMockCaps.h"
#include "GrMockGpuCommandBuffer.h"
#include "GrMockStencilAttachment.h"
#include "GrMockTexture.h"

int GrMockGpu::NextInternalTextureID() {
    static int gID = 0;
    return sk_atomic_inc(&gID) + 1;
}

int GrMockGpu::NextExternalTextureID() {
    // We use negative ints for the "testing only external textures" so they can easily be
    // identified when debugging.
    static int gID = 0;
    return sk_atomic_dec(&gID) - 1;
}

int GrMockGpu::NextInternalRenderTargetID() {
    // We start off with large numbers to differentiate from texture IDs, even though their
    // technically in a different space.
    static int gID = SK_MaxS32;
    return sk_atomic_dec(&gID);
}

int GrMockGpu::NextExternalRenderTargetID() {
    // We use large negative ints for the "testing only external render targets" so they can easily
    // be identified when debugging.
    static int gID = SK_MinS32;
    return sk_atomic_inc(&gID);
}

sk_sp<GrGpu> GrMockGpu::Make(const GrMockOptions* mockOptions,
                             const GrContextOptions& contextOptions, GrContext* context) {
    static const GrMockOptions kDefaultOptions = GrMockOptions();
    if (!mockOptions) {
        mockOptions = &kDefaultOptions;
    }
    return sk_sp<GrGpu>(new GrMockGpu(context, *mockOptions, contextOptions));
}


GrGpuRTCommandBuffer* GrMockGpu::createCommandBuffer(
                                            GrRenderTarget* rt, GrSurfaceOrigin origin,
                                            const GrGpuRTCommandBuffer::LoadAndStoreInfo&,
                                            const GrGpuRTCommandBuffer::StencilLoadAndStoreInfo&) {
    return new GrMockGpuRTCommandBuffer(this, rt, origin);
}

GrGpuTextureCommandBuffer* GrMockGpu::createCommandBuffer(GrTexture* texture,
                                                          GrSurfaceOrigin origin) {
    return new GrMockGpuTextureCommandBuffer(texture, origin);
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

    GrMipMapsStatus mipMapsStatus = mipLevelCount > 1 ? GrMipMapsStatus::kValid
                                                      : GrMipMapsStatus::kNotAllocated;
    GrMockTextureInfo texInfo;
    texInfo.fConfig = desc.fConfig;
    texInfo.fID = NextInternalTextureID();
    if (desc.fFlags & kRenderTarget_GrSurfaceFlag) {
        GrMockRenderTargetInfo rtInfo;
        rtInfo.fConfig = desc.fConfig;
        rtInfo.fID = NextInternalRenderTargetID();
        return sk_sp<GrTexture>(new GrMockTextureRenderTarget(this, budgeted, desc, mipMapsStatus,
                                                              texInfo, rtInfo));
    }
    return sk_sp<GrTexture>(new GrMockTexture(this, budgeted, desc, mipMapsStatus, texInfo));
}

sk_sp<GrTexture> GrMockGpu::onWrapBackendTexture(const GrBackendTexture& tex,
                                                 GrWrapOwnership ownership) {
    GrSurfaceDesc desc;
    desc.fWidth = tex.width();
    desc.fHeight = tex.height();

    GrMockTextureInfo info;
    SkAssertResult(tex.getMockTextureInfo(&info));
    desc.fConfig = info.fConfig;

    GrMipMapsStatus mipMapsStatus = tex.hasMipMaps() ? GrMipMapsStatus::kValid
                                                     : GrMipMapsStatus::kNotAllocated;

    return sk_sp<GrTexture>(new GrMockTexture(this, GrMockTexture::kWrapped, desc, mipMapsStatus,
                                              info));
}

sk_sp<GrTexture> GrMockGpu::onWrapRenderableBackendTexture(const GrBackendTexture& tex,
                                                           int sampleCnt,
                                                           GrWrapOwnership ownership) {
    GrSurfaceDesc desc;
    desc.fFlags = kRenderTarget_GrSurfaceFlag;
    desc.fWidth = tex.width();
    desc.fHeight = tex.height();

    GrMockTextureInfo texInfo;
    SkAssertResult(tex.getMockTextureInfo(&texInfo));
    desc.fConfig = texInfo.fConfig;

    GrMipMapsStatus mipMapsStatus =
            tex.hasMipMaps() ? GrMipMapsStatus::kValid : GrMipMapsStatus::kNotAllocated;

    GrMockRenderTargetInfo rtInfo;
    rtInfo.fConfig = texInfo.fConfig;
    // The client gave us the texture ID but we supply the render target ID.
    rtInfo.fID = NextInternalRenderTargetID();

    return sk_sp<GrTexture>(
            new GrMockTextureRenderTarget(this, desc, mipMapsStatus, texInfo, rtInfo));
}

sk_sp<GrRenderTarget> GrMockGpu::onWrapBackendRenderTarget(const GrBackendRenderTarget& rt) {
    GrSurfaceDesc desc;
    desc.fFlags = kRenderTarget_GrSurfaceFlag;
    desc.fWidth = rt.width();
    desc.fHeight = rt.height();

    GrMockRenderTargetInfo info;
    SkAssertResult(rt.getMockRenderTargetInfo(&info));
    desc.fConfig = info.fConfig;

    return sk_sp<GrRenderTarget>(
            new GrMockRenderTarget(this, GrMockRenderTarget::kWrapped, desc, info));
}

sk_sp<GrRenderTarget> GrMockGpu::onWrapBackendTextureAsRenderTarget(const GrBackendTexture& tex,
                                                                    int sampleCnt) {
    GrSurfaceDesc desc;
    desc.fFlags = kRenderTarget_GrSurfaceFlag;
    desc.fWidth = tex.width();
    desc.fHeight = tex.height();

    GrMockTextureInfo texInfo;
    SkAssertResult(tex.getMockTextureInfo(&texInfo));
    desc.fConfig = texInfo.fConfig;
    desc.fSampleCnt = sampleCnt;

    GrMockRenderTargetInfo rtInfo;
    rtInfo.fConfig = texInfo.fConfig;
    // The client gave us the texture ID but we supply the render target ID.
    rtInfo.fID = NextInternalRenderTargetID();

    return sk_sp<GrRenderTarget>(
            new GrMockRenderTarget(this, GrMockRenderTarget::kWrapped, desc, rtInfo));
}

GrBuffer* GrMockGpu::onCreateBuffer(size_t sizeInBytes, GrBufferType type,
                                    GrAccessPattern accessPattern, const void*) {
    return new GrMockBuffer(this, sizeInBytes, type, accessPattern);
}

GrStencilAttachment* GrMockGpu::createStencilAttachmentForRenderTarget(const GrRenderTarget* rt,
                                                                       int width,
                                                                       int height) {
    static constexpr int kBits = 8;
    fStats.incStencilAttachmentCreates();
    return new GrMockStencilAttachment(this, width, height, kBits, rt->numColorSamples());
}

#if GR_TEST_UTILS
GrBackendTexture GrMockGpu::createTestingOnlyBackendTexture(const void* pixels, int w, int h,
                                                            GrPixelConfig config, bool isRT,
                                                            GrMipMapped mipMapped) {
    GrMockTextureInfo info;
    info.fConfig = config;
    info.fID = NextExternalTextureID();
    fOutstandingTestingOnlyTextureIDs.add(info.fID);
    return GrBackendTexture(w, h, mipMapped, info);
}

bool GrMockGpu::isTestingOnlyBackendTexture(const GrBackendTexture& tex) const {
    SkASSERT(kMock_GrBackend == tex.backend());

    GrMockTextureInfo info;
    if (!tex.getMockTextureInfo(&info)) {
        return false;
    }

    return fOutstandingTestingOnlyTextureIDs.contains(info.fID);
}

void GrMockGpu::deleteTestingOnlyBackendTexture(const GrBackendTexture& tex) {
    SkASSERT(kMock_GrBackend == tex.backend());

    GrMockTextureInfo info;
    if (tex.getMockTextureInfo(&info)) {
        fOutstandingTestingOnlyTextureIDs.remove(info.fID);
    }
}

GrBackendRenderTarget GrMockGpu::createTestingOnlyBackendRenderTarget(int w, int h,
                                                                      GrColorType colorType,
                                                                      GrSRGBEncoded srgbEncoded) {
    auto config = GrColorTypeToPixelConfig(colorType, srgbEncoded);
    if (kUnknown_GrPixelConfig == config) {
        return {};
    }
    GrMockRenderTargetInfo info = {config, NextExternalRenderTargetID()};
    static constexpr int kSampleCnt = 1;
    static constexpr int kStencilBits = 8;
    return {w, h, kSampleCnt, kStencilBits, info};
}

void GrMockGpu::deleteTestingOnlyBackendRenderTarget(const GrBackendRenderTarget&) {}
#endif
