/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/dawn/GrDawnTexture.h"

#include "src/core/SkConvertPixels.h"
#include "src/gpu/dawn/GrDawnGpu.h"
#include "src/gpu/dawn/GrDawnTextureRenderTarget.h"
#include "src/gpu/dawn/GrDawnUtil.h"

GrDawnTexture::GrDawnTexture(GrDawnGpu* gpu,
                           dawn::Texture texture,
                           dawn::TextureView textureView,
                           const GrSurfaceDesc& desc,
                           const GrDawnImageInfo& info,
                           GrMipMapsStatus mipMapsStatus)
    : GrSurface(gpu, desc, GrProtected::kNo)
    , GrTexture(gpu, desc, GrProtected::kNo, GrTextureType::k2D, mipMapsStatus)
    , fInfo(info)
    , fTexture(texture)
    , fTextureView(textureView) {
}

sk_sp<GrDawnTexture> GrDawnTexture::Make(GrDawnGpu* gpu, const GrSurfaceDesc& desc,
                                         GrRenderable renderable, int sampleCnt,
                                         SkBudgeted budgeted, int mipLevels,
                                         GrMipMapsStatus status) {
    bool renderTarget = renderable == GrRenderable::kYes;
    dawn::TextureFormat format;
    if (!GrPixelConfigToDawnFormat(desc.fConfig, &format)) {
        return nullptr;
    }

    dawn::TextureDescriptor textureDesc;

    textureDesc.usage =
        dawn::TextureUsageBit::Sampled |
        dawn::TextureUsageBit::CopySrc |
        dawn::TextureUsageBit::CopyDst;

    if (renderTarget) {
        textureDesc.usage |= dawn::TextureUsageBit::OutputAttachment;
    }

    textureDesc.size.width = desc.fWidth;
    textureDesc.size.height = desc.fHeight;
    textureDesc.size.depth = 1;
    textureDesc.format = format;
    textureDesc.mipLevelCount = std::max(mipLevels, 1);
    textureDesc.sampleCount = sampleCnt;

    dawn::Texture tex = gpu->device().CreateTexture(&textureDesc);

    if (!tex) {
        return nullptr;
    }

    dawn::TextureView textureView = tex.CreateDefaultView();

    if (!textureView) {
        return nullptr;
    }

    GrDawnImageInfo info;
    info.fTexture = tex;
    info.fFormat = textureDesc.format;
    info.fLevelCount = mipLevels;
    sk_sp<GrDawnTexture> result;
    if (renderTarget) {
        result = sk_sp<GrDawnTextureRenderTarget>(new GrDawnTextureRenderTarget(gpu,
                                                                              tex,
                                                                              textureView,
                                                                              desc,
                                                                              sampleCnt,
                                                                              info,
                                                                              status));
    } else {
        result = sk_sp<GrDawnTexture>(new GrDawnTexture(gpu, tex, textureView,
                                                      desc, info, status));
    }
    result->registerWithCache(budgeted);
    return result;
}

GrBackendFormat GrDawnTexture::backendFormat() const {
    return GrBackendFormat::MakeDawn(fInfo.fFormat);
}

sk_sp<GrDawnTexture> GrDawnTexture::MakeWrapped(GrDawnGpu* gpu, const GrSurfaceDesc& desc,
                                                GrRenderable renderable, int sampleCnt,
                                                GrMipMapsStatus status,
                                                GrWrapCacheable cacheable,
                                                const GrDawnImageInfo& info) {
    dawn::Texture texture = info.fTexture;
    dawn::TextureView textureView = texture.CreateDefaultView();

    sk_sp<GrDawnTexture> tex;
    if (GrRenderable::kYes == renderable) {
        tex = sk_sp<GrDawnTexture>(new GrDawnTextureRenderTarget(gpu, texture, textureView, desc,
                                                                 sampleCnt, info, status));
    } else {
        tex = sk_sp<GrDawnTexture>(new GrDawnTexture(gpu, texture, textureView, desc, info, status));
    }
    tex->registerWithCacheWrapped(cacheable);
    return tex;
}

GrDawnTexture::~GrDawnTexture() {
}

GrDawnGpu* GrDawnTexture::getDawnGpu() const {
    SkASSERT(!this->wasDestroyed());
    return static_cast<GrDawnGpu*>(this->getGpu());
}

void GrDawnTexture::onRelease() {
    INHERITED::onRelease();
}

void GrDawnTexture::onAbandon() {
    INHERITED::onAbandon();
}

GrBackendTexture GrDawnTexture::getBackendTexture() const {
    return GrBackendTexture(this->width(), this->height(), fInfo);
}

void GrDawnTexture::upload(const GrMipLevel texels[], int mipLevels,
                           dawn::CommandEncoder copyEncoder) {
    upload(texels, mipLevels, SkIRect::MakeWH(width(), height()), copyEncoder);
}

void GrDawnTexture::upload(const GrMipLevel texels[], int mipLevels, const SkIRect& rect,
                           dawn::CommandEncoder copyEncoder) {
    dawn::Device device = getDawnGpu()->device();
    uint32_t x = rect.x();
    uint32_t y = rect.y();
    uint32_t width = rect.width();
    uint32_t height = rect.height();
    for (int i = 0; i < mipLevels; i++) {
        const void* src = texels[i].fPixels;
        size_t srcRowBytes = texels[i].fRowBytes;
        SkColorType colorType = GrColorTypeToSkColorType(GrPixelConfigToColorType(this->config()));
        size_t trimRowBytes = width * SkColorTypeBytesPerPixel(colorType);
        size_t dstRowBytes = trimRowBytes;
        if ((dstRowBytes & 0xFF) != 0) {
            dstRowBytes = (dstRowBytes + 0xFF) & ~0xFF;
        }
        size_t size = dstRowBytes * height;
        GrDawnStagingBuffer* stagingBuffer = getDawnGpu()->getStagingBuffer(size);
        SkRectMemcpy(stagingBuffer->fData, dstRowBytes, src, srcRowBytes, trimRowBytes, height);
        dawn::Buffer buffer = stagingBuffer->fBuffer;
        buffer.Unmap();
        stagingBuffer->fData = nullptr;
        dawn::BufferCopyView srcBuffer;
        srcBuffer.buffer = buffer;
        srcBuffer.offset = 0;
        srcBuffer.rowPitch = dstRowBytes;
        srcBuffer.imageHeight = height;
        dawn::TextureCopyView dstTexture;
        dstTexture.texture = fTexture;
        dstTexture.mipLevel = i;
        dstTexture.origin = {x, y, 0};
        dawn::Extent3D copySize = {width, height, 1};
        copyEncoder.CopyBufferToTexture(&srcBuffer, &dstTexture, &copySize);
        x /= 2;
        y /= 2;
        width /= 2;
        height /= 2;
    }
}
