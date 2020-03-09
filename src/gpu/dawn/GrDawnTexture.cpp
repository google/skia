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
                             const SkISize& dimensions,
                             GrPixelConfig config,
                             wgpu::TextureView textureView,
                             const GrDawnImageInfo& info,
                             GrMipMapsStatus mipMapsStatus)
        : GrSurface(gpu, dimensions, config, GrProtected::kNo)
        , GrTexture(gpu, dimensions, config, GrProtected::kNo, GrTextureType::k2D, mipMapsStatus)
        , fInfo(info)
        , fTextureView(textureView) {}

sk_sp<GrDawnTexture> GrDawnTexture::Make(GrDawnGpu* gpu, const SkISize& dimensions,
                                         GrPixelConfig config, wgpu::TextureFormat format,
                                         GrRenderable renderable, int sampleCnt,
                                         SkBudgeted budgeted, int mipLevels,
                                         GrMipMapsStatus status) {
    bool renderTarget = renderable == GrRenderable::kYes;
    wgpu::TextureDescriptor textureDesc;

    textureDesc.usage =
        wgpu::TextureUsage::Sampled |
        wgpu::TextureUsage::CopySrc |
        wgpu::TextureUsage::CopyDst;

    if (renderTarget) {
        textureDesc.usage |= wgpu::TextureUsage::OutputAttachment;
    }

    textureDesc.size.width = dimensions.fWidth;
    textureDesc.size.height = dimensions.fHeight;
    textureDesc.size.depth = 1;
    textureDesc.format = format;
    textureDesc.mipLevelCount = std::max(mipLevels, 1);
    textureDesc.sampleCount = sampleCnt;

    wgpu::Texture tex = gpu->device().CreateTexture(&textureDesc);

    if (!tex) {
        return nullptr;
    }

    wgpu::TextureView textureView = tex.CreateView();

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
                                                                                dimensions,
                                                                                config,
                                                                                textureView,
                                                                                sampleCnt,
                                                                                info,
                                                                                status));
    } else {
        result = sk_sp<GrDawnTexture>(
                new GrDawnTexture(gpu, dimensions, config, textureView, info, status));
    }
    result->registerWithCache(budgeted);
    return result;
}

GrBackendFormat GrDawnTexture::backendFormat() const {
    return GrBackendFormat::MakeDawn(fInfo.fFormat);
}

sk_sp<GrDawnTexture> GrDawnTexture::MakeWrapped(GrDawnGpu* gpu, const SkISize& dimensions,
                                                GrPixelConfig config, GrRenderable renderable,
                                                int sampleCnt, GrMipMapsStatus status,
                                                GrWrapCacheable cacheable,
                                                const GrDawnImageInfo& info) {
    wgpu::TextureView textureView = info.fTexture.CreateView();
    if (!textureView) {
        return nullptr;
    }

    sk_sp<GrDawnTexture> tex;
    if (GrRenderable::kYes == renderable) {
        tex = sk_sp<GrDawnTexture>(new GrDawnTextureRenderTarget(
                gpu, dimensions, config, textureView, sampleCnt, info, status));
    } else {
        tex = sk_sp<GrDawnTexture>(
                new GrDawnTexture(gpu, dimensions, config, textureView, info, status));
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
                           wgpu::CommandEncoder copyEncoder) {
    this->upload(texels, mipLevels, SkIRect::MakeWH(width(), height()), copyEncoder);
}

void GrDawnTexture::upload(const GrMipLevel texels[], int mipLevels, const SkIRect& rect,
                           wgpu::CommandEncoder copyEncoder) {
    wgpu::Device device = this->getDawnGpu()->device();

    uint32_t x = rect.x();
    uint32_t y = rect.y();
    uint32_t width = rect.width();
    uint32_t height = rect.height();

    for (int i = 0; i < mipLevels; i++) {
        const void* src = texels[i].fPixels;
        size_t srcRowBytes = texels[i].fRowBytes;
        SkColorType colorType = GrColorTypeToSkColorType(GrPixelConfigToColorType(this->config()));
        size_t trimRowBytes = width * SkColorTypeBytesPerPixel(colorType);
        size_t dstRowBytes = GrDawnRoundRowBytes(trimRowBytes);
        size_t size = dstRowBytes * height;
        GrDawnStagingBuffer* stagingBuffer = getDawnGpu()->getStagingBuffer(size);
        SkRectMemcpy(stagingBuffer->fData, dstRowBytes, src, srcRowBytes, trimRowBytes, height);
        wgpu::Buffer buffer = stagingBuffer->fBuffer;
        buffer.Unmap();
        stagingBuffer->fData = nullptr;

        wgpu::BufferCopyView srcBuffer;
        srcBuffer.buffer = buffer;
        srcBuffer.offset = 0;
        srcBuffer.rowPitch = dstRowBytes;
        srcBuffer.imageHeight = height;

        wgpu::TextureCopyView dstTexture;
        dstTexture.texture = fInfo.fTexture;
        dstTexture.mipLevel = i;
        dstTexture.origin = {x, y, 0};

        wgpu::Extent3D copySize = {width, height, 1};
        copyEncoder.CopyBufferToTexture(&srcBuffer, &dstTexture, &copySize);
        x /= 2;
        y /= 2;
        width = SkTMax(1u, width / 2);
        height = SkTMax(1u, height / 2);
    }
}
