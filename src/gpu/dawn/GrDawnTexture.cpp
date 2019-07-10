/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/dawn/GrDawnTexture.h"

#include "src/gpu/dawn/GrDawnGpu.h"
#include "src/gpu/dawn/GrDawnTextureRenderTarget.h"
#include "src/gpu/dawn/GrDawnUtil.h"
#include "src/core/SkConvertPixels.h"

GrDawnTexture::GrDawnTexture(GrDawnGpu* gpu,
                           dawn::Texture texture,
                           dawn::TextureView textureView,
                           const GrSurfaceDesc& desc,
                           const GrDawnImageInfo& info,
                           GrMipMapsStatus mipMapsStatus)
    : GrSurface(gpu, desc)
    , GrTexture(gpu, desc, GrTextureType::k2D, mipMapsStatus)
    , fInfo(info)
    , fTexture(texture)
    , fTextureView(textureView) {
}

sk_sp<GrDawnTexture> GrDawnTexture::Make(GrDawnGpu* gpu, const GrSurfaceDesc& desc,
                                       SkBudgeted budgeted, GrMipMapsStatus status) {
    bool renderTarget = SkToBool(desc.fFlags & kRenderTarget_GrSurfaceFlag);
    dawn::TextureFormat format;
    if (!GrPixelConfigToDawnFormat(desc.fConfig, &format)) {
        return nullptr;
    }

    dawn::TextureDescriptor textureDesc;

    textureDesc.usage =
        dawn::TextureUsageBit::Sampled |
        dawn::TextureUsageBit::TransferSrc |
        dawn::TextureUsageBit::TransferDst;

    if (renderTarget) {
        textureDesc.usage |= dawn::TextureUsageBit::OutputAttachment;
    }

    textureDesc.dimension = dawn::TextureDimension::e2D;
    textureDesc.size.width = desc.fWidth;
    textureDesc.size.height = desc.fHeight;
    textureDesc.size.depth = 1;
    textureDesc.arrayLayerCount = 1;
    textureDesc.format = format;
    textureDesc.mipLevelCount = 1;
    textureDesc.sampleCount = 1;

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
    info.fLevelCount = 0; // FIXME
    sk_sp<GrDawnTexture> result;
    if (renderTarget) {
        result = sk_sp<GrDawnTextureRenderTarget>(new GrDawnTextureRenderTarget(gpu,
                                                                              tex,
                                                                              textureView,
                                                                              desc,
                                                                              info,
                                                                              status));
    } else {
        result = sk_sp<GrDawnTexture>(new GrDawnTexture(gpu, tex, textureView,
                                                      desc, info, status));
    }
    result->registerWithCache(budgeted);
    return result;
}

sk_sp<GrDawnTexture> GrDawnTexture::MakeWrapped(GrDawnGpu* gpu, const GrSurfaceDesc& desc,
                                              GrMipMapsStatus status, GrWrapCacheable cacheable,
                                              const GrDawnImageInfo& info) {
    dawn::Texture texture = info.fTexture;
    dawn::TextureView textureView = texture.CreateDefaultView();

    sk_sp<GrDawnTexture> tex;
    if (desc.fFlags & kRenderTarget_GrSurfaceFlag) {
        tex = sk_sp<GrDawnTexture>(new GrDawnTextureRenderTarget(gpu, texture, textureView, desc, info, status));
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
        size_t srcRowBytes = texels[i].fRowBytes;
        SkBitmap bitmap;
        SkPixmap pixmap;
        const char* src;
        SkColorType colorType = GrColorTypeToSkColorType(GrPixelConfigToColorType(this->config()));
        if (kRGBA_4444_GrPixelConfig == this->config() ||
            kRGB_565_GrPixelConfig == this->config() ||
            kGray_8_GrPixelConfig == this->config()) {
            SkImageInfo srcInfo = SkImageInfo::Make(width, height, colorType, kOpaque_SkAlphaType);
            colorType = kRGBA_8888_SkColorType;
            SkImageInfo dstInfo = SkImageInfo::Make(width, height, colorType, kPremul_SkAlphaType);
            SkPixmap srcPixmap(srcInfo, texels[i].fPixels, srcRowBytes);
            srcRowBytes = width * SkColorTypeBytesPerPixel(colorType);
            if ((srcRowBytes & 0xFF) != 0) {
                srcRowBytes = (srcRowBytes + 0xFF) & ~0xFF;
            }
            bitmap.allocPixels(dstInfo, srcRowBytes);
            bitmap.writePixels(srcPixmap);
            if (!bitmap.peekPixels(&pixmap)) {
                continue;
            }
            src = static_cast<const char*>(pixmap.addr());
        } else {
            src = static_cast<const char*>(texels[i].fPixels);
        }
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
        dstTexture.level = i;
        dstTexture.slice = 0;
        dstTexture.origin = {x, y, 0};
        dawn::Extent3D copySize = {width, height, 1};
        copyEncoder.CopyBufferToTexture(&srcBuffer, &dstTexture, &copySize);
        x /= 2;
        y /= 2;
        width /= 2;
        height /= 2;
    }
}
