/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDawnTexture.h"

#include "GrDawnGpu.h"
#include "GrDawnTextureRenderTarget.h"
#include "GrDawnUtil.h"

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
    textureDesc.arrayLayer = 1;
    textureDesc.format = GrPixelConfigToDawnFormat(desc.fConfig);
    textureDesc.levelCount = 1;

    dawn::Texture tex = gpu->device().CreateTexture(&textureDesc);

    if (!tex) {
        return nullptr;
    }

    dawn::TextureView textureView = tex.CreateDefaultTextureView();

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
                                              GrMipMapsStatus status,
                                              const GrDawnImageInfo& info) {
    dawn::Texture texture = info.fTexture;
    dawn::TextureView textureView = texture.CreateDefaultTextureView();

    sk_sp<GrDawnTexture> tex(new GrDawnTexture(gpu, texture, textureView, desc, info, status));
    tex->registerWithCacheWrapped();
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
                          dawn::CommandBufferBuilder copyBuilder) {
    upload(texels, mipLevels, SkIRect::MakeWH(width(), height()), copyBuilder);
}

void GrDawnTexture::upload(const GrMipLevel texels[], int mipLevels, const SkIRect& rect,
                          dawn::CommandBufferBuilder copyBuilder) {
    dawn::Device device = getDawnGpu()->device();
    uint32_t x = rect.x();
    uint32_t y = rect.y();
    uint32_t width = rect.width();
    uint32_t height = rect.height();
    for (int i = 0; i < mipLevels; i++) {
        size_t origRowBytes = texels[i].fRowBytes;
        SkBitmap bitmap;
        SkPixmap pixmap;
        const char* src;
        if (kRGBA_4444_GrPixelConfig == this->config() ||
            kRGB_565_GrPixelConfig == this->config() ||
            kGray_8_GrPixelConfig == this->config()) {
            SkImageInfo info;
            info = SkImageInfo::Make(width, height, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
            SkImageInfo srcInfo;
            SkColorType colorType = GrColorTypeToSkColorType(GrPixelConfigToColorType(this->config()));
            srcInfo = SkImageInfo::Make(width, height, colorType, kOpaque_SkAlphaType);
            SkPixmap srcPixmap(srcInfo, texels[i].fPixels, origRowBytes);
            origRowBytes = width * GrBytesPerPixel(kRGBA_8888_GrPixelConfig);
            if ((origRowBytes & 0xFF) != 0) {
                origRowBytes = (origRowBytes + 0xFF) & ~0xFF;
            }
            bitmap.allocPixels(info, origRowBytes);
            bitmap.writePixels(srcPixmap);
            if (!bitmap.peekPixels(&pixmap)) {
                continue;
            }
            src = static_cast<const char*>(pixmap.addr());
        } else {
            src = static_cast<const char*>(texels[i].fPixels);
        }
        size_t rowBytes = origRowBytes;
        if ((rowBytes & 0xFF) != 0) {
            rowBytes = (rowBytes + 0xFF) & ~0xFF;
        }
        size_t size = rowBytes * height;
        GrDawnStagingBuffer* stagingBuffer = getDawnGpu()->getStagingBuffer(size);
        if (rowBytes == origRowBytes) {
            memcpy(stagingBuffer->fData, src, size);
        } else {
            char* dst = static_cast<char*>(stagingBuffer->fData);
            for (uint32_t row = 0; row < height; row++) {
                memcpy(dst, src, origRowBytes);
                dst += rowBytes;
                src += texels[i].fRowBytes;
            }
        }
        dawn::Buffer buffer = stagingBuffer->fBuffer;
        buffer.Unmap();
        stagingBuffer->fData = nullptr;
        dawn::BufferCopyView srcBuffer;
        srcBuffer.buffer = buffer;
        srcBuffer.offset = 0;
        srcBuffer.rowPitch = rowBytes;
        srcBuffer.imageHeight = height;
        dawn::TextureCopyView dstTexture;
        dstTexture.texture = fTexture;
        dstTexture.level = i;
        dstTexture.slice = 0;
        dstTexture.origin = {x, y, 0};
        dstTexture.aspect = dawn::TextureAspect::Color;
        dawn::Extent3D copySize = {width, height, 1};
        copyBuilder
            .CopyBufferToTexture(&srcBuffer, &dstTexture, &copySize);
        x /= 2;
        y /= 2;
        width /= 2;
        height /= 2;
    }
}
